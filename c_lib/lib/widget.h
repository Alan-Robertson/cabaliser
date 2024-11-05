#ifndef WIDGET_H
#define WIDGET_H

#include "tableau.h"
#include "tableau_operations.h"

#include "instructions.h"
#include "instructions.h"

#include "qubit_map.h"
#include "adjacency.h"

#include "pauli_tracker.h"

#define WMAP_LOOKUP(widget, idx) (widget->q_map[idx])

struct widget_t {
    size_t n_qubits;
    size_t n_initial_qubits;
    size_t max_qubits;
    struct tableau_t* tableau;
    struct clifford_queue_t* queue;
    qubit_map_t* q_map;
    void* pauli_tracker;
    void* pauli_tracker_input; 
};
typedef struct widget_t widget_t;


/*
 * widget_create
 * Constructor for widget object
 * :: initial_qubits : const size_t :: Initial number of allocated qubits for the widget
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated
 * Setting intial qubits is essential to ensuring that incorrect targets are not written to
 */
widget_t* widget_create(const size_t initial_qubits, const size_t max_qubits);


/*
 * widget_destroy
 * Destructor for the widget object
 * :: wid : widget_t* :: widget to free
 */
void widget_destroy(widget_t* wid);


/*
 * widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords
 * :: wid : widget_t* :: Widget to decompose
 * Acts in place on the tableau
 */
void widget_decompose(widget_t* wid);

/*
 * widget_get_n_qubits
 * widget_get_n_initial_qubits
 * widget_get_max_qubits
 * Getter methods for the widget
 * :: wid : const widget_t* ::
 */
size_t widget_get_n_qubits(const widget_t* wid);
size_t widget_get_n_initial_qubits(const widget_t* wid);
size_t widget_get_max_qubits(const widget_t* wid);


/*
 * widget_get_io_map
 * Indicates the mapping of input to output qubits
 * Input qubits are ordered from 1 to n, output mapping indicates the qubit number
 * :: wid : const widget_t* :: Widget to get the mapping for
 */
size_t* widget_get_io_map(const widget_t* wid);
struct adjacency_obj widget_get_adjacencies(const widget_t* wid, const size_t target_qubit);


#endif
