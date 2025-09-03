#pragma once
#include <cstdlib>
#include <cstdint>
#include <random>
#include <ctime>

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

struct DHKeypair
{
    uint64_t p; // prime modulus
    uint64_t g; // base (generator)
    uint64_t priv; // private key
    uint64_t pub; // public key

    DHKeypair (uint64_t pt, uint64_t gt) 
        : p {pt}, g {gt}
        {
            std::mt19937_64 mt{};
            
            priv = mt() % p;

            pub = mod_pow(g, priv, p);
        }
    
    uint64_t compute_shared_secret(uint64_t other_pub) {
        return mod_pow(other_pub, priv, p);
    }
};
