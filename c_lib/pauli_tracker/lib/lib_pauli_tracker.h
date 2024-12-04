#include <stdint.h>

typedef struct MappedPauliTracker MappedPauliTracker;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * lib_pauli_tracker_create
 * Constructor for the pauli tracker object
 * :: n_qubits : usize :: Number of initialised qubits in the tracker
 * Returns a pointer to a dynamically allocated MappedPauliTracker object  
 */
MappedPauliTracker* lib_pauli_tracker_create(uintptr_t n_qubits);


/*
 * lib_pauli_tracker_destroy
 * Destructor for the pauli tracker object
 * :: lib_pauli_tracker : *mut MappedPauliTracker :: Pointer to the pauli tracker object to be freed 
 * Acts in place to free the pauli tracker object
 */
void lib_pauli_tracker_destroy(MappedPauliTracker *pauli_tracker);

/*
 * lib_pauli_tracker_graph_destroy
 * Destructor for the graph object
 * :: graph : *mut partial_order_graph::PartialOrderGraph :: Pointer to the graph object to be freed 
 * Acts in place
 */
void lib_pauli_tracker_graph_destroy(void* graph);


/*
 * pauli_track_x
 * Add a row to the pauli tracker object with an 'X' at the target qubit  
 * All other locations will be the identity
 * :: lib_pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
void lib_pauli_track_x(MappedPauliTracker *pauli_tracker, uintptr_t measured_qubit, uintptr_t measurement_target);

/*
 * pauli_track_y
 * Add a row to the pauli tracker object with a 'Y' at the target qubit  
 * All other locations will be the identity
 * :: lib_pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
void lib_pauli_track_y(MappedPauliTracker *pauli_tracker, uintptr_t measured_qubit, uintptr_t measurement_target);


/*
 * pauli_track_z
 * Add a row to the pauli tracker object with an 'Z' at the target qubit  
 * All other locations will be the identity
 * :: lib_pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
void lib_pauli_track_z(MappedPauliTracker *pauli_tracker, uintptr_t measured_qubit, uintptr_t measurement_target);


/*
 * lib_pauli_tracker_print
 * Small printing function for the pauli tracker
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object to print 
 * Acts in place, and writes to stdout
 */
void lib_pauli_tracker_print(const MappedPauliTracker* const mapped_pauli_tracker);

/*
 *
 */
void* lib_pauli_tracker_partial_order_graph(MappedPauliTracker* tracker);

void lib_pauli_tracker_graph_print(void*);

#ifdef __cplusplus
}
#endif
