#include "widget.h"

/*
 * widget_create
 * Constructor for widget object
 * :: initial_qubits : const size_t :: Initial number of qubits that are allocated 
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated 
 */
widget_t* widget_create(const size_t initial_qubits, const size_t max_qubits)
{
    // Max qubits must be a multiple of 64:
    const size_t aligned_max_qubits = max_qubits + (
            !!(max_qubits % 64)) * (64 - (max_qubits % 64)); 

    widget_t* wid = (widget_t*)malloc(sizeof(widget_t));  
    wid->n_initial_qubits = initial_qubits;
    wid->n_qubits = initial_qubits;
    wid->max_qubits = max_qubits; 
    wid->tableau = tableau_create(aligned_max_qubits);
    wid->queue = clifford_queue_create(aligned_max_qubits);
    wid->q_map = qubit_map_create(initial_qubits, max_qubits); 
    wid->pauli_tracker = pauli_tracker_create(max_qubits);
    return wid;
}

uint8_t widget_get_clifford_from_table(widget_t* wid, size_t i) {
    return wid->queue->table[i];
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
     
    for (size_t i = 0;
         i < SLICE_LEN_SIZE_T(wid->tableau->n_qubits);
         i++)  
    {
        if (slice[i] > 0 )
        {
            CHUNK_OBJ obj = slice[i];
            while (obj > 0)
            {
                uint32_t edge =  __CHUNK_CTZ(obj);
                obj ^= (1ull << edge);
                edge += i * sizeof(CHUNK_OBJ) * 8;
    
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
    simd_widget_decompose(wid);
}

void __widget_decompose(widget_t* wid)
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
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            DPRINT(DEBUG_3, "Applying S to %lu\n", i); 
            // Applies the Sdag gate
            // X and Zbit is always 1 
            // Operation is r ^= x~z; z ^= x
            // x is 1 via prior diagonalisation, z is 1 by switch invariant 
            // Operation is:
            // r ^= 0 -> r
            // z = x.~z = 1.~1 -> 0 
            // Hence operation is performed by zeroing the z bit  
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
            // Applies a corresponding S gate
            clifford_queue_local_clifford_right(wid->queue, _S_, i);
        }
    }

    // Z to set phases to 0
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        //
        // Action of Z gate
        // r ^= x 
        // As X is diagonal, this only acts on one bit
        if (__inline_slice_get_bit(wid->tableau->phases, i))
        {
            DPRINT(DEBUG_3, "Applying Z to %lu\n", i); 
            clifford_queue_local_clifford_right(wid->queue, _Z_, i);
        }
    }

    // The previous loop zeros the phases, this loop just does it faster
    // Effective action of a Z gate
    for (size_t i = 0; i < wid->tableau->slice_len; i += CHUNK_STRIDE)
    {
        wid->tableau->phases[i] = 0; 
    }

    return;
}
