#include "widget_python_api.h"

/*
 * widget_get_adjacencies_api
 * For a qubit in the tableau, list all adjacent qubits 
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: target_qubit : const size_t :: The target qubit 
 * :: adj : struct adjacency_obj* :: Pointer to object to write to
 * Returns a heap allocated array of uint64_t objects
 */
void widget_get_adjacencies_api(const widget_t* wid, const size_t target_qubit, struct adjacency_obj* adj)
{
    *adj = widget_get_adjacencies(wid, target_qubit); 
}

/*
 * widget_destroy_adjacencies 
 * Frees adjacency resources
 * :: adj : struct adjacency_obj :: Adjacency object
 * Frees the array of adjacencies from the object
 */
void widget_destroy_adjacencies(struct adjacency_obj* adj)
{
    if (adj->adjacencies != NULL) 
    {
        free(adj->adjacencies);
    }
}

/*
 * widget_get_io_map_api
 * Returns an array of io map objects
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: arr : uint32_t** :: Pointer to io map to return 
 * Writes the pointer to the io map to *arr
 */
void widget_get_io_map_api(const widget_t* wid, qubit_map_t** arr)
{
    *arr = wid->q_map;
}

/*
 * widget_get_local_cliffords_api
 * Returns an array of local clifford objects
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: arr : uint32_t** :: Pointer to io map to return 
 * Writes the pointer to the io map to *arr
 */
void widget_get_local_cliffords_api(const widget_t* wid, instruction_t** arr)
{
    *arr = wid->queue->table;
}

/*
 * widget_get_measurements_api
 * Returns an array of measurements objects
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: arr : uint32_t** :: Pointer to io map to return 
 * Writes the pointer to the io map to *arr
 */
void widget_get_measurement_tags_api(const widget_t* wid, non_clifford_tag_t** arr)
{
    *arr = wid->queue->non_cliffords;
}

/*
 * wiget_get_pauli_tracker_api
 * Gets the pauli tracker pointer from the widget
 * :: wid : const widget_t* :: Widget pointer
 * :: tracker  : void** :: Address to write pointer to
 */
void widget_get_pauli_tracker_api(
    const widget_t* wid,
    void** tracker)
{
    *tracker = wid->pauli_tracker; 
}

/*
 * widget_print_tableau
 * Prints the tableau
 */
void widget_print_tableau_api(
    const widget_t* wid)
{
    size_t tmp = wid->tableau->n_qubits;
    wid->tableau->n_qubits = wid->n_qubits;
    tableau_print(wid->tableau); 
    wid->tableau->n_qubits = tmp;
}
