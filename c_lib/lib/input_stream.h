#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H


#include "instructions.h"
#include "tableau_operations.h"
#include "conditional_operations.h"
#include "widget.h"


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
    const size_t n_instructions);


/*
 * apply_local_cliffords
 * Empties the local clifford table and applies the local cliffords
 * :: wid : widget_t* :: The widget
 * Acts in place over the tableau and the clifford table
 */
void apply_local_cliffords(widget_t* wid);

/*
 * teleport_input
 * Sets the widget up to accept teleported inputs
 * :: wid : widget_t* :: Widget on which to teleport inputs 
 * This just applies a hadamard to the first $n$ qubits, then performs pairwise CNOT operations between qubits $i$ and $n + i$  
 * This operation should be called before any gates are passed
 */
void teleport_input(widget_t* wid);


#endif
