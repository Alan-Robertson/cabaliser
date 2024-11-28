#include "widget.h"

/*
 * widget_create
 * Constructor for widget object
 * :: initial_qubits : const size_t :: Initial number of qubits that are allocated 
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated 
 */
widget_t* widget_create(const size_t initial_qubits, const size_t max_qubits)
{
    widget_t* wid = (widget_t*)malloc(sizeof(widget_t));  
    wid->n_initial_qubits = initial_qubits;
    wid->n_qubits = initial_qubits;
    wid->max_qubits = max_qubits; 
    wid->tableau = tableau_create(max_qubits);
    wid->queue = clifford_queue_create(max_qubits);
    wid->q_map = qubit_map_create(initial_qubits, max_qubits); 
    wid->pauli_tracker = pauli_tracker_create(max_qubits);

    return wid;
}

/*
 * widget_destroy
 * Destructor for the widget object
 * :: wid : widget_t* :: widget to free
 */
void widget_destroy(widget_t* wid)
{
    tableau_destroy(wid->tableau);
    clifford_queue_destroy(wid->queue);
    qubit_map_destroy(wid->q_map);
    pauli_tracker_destroy(wid->pauli_tracker);
    free(wid);
}

/*
 * widget_get_n_qubits
 * widget_get_n_initial_qubits
 * widget_get_max_qubits
 * Getter methods for the widget
 * :: wid : const widget_t* :: 
 */
size_t widget_get_n_qubits(const widget_t* wid)
{
    return wid->n_qubits;
}
size_t widget_get_n_initial_qubits(const widget_t* wid)
{
    return wid->n_initial_qubits;
}
size_t widget_get_max_qubits(const widget_t* wid)
{
    return wid->max_qubits;
}

/*
 * widget_get_adjacencies
 * For a qubit in the tableau, list all adjacent qubits 
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: target_qubit : const size_t :: The target qubit 
 * Returns a heap allocated array of uint64_t objects
 */
struct adjacency_obj widget_get_adjacencies(const widget_t* wid, const size_t target_qubit)
{
    struct adjacency_obj adj; 
    adj.adjacencies = malloc(wid->n_qubits * sizeof(uint32_t));
    adj.targ = target_qubit;
    adj.n_adjacent = 0;

    tableau_slice_p slice = wid->tableau->slices_z[target_qubit];
     
    #pragma omp parallel for 
    for (size_t i = 0; i < wid->tableau->slice_len; i++)  
    {
        if (slice[i] > 0 )
        {
            CHUNK_OBJ obj = slice[i];
            while (obj > 0)
            {
                uint32_t edge =  __CHUNK_CTZ(obj);
                obj ^= (1ull << edge);  
                edge += i * sizeof(CHUNK_OBJ);
    
                // Test that fetch occurs prior to addition
                // TODO wrap in macro for multi-arch support 
                uint32_t idx = __sync_fetch_and_add(&(adj.n_adjacent), 1); 
                adj.adjacencies[idx] = (uint32_t)edge;
            }
        }
    }
    
    // This is always a smaller allocation and so setting this variable should do nothing
    if (adj.n_adjacent > 0) 
    {
        adj.adjacencies = realloc(adj.adjacencies, adj.n_adjacent * sizeof(uint32_t));
    }
    else
    {
        free(adj.adjacencies);
        adj.adjacencies = NULL;
    }
    return adj;
}


/*
 * widget_get_io_map
 * Indicates the mapping of input to output qubits
 * Input qubits are ordered from 1 to n, output mapping indicates the qubit number
 * :: wid : const widget_t* :: Widget to get the mapping for
 */
size_t* widget_get_io_map(const widget_t* wid) 
{
    const size_t n_bytes = sizeof(size_t) * wid->n_initial_qubits; 
    size_t* map = (size_t*)malloc(n_bytes); 
    memcpy(map, wid->q_map, n_bytes);
    return map; 
}

/*
 * widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords 
 * :: wid : widget_t* :: Widget to decompose 
 * Acts in place on the tableau 
 */
void widget_decompose(widget_t* wid)
{
    tableau_remove_zero_X_columns(wid->tableau, wid->queue);

    tableau_transpose(wid->tableau);

    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            tableau_X_diag_element(wid->tableau, wid->queue, i);
        }

        tableau_X_diag_col_upper(wid->tableau, i);
    }


    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            tableau_X_diag_element(wid->tableau, wid->queue, i);
        }
        tableau_X_diag_col_lower(wid->tableau, i);
    }


    // Phase operation to set Z diagonal to zero 
    // Loop acts on separate cache line entries for each element
    #pragma omp parallel for
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
            clifford_queue_local_clifford_right(wid->queue, _S_, i);

        }
    }

    // Z to set phases to 0
    #pragma omp parallel for
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->phases, i))
        {
             clifford_queue_local_clifford_right(wid->queue, _Z_, i);
        }
    }
    // The previous loop zeros the phases, this loop just does it faster
    #pragma omp parallel for
    for (size_t i = 0; i < wid->tableau->slice_len; i++)
    {
        wid->tableau->phases[i] = 0;    
    }

//    // TODO double check that this is doing something
//    #pragma omp parallel for
//    for (size_t i = 0; i < wid->n_qubits; i++)
//    {
//        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
//        {
//            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
//            clifford_queue_local_clifford_right(wid->queue, _S_, i);
//        }
//    }
//    

    return;
}
