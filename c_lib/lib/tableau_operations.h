#ifndef TABLEAU_OPERATIONS_H
#define TABLEAU_OPERATIONS_H

#include <stdatomic.h>

#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include <x86gprintrin.h>


#include "omp.h"
#include "tableau.h"
#include "instructions.h"

#define OPT_PRAGMA
#ifndef OPT_PRAGMA
#define OPT_PRAGMA
    #pragma GCC optimize("Ofast,unroll-loops")
    #pragma GCC target("avx2,tune=native")
#endif


/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que);


/*
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * :: tab : tableau_t* :: Tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau   
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que);

/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau
 */
void tableau_X_diagonal(tableau_t* tab, clifford_queue_t* c_que);


/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau
 */
void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* c_que, const size_t idx);
void tableau_X_diag_col_upper(tableau_t* tab, const size_t idx);
void tableau_X_diag_col_lower(tableau_t* tab, const size_t idx);





/*
 * tableau_<clifford>
 * Single qubit clifford operations
 * :: tab : tableau_t* :: The tableau to operate on 
 * :: targ : const size_t :: The target qubit
 * Acts in place on the tableau
 */
void tableau_I(tableau_t* tab, const size_t targ); 
void tableau_X(tableau_t* tab, const size_t targ); 
void tableau_Y(tableau_t* tab, const size_t targ); 
void tableau_Z(tableau_t* tab, const size_t targ); 
void tableau_H(tableau_t* tab, const size_t targ); 
void tableau_S(tableau_t* tab, const size_t targ); 
void tableau_R(tableau_t* tab, const size_t targ); 
void tableau_HX(tableau_t* tab, const size_t targ); 
void tableau_SX(tableau_t* tab, const size_t targ); 
void tableau_RX(tableau_t* tab, const size_t targ); 
void tableau_HY(tableau_t* tab, const size_t targ); 
void tableau_HZ(tableau_t* tab, const size_t targ); 
void tableau_SH(tableau_t* tab, const size_t targ); 
void tableau_RH(tableau_t* tab, const size_t targ); 
void tableau_HS(tableau_t* tab, const size_t targ); 
void tableau_HR(tableau_t* tab, const size_t targ); 
void tableau_HSX(tableau_t* tab, const size_t targ);
void tableau_HRX(tableau_t* tab, const size_t targ);
void tableau_SHY(tableau_t* tab, const size_t targ);
void tableau_RHY(tableau_t* tab, const size_t targ);
void tableau_HSH(tableau_t* tab, const size_t targ);
void tableau_HRH(tableau_t* tab, const size_t targ);
void tableau_RHS(tableau_t* tab, const size_t targ);
void tableau_SHR(tableau_t* tab, const size_t targ);


void tableau_CNOT(tableau_t* tab, const size_t ctrl, const size_t targ);
void tableau_CZ(tableau_t* tab, const size_t ctrl, const size_t targ);

#ifdef TABLEAU_OPERATIONS_SRC

  void (*SINGLE_QUBIT_OPERATIONS[N_LOCAL_CLIFFORDS])(tableau_t*, const size_t targ) = {
        tableau_I, 
        tableau_X, 
        tableau_Y, 
        tableau_Z, 
        tableau_H, 
        tableau_S, 
        tableau_R, 
        tableau_HX,
        tableau_SX,
        tableau_RX,
        tableau_HY,
        tableau_HZ,
        tableau_SH,
        tableau_RH,
        tableau_HS,
        tableau_HR,
        tableau_HSX,
        tableau_HRX,
        tableau_SHY,
        tableau_RHY,
        tableau_HSH,
        tableau_HRH,
        tableau_RHS,
        tableau_SHR};

    void (*TWO_QUBIT_OPERATIONS[N_LOCAL_CLIFFORDS])(tableau_t*, const size_t ctrl, const size_t targ) = {
        tableau_CNOT,
        tableau_CZ
};

#else
    extern void (*SINGLE_QUBIT_OPERATIONS[])(tableau_t*, const size_t targ);
    extern void (*TWO_QUBIT_OPERATIONS[])(tableau_t*, const size_t ctrl, const size_t targ);
#endif



#endif

