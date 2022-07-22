//
// Created by Sherwin Rad on 2/20/22.
//

#ifndef YSL_MATH_TRIG_H
#define YSL_MATH_TRIG_H

#include <cmath>
#include <xmmintrin.h>
#include <smmintrin.h>

#include "types.h"

namespace ysl
{
// COSINE
template<typename T>
[[nodiscard]] inline T cos(T f)
{
	return ::cos(f);
}

[[nodiscard]] inline f32 cos(f32 f)
{
	return cosf(f);
}

// SIN
template<typename T>
[[nodiscard]] inline T sin(T f)
{
	return ::sin(f);
}

[[nodiscard]] inline f32 sin(f32 f)
{
	return sinf(f);
}

// TAN
template<typename T>
[[nodiscard]] inline T tan(T f)
{
	return ::tan(f);
}

[[nodiscard]] inline f32 tan(f32 f)
{
	return tanf(f);
}

// ATAN
template<typename T>
[[nodiscard]] inline T atan(T f)
{
	return ::atan(f);
}

[[nodiscard]] inline f32 atan(f32 f)
{
	return atanf(f);
}

// ATAN 2
template<typename T>
[[nodiscard]] inline T atan2(T x, T y)
{
	return ::atan2(x, y);
}

[[nodiscard]] inline f32 atan2(f32 x, f32 y)
{
	return atan2f(y, x);
}

// ACOS
template<typename T>
[[nodiscard]] inline T acos(T f)
{
	return ::acos(f);
}

[[nodiscard]] inline f32 acos(f32 f)
{
	return acosf(f);
}

// ASIN
template<typename T>
[[nodiscard]] inline T asin(T f)
{
	return ::asin(f);
}

[[nodiscard]] inline f32 asin(f32 f)
{
	return asinf(f);
}

// COSSIN
template<typename T>
inline void cossin(T &x, T &y, T z)
{
	x = cos(z);
	y = sin(z);
}

inline void cossin(f32 &x, f32 &y, f32 z)
{
	x = cosf(z);
	y = sinf(z);
}

// SQRT
template<typename T>
[[nodiscard]] inline T sqrt(const T s)
{
	return ::sqrt(s);
}

[[nodiscard]] inline double sqrt(const double d)
{
	return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set1_pd(d)));
}

[[nodiscard]] inline f32 sqrt(const f32 f)
{
	return _mm_cvtss_f32(_mm_sqrt_ps(_mm_set1_ps(f)));
}

// RSQRT
template<typename T>
[[nodiscard]] inline T rsqrt(T f)
{
	return 1.0f / sqrt(f);
}

[[nodiscard]] inline f32 rsqrt(f32 f)
{
	return _mm_cvtss_f32(_mm_rsqrt_ps(_mm_set1_ps(f)));
}

} // namespace: ysl

#endif //YVESMATH__TRIG_H_