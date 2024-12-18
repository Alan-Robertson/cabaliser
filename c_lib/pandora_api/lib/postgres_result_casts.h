#ifndef POSTGRES_RESULT_CASTS_H
#define POSTGRES_RESULT_CASTS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>
// #include <postgresql/libpq-fe.h>

#include "instruction_table.h"

#include "pandora_gates.h"


/*
 * pg_result_print
 * Prints the output of a query
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
void pg_result_print(PGresult* result);

/*
 * pg_result_to_int32_t
 * Converts the first output of a PGresult object to an int32_t
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
int32_t pg_result_to_int32_t(PGresult* result);

/*
 * pg_result_to_pandora_gates
 * Converts the output of a query on the pandora table to an array of pandora_gate_t structs  
 * :: result : PGresult* :: Result object
 * Does not clear the connection after use
 */
size_t pg_result_to_pandora_gates(PGresult* result, instruction_stream_u** stream);


#endif
