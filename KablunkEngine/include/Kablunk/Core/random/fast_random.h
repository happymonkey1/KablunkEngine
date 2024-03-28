#pragma once

#include "Kablunk/Core/CoreTypes.h"

namespace kb::random
{ // start namespace kb::random

namespace util
{ // start namespace ::util

inline auto pcg_hash(u64 p_x) -> kb::u64
{
    p_x ^= p_x >> 30;
    p_x *= 0xbf58476d1ce4e5b9;
    p_x ^= p_x >> 27;
    p_x *= 0x94d049bb133111eb;
    p_x ^= p_x >> 31;
    return p_x;
}

} // end namespace ::util

class fast_random
{
public:
    // generate a fast psuedo random uint64 with a seed
    [[nodiscard]] static auto random_u64(u64& p_seed) noexcept -> u64
    {
        p_seed = util::pcg_hash(p_seed);
        return p_seed;
    }

    // generate a fast psudeo random uint64 with a private seed
    [[nodiscard]] static auto random_u64() noexcept -> u64
    {
        return random_u64(k_init_seed);
    }

    // generate a uniform random u64 within a range
    [[nodiscard]] static auto random_u64_range(u64 p_min, u64 p_max) noexcept -> u64
    {
        // ref: https://stackoverflow.com/a/17554531
        u64 r;
        const u64 range = 1 + p_max - p_min;
        const u64 buckets = std::numeric_limits<u64>::max() / range;
        const u64 limit = buckets * range;

        /* Create equal size buckets all in a row, then fire randomly towards
         * the buckets until you land in one of them. All buckets are equally
         * likely. If you land off the end of the line of buckets, try again. */
        do
        {
            r = random_u64();
        } while (r >= limit);

        return p_min + (r / buckets);
    }

    // generate a fast pseudo random float in the range [0., 1.]
    [[nodiscard]] static auto random_f32() noexcept -> f32
    {
        return static_cast<float>(random_u64()) / static_cast<float>(std::numeric_limits<u64>::max());
    }

    // generate a fast psuedo random float in range [p_min, p_max]
    [[nodiscard]] static auto random_f32_range(f32 p_min, f32 p_max) noexcept -> f32
    {
        return random_f32() * (p_max - p_min) + p_min;
    }

    // simulate a random chance based on a percentage
    // returns true if the situation "succeeds" (rng > input percentage)
    [[nodiscard]] static auto random_chance_check_f32(f32 p_percentage) noexcept -> bool
    {
        u64 random = random_u64();
        const f32 chance = static_cast<f32>(random) / static_cast<f32>(std::numeric_limits<u64>::max());
        return chance < p_percentage;
    }

    // set a fixed seed for the random generation
    static auto set_seed(u64 p_seed) noexcept -> void { k_init_seed = p_seed; }

private:
    // initialize seed based on milliseconds since the epoch
    // should be a decent starting seed
    inline static u64 k_init_seed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count() % 87961661;
};

} // end namespace kb::random
