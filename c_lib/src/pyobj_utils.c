#include "pyobj_utils.h"


void pauli_string_conv(uint8_t* restrict  src, char* restrict  dst, const size_t len)
{
    // TODO: Potential speedup using avx masking here
    const static char table[4] = {'I', 'Z', 'X', 'Y'};  
    for (size_t i = 0; i < len; i++) 
    {
        dst[i] = table[src[i]]; 
    }
    return;
}
