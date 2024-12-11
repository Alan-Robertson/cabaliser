// Type your code here, or load an example.
#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdio.h>
#include <time.h>

int check(char* a, char* b, char* c, char* d, size_t n) {
    char lut[16] = {
        0, 0, 0, 0, 0, 0, 1, 3, 0, 3, 0, 1, 0, 1, 3, 0
    };

    uint32_t sum = 0;


    for (size_t i = 0; i < n; i++) {
        int idx = (((a[i/8] >> (i % 8)) & 0x1 ) << 3) | 
        (((b[i/8] >> (i % 8)) & 0x1) << 2) | 
        (((c[i/8] >> (i % 8)) & 0x1) << 1)| 
        (((d[i/8] >> (i % 8)) & 0x1) << 0);
        // printf("%x\n", idx);
        sum += lut[idx];
    }
    return sum % 4;
}

int op(uint64_t *restrict a, uint64_t *restrict b, uint64_t *restrict c, uint64_t *restrict d, size_t n) {
    uint64_t pos = 0;
    uint64_t neg = 0;

    for (size_t i = 0; i < n - n % 4; i++) {
        uint64_t plus = 0;
        uint64_t minus = 0;

        plus = (~a[i] & b[i] & c[i] & ~d[i]) | (a[i] & ~b[i] & c[i] & d[i]) | (a[i] & b[i] & ~c[i] & d[i]);
        pos += __builtin_popcountll(plus);

        minus = (a[i] & ~b[i] & ~c[i] & d[i]) | (~a[i] & b[i] & c[i] & d[i]) | (a[i] & b[i] & c[i] & ~d[i]);
        neg += __builtin_popcountll(minus);
    }

    return (pos - neg) % 4;
}

int op2(void *restrict w, void *restrict x, void *restrict y, void *restrict z, size_t n_bytes) {
    uint64_t pos = 0;
    uint64_t neg = 0;
    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i a = _mm256_loadu_si256(w + i);
        __m256i b = _mm256_loadu_si256(x + i);
        __m256i c = _mm256_loadu_si256(y + i);
        __m256i d = _mm256_loadu_si256(z + i);

        __m256i plus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(d, a), _mm256_and_si256(b, c)),
        _mm256_and_si256(_mm256_and_si256(a, d), _mm256_xor_si256(b, c)));
        
        __m256i minus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(c, b), _mm256_and_si256(a, d)),
        _mm256_and_si256(_mm256_and_si256(b, c), _mm256_xor_si256(a, d)));


        pos += _mm_popcnt_u64(_mm256_extract_epi64(plus, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 3));

        neg += _mm_popcnt_u64(_mm256_extract_epi64(minus, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 3));

    }

    return ((pos - neg) % 4 + 4) % 4;
}

int op2b(void *restrict w, void *restrict x, void *restrict y, void *restrict z, size_t n_bytes) {
    uint64_t acc0 = 0;
    uint64_t acc1 = 0;
    uint64_t acc2 = 0;
    uint64_t acc3 = 0;
    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i a = _mm256_loadu_si256(w + i);
        __m256i b = _mm256_loadu_si256(x + i);
        __m256i c = _mm256_loadu_si256(y + i);
        __m256i d = _mm256_loadu_si256(z + i);

        __m256i plus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(d, a), _mm256_and_si256(b, c)),
        _mm256_and_si256(_mm256_and_si256(a, d), _mm256_xor_si256(b, c)));
        
        __m256i minus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(c, b), _mm256_and_si256(a, d)),
        _mm256_and_si256(_mm256_and_si256(b, c), _mm256_xor_si256(a, d)));


        acc0 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 0)) - _mm_popcnt_u64(_mm256_extract_epi64(minus, 0));
        acc1 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 1)) - _mm_popcnt_u64(_mm256_extract_epi64(minus, 1));
        acc2 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 2)) - _mm_popcnt_u64(_mm256_extract_epi64(minus, 2));
        acc3 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 3)) - _mm_popcnt_u64(_mm256_extract_epi64(minus, 3));
    }

    return (acc0 + acc1 + acc2 + acc3) % 4;
}

int op2c(void *restrict w, void *restrict x, void *restrict y, void *restrict z, size_t n_bytes) {
    uint64_t acc0 = 0;
    uint64_t acc1 = 0;
    uint64_t acc2 = 0;
    uint64_t acc3 = 0;
    uint64_t bcc0 = 0;
    uint64_t bcc1 = 0;
    uint64_t bcc2 = 0;
    uint64_t bcc3 = 0;
    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i a = _mm256_loadu_si256(w + i);
        __m256i b = _mm256_loadu_si256(x + i);
        __m256i c = _mm256_loadu_si256(y + i);
        __m256i d = _mm256_loadu_si256(z + i);

        __m256i plus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(d, a), _mm256_and_si256(b, c)),
        _mm256_and_si256(_mm256_and_si256(a, d), _mm256_xor_si256(b, c)));
        
        __m256i minus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(c, b), _mm256_and_si256(a, d)),
        _mm256_and_si256(_mm256_and_si256(b, c), _mm256_xor_si256(a, d)));


        acc0 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 0));
        bcc0 += _mm_popcnt_u64(_mm256_extract_epi64(minus, 0));
        acc1 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 1));
        bcc1 += _mm_popcnt_u64(_mm256_extract_epi64(minus, 1));
        acc2 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 2));
        bcc2 += _mm_popcnt_u64(_mm256_extract_epi64(minus, 2));
        acc3 += _mm_popcnt_u64(_mm256_extract_epi64(plus, 3));
        bcc3 += _mm_popcnt_u64(_mm256_extract_epi64(minus, 3));
    }

    return (((acc0 + acc1 + acc2 + acc3) - (bcc0 + bcc1 + bcc2 + bcc3)) % 4 + 4)%4;
}

int op3(void *restrict w, void *restrict x, void *restrict y, void *restrict z, size_t n_bytes) {
    __m256i mask = _mm256_set1_epi16(0x01);
    __m256i mask2 = _mm256_set1_epi16(0x100);

    __m256i lut = _mm256_setr_epi8(
        0, 0, 0, 0, 0, 0, 1, 3, 0, 3, 0, 1, 0, 1, 3, 0,
        0, 0, 0, 0, 0, 0, 1, 3, 0, 3, 0, 1, 0, 1, 3, 0
    );

    __m256i acc = _mm256_set1_epi8(0);


    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i a = _mm256_loadu_si256(w + i);
        __m256i b = _mm256_loadu_si256(x + i);
        __m256i c = _mm256_loadu_si256(y + i);
        __m256i d = _mm256_loadu_si256(z + i);

        for (int j = 0; j < 8; j++) {
            __m256i p = _mm256_or_si256(
            _mm256_or_si256(
                _mm256_or_si256(
                    _mm256_slli_epi16(
                        _mm256_and_si256(a, mask), 3
                        ),
                    _mm256_slli_epi16(
                        _mm256_and_si256(b, mask), 2
                        )
                ),
                _mm256_or_si256(
                    _mm256_slli_epi16(
                        _mm256_and_si256(c, mask), 1
                        ),
                    _mm256_slli_epi16(
                        _mm256_and_si256(d, mask), 0
                        )
                )
            ),
            _mm256_or_si256(
                _mm256_or_si256(
                    _mm256_slli_epi16(
                        _mm256_and_si256(a, mask2), 3
                        ),
                    _mm256_slli_epi16(
                        _mm256_and_si256(b, mask2), 2
                        )
                ),
                _mm256_or_si256(
                    _mm256_slli_epi16(
                        _mm256_and_si256(c, mask2), 1
                        ),
                    _mm256_slli_epi16(
                        _mm256_and_si256(d, mask2), 0
                        )
                )
            ));

            acc = _mm256_add_epi8(_mm256_shuffle_epi8(lut, p), acc);

            a = _mm256_srli_epi16(a, 1);
            b = _mm256_srli_epi16(b, 1);
            c = _mm256_srli_epi16(c, 1);
            d = _mm256_srli_epi16(d, 1);
        }
    }
    acc = _mm256_sad_epu8(acc, _mm256_set1_epi8(0));
    __m128i acc2 = _mm_add_epi64(_mm256_castsi256_si128(acc),
                 _mm256_extracti128_si256(acc, 1));

    return (_mm_extract_epi64(acc2, 0) +  _mm_extract_epi64(acc2, 1)) % 4;
}

int op5(void *restrict w, void *restrict x, void *restrict y, void *restrict z, size_t n_bytes) {
    __m256i pos = _mm256_setzero_si256();
    __m256i neg = _mm256_setzero_si256();

    __m256i acc = _mm256_setzero_si256();
    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i a = _mm256_loadu_si256(w + i);
        __m256i b = _mm256_loadu_si256(x + i);
        __m256i c = _mm256_loadu_si256(y + i);
        __m256i d = _mm256_loadu_si256(z + i);

        __m256i plus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(d, a), _mm256_and_si256(b, c)),
        _mm256_and_si256(_mm256_and_si256(a, d), _mm256_xor_si256(b, c)));
        
        __m256i minus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(c, b), _mm256_and_si256(a, d)),
        _mm256_and_si256(_mm256_and_si256(b, c), _mm256_xor_si256(a, d)));

        acc ^= _mm256_and_si256(pos, plus);
        acc ^= _mm256_and_si256(neg, minus);

        pos = _mm256_xor_si256(pos, plus);
        neg = _mm256_xor_si256(neg, minus);
    }

    uint64_t total = 0;

    total += _mm_popcnt_u64(_mm256_extract_epi64(pos, 0))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 1))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 2))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 3));

    total -= _mm_popcnt_u64(_mm256_extract_epi64(neg, 0))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 1))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 2))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 3));
    
    total += (_mm_popcnt_u64(_mm256_extract_epi64(acc, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 3))) << 1;


    return total % 4;
}


#define TEST_SIZE 8384
int main(int argc, char const *argv[])
{
    /*
            0, 0, 0, 0, 
            0, 0, 1, 3, 
            0, 3, 0, 1,
            0, 1, 3, 0

    
    
    */

    // char arr[] = {1, 1, 0, 1};
    // printf("%d\n", check(arr, arr+1, arr+2, arr+3, 1));


    int seed = atoi(argv[1]);
    srand(seed);
    printf("rand %d\n", rand());
    char *a = malloc(TEST_SIZE);
    char *b = malloc(TEST_SIZE);
    char *c = malloc(TEST_SIZE);
    char *d = malloc(TEST_SIZE);
    
    for (int i = 0; i < TEST_SIZE; i++){
        a[i] = rand();
        b[i] = rand();
        c[i] = rand();
        d[i] = rand();
    }
    // a[0] = !!(seed & 0x8);
    // b[0] = !!(seed & 0x4);
    // c[0] = !!(seed & 0x2);
    // d[0] = !!(seed & 0x1);
    int c_val = 0;
    int op_val = 0;
    int op2_val = 0;
    int op2b_val = 0;
    int op2c_val = 0;
    int op3_val = 0;
    int op5_val = 0;

    double start, end;

    start = clock();
    for (int i = 0; i < 10000; i++) {
        c_val += check(a, b, c, d, TEST_SIZE * 8);
    }
    end = clock();
    printf("c %d %lf\n", c_val, (end - start)/CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 10000; i++) {
         op_val += op((void*)a,(void*) b,(void*) c,(void*) d, TEST_SIZE/8);
    }
    end = clock();
    printf("o %d %lf\n", op_val, (end - start)/CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 10000; i++) {
        op2_val += op2(a, b, c, d, TEST_SIZE);
    }
    end = clock();
    printf("2a %d %lf\n", op2_val, (end - start)/CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 10000; i++) {
        op2b_val += op2b(a, b, c, d, TEST_SIZE);
    }
    end = clock();
    printf("2b %d %lf\n", op2b_val, (end - start)/CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 10000; i++) {
        op2c_val += op2c(a, b, c, d, TEST_SIZE);
    }
    end = clock();
    printf("2c %d %lf\n", op2c_val, (end - start)/CLOCKS_PER_SEC);


    start = clock();
    for (int i = 0; i < 10000; i++) {
        op3_val += op3(a, b, c, d, TEST_SIZE);
    }
    end = clock();
    printf("3 %d %lf\n", op3_val, (end - start)/CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 10000; i++) {
        op5_val += op5(a, b, c, d, TEST_SIZE);
    }
    end = clock();
    printf("5 %d %lf\n", op5_val, (end - start)/CLOCKS_PER_SEC);

    return 0;
}
