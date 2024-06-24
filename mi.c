#include <mi.h>


sticky optimized uint16_t get16bits(const char *data) {
    return *((const uint16_t *)data);
}

uint64_t MiFastHash(const char *data, int len) {
    if (len <= 0 || data == NULL) return 0;

    uint32_t hash1 = (uint32_t)len, hash2 = (uint32_t)len, tmp;
    int rem = len & 3;
    len >>= 2;

    // Main loop, processing both forward and reverse in a single iteration
    const char *data_end = data + len * 2 * sizeof(uint16_t);
    const char *data_rev = data_end - sizeof(uint16_t);
    for (; data < data_end; data += 2 * sizeof(uint16_t), data_rev -= 2 * sizeof(uint16_t)) {
        // Forward hashing
        hash1 += get16bits(data);
        tmp = (uint32_t)(get16bits(data + 2) << 11) ^ hash1;
        hash1 = (hash1 << 16) ^ tmp;
        hash1 += hash1 >> 11;

        // Reverse hashing
        hash2 += get16bits(data_rev);
        tmp = (uint32_t)(get16bits(data_rev - 2) << 11) ^ hash2;
        hash2 = (hash2 << 16) ^ tmp;
        hash2 += hash2 >> 11;
    }

    // Handle end cases for the forward direction
    const char *data_fwd_rem = data_end;
    switch (rem) {
        case 3:
            hash1 += get16bits(data_fwd_rem);
            hash1 ^= hash1 << 16;
            hash1 ^= (uint32_t)(signed char)data_fwd_rem[sizeof(uint16_t)] << 18;
            hash1 += hash1 >> 11;
            break;
        case 2:
            hash1 += get16bits(data_fwd_rem);
            hash1 ^= hash1 << 11;
            hash1 += hash1 >> 17;
            break;
        case 1:
            hash1 += (uint32_t)((signed char)*data_fwd_rem);
            hash1 ^= hash1 << 10;
            hash1 += hash1 >> 1;
            break;
    }

    // Handle end cases for the reverse direction
    const char *data_rev_rem = data - sizeof(uint16_t);
    switch (rem) {
        case 3:
            hash2 += get16bits(data_rev_rem);
            hash2 ^= hash2 << 16;
            hash2 ^= (uint32_t)(signed char)data_rev_rem[-sizeof(uint16_t)] << 18;
            hash2 += hash2 >> 11;
            break;
        case 2:
            hash2 += get16bits(data_rev_rem);
            hash2 ^= hash2 << 11;
            hash2 += hash2 >> 17;
            break;
        case 1:
            hash2 += (uint32_t)((signed char)*data_rev_rem);
            hash2 ^= hash2 << 10;
            hash2 += hash2 >> 1;
            break;
    }

    // Force "avalanching" of final 127 bits for both hashes
    hash1 ^= hash1 << 3;
    hash1 += hash1 >> 5;
    hash1 ^= hash1 << 4;
    hash1 += hash1 >> 17;
    hash1 ^= hash1 << 25;
    hash1 += hash1 >> 6;

    hash2 ^= hash2 << 3;
    hash2 += hash2 >> 5;
    hash2 ^= hash2 << 4;
    hash2 += hash2 >> 17;
    hash2 ^= hash2 << 25;
    hash2 += hash2 >> 6;

    // Combine the two hashes into a uint64_t
    uint64_t combinedHash = ((uint64_t)hash1 << 32) | hash2;
    return combinedHash;
}
