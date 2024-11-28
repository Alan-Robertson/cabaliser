#ifndef INSTRUCTIONS_TABLE_H
#define INSTRUCTIONS_TABLE_H

#include <stdint.h>

#define OPCODE_WIDTH ((uint8_t)(1))
#define LOCAL_CLIFFORD_MASK ((uint8_t)(1 << 5)) 
#define NON_LOCAL_CLIFFORD_MASK ((uint8_t)(1 << 6)) 
#define RZ_MASK ((uint8_t)(1 << 7)) 

#define MEASUREMENT_CONDITIONED_MASK = ((uint8_t)((1 << 6) | (1 << 7)))    

#define N_LOCAL_CLIFFORDS 24 
#define N_NON_LOCAL_CLIFFORDS 2 

#define N_LOCAL_CLIFFORD_INSTRUCTIONS (7)
#define N_NON_LOCAL_CLIFFORD_INSTRUCTIONS (2)

#define INSTRUCTION_TYPE_MASK (LOCAL_CLIFFORD_MASK | NON_LOCAL_CLIFFORD_MASK | RZ_MASK) 
#define INSTRUCTION_TYPE(instruction) ((uint8_t)(instruction >> 5)) 

#define INSTRUCTION_OPERATOR_MASK (~INSTRUCTION_TYPE_MASK) 

#define _I_ (0x00 | LOCAL_CLIFFORD_MASK)
#define _X_ (0x01 | LOCAL_CLIFFORD_MASK)
#define _Y_ (0x02 | LOCAL_CLIFFORD_MASK)
#define _Z_ (0x03 | LOCAL_CLIFFORD_MASK)
#define _H_ (0x04 | LOCAL_CLIFFORD_MASK)
#define _S_ (0x05 | LOCAL_CLIFFORD_MASK)
#define _R_ (0x06 | LOCAL_CLIFFORD_MASK)

#define _HX_ (0x07 | LOCAL_CLIFFORD_MASK)
#define _SX_ (0x08 | LOCAL_CLIFFORD_MASK)
#define _RX_ (0x09 | LOCAL_CLIFFORD_MASK)
#define _HY_ (0x0a | LOCAL_CLIFFORD_MASK)
#define _HZ_ (0x0b | LOCAL_CLIFFORD_MASK)
#define _SH_ (0x0c | LOCAL_CLIFFORD_MASK)
#define _RH_ (0x0d | LOCAL_CLIFFORD_MASK)
#define _HS_ (0x0e | LOCAL_CLIFFORD_MASK)
#define _HR_ (0x0f | LOCAL_CLIFFORD_MASK)
#define _HSX_ (0x10 | LOCAL_CLIFFORD_MASK)
#define _HRX_ (0x11 | LOCAL_CLIFFORD_MASK)
#define _SHY_ (0x12 | LOCAL_CLIFFORD_MASK)
#define _RHY_ (0x13 | LOCAL_CLIFFORD_MASK)
#define _HSH_ (0x14 | LOCAL_CLIFFORD_MASK)
#define _HRH_ (0x15 | LOCAL_CLIFFORD_MASK)
#define _RHS_ (0x16 | LOCAL_CLIFFORD_MASK)
#define _SHR_ (0x17 | LOCAL_CLIFFORD_MASK)

#define _CNOT_ (0x00 | NON_LOCAL_CLIFFORD_MASK) 

#define _CZ_ (0x01 | NON_LOCAL_CLIFFORD_MASK) 
#define _RZ_ (RZ_MASK)

#define _NOP_ (0xff) 

#define _MCX_ = (0x01 | MEASUREMENT_CONDITIONED_MASK)
#define _MCY_ = (0x02 | MEASUREMENT_CONDITIONED_MASK)
#define _MCZ_ = (0x03 | MEASUREMENT_CONDITIONED_MASK)


/*
 * instruction_struct
 * Simple struct that allows for easy instruction parsing  
 */
typedef uint8_t instruction_t;
typedef uint32_t non_clifford_tag_t;


/*
 * single qubit instruction struct
 *
 */
struct single_qubit_instruction
{
    instruction_t opcode;
    uint32_t arg;
};
struct two_qubit_instruction
{
    instruction_t opcode;
    uint32_t ctrl;
    uint32_t targ;
};
struct rz_instruction
{
    instruction_t opcode;
    uint32_t arg;
    uint32_t tag; // Tag for gate type, supports up to 2**32 unique non-Clifford gates per widget 
    // As each non-Clifford results in a teleportation, we should saturate memory bounds before 
    // this limit is reached   
};


struct measurement_conditioned_instruction
{
    instruction_t opcode; // Type of operation is encoded in opcode 
    uint32_t ctrl;  
    uint32_t targ;
}; 

/* 
 * instruction_stream
 * Union of instruction types for readability
 * All types are aliased on the first field   
 * This first field is the opcode, which can be used as a distinguisher   
 */
union instruction_stream
{
    instruction_t instruction;
    struct single_qubit_instruction single;
    struct two_qubit_instruction multi;
    struct rz_instruction rz; 
};
typedef union instruction_stream instruction_stream_u;

#endif
