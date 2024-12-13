#define INSTRUCTIONS_TABLE
#define TABLEAU_OPERATIONS_SRC 

#include "tableau_operations.h"

/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que)
{
    // Can't parallelise as the H operation is already parallel
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
       if (CTZ_SENTINEL == tableau_ctz(tab->slices_x[i], tab->slice_len)) 
       {
         DPRINT(DEBUG_3, "Empty X Col, applying H to %lu\n", i);
         tableau_H(tab, i);
         clifford_queue_local_clifford_right(c_que, _H_, i);
       } 
    } 
    return;
}


/*
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * Operation unused as it only applies to the non-transposed matrix
 * :: tab : tableau_t* :: Tableau to act on
 *
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    size_t i;
    #pragma omp parallel for private(i)
    for (i = 0; i < tab->n_qubits; i++)
    { 
        if (__inline_slice_get_bit(tab->slices_z[i], i))  
        {
            DPRINT(DEBUG_3, "Zero Diagonal: Phase on %lu ", i);
            tableau_R(tab, i);
            clifford_queue_local_clifford_right(c_que, i, _S_);   
        }
    }
    return;
}


void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx)
{

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Strategy 1: Swapping %lu <-> %lu\n", j, idx);
            tableau_idx_swap_transverse(tab, idx, j);
            return; 
        } 
    } 

    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
        tableau_transverse_hadamard(tab, idx);
        clifford_queue_local_clifford_right(queue, _H_, idx);   
        return;
    }

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_z[j], idx))
        {
            // Backup Strategy
            tableau_idx_swap_transverse(tab, idx, j);

            DPRINT(DEBUG_3, "Strategy 3: Applying Hadamard to %lu\n", idx);

            tableau_transverse_hadamard(tab, idx);
            clifford_queue_local_clifford_right(queue, _H_, idx);   
            return; 
        } 
    }
    DPRINT(DEBUG_3, "Applying Hadamard to %lu\n", idx);
    return;
}

void tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    size_t  j;
    #pragma omp parallel
    {
        #pragma omp for 
        for (j = idx + 1; j < tab->n_qubits; j++) 
        {
            if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
            {
                DPRINT(DEBUG_3, "Slice XOR Upper: %lu %lu\n", idx, j);
                tableau_slice_xor(tab, idx, j);
            }
        }
    }
    return;
}


void tableau_X_diag_col_lower(tableau_t* tab, const size_t idx)
{
    size_t j;
    CHUNK_OBJ* slice = tab->slices_x[idx];

    #pragma omp parallel
    {
        #pragma omp for
        for (j = 0; j < idx; j++) 
        {
            if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
            {
                DPRINT(DEBUG_3, "Slice XOR Lower: %lu %lu\n", idx, j);
                tableau_slice_xor(tab, idx, j);
            }
        }
    }
    return;
}

/*
 * tableau_X_upper_right_triangular
 * Makes the X block upper right triangular 
 * :: tab : tableau_t* :: Tableau to operate on    
 */
void tableau_X_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    tableau_transpose(tab);

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // X(i, i) != 1 
        if (0 == __inline_slice_get_bit(tab->slices_x[i], i))
        {  
            tableau_X_diag_element(tab, c_que, i);
        }
    }
    return;
}


void tableau_H(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    {
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] & slice_z[i], __ATOMIC_RELAXED);      
        }  
    }
    DPRINT(DEBUG_3, "Hadamard on %lu\n", targ);
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_S(tableau_t* tab, const size_t targ)
{
    /*
     * x -> x  
     * z -> z ^ x
     * r -> r ^ x.z
     *
     */
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    {
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] & slice_z[i], __ATOMIC_RELAXED);      
            slice_z[i] ^= slice_x[i]; 
        }  
    }
}

void tableau_Z(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * Doubled S gate
 * S: (r ^= x.z; z ^= x)
 * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
 * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
 * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
 * r2 = r0 ^ x; z2 = z0 
 */

    size_t i;
    #pragma omp parallel private(i) 
    {
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i], __ATOMIC_RELAXED);      
        }  
    }
}


void tableau_R(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Triple S gate
     * S : (r ^= x.z; z ^= x)
     * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
     *
     * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
     * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
     * r2 = r0 ^ x; z2 = z0 
     *
     * r3 = r0 ^ x ^ x.z2; z3 = z2 ^ x 
     * r3 = r0 ^ x ^ x.z0; z3 = z0 ^ x 
     *
     * R : (r ^= x.~z; z ^= x)
     */

    size_t i;
    #pragma omp parallel private(i) 
    {
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] & ~slice_z[i], __ATOMIC_RELAXED);
            slice_z[i] ^= slice_x[i]; 
        }  
    }
}


void tableau_I(tableau_t* tab, const size_t targ)
{
    return;
}

void tableau_X(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * HZH Gate
 * Flip X and Z, perform a Z, then flip X and Z again
 * H : (r ^= x.z; x <-> z) 
 * Z : (r ^= x)
 * 
 * H
 * r_1 = r_0 ^ x0.z0; x1 = z0; z1 = x0  
 * 
 * Z
 * r_2 = r_1 ^ x1; x2 = x1; z2 = z1  
 * r_2 = r_0 ^ x0.z0 ^ z0; x2 = z0; z2 = x0  
 *
 * H
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z2.x2; x3 = z2; z3 = x2  
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z0.x0; x3 = x0; z3 = z0  
 * r_3 = r_0 ^ z0;
 *
 */
    size_t i;
    #pragma omp parallel private(i) 
    {
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_z[i], __ATOMIC_RELAXED);      
        }
    }
}

void tableau_Y(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Y = XZ
     * Z : (r ^= x)
     * X : (r ^= z)
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ z 
     *
     * Y : r ^= x ^ z
     */
    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_z[i] ^ slice_x[i], __ATOMIC_RELAXED);      
        }  
    }
}

void tableau_HX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * Swap x and z
     */
    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, ~slice_x[i] & slice_z[i], __ATOMIC_RELAXED);      
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_SX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * z_2 = z_0 ^ x
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, ~slice_x[i] & slice_z[i], __ATOMIC_RELAXED);      
            slice_z[i] ^= slice_x[i]; 
        }  
    }
}



void tableau_RX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.~z 
     * r_2 = r_0 ^ (z | x) 
     * z_2 = z_0 ^ x
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] | slice_z[i], __ATOMIC_RELAXED);      
            slice_z[i] ^= slice_x[i]; 
        }  
    }
}

void tableau_HZ(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Z : (r ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ x.z 
     * r_2 = r_0 ^ (x & ~z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, ~slice_z[i] & slice_x[i], __ATOMIC_RELAXED);      
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}



void tableau_HY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Y : r ^= x ^ z
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x ^ z 
     *
     * r_2 = r_0 ^ x ^ z ^ x.z 
     * r_2 = r_0 ^ (x | z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_z[i] ^ slice_x[i], __ATOMIC_RELAXED);      
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_SH(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    /*
     * H : (r ^= x.z; x <-> z) 
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.x_0
     * r_2 = r_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     * 
     */
    // DONE


    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(
            slice_z + i, 
            _mm256_xor_si256(x, z)
        );
    }
}


void tableau_RH(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    /*
     * H : (r ^= x.z; x <-> z) 
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.~z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.~x_0
     * r_2 = r_0 ^ z_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     *
     */
    // DONE
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, z);
        _mm256_store_si256(slice_z + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, z));
    }  
}


void tableau_HS(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    /*
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * x_1 = x
     * z_1 = z_0 ^ x
     * r_1 = r_0 ^ x.z 
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.z ^ x.(x ^ z) 
     * r_2 = r_0 ^ x.z ^ x.~z
     * r_2 = x ^ r_0
     * 
     * z_2 = x_1 = x
     * x_2 = z_1 = z ^ x  
     *
     */
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);
        __m256i r = _mm256_load_si256(slice_r + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
        _mm256_store_si256(slice_r + i, _mm256_xor_si256(r, x));
    }  
}

void tableau_HR(tableau_t* tab, const size_t targ)
{
    void* slice_x = (void*)(tab->slices_x[targ]); 
    void* slice_z = (void*)(tab->slices_z[targ]); 
    /*
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.~z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.~z_0 ^ x.(z_0 ^ x)  
     * x = 0 -> r_0 ^ 0 ^ 0 
     * x = 1 -> r_0 ^ ~z_0 ^ ~z_0   
     * r_2 = r_0
     *
     * z_2 = x_1 = x
     *
     * x_2 = z_1 = z_0 ^ x_0
     *
     */
    // DONE
    for (size_t i = 0; i < tab->n_qubits / 8; i += TABLEAU_SIMD_STRIDE)
    {
        __m256i x = _mm256_load_si256(slice_x + i);   
        __m256i z = _mm256_load_si256(slice_z + i);

        _mm256_store_si256(slice_x + i, _mm256_xor_si256(x, z));
        _mm256_store_si256(slice_z + i, x);
    }  
}

void tableau_HSX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.z_0
     * r_2 = r_0 ^ z_0 ^ x.z_0 
     * r_2 = r_0 ^ ~x.z_0
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ x_0 ^ z_0
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] ^ slice_z[i], __ATOMIC_RELAXED);
            slice_z[i] ^= slice_x[i];
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HRX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.~z_0
     * r_2 = r_0 ^ z_0 ^ x.~z_0 
     * r_2 = r_0 ^ (x | z_0)
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ z
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_z[i], __ATOMIC_RELAXED);
            slice_z[i] ^= slice_x[i];
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_SHY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_z[i] ^ slice_x[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_RHY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HSH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, ~slice_x[i] & slice_z[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
}

void tableau_HRH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] & slice_z[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
}


void tableau_RHS(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] | slice_z[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
}


void tableau_SHR(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, slice_x[i] & ~slice_z[i], __ATOMIC_RELAXED);
            slice_x[i] ^= slice_z[i];
        }  
    }
}

void tableau_CNOT(tableau_t* tab, const size_t ctrl, const size_t targ)
{
    /*
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & (1 ^ x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     */ 

    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[ctrl]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(
                slice_r + i,
                (ctrl_slice_x[i] & targ_slice_z[i] & ~(targ_slice_x[i] ^ ctrl_slice_z[i])),
                 __ATOMIC_RELAXED
            );
            targ_slice_x[i] ^= ctrl_slice_x[i];
            ctrl_slice_z[i] ^= targ_slice_z[i];
        }  
    }
}

void tableau_CZ(tableau_t* tab, const size_t ctrl, const size_t targ)
{
    /* CZ = H_b CNOT H_b 
     * H : (r ^= x.z; x <-> z) 
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & ~(x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     * H:
     * r_1 = r_0 ^ x_b.z_b 
     * x_a1 = x_a0
     * x_b1 = z_b0 
     * z_a1 = z_a0
     * z_b1 = x_b0 
     *
     * CNOT:
     * r_2 = r_1 ^ (x_a0 & z_b1 & ~(x_b1 ^ z_a0))
     * r_2 = r_0 ^ (x_b0 & z.b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0))
     * x_a2 = x_a1
     * x_a2 = x_a0 
     * x_b2 = x_b1 ^ x_a1
     * x_b2 = z_b0 ^ x_a0
     * z_a2 = z_a1 ^ z_b1 
     * z_a2 = z_a0 ^ x_b0 
     * z_b2 = z_b1
     * z_b2 = x_b0
     *
     * H: 
     * r_3 = r_2 ^ (x_b2 & z_b2)  
     * r_3 = r_0 ^ (x_b0 & z_b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0)) ^ ((z_b0 ^ x_a0) & x_b0); 
     * x_a3 = x_a2 = x_a0 
     * x_b3 = z_b2 = x_b0 
     * z_a3 = z_a2 = z_a0 ^ x_b0 
     * z_b3 = x_b2 = z_b0 ^ x_a0 
     *
     */ 

    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[ctrl]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    size_t i;
    // TODO the compiler isn't able to avx this section of code itself, CNOT appears to be near instant while CZ is non-trivial
    #pragma omp parallel private(i)
    { 
        #pragma omp for simd
        for (i = 0; i < tab->slice_len; i++)
        {
            __atomic_fetch_xor(slice_r + i, (
                (ctrl_slice_x[i] & targ_slice_x[i] & ctrl_slice_z[i]) 
                ^ (ctrl_slice_x[i] & targ_slice_x[i] & targ_slice_z[i]) 
            ), __ATOMIC_RELAXED); 
            targ_slice_z[i] ^= ctrl_slice_x[i];
            ctrl_slice_z[i] ^= targ_slice_x[i];
        }  
    }
}