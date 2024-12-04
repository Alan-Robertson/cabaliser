use pauli_tracker::{
    collection::{Init, Map},
    pauli::{self},
    tracker::{Tracker, frames::Frames},
};

use crate::{
    partial_order_graph,
    const_vec,
};

// Intermediate type wrappers 
type BitVec = bitvec::vec::BitVec;
type PauliStack = pauli::PauliStack<BitVec>;

// Define the specific types that you need
type PauliTracker = Frames::<Map<PauliStack>>;

/*
 * Pauli Tracker with mapping 
 */
#[derive(Debug)]
pub struct MappedPauliTracker {
    pub pauli_tracker : PauliTracker,  // Pauli tracker object 
    pub mapper : Vec<usize>, // Array where indicies represent rows and values represent measured qubits
}

/*
 * pauli_tracker_create
 * Constructor for the pauli tracker object
 * :: n_qubits : usize :: Number of initialised qubits in the tracker
 * Returns a pointer to a dynamically allocated MappedPauliTracker object  
 */
#[no_mangle]
pub extern "C" fn lib_pauli_tracker_create(n_qubits: usize) -> *mut MappedPauliTracker {
    return Box::into_raw(
        Box::new(
            MappedPauliTracker {
                pauli_tracker : PauliTracker::init(n_qubits),
                mapper : Vec::new(),
            }
        )
    );
}

/*
 * get_mapper 
 * Returns the inverse of the mapper 
 */
#[no_mangle]
pub extern "C" fn lib_pauli_tracker_get_inv_mapper(
    mapped_tracker: *mut MappedPauliTracker,
    n_qubits: usize
) -> *mut Vec<usize> {

    let mapping = Box::into_raw(
        Box::new(
            vec![usize::MAX; n_qubits]
        )
    );

    unsafe {
        for (index, value) in mapped_tracker
            .as_ref()
            .unwrap()
            .mapper
            .iter()
            .enumerate()
        {
            (*mapping)[index] = *value; 
        }
    }

    return mapping;
}

/*
 * Constructs a const vec from the mapper
 * This is needed to ensure that the original function doesn't drop the underlying vector
 */
#[no_mangle]
pub extern "C" fn lib_pauli_mapper_to_const_vec(vec: *mut Vec<usize>) -> *mut const_vec::ConstVec<usize>
{
    unsafe{
        return const_vec::vec_to_const_vec::<usize>(vec.as_ref().unwrap()); 
    }
}

/*
 * lib_pauli_tracker_destroy_inv_map
 * Destructor for the inverse mapper object
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_destroy_inv_map(
    vec: *mut Vec<usize>,
    inv_map: *mut const_vec::ConstVec<usize>) 
{
    unsafe {
        let _ = Box::from_raw(inv_map);
    }
    unsafe {
        let _ = Box::from_raw(vec);
    }
}

/*
 * pauli_tracker_destroy
 * Destructor for the pauli tracker object
 * :: pauli_tracker : *mut MappedPauliTracker :: Pointer to the pauli tracker object to be freed 
 * Acts in place to free the pauli tracker object
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_destroy(pauli_tracker: *mut MappedPauliTracker) {
    unsafe {
        let _ = Box::from_raw(pauli_tracker);
    }
}


/*
 * pauli_track_x
 * Add a row to the pauli tracker object with an 'X' at the target qubit  
 * All other locations will be the identity
 * :: pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
#[no_mangle]
extern "C" fn lib_pauli_track_x(mapped_pauli_tracker: &mut MappedPauliTracker, measured_qubit: usize, measurement_target: usize) {
    mapped_pauli_tracker.mapper.push(measured_qubit);
    mapped_pauli_tracker.pauli_tracker.track_x(measurement_target);
}

/*
 * pauli_track_y
 * Add a row to the pauli tracker object with a 'Y' at the target qubit  
 * All other locations will be the identity
 * :: pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
#[no_mangle]
extern "C" fn lib_pauli_track_y(mapped_pauli_tracker: &mut MappedPauliTracker, measured_qubit: usize, measurement_target: usize) {
    mapped_pauli_tracker.mapper.push(measured_qubit);
    mapped_pauli_tracker.pauli_tracker.track_y(measurement_target);
}

/*
 * pauli_track_z
 * Add a row to the pauli tracker object with an 'X' at the target qubit  
 * All other locations will be the identity
 * :: pauli_tracker : &mut MappedPauliTracker :: Pauli tracker object  
 * :: measured_qubit : usize :: The qubit being measured 
 * :: measurement_target : usize :: The qubit with the conditional pauli 
 * Acts in place on the Pauli tracker object
 */
#[no_mangle]
extern "C" fn lib_pauli_track_z(mapped_pauli_tracker: &mut MappedPauliTracker, measured_qubit: usize, measurement_target: usize) {
    mapped_pauli_tracker.mapper.push(measured_qubit);
    mapped_pauli_tracker.pauli_tracker.track_z(measurement_target);
}

/*
 * pauli_tracker_greedy_order
 * Returns the measurement order and dependencies 
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_greedy_order(mapped_pauli_tracker: &mut MappedPauliTracker) -> *mut partial_order_graph::PartialOrderGraph {
     return partial_order_graph::lib_pauli_tracker_partial_order_graph(mapped_pauli_tracker);
}

/*
 * Simple wrapper around the mapper 
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_index_to_qubit(
    mapped_pauli_tracker: &MappedPauliTracker,
    index: usize
) -> usize 
{
        return (*mapped_pauli_tracker).mapper[index];
}


/*
 * pauli_tracker_print
 * Small printing function for the pauli tracker
 * :: mapped_pauli_tracker : &MappedPauliTracker :: Pauli tracker object to print 
 * Acts in place, and writes to stdout
 */
#[no_mangle]
extern "C" fn lib_pauli_tracker_print(mapped_pauli_tracker: &MappedPauliTracker) {
    println!("{mapped_pauli_tracker:?}");
}
