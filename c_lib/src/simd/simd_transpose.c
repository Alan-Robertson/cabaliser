#include "simd_transpose.h"

// Transposes two blocks
static inline
void __inline_simd_transpose_2x16(uint8_t** src, uint8_t** targ)
{
    __m256i msrc = _mm256_set_epi16(
            *(uint16_t*)src[15],
            *(uint16_t*)src[14],
            *(uint16_t*)src[13],
            *(uint16_t*)src[12],
            *(uint16_t*)src[11],
            *(uint16_t*)src[10],
            *(uint16_t*)src[9],
            *(uint16_t*)src[8],
            *(uint16_t*)src[7],
            *(uint16_t*)src[6],
            *(uint16_t*)src[5],
            *(uint16_t*)src[4],
            *(uint16_t*)src[3],
            *(uint16_t*)src[2],
            *(uint16_t*)src[1],
            *(uint16_t*)src[0]
            );     

    // Transpose high and low bytes
    // This shuffle is within each 128 byte lane
    __m256i shuffle_mask = _mm256_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15, 16, 18, 20, 22, 24, 26, 28, 30, 17, 19, 21, 23, 25, 27, 29, 31); 
    msrc = _mm256_shuffle_epi8(msrc, shuffle_mask); 
    
    // Transpose between 128 bit lanes
    msrc = _mm256_permute4x64_epi64(msrc, 0xd8);

    // bmi2 operations act 3x faster on registers
    register uint64_t a_tl = _mm_extract_epi64(__builtin_ia32_extract128i256(msrc, 0), 0);
    register uint64_t a_tr = _mm_extract_epi64(__builtin_ia32_extract128i256(msrc, 1), 0);
    register uint64_t a_bl = _mm_extract_epi64(__builtin_ia32_extract128i256(msrc, 0), 1);
    register uint64_t a_br = _mm_extract_epi64(__builtin_ia32_extract128i256(msrc, 1), 1);

    // Scopes to attempt to force register use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0101010101010101ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0101010101010101ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[0][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0202020202020202ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0202020202020202ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[1][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0404040404040404ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0404040404040404ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[2][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0808080808080808ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0808080808080808ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[3][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x1010101010101010ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x1010101010101010ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[4][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x2020202020202020ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x2020202020202020ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[5][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x4040404040404040ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x4040404040404040ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[6][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x8080808080808080ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x8080808080808080ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[7][0] |= (uint64_t)col_l;
    }


    
    // Scope to permit register re-use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0101010101010101ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0101010101010101ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[8][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0202020202020202ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0202020202020202ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[9][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0404040404040404ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0404040404040404ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[10][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0808080808080808ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0808080808080808ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[11][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x1010101010101010ull);
            register uint64_t col_br = _pext_u64(a_br, 0x1010101010101010ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[12][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x2020202020202020ull);
            register uint64_t col_br = _pext_u64(a_br, 0x2020202020202020ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[13][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x4040404040404040ull);
            register uint64_t col_br = _pext_u64(a_br, 0x4040404040404040ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[14][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x8080808080808080ull);
            register uint64_t col_br = _pext_u64(a_br, 0x8080808080808080ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[15][0] |= (uint64_t)col_l;
    }

    return;
}
void simd_transpose_2x16(uint8_t** src, uint8_t** targ)
{
    __inline_simd_transpose_2x16(src, targ); 
}


void simd_transpose_64x64(uint64_t* block_a[64], uint64_t* block_b[64])
{
     uint64_t src_block[64] = {0};
     uint64_t targ_block[64] = {0};
    
     uint64_t* src_ptr[16];
     uint64_t* targ_ptr[16] = {NULL};


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }
            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(src_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_b[i + 16 * col])) + row); 
            }
            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }

    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], src_block + i, 8); 
        memcpy(block_b[i], targ_block + i, 8); 
    } 
 
    return;
}
void simd_transpose_64x64_inplace(uint64_t* block_a[64])
{
     uint64_t targ_block[64] = {0};
    
     uint64_t* src_ptr[16];
     uint64_t* targ_ptr[16] = {NULL};


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }

            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);                     
        }
    }
    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], targ_block + i, 8); 
    } 
 
    return;
}


/*
 * Scalar implementation of the simd transpose
 * Used for regression testing
 */
static inline
void __inline_chunk_transpose_2x16(uint8_t** src, uint8_t** targ)
{
    for (size_t row = 0; row < 16; row++)  
    {
        uint64_t dispersed_vals = 0;
        for (size_t i = 0; i < 16; i++)  
        {
            dispersed_vals |= (!!(((uint16_t**)src)[i][0] & (1 << row))) << i; 
        }
        *((uint16_t*)(targ[row])) = dispersed_vals;
    } 
}
void __attribute__((noinline))
chunk_transpose_2x16(uint8_t** src, uint8_t** targ)
{
    __inline_chunk_transpose_2x16(src, targ); 
}



void __attribute__((noinline))
chunk_transpose_64x64(uint64_t* block_a[64], uint64_t* block_b[64])
{
     uint64_t src_block[64] = {0};
     uint64_t targ_block[64] = {0};
    
     uint64_t* src_ptr[16];
     uint64_t* targ_ptr[16] = {NULL};

    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }
            chunk_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(src_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_b[i + 16 * col])) + row); 
            }
            chunk_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }

    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], src_block + i, 8); 
        memcpy(block_b[i], targ_block + i, 8); 
    } 
 
    return;
}



static inline
uint8_t __inline_get_bit(
    uint64_t* slice,
    const size_t index)
{
    uint64_t mask = 1ull << (index % 64);
    return !!(slice[index / 64] & mask); 
}

void __inline_set_bit(
    uint64_t* slice,
    const size_t index,
    const uint8_t value)
{
    slice[index / 64] &= ~(1ull << (index % 64)); 
    slice[index / 64] |= (1ull & value) << (index % 64); 
}

void transpose_naive(uint64_t** block, const size_t size)
{

    for (size_t i = 0; i < size; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        uint64_t* ptr = block[i]; 
    
        for (size_t j = i + 1; j < size; j++)
        {
            uint8_t val_a = __inline_get_bit(ptr, j); 
            uint8_t val_b = __inline_get_bit(block[j], i); 

            __inline_set_bit(ptr, j, val_b);
            __inline_set_bit(block[j], i, val_a);
        }    
    }
    return;
}


