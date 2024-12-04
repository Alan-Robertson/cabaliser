#include <string.h>
#include "test_transpose.h"
#include "simd_transpose.h"


void test_2x16()
{
    const size_t n_channels = 16;
    const size_t stride = 256;

    uint64_t* arr_a = NULL;

    uint64_t* arr_b_naive = NULL;
    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    const size_t n_bytes = sizeof(uint64_t) * stride * n_channels;

    // Set up aligned memory
    int ret_val = posix_memalign((void**)&arr_a, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_b_naive, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_b_chunk, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_b_simd, 64, n_bytes);  

    assert(0 == ret_val);

    // Flush the memory 
    memset(arr_a, 0, n_bytes); 
    memset(arr_b_naive, 0, n_bytes); 
    memset(arr_b_chunk, 0, n_bytes); 
    memset(arr_b_simd, 0, n_bytes); 

    uint64_t* ptrs_a[16];     
    uint64_t* ptrs_b_naive[16];     
    uint64_t* ptrs_b_chunk[16];     
    uint64_t* ptrs_b_simd[16];     

    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a[i] = arr_a + (i * stride); 
        ptrs_b_naive[i] = arr_b_naive + (i * stride); 
        ptrs_b_chunk[i] = arr_b_chunk + (i * stride); 
        ptrs_b_simd[i] = arr_b_simd + (i * stride); 
    }

    // Setting up the test
    // Left Column
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[0][0] |= 1 << i;
    }

    // Right block offset 1 
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[i + 8][0] |= 2;// << (i + 8);
    }

    // Below block 
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[3][0] |= 1 << (i + 8);
    }

    // Diagonal
    for (size_t i = 1; i < 8; i++)
    {
        ptrs_a[i][0] |= (1 << (i - 1));
    }

    // Lower left block
    for (size_t i = 8; i < 16; i++)
    {
        ptrs_a[i][0] |= (1 << (i - 1));
    }

    memcpy(arr_b_naive, arr_a, n_bytes);

    transpose_naive(ptrs_b_naive, 16);
    simd_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_simd);
    chunk_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_chunk);

    for (size_t i = 0; i < 16; i++)
    {
        assert(ptrs_b_chunk[i][0] == ptrs_b_simd[i][0]);
        assert(ptrs_b_naive[i][0] == ptrs_b_simd[i][0]);
    }

    free(arr_a);
    free(arr_b_simd);
    free(arr_b_chunk);

    return;
}

void test_64x64()
{
    const size_t n_channels = 64;
    const size_t stride = 1; 

    uint64_t* arr_a_naive = NULL;
    uint64_t* arr_a_chunk = NULL;
    uint64_t* arr_a_simd = NULL;

    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    // Creating a single 64x64 block
    const size_t n_bytes = sizeof(uint64_t) * n_channels;
    const size_t col_size = sizeof(uint64_t);

    // Set up aligned memory
    int ret_val = posix_memalign((void**)&arr_a_naive, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_a_chunk, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_a_simd, 64, n_bytes);  

    ret_val |= posix_memalign((void**)&arr_b_chunk, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_b_simd, 64, n_bytes);  
    assert(0 == ret_val);

    // Flush the memory 
    memset(arr_a_naive, 0, n_bytes); 
    memset(arr_a_chunk, 0, n_bytes); 
    memset(arr_a_simd, 0, n_bytes); 

    memset(arr_b_chunk, 0, n_bytes); 
    memset(arr_b_simd, 0, n_bytes); 


    uint64_t* ptrs_a_naive[64] = {NULL};     
    uint64_t* ptrs_a_chunk[64] = {NULL};     
    uint64_t* ptrs_a_simd[64] = {NULL};     

    uint64_t* ptrs_b_chunk[64] = {NULL};     
    uint64_t* ptrs_b_simd[64] = {NULL};     
    
    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a_naive[i] = arr_a_naive + i;
        ptrs_a_chunk[i] = arr_a_chunk + i;
        ptrs_a_simd[i] = arr_a_simd + i;

        ptrs_b_chunk[i] = arr_b_chunk + i;
        ptrs_b_simd[i] = arr_b_simd + i;
    }

    for (size_t i = 0; i < n_bytes; i++)
    {
        ((uint8_t*)arr_a_naive)[i] = (i | (i % 3)) | (i << (i % 7));
    }
    memcpy(arr_a_chunk, arr_a_naive, n_bytes); 
    memcpy(arr_a_simd, arr_a_naive, n_bytes); 

    // Initial state equal    
    assert(n_bytes == 8 * 64);
    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_a_naive)[i] == ((uint8_t*)arr_a_simd)[i]);
        assert(((uint8_t*)arr_a_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
    }

    transpose_naive(ptrs_a_naive, 64);
    simd_transpose_64x64(ptrs_a_simd, ptrs_b_simd);
    chunk_transpose_64x64(ptrs_a_chunk, ptrs_b_chunk);

    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_a_naive)[i] == ((uint8_t*)arr_b_chunk)[i]);
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
        assert(((uint8_t*)arr_a_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
    }

    transpose_naive(ptrs_a_naive, 64);
    simd_transpose_64x64(ptrs_a_simd, ptrs_b_simd);
    chunk_transpose_64x64(ptrs_a_chunk, ptrs_b_chunk);

    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
        assert(((uint8_t*)arr_a_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
    }
    
    free(arr_a_chunk);
    free(arr_a_simd);
    free(arr_b_chunk);
    free(arr_b_simd);
}


void test_inplace_64x64()
{
    const size_t n_channels = 64;
    const size_t stride = 1; 

    uint64_t* arr_a_chunk = NULL;
    uint64_t* arr_a_simd = NULL;

    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    // Creating a single 64x64 block
    const size_t n_bytes = sizeof(uint64_t) * n_channels;
    const size_t col_size = sizeof(uint64_t);

    // Set up aligned memory
    
    int ret_val = posix_memalign((void**)&arr_a_chunk, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_a_simd, 64, n_bytes);  
    ret_val |= posix_memalign((void**)&arr_b_chunk, 64, n_bytes);  

    assert(0 == ret_val);


    // Flush the memory 
    memset(arr_a_chunk, 0, n_bytes); 
    memset(arr_a_simd, 0, n_bytes); 

    memset(arr_b_chunk, 0, n_bytes); 


    uint64_t* ptrs_a_chunk[64] = {NULL};     
    uint64_t* ptrs_a_simd[64] = {NULL};     

    uint64_t* ptrs_b_chunk[64] = {NULL};     
    
    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a_chunk[i] = arr_a_chunk + i;
        ptrs_a_simd[i] = arr_a_simd + i;

        ptrs_b_chunk[i] = arr_b_chunk + i;
    }

    for (size_t i = 0; i < n_bytes; i++)
    {
        ((uint8_t*)arr_a_chunk)[i] = (i | (i % 3)) | (i << (i % 7));
        ((uint8_t*)arr_a_simd)[i] = (i | (i % 3)) | (i << (i % 7));
    }

    // Initial state equal    
    assert(n_bytes == 8 * 64);
    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_a_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
    }

    simd_transpose_64x64_inplace(ptrs_a_simd);
    chunk_transpose_64x64(ptrs_a_chunk, ptrs_b_chunk);

    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
    }

    simd_transpose_64x64_inplace(ptrs_a_simd);
    chunk_transpose_64x64(ptrs_a_chunk, ptrs_b_chunk);

    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_a_chunk)[i] == ((uint8_t*)arr_a_simd)[i]);
    }
    
    free(arr_a_chunk);
    free(arr_a_simd);
    free(arr_b_chunk);
}

int main()
{
    for (size_t i = 0; i < 1000; i++)
    {
        srand(i);
        test_2x16();
        test_64x64();
        test_inplace_64x64();
    }
    return 0;
}
