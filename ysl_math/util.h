#ifndef YSL_MATH_UTILITY_H
#define YSL_MATH_UTILITY_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <immintrin.h>

#include "types.h"

namespace ysl
{

// Bit_cast code from: https://en.cppreference.com/w/cpp/numeric/bit_cast
// bit_cast is not supported on Apple's Clang, so here we are
// As of 4/1/22 I switched to visual studio (MSVC) so this code is no longer needed, but kept
// for future reference. 
//template<typename To, typename From>
//std::enable_if_t<
//	sizeof(To) == sizeof(From) &&
//	std::is_trivially_copyable_v<From> &&
//	std::is_trivially_copyable_v<To>,
//	To>
//	bit_cast(const From &src) noexcept
//{
//	static_assert(std::is_trivially_constructible_v<To>,
//				  "This implementation additionally requires destination type to be trivially constructible");
//	To dst;
//	std::memcpy(&dst, &src, sizeof(To));
//	return dst;
//}

// Constants
constexpr f32 infinity = std::numeric_limits<f32>::infinity();
constexpr f32 pi = 3.14159265358932385f;
constexpr f32 pi_180 = 0.0174532925f;

static std::vector<u32> state(8);

static void init_seed()
{
	std::random_device gen;
	std::seed_seq seq{gen(), gen(), gen(), gen(), gen(), gen(), gen(), gen()};
	seq.generate(state.begin(), state.end());
}


// Regular math functions
[[nodiscard]] inline f32 min(f32 f1, f32 f2)
{
	return fmin(f1, f2);
}

template<typename T>
[[nodiscard]] inline T min(T f1, T f2)
{
	return min(f1, f2);
}

[[nodiscard]] inline f32 max(f32 f1, f32 f2)
{
	return fmax(f1, f2);
}

template<typename T>
[[nodiscard]] inline T max(T f1, T f2)
{
	return max(f1, f2);
}

// ABS
[[nodiscard]] inline f32 abs(const f32 &f)
{
	return fabsf(f);
}

[[nodiscard]] inline double abs(const double &f)
{
	return fabs(f);
}

[[nodiscard]] inline i32 abs(i32 f)
{
	return ::abs(f);
}

// POW
[[nodiscard]] inline f32 pow(f32 f1, f32 f2)
{
	return powf(f1, f2);
}
template<typename T>
[[nodiscard]] inline T pow(T f1, T f2)
{
	return pow(f1, f2);
}

[[nodiscard]] inline f32 rcp(f32 f)
{
	return _mm_cvtss_f32(_mm_rcp_ps(_mm_set1_ps(f)));
}

[[nodiscard]] inline bool is_equal(f32 f1, f32 f2, f32 epsilon = 0.001f)
{
	return fabs(f1-f2) < epsilon;
}

namespace util
{

// Utility functions
[[nodiscard]] constexpr inline f32 degrees_to_radians(f32 degrees)
{
	return degrees * pi_180;
}


// Random f32 generation from:
// https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
inline void random_f32_8(f32 *f_out)
{
	__m256i states = _mm256_loadu_epi32(&state[0]);
	const __m256i constant1 = _mm256_set1_epi32(747796405u);
	const __m256i constant2 = _mm256_set1_epi32(2891336453u);
	const __m256i constant3 = _mm256_set1_epi32(277803737u);

	states = _mm256_add_epi32(_mm256_mul_epi32(states, constant1), constant2);

	const __m256i word = _mm256_mul_epi32(
		_mm256_xor_si256(
			_mm256_srav_epi32(states,
							  _mm256_add_epi32(
								  _mm256_srav_epi32(states,
													_mm256_set1_epi32(28u)),
								  _mm256_set1_epi32(4u))),
			states),
		constant3);

	states = _mm256_xor_si256(
		_mm256_srav_epi32(word,
						  _mm256_set1_epi32(22u)),
		word);

	constexpr u32 tof32 = 0x2f800004u;

	const __m256 tof322 = _mm256_set1_ps(reinterpret_cast<const f32 &>(tof32));
	__m256 f32s = _mm256_mul_ps(_mm256_cvtepi32_ps(states), tof322);
	f32s = _mm256_add_ps(f32s, _mm256_set1_ps(0.5f));
	_mm256_storeu_ps(f_out, f32s);
}

inline f32 random_f32()
{

	u32 state_ = rand();//state[0];
	constexpr u32 tof32 = 0x2f800004u;

	u32 state2 = state_ * 747796405u + 2891336453u;
	u32 word = ((state2 >> ((state2 >> 28u) + 4u)) ^ state2) * 277803737u;
	state_ = (word >> 22u) ^ word;

	return static_cast<f32>(state_) * reinterpret_cast<const f32 &>(tof32);
}

inline f32 random_f32(f32 min, f32 max)
{
	return min + (max - min) * random_f32();
}

inline i32 random_i32(i32 min, i32 max)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<i32> dist(min, max);

	return dist(mt);

}

constexpr inline f32 clamp(f32 x, f32 min, f32 max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

} // namespace util

} // namespace ysl

#endif