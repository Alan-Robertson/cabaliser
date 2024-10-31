#include "tableau_operations.h"
#include "simd.h"

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

    void* ctrl_slice_x = (void*)(tab->slices_x[ctrl]); 
    void* ctrl_slice_z = (void*)(tab->slices_z[ctrl]); 
    void* targ_slice_x = (void*)(tab->slices_x[targ]); 
    void* targ_slice_z = (void*)(tab->slices_z[targ]); 
    void* slice_r = (void*)(tab->phases); 

    // Manually unrolled into two 32 byte chunks
    #pragma omp parallel for
    for (size_t i = 0; i < tab->n_qubits; i += 32)
    {
        {
            __m256i ctrl_x = _mm256_load_si256(ctrl_slice_x + i);
            __m256i ctrl_z = _mm256_load_si256(ctrl_slice_z + i);
            __m256i targ_x = _mm256_load_si256(targ_slice_x + i);
            __m256i targ_z = _mm256_load_si256(targ_slice_z + i);
            __m256i phase = _mm256_load_si256(slice_r + i);

            //__atomic_fetch_xor(slice_r + i, (
            //      (targ_slice_x[i] & targ_slice_z[i]) 
            //    ^ (ctrl_slice_x[i] & targ_slice_x[i] & ~(targ_slice_z[i] ^ ctrl_slice_z[i]))
            //    ^ ((targ_slice_z[i] ^ ctrl_slice_x[i]) & targ_slice_x[i])
            //    ), __ATOMIC_RELAXED);

            __m256i dst_a = _mm256_and_si256(targ_x, targ_z);   
            __m256i dst_b = _mm256_and_si256(ctrl_x, targ_x);   
            __m256i dst_c = _mm256_xor_si256(targ_z, ctrl_z);   
            __m256i dst_d = _mm256_xor_si256(targ_z, ctrl_x);   

            phase = _mm256_xor_si256(dst_a, phase);
            dst_b = _mm256_andnot_si256(dst_c, dst_b);  
            dst_d = _mm256_and_si256(dst_d, targ_x); 

            phase = _mm256_xor_si256(dst_b, phase); 
            phase = _mm256_xor_si256(dst_d, phase); 
        
            //ctrl_slice_z[i] ^= targ_slice_x[i];
            //targ_slice_z[i] ^= ctrl_slice_x[i];

            ctrl_z = _mm256_xor_si256(ctrl_z, targ_x); 
            targ_z = _mm256_xor_si256(targ_z, ctrl_x); 

            _mm256_store_si256(ctrl_slice_z + i, ctrl_z); 
            _mm256_store_si256(targ_slice_z + i, targ_z);
            _mm256_store_si256(slice_r + i, phase);
        }
    }
}
