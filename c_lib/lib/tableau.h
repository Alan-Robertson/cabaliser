#ifndef TABLEAU_H
#define TABLEAU_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef struct tableau_t tableau_t;

#include "consts.h"
#include "debug.h"
#include "instructions.h"

#include "simd_transpose.h"
#include "simd_rowsum.h"

#define ROW_MAJOR (1)
#define COL_MAJOR (0)

#define CTZ_SENTINEL (~0ll)

#define SLICE_LEN_BYTES(n_qubits, stride_bytes) (size_t)(((n_qubits) / 8) + (!!(n_qubits % (stride_bytes * 8))) * ((stride_bytes) - (n_qubits / 8) % (stride_bytes)))

#define SLICE_LEN(n_qubits, stride_bytes) (SLICE_LEN_BYTES(n_qubits, stride_bytes) / (stride_bytes))

#define SLICE_LEN_CACHE(n_qubits) (SLICE_LEN(n_qubits, CACHE_SIZE))
#define SLICE_LEN_SIZE_T(n_qubits) (SLICE_LEN(n_qubits, sizeof(size_t)))
#define SLICE_LEN__m128(n_qubits) (SLICE_LEN(n_qubits, sizeof(__m128)))
#define SLICE_LEN__m256(n_qubits) (SLICE_LEN(n_qubits, sizeof(__m256)))


#define GET_CHUNK_PTR(slice, slice_len_bytes, row_idx, col_idx) ((uint64_t*)(((void*)slices) + ((slice_len_bytes) * (col_idx) + ((row_idx) / 8))))
#define GET_CHUNK(slice, slice_len_bytes, row_idx, col_idx) (*GET_CHUNK_PTR(slice, slice_len_bytes, row_idx, col_idx))




#define SLICE_IDX(ptr, idx, slice_len) ((void*)(ptr) + (idx * slice_len)) 
#define SLICE_X_IDX(tab, idx) SLICE_IDX(tab->slices_x[0], idx, tab->slice_len)
#define SLICE_Z_IDX(tab, idx) SLICE_IDX(tab->slices_z[0], idx, tab->slice_len)

// Non-simd stride helpers
#define CHUNK_OBJ uint64_t
#define CHUNK_SIZE sizeof(CHUNK_OBJ)
#define CHUNK_STRIDE CHUNK_SIZE
#define CHUNK_SIZE_BYTES CHUNK_SIZE
#define CHUNK_SIZE_BITS (CHUNK_SIZE_BYTES * BITS_TO_BYTE)
#define __CHUNK_CTZ __builtin_ctzll 

#define CHUNK_IDX(slice, offset_byte) (*(CHUNK_OBJ*)((void*)slice + offset_byte))

#define TABLEAU_SIMD_VEC __m256i
#define TABLEAU_SIMD_LANE_SIZE sizeof(TABLEAU_SIMD_VEC)
#define TABLEAU_SIMD_STRIDE (TABLEAU_SIMD_LANE_SIZE) 
#define TABLEAU_STRIDE(tab) (SLICE_LEN_CACHE(tab->n_qubits) * CACHE_SIZE)

#define CACHE_CHUNKS (CACHE_SIZE / CHUNK_SIZE_BYTES)

struct aligned_chunk {
   CHUNK_OBJ components[CACHE_CHUNKS]; 
};
typedef CHUNK_OBJ tableau_slice;
typedef CHUNK_OBJ* tableau_slice_p;


struct tableau_t {
    size_t n_qubits; // Number of qubits
    size_t slice_len; // Number of bytes
    void* chunks; // Pointer to allocated chunks
    tableau_slice_p* slices_x; // Slice representation pointers 
    tableau_slice_p* slices_z; // Slice representation pointers 
    tableau_slice_p phases; // Phase terms
    bool orientation; // Row or column major order
};

/*
 * tableau_create 
 * Constructor class for tableau  
 * :: n_qubits :: Number of qubits   
 * Acts in place and frees all attributes associated with the tableau object
 */
tableau_t* tableau_create(const size_t n_qubits);


/*
 * tableau_destroy 
 * Destructor class for tableau  
 * :: tableau_t* tab :: Tableau to be freed 
 * Acts in place and frees all attributes associated with the tableau object
 */
void tableau_destroy(tableau_t* tab);


/*
 * slice_set_bit
 * Sets a bit in a slice 
 * :: slice : struct tableau_slice* :: The slice object that the bit should be set for
 * :: index : const size_t :: The index to be set
 * :: value : const uint8_t :: The value to set
 * This function acts in place on the slice object
 *
 * This method has both an explicit inline implementation in the local module, and an exposed 
 * non static inlined method. 
 * The enforced behaviour of static inline is only guaranteed for gcc
 */
void slice_set_bit(
 tableau_slice* slice,
 const size_t index,
 const uint8_t value);

static inline
void __inline_slice_set_bit(
    tableau_slice* slice,
    const size_t index,
    const uint8_t value)
{
    slice[index / CHUNK_SIZE_BITS] &= ~(1ull << (index % CHUNK_SIZE_BITS)); 
    slice[index / CHUNK_SIZE_BITS] |= (1ull & value) << (index % CHUNK_SIZE_BITS); 

    return; 
}
static inline
void __atomic_inline_slice_set_bit(
    tableau_slice_p slice,
    const size_t index,
    const uint8_t value)
{
    __atomic_and_fetch(slice + (index / CHUNK_SIZE_BITS), ~(1ull << (index % CHUNK_SIZE_BITS)), __ATOMIC_ACQUIRE); 
    __atomic_or_fetch(slice + (index / CHUNK_SIZE_BITS), (1ull & value) << (index % CHUNK_SIZE_BITS), __ATOMIC_RELEASE); 

    return; 
}

/*
 * slice_get_bit
 * Gets a bit from a slice 
 * This method has both an explicit inline implementation in the local module, and an exposed 
 * non static inlined method. 
 * The enforced behaviour of static inline is only guaranteed for gcc 
 */
uint8_t slice_get_bit(tableau_slice_p slice, const size_t index);
static inline
uint8_t __inline_slice_get_bit(
    tableau_slice_p slice,
    const size_t index)
{
    CHUNK_OBJ mask = 1ull << (index % CHUNK_SIZE_BITS);
    return !!(slice[index / CHUNK_SIZE_BITS] & mask); 
}

/*
 * tableau_print 
 * Inefficient method for printing a tableau 
 * :: tab : const tableau_t* :: Tableau to print
 */
void tableau_print(const tableau_t* tab);

/*
 * tableau_print_phases 
 * Inefficient method for printing phase terms of a tableau 
 * :: tab : const tableau_t* :: Tableau to print
 */
void tableau_print_phases(const tableau_t* tab);

/*
 * tableau_transpose
 * Transposes a tableau
 * The naive version of this function is used for testing 
 * :: tab : tableau_t* :: Tableau to transpose
 * Flips the orientation field and the member fields
 * This flips the alignment of the cache lines
 */
void tableau_transpose(tableau_t* tab);
void tableau_transpose_naive(tableau_t* tab);

/*
 * tableau_rowsum
 * tableau_rowsum_offset 
 * Performs a rowsum between two rows of stabilisers 
 * The offset version of the function starts
 * from an offset bit index 
 * :: tab : tableau_t const* :: Tableau object
 * :: ctrl : const size_t :: Control of the rowsum
 * :: targ : const size_t :: Target of the rowsum
 */
void tableau_rowsum(
    tableau_t* tab,
    const size_t ctrl,
    const size_t targ);
void tableau_rowsum_offset(tableau_t* tab,
    const size_t ctrl,
    const size_t targ,
    const size_t offset);

/*
 * tableau_slice_empty_x
 * Fast operation for checking if an x slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_x(const tableau_t* tab, size_t idx);

/*
 * tableau_slice_empty_z
 * Fast operation for checking if a z slice is empty
 *  :: tab : const tableau_t* :: The tableau object
 *  :: idx : const size_t :: Index of the slice
 */
bool tableau_slice_empty_z(const tableau_t* tab, size_t idx);

/*
 * tableau_ctz
 * Fast operation for checking if a slice is empty
 *  :: slice : const tableau_t* :: The tableau object
 *  :: len : const size_t :: Index of the slice
 */
size_t tableau_ctz(CHUNK_OBJ* slice, const size_t len);

/*
 * tableau_transverse_hadamard
 * Applies a hadamard when transposed 
 * :: tab : tableau_t*  :: Tableau object
 * :: c_que :  clifford_queue_t* :: Clifford queue 
 * :: i : const size_t :: Index to target 
 *
 */
void tableau_transverse_hadamard(tableau_t const* tab, const size_t targ);

/*
 * tableau_idx_swap_transverse 
 * Swaps indicies over both the X and Z slices  
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void tableau_idx_swap_transverse(tableau_t* tab, const size_t i, const size_t j);

/*
 * tableau_set_n_qubits
 * Truncates the tableau to a set number of qubits
 * :: tab : tableau_t* :: The tableau
 * :: n_qubits : const size_t :: The number of qubits
 * Acts in place on the tableau
 * There is an implicit assumption that the number of qubits should not be greater than the initially allocated number of qubits
 */
void tableau_set_n_qubits(tableau_t* tab, const size_t n_qubits);

#endif 
