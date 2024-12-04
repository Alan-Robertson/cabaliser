#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct clifford_queue_t clifford_queue_t;

#include "instruction_table.h"
#include "tableau.h"
#include "consts.h"
#include "widget.h"

#define N_INSTRUCTION_TYPES (8)

#define LOCAL_CLIFFORD_LEFT(left, right) (SINGLE_QUBIT_CLIFFORD_MAP[(left & INSTRUCTION_OPERATOR_MASK) * N_LOCAL_CLIFFORDS + (right & INSTRUCTION_OPERATOR_MASK)])

#define LOCAL_CLIFFORD_RIGHT(left, right) (SINGLE_QUBIT_CLIFFORD_MAP_RIGHT[(left & INSTRUCTION_OPERATOR_MASK) * N_LOCAL_CLIFFORDS + (right & INSTRUCTION_OPERATOR_MASK)])


#define NON_LOCAL_CZ_MAP_CTRL(clifford) (CZ_MAP_CTRL[clifford & INSTRUCTION_OPERATOR_MASK]) 
#define NON_LOCAL_CZ_MAP_TARG(clifford) (CZ_MAP_TARG[clifford & INSTRUCTION_OPERATOR_MASK]) 

#define NON_LOCAL_CNOT_MAP_TARG_TARG(clifford) (CNOT_MAP_TARG_TARG[clifford & INSTRUCTION_OPERATOR_MASK]) 
#define NON_LOCAL_CNOT_MAP_CTRL_CTRL(clifford) (CNOT_MAP_CTRL_CTRL[clifford & INSTRUCTION_OPERATOR_MASK]) 
#define NON_LOCAL_CNOT_MAP_CTRL_TARG(clifford) (CNOT_MAP_CTRL_TARG[clifford & INSTRUCTION_OPERATOR_MASK]) 
#define NON_LOCAL_CNOT_MAP_TARG_CTRL(clifford) (CNOT_MAP_TARG_CTRL[clifford & INSTRUCTION_OPERATOR_MASK]) 


#define INSTRUCTION_OP_MASK (0x1f)

/*
 * clifford_queue
 * Implemented a fixed length queue of single qubit Clifford instructions
 * Single qubit additions to the queue simply twirl the queue 
 * Two qubit additions to the queue either:
 *   - Commute through and twirl performing a local operation on the graph state 
 *   - Trigger a dump of the instructions to the tableau 
 */
struct clifford_queue_t
{
    size_t n_qubits; // Number of qubits
    instruction_t* table; // Queued instructions for each qubit 
    non_clifford_tag_t* non_cliffords; // Terminating non-clifford instructions 
};

/*
 * clifford_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the us of the destructor function clifford_queue_destroy
 */
clifford_queue_t* clifford_queue_create(const size_t n_qubits);

/*
 * clifford_queue_destroy
 * Destructor for an instruction queue 
 * :: que : clifford_queue_t* :: Instruction queue to be deallocated
 * Frees queue related memory from the heap
 */
void clifford_queue_destroy(clifford_queue_t* que);





// Only applies to the instructions src file
#ifdef INSTRUCTIONS_SRC


const instruction_t SINGLE_QUBIT_CLIFFORD_MAP[168] = {
    /* I */ _I_, _X_, _Y_, _Z_, _H_, _S_, _R_, _HX_, _SX_, _RX_, _HY_, _HZ_, _SH_, _RH_, _HS_, _HR_, _HSX_, _HRX_, _SHY_, _RHY_, _HSH_, _HRH_, _RHS_, _SHR_,
    /* X */ _X_, _I_, _Z_, _Y_, _HZ_, _RX_, _SX_, _HY_, _R_, _S_, _HX_, _H_, _SHY_, _RHY_, _HR_, _HS_, _HRX_, _HSX_, _SH_, _RH_, _HRH_, _HSH_, _SHR_, _RHS_,
    /* Y */ _Y_, _Z_, _I_, _X_, _HY_, _SX_, _RX_, _HZ_, _S_, _R_, _H_, _HX_, _RHY_, _SHY_, _HSX_, _HRX_, _HS_, _HR_, _RH_, _SH_, _RHS_, _SHR_, _HSH_, _HRH_,
    /* Z */ _Z_, _Y_, _X_, _I_, _HX_, _R_, _S_, _H_, _RX_, _SX_, _HZ_, _HY_, _RH_, _SH_, _HRX_, _HSX_, _HR_, _HS_, _RHY_, _SHY_, _SHR_, _RHS_, _HRH_, _HSH_,
    /* H */ _H_, _HX_, _HY_, _HZ_, _I_, _HS_, _HR_, _X_, _HSX_, _HRX_, _Y_, _Z_, _HSH_, _HRH_, _S_, _R_, _SX_, _RX_, _SHR_, _RHS_, _SH_, _RH_, _RHY_, _SHY_,
    /* S */ _S_, _SX_, _RX_, _R_, _SH_, _Z_, _I_, _RH_, _Y_, _X_, _SHY_, _RHY_, _HX_, _H_, _HRH_, _SHR_, _HSH_, _RHS_, _HZ_, _HY_, _HR_, _HRX_, _HS_, _HSX_,
    /* R */ _R_, _RX_, _SX_, _S_, _RH_, _I_, _Z_, _SH_, _X_, _Y_, _RHY_, _SHY_, _H_, _HX_, _RHS_, _HSH_, _SHR_, _HRH_, _HY_, _HZ_, _HSX_, _HS_, _HRX_, _HR_
};


const instruction_t SINGLE_QUBIT_CLIFFORD_MAP_RIGHT[168] = {
/* I */ _I_, _X_, _Y_, _Z_, _H_, _S_, _R_, _HX_, _SX_, _RX_, _HY_, _HZ_, _SH_, _RH_, _HS_, _HR_, _HSX_, _HRX_, _SHY_, _RHY_, _HSH_, _HRH_, _RHS_, _SHR_,
/* X */ _X_, _I_, _Z_, _Y_, _HX_, _SX_, _RX_, _H_, _S_, _R_, _HZ_, _HY_, _RH_, _SH_, _HSX_, _HRX_, _HS_, _HR_, _RHY_, _SHY_, _HRH_, _HSH_, _SHR_, _RHS_,
/* Y */ _Y_, _Z_, _I_, _X_, _HY_, _RX_, _SX_, _HZ_, _R_, _S_, _H_, _HX_, _SHY_, _RHY_, _HRX_, _HSX_, _HR_, _HS_, _SH_, _RH_, _SHR_, _RHS_, _HRH_, _HSH_,
/* Z */ _Z_, _Y_, _X_, _I_, _HZ_, _R_, _S_, _HY_, _RX_, _SX_, _HX_, _H_, _RHY_, _SHY_, _HR_, _HS_, _HRX_, _HSX_, _RH_, _SH_, _RHS_, _SHR_, _HSH_, _HRH_,
/* H */ _H_, _HZ_, _HY_, _HX_, _I_, _SH_, _RH_, _Z_, _RHY_, _SHY_, _Y_, _X_, _S_, _R_, _HSH_, _HRH_, _RHS_, _SHR_, _RX_, _SX_, _HS_, _HR_, _HSX_, _HRX_,
/* S */ _S_, _RX_, _SX_, _R_, _HS_, _Z_, _I_, _HRX_, _X_, _Y_, _HSX_, _HR_, _HRH_, _RHS_, _HZ_, _H_, _HX_, _HY_, _HSH_, _SHR_, _RH_, _RHY_, _SHY_, _SH_,
/* R */ _R_, _SX_, _RX_, _S_, _HR_, _I_, _Z_, _HSX_, _Y_, _X_, _HRX_, _HS_, _SHR_, _HSH_, _H_, _HZ_, _HY_, _HX_, _RHS_, _HRH_, _SHY_, _SH_, _RH_, _RHY_
};

const instruction_t CZ_MAP_CTRL[N_LOCAL_CLIFFORDS] = {
_I_, _X_, _Y_, _Z_, _NOP_, _S_, _R_, _NOP_, _SX_, _RX_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_};

const instruction_t CZ_MAP_TARG[N_LOCAL_CLIFFORDS] = {
_I_, _Z_, _Z_, _I_, _NOP_, _I_, _I_, _NOP_, _Z_, _Z_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_};

const instruction_t CNOT_MAP_CTRL_CTRL[N_LOCAL_CLIFFORDS] = {
_I_, _X_, _Y_, _Z_, _NOP_, _S_, _R_, _NOP_, _SX_, _RX_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_};
const instruction_t CNOT_MAP_CTRL_TARG[N_LOCAL_CLIFFORDS] = {
_I_, _X_, _X_, _I_, _NOP_, _I_, _I_, _NOP_, _X_, _X_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_};


const instruction_t CNOT_MAP_TARG_TARG[N_LOCAL_CLIFFORDS] = {
_I_, _X_, _Y_, _Z_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _HSH_, _HRH_, _RHS_, _SHR_};
const instruction_t CNOT_MAP_TARG_CTRL[N_LOCAL_CLIFFORDS] = {
_I_, _I_, _Z_, _Z_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _NOP_, _I_, _I_, _Z_, _Z_};




/*
 * __inline_clifford_queue_local_clifford_right
 * __inline_clifford_queue_local_clifford_left
 * Applies a local Clifford from either the left or right of the queue
 * :: que : clifford_queue_t* :: The queue object
 * :: cliff : instrucion_t :: The local clifford instruction
 * :: target : size_t :: The target qubit
 * WARNING Left hand cliffords may only be gate cliffords
 */
static inline
void __inline_clifford_queue_local_clifford_right(clifford_queue_t* que, instruction_t cliff, size_t target)
{
    que->table[target] = LOCAL_CLIFFORD_RIGHT(cliff, que->table[target]);
}
static inline
void __inline_clifford_queue_local_clifford_left(clifford_queue_t* que, instruction_t cliff, size_t target)
{
    que->table[target] = LOCAL_CLIFFORD_LEFT(que->table[target], cliff);
}

#else

extern const instruction_t SINGLE_QUBIT_CLIFFORD_MAP[168]; 
extern const instruction_t SINGLE_QUBIT_CLIFFORD_MAP_RIGHT[168]; 
extern const instruction_t CNOT_MAP_TARG_CTRL[N_LOCAL_CLIFFORDS];

#endif

/*
 * clifford_queue_local_clifford_right 
 * Applies clifford operator from the right of the expression  
 * This is used when extracting Clifford terms from the graph state preparaion
 * :: que : clifford_queue_t* :: The clifford queue
 * :: cliff : instruction_t :: The instruction 
 * :: target : size_t :: The target qubit 
 */
void clifford_queue_local_clifford_right(clifford_queue_t* que, const instruction_t cliff, const size_t target);

#endif
