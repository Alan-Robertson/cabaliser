#define INSTRUCTIONS_TABLE

#include <stdlib.h>

#include "widget.h"
#include "tableau_operations.h"
#include "input_stream.h"
#include "instructions.h"


instruction_stream_u* create_instruction_stream(const size_t n_qubits, const size_t n_gates)
{

    instruction_stream_u* inst = malloc(n_gates * sizeof(instruction_stream_u));  

    for (size_t i = 0; i < n_gates / 4; i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    for (size_t i = n_gates / 4; i < n_gates / 2 ; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() % N_NON_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t ctrl = rand() % n_qubits; 
        size_t targ;

        while ((targ = (rand() % n_qubits)) == ctrl){}; 

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }

    for (size_t i = n_gates / 2; i < 3 * (n_gates / 4); i++)
    {
        uint8_t opcode = LOCAL_CLIFFORD_MASK | (rand() % N_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t arg = rand() % n_qubits; 
        inst[i].single.opcode = opcode;
        inst[i].single.arg = arg;
    }

    for (size_t i = n_gates - (3 * (n_gates / 4)); i < n_gates; i++)
    {
        uint8_t opcode = NON_LOCAL_CLIFFORD_MASK | (rand() % N_NON_LOCAL_CLIFFORD_INSTRUCTIONS);
        size_t ctrl = rand() % n_qubits; 
        size_t targ;

        while ((targ = (rand() % n_qubits)) == ctrl){}; 

        inst[i].multi.opcode = opcode;
        inst[i].multi.ctrl = ctrl;
        inst[i].multi.targ = targ;
    }
    return inst;
}

void instruction_stream_benchmark(
    const size_t n_qubits,
    const size_t n_gates)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);
    instruction_stream_u* inst = create_instruction_stream(n_qubits, n_gates);
    parse_instruction_block(wid, inst, n_gates);

    widget_destroy(wid);
    free(inst);
    return;
} 

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("Insufficient parameters, requires <n_qubits> <n_gates> <seed>\n");
    return 0;
    }

    size_t tableau_size = atoi(argv[1]);
    size_t n_gates = atoi(argv[2]);
    uint32_t seed = atoi(argv[3]);
     
    srand(seed);

    instruction_stream_u* inst = create_instruction_stream(tableau_size, n_gates);
    free(inst);

    instruction_stream_benchmark(tableau_size, n_gates);

    return 0;
}
