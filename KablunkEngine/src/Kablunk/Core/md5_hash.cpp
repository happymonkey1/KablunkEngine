#include "kablunkpch.h"
#include "Kablunk/Core/md5_hash.h"

namespace kb
{ // start namespace kb

// reference https://en.wikipedia.org/wiki/MD5

namespace
{
u32 s_shift_amounts[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};
u32 K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};
}


// #TODO hand vectorize
auto compute_md5_hash(const void* p_buffer, size_t p_size) noexcept -> u128
{
    u32 a0 = 0x67452301;   // A
    u32 b0 = 0xefcdab89;   // B
    u32 c0 = 0x98badcfe;   // C
    u32 d0 = 0x10325476;   // D

    // pre-padding
    size_t new_size = p_size % 64 == 56 ? p_size :
        (56 - (p_size % 64));

    const u8* padded_buffer = nullptr;
    bool needs_free = false;
    if (new_size == p_size)
        padded_buffer = static_cast<const u8*>(p_buffer);
    else
    {
        const auto new_buf = new u8[new_size];

        new_buf[p_size] = 0x80;
        if (const i64 pad_zeros_count = static_cast<i64>(new_size) - (static_cast<i64>(p_size) + 1); pad_zeros_count > 0)
            std::memset(new_buf + p_size + 1, 0, pad_zeros_count);

        padded_buffer = new_buf;
        needs_free = true;
    }

    if (needs_free)
        delete[] padded_buffer;

    constexpr size_t k_words_per_chunk = 16ull;
    constexpr size_t k_chunk_size_bytes = sizeof(u32) * k_words_per_chunk;
    // process in 512 bit chunks
    for (size_t i = 0; i < new_size; i += k_chunk_size_bytes)
    {
        // extract 16 32-bit words
        u32 M[k_words_per_chunk]{ 0 };
        for (size_t j = 0; j < k_words_per_chunk; ++j)
            M[j] = padded_buffer[i + (j * sizeof(u32))];

        // initial hash values for chunk
        u32 A = a0;
        u32 B = b0;
        u32 C = c0;
        u32 D = d0;

        // main loop
        for (size_t k = 0; k < 64; ++k)
        {
            u32 F, g;

            if (k <= 15)
            {
                F = (B & C) | ((~B) & D);
                g = k;
            }
            else if (i >= 16 && i <= 31)
            {
                F = (D & B) | ((~D) & C);
                g = (5 * k + 1) % 16;
            }
            else if (i >= 32 && i <= 47)
            {
                F = B ^ C ^ D;
                g = (3 * i + 5) % 16;
            }
            else
            {
                F = C ^ (B | (~D));
                g = (7 * k) % 16;
            }

            F = F + A + K[i] + M[g];
            A = D;
            D = C;
            C = B;
            B = B + left_rotate(F, s_shift_amounts[k]);
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }


    return u128{
        static_cast<u8>(a0 & 0xFF), static_cast<u8>((a0 >> 8) & 0xFF), static_cast<u8>((a0 >> 16) & 0xFF), static_cast<u8>((a0 >> 24) & 0xFF),
        static_cast<u8>(b0 & 0xFF), static_cast<u8>((b0 >> 8) & 0xFF), static_cast<u8>((b0 >> 16) & 0xFF), static_cast<u8>((b0 >> 24) & 0xFF),
        static_cast<u8>(c0 & 0xFF), static_cast<u8>((c0 >> 8) & 0xFF), static_cast<u8>((c0 >> 16) & 0xFF), static_cast<u8>((c0 >> 24) & 0xFF),
        static_cast<u8>(d0 & 0xFF), static_cast<u8>((d0 >> 8) & 0xFF), static_cast<u8>((d0 >> 16) & 0xFF), static_cast<u8>((d0 >> 24) & 0xFF),
    };
}

} // end namespace kb
