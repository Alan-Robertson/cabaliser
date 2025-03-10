#ifndef SIMD_GAUSSIAN_ELIMINATION_H
#define SIMD_GAUSSIAN_ELIMINATION_H


#include "widget.h"
#include "tableau_operations.h"

#include "simd_headers.h"

typedef struct widget_t widget_t;

/*
 * simd_widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords 
 * :: wid : widget_t* :: Widget to decompose 
 * Acts in place on the tableau 
 */
void simd_widget_decompose(struct widget_t* wid);
void naive_widget_decompose(struct widget_t* wid);


/*
 * simd_tableau_idx_swap_transverse 
 * Swaps indicies over both the X and Z slices  
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void simd_tableau_idx_swap_transverse(tableau_t* restrict tab, const size_t i, const size_t j);


void simd_tableau_X_diag_col_upper(tableau_t* tab, const size_t idx);


void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx);
size_t simd_tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx);


void simd_tableau_elim(widget_t* wid);

void tableau_elim_upper(widget_t* wid);
void tableau_elim_lower(widget_t* wid);


void decomp_load_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
);

void decomp_store_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
);

void debug_print_block(uint64_t block[64]);



/*
 * simd_swap
 * TODO: Fix
 * Swaps indicies over both the X and Z slices
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over
 * :: i :: const size_t :: Index to swap
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau
 */
void simd_swap(
    void* slice_i_x,
    void* slice_i_z,
    void* slice_j_x,
    void* slice_j_z,
    size_t slice_len);

void simd_tableau_transverse_hadamard(tableau_t const* tab, const size_t targ);

#endif
