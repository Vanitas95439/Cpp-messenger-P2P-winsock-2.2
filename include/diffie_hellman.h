#pragma once
#include <cstdlib>
#include <cstdint>
#include <random>
#include <ctime>

#include "mod_pow.h"

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
            
            priv = mt();

            pub = mod_pow(g, priv, p);
        }
    
    uint64_t compute_shared_secret(uint64_t other_pub) {
        return mod_pow(other_pub, priv, p);
    }
};
