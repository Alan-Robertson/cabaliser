#include "input_stream.h"

/*
 * local_clifford_gate
 * Applies a local Clifford operation to the widget
 * Local in this context implies a single qubit operation
 * :: wid : widget_t* :: The widget
 * :: inst : single_qubit_instruction_t* :: The local Clifford operation 
 * Acts in place on the widget, should only update one entry in the local Clifford table 
 */
static inline
void __inline_local_clifford_gate(
    widget_t* wid,
    struct single_qubit_instruction* inst)
{
    size_t target = wid->q_map[inst->arg]; 
    wid->queue->table[target] = LOCAL_CLIFFORD_LEFT(inst->opcode, wid->queue->table[target]);

    // Pauli Correction Tracking
    PAULI_TRACKER_LOCAL(inst->opcode)(wid->pauli_tracker, target);
    return;
} 

/*
 * apply_local_cliffords
 * Empties the local clifford table and applies the local cliffords
 * :: wid : widget_t* :: The widget
 * Acts in place over the tableau and the clifford table
 */
void apply_local_cliffords(widget_t* wid)
{
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        SINGLE_QUBIT_OPERATIONS[wid->queue->table[i] & INSTRUCTION_OPERATOR_MASK](wid->tableau, i);
        wid->queue->table[i] = _I_; 
    }
}

/*
 * non_local_clifford_gate
 * Applies a non-local Clifford operation to the widget
 * Local in this context implies a single qubit operation
 * :: wid : widget_t* :: The widget
 * :: inst : single_qubit_instruction_t* :: The local Clifford operation 
 * Acts in place on the widget, should only update one entry in the local Clifford table 
 */
static inline
void __inline_non_local_clifford_gate(
    widget_t* wid,
    struct two_qubit_instruction* inst)
{
    // First pass dump the tables and implement the instruction
    size_t ctrl = wid->q_map[inst->ctrl]; 
    size_t targ = wid->q_map[inst->targ]; 

    // Execute the queued cliffords 
    // TODO : Merge these operations with CX/CZ gates to cut runtime to one third 
    SINGLE_QUBIT_OPERATIONS[wid->queue->table[ctrl] & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl);
    wid->queue->table[ctrl] = _I_;

    SINGLE_QUBIT_OPERATIONS[wid->queue->table[targ] & INSTRUCTION_OPERATOR_MASK](wid->tableau, targ);
    wid->queue->table[targ] = _I_;
    TWO_QUBIT_OPERATIONS[inst->opcode & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl, targ);

    // Pauli Correction Tracking
    PAULI_TRACKER_NON_LOCAL(inst->opcode)(wid->pauli_tracker, ctrl, targ);

    return;
} 

/*
 * rz_gate 
 * Implements an rz gate as a terminating operation
 * :: wid : widget_t* :: The widget in question 
 * :: inst : rz_instruction* :: The rz instruction indicating an angle 
 * Teleports an RZ operation, allocating a new qubit in the process
 * Compilation fails if the number of qubits exceeds some maximum 
 */
static inline
void __inline_rz_gate(
    widget_t* wid,
    struct rz_instruction* inst) 
{
    
    // This could be handled with a better return
    assert(wid->n_qubits < wid->max_qubits);

    const size_t ctrl = WMAP_LOOKUP(wid, inst->arg);
    const size_t targ = wid->n_qubits; 

    wid->queue->non_cliffords[ctrl] = inst->tag;
    wid->q_map[inst->arg] = wid->n_qubits;

    SINGLE_QUBIT_OPERATIONS[wid->queue->table[ctrl] & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl);
    wid->queue->table[ctrl] = _I_;

    SINGLE_QUBIT_OPERATIONS[wid->queue->table[targ] & INSTRUCTION_OPERATOR_MASK](wid->tableau, targ);
    wid->queue->table[targ] = _I_;

    tableau_CNOT(wid->tableau, ctrl, targ);

    // Propagate tracked Pauli corrections 
    pauli_track_z(wid->pauli_tracker, ctrl, targ);

    // Number of qubits increases by one
    wid->n_qubits += 1;  

    return;
}


// Table of indirections  
void (*instruction_switch[N_INSTRUCTION_TYPES])(widget_t*, void*) = {
        (void (*)(widget_t*, void*))NULL, // 0x00
        (void (*)(widget_t*, void*))__inline_local_clifford_gate, // 0x01
        (void (*)(widget_t*, void*))__inline_non_local_clifford_gate, // 0x02
        (void (*)(widget_t*, void*))NULL, // 0x03
        (void (*)(widget_t*, void*))__inline_rz_gate, // 0x04
        (void (*)(widget_t*, void*))NULL, // 0x05
        (void (*)(widget_t*, void*))NULL, // 0x06
        (void (*)(widget_t*, void*))NULL, // 0x07
};

/*
 * parse_instruction_block
 * Parses a block of instructions 
 * :: wid : widget_t* :: Current widget 
 * :: instructions : instruction_stream_u* :: Array of instructions 
 * :: n_instructions : const size_t :: Number of instructions in the stream 
 * This implementation may be replaced with a thread pool dispatcher
 */
void parse_instruction_block(
    widget_t* wid,
    instruction_stream_u* instructions,
    const size_t n_instructions)
{
    #pragma GCC unroll 8
    for (size_t i = 0; i < n_instructions; i++)
    {
        instruction_switch[
            INSTRUCTION_TYPE((instructions + i)->instruction) 
            ](wid, instructions + i);
    }
    return;
}

/*
 * teleport_input
 * Sets the widget up to accept teleported inputs
 * :: wid : widget_t* :: Widget on which to teleport inputs 
 * This just applies a hadamard to the first $n$ qubits, then performs pairwise CNOT operations between qubits $i$ and $n + i$  
 * This operation should be called before any gates are passed
 */
void teleport_input(widget_t* wid)
{
    // Check that we have sufficient memory for this operation
    assert(wid->max_qubits > 2 * wid->n_initial_qubits);

    // Double the number of initial qubits 
    wid->n_qubits *= 2;
    
    // This could be replaced with a different tableau preparation step
    for (size_t i = 0; i < wid->n_initial_qubits; i++)
    {
        tableau_H(wid->tableau, i);
        tableau_H(wid->tableau, i + wid->n_initial_qubits);

        // Construct a Bell state
        tableau_CZ(wid->tableau, i, wid->n_initial_qubits + i);

        // Fix up the map, we should now be indexing off the target qubit  
        wid->q_map[i] += wid->n_initial_qubits;      
        // TODO: Stop proxying the input qubits like this 
        // pauli_track_z(wid->pauli_tracker, i, wid->n_initial_qubits + i);
    
        pauli_track_x(wid->pauli_tracker, i, wid->n_initial_qubits + i);
    }

    return;
}
