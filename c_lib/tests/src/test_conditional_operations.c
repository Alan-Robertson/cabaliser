#include <assert.h>
#include <stdlib.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "input_stream.h"
#include "instructions.h"
#include "conditional_operations.h"

/*
 * Test direct application of operations
 */
void test_conditional_operations()
{
    size_t n_qubits = 10;
    widget_t* wid  = widget_create(10, 10);
    for (size_t i = 0; i < n_qubits - 1; i++) 
    {
        conditional_x(wid, i, i + 1);
        conditional_z(wid, i, i + 1);
        conditional_y(wid, i, i + 1);
    }

    widget_destroy(wid);
    return;
}

void test_operation_stream()
{
    size_t n_qubits = 10;
    widget_t* wid  = widget_create(10, 10);

    uint8_t ops[3] = {_MCX_, _MCY_, _MCZ_}; 

    instruction_stream_u inst;
    for (size_t op = 0; op < 3; op++)
    {
        inst.cond.opcode = ops[op]; 
        for (uint32_t i = 0; i < n_qubits - 1; i++) 
        {
            inst.cond.ctrl = i;
            inst.cond.targ = i + 1;
            parse_instruction_block(wid, &inst, 1);
        }
    }
    widget_destroy(wid);
    return;
}


int main()
{
    test_conditional_operations();
    return 0;
}
