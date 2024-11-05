#ifndef PAULI_TRACKER_H
#define PAULI_TRACKER_H

#include <stddef.h>

#include "instruction_table.h"
#include "lib_pauli_tracker.h" 
#include "lib_pauli_tracker_cliffords.h" 

#define INPUT_TRACKING(widget, target) (widget->n_initial_qubits + target) 

struct pauli_frame_t
{
    size_t len;
    void*  elements;
};

struct dependency_t
{
    size_t len;
    size_t* dependencies; 
};

/*
 * pauli_tracker_create
 * Creates a new pauli tracker object
 * :: n_qubits : size_t :: Number of qubits in the tracker 
 * Creates a new opaque object via a rust library binding
 */
void* pauli_tracker_create(size_t n_qubits);


/*
 * pauli_tracker_destroy
 * Destroys the pauli tracker object
 * :: tracker : void* :: Opaque pointer to rust object 
 * Wrapper around the rust destructor method 
 */
void pauli_tracker_destroy(void* tracker);

/*
 * pauli_track_x
 * :: tracker : void* :: Opaque pointer to rust tracker object 
 * :: uintptr_t : measured_qubit :: Qubit that was measured 
 * :: target_qubit : uintptr_t :: Target of the correction operator
 * Adds a tracking term for the target qubit depending on the measurement result 
 */
void pauli_track_x(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit);

/*
 * pauli_track_z
 * :: tracker : void* :: Opaque pointer to rust tracker object 
 * :: uintptr_t : measured_qubit :: Qubit that was measured 
 * :: target_qubit : uintptr_t :: Target of the correction operator
 * Adds a tracking term for the target qubit depending on the measurement result 
 */
void pauli_track_z(
    void* tracker, 
    uintptr_t measured_qubit,
    uintptr_t target_qubit);

/*
 * pauli_track_I
 * :: tracker : void* :: Tracker object 
 * :: target : size_t :: Target of operation 
 */
void pauli_track_I_(MappedPauliTracker* tracker, size_t target);

/*
 * pauli_tracker_disable
 * The pauli tracker panics in certain situations
 * For testing it is sometimes useful to disable it
 */
void pauli_tracker_disable();


// TODO: Double check these equivalence classes
#ifdef PAULI_TRACKER_SRC
const void (*PAULI_TRACKER_LOCAL_TABLE[24])(MappedPauliTracker*, size_t) = {
 pauli_track_I_, // _I_
 pauli_track_I_, // _X_
 pauli_track_I_, // _Y_
 pauli_track_I_, // _Z_
 pauli_tracker_h, // _H_
 pauli_tracker_s, // _S_
 pauli_tracker_s, // _R_
 pauli_tracker_h, // _HX_
 pauli_tracker_s, // _SX_
 pauli_tracker_s, // _RX_
 pauli_tracker_h, // _HY_
 pauli_tracker_h, // _HZ_
 pauli_tracker_sh, // _SH_
 pauli_tracker_sh, // _RH_
 pauli_tracker_hs, // _HS_
 pauli_tracker_hs, // _HR_
 pauli_tracker_hs, // _HSX_
 pauli_tracker_hs, // _HRX_
 pauli_tracker_sh, // _SHY_
 pauli_tracker_sh, // _RHY_
 pauli_tracker_shs, // _HSH_
 pauli_tracker_shs, // _HRH_,
 pauli_tracker_shs, // _RHS_,
 pauli_tracker_shs  // _SHR_
};

const void (*PAULI_TRACKER_NON_LOCAL_TABLE[2])(MappedPauliTracker*, size_t ctrl, size_t targ) = { 
    pauli_tracker_cx,
    pauli_tracker_cz
};

#else

extern const void (*PAULI_TRACKER_LOCAL_TABLE[24])(MappedPauliTracker*, size_t); 
extern const void (*PAULI_TRACKER_NON_LOCAL_TABLE[2])(MappedPauliTracker*, size_t ctrl, size_t targ); 

#endif

#define PAULI_TRACKER_LOCAL(opcode) (PAULI_TRACKER_LOCAL_TABLE[opcode - _I_]) 

#define PAULI_TRACKER_NON_LOCAL(opcode) (PAULI_TRACKER_NON_LOCAL_TABLE[opcode - _CNOT_]) 

#endif
