#define PAULI_TRACKER_SRC
#include "pauli_tracker.h"

#include <stdio.h>

/*
 * pauli_tracker_create
 * Creates a pauli tracker object
 * :: n_qubits : size_t :: Number of qubits to track
 * Returns an opaque pointer to a rust object
 */
void* pauli_tracker_create(size_t n_qubits)
{
    return lib_pauli_tracker_create(n_qubits);
}

/*
 * pauli_tracker_destroy
 * Destroys the pauli tracker object
 * :: tracker : void* :: Opaque pointer to rust object
 * Wrapper around the rust destructor method
 */
void pauli_tracker_destroy(void* tracker)
{
    lib_pauli_tracker_destroy(tracker);
}

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
    uintptr_t target_qubit)
{
    lib_pauli_track_x(tracker, measured_qubit, target_qubit);
}


/*
 * pauli_track_y
 * :: tracker : void* :: Opaque pointer to rust tracker object
 * :: uintptr_t : measured_qubit :: Qubit that was measured
 * :: target_qubit : uintptr_t :: Target of the correction operator
 * Adds a tracking term for the target qubit depending on the measurement result
 */
void pauli_track_y(
    void* tracker,
    uintptr_t measured_qubit,
    uintptr_t target_qubit)
{
    lib_pauli_track_y(tracker, measured_qubit, target_qubit);
}

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
    uintptr_t target_qubit)
{
    lib_pauli_track_z(tracker, measured_qubit, target_qubit);
}


/*
 * pauli_track_I
 * :: tracker : void* :: Tracker object
 * :: target : size_t :: Target of operation
 */
void pauli_track_I_(MappedPauliTracker* tracker, size_t target)
{
    return;
}


/*
 * pauli_track_II_
 * :: tracker : void* :: Tracker object
 * :: target : size_t :: Target of operation
 */
void pauli_track_II_(MappedPauliTracker* tracker, size_t ctrl, size_t target)
{
    return;
}

/*
 * pauli_tracker_disable
 * The pauli tracker panics in certain situations
 * For testing it is sometimes useful to disable it
 */
void pauli_tracker_disable()
{
    for (size_t i = 0; i < N_LOCAL_CLIFFORDS; i++)
    {
        PAULI_TRACKER_LOCAL_TABLE[i] = pauli_track_I_;
    }
    for (size_t i = 0; i < N_NON_LOCAL_CLIFFORDS; i++)
    {
        PAULI_TRACKER_NON_LOCAL_TABLE[i] = pauli_track_II_;
    }
}
