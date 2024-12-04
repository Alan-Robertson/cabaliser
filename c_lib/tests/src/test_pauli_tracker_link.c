#include <stdio.h>
#include "lib_pauli_tracker.h"
#include "input_stream.h"
#include "widget.h"

void test_create_destroy()
{
    MappedPauliTracker* live = lib_pauli_tracker_create(10);
    lib_pauli_track_x(live, 0, 1);

    void* graph = lib_pauli_tracker_partial_order_graph(live);  
    
    lib_pauli_tracker_destroy(live);
}

/*
 * Coverage for valgrind
 */
void test_widget()
{
    size_t n_qubits = 1;
    widget_t* wid = widget_create(n_qubits, 3);
    teleport_input(wid, n_qubits);

    void* graph = lib_pauli_tracker_partial_order_graph(wid->pauli_tracker);  
    
    lib_pauli_tracker_graph_destroy(graph);
 
    widget_destroy(wid);
}


int main(void) {
    test_create_destroy();
    test_widget();
}
