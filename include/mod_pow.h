#pragma once
#include <cstdint>

uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod; 
    while (exp > 0) {
        if (exp & 1ULL) {
            result = (result * base) % mod; 
        }
        base = (base * base) % mod; 
        exp >>= 1ULL; 
    }
    return result;
}