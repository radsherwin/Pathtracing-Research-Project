#ifndef YSL_MATH_TYPES_H
#define YSL_MATH_TYPES_H

#include <cstdint>

#define BASE_FAST_LEAST 1

// integer types
#if BASE_FAST_LEAST == 0
using i8 =  int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 =  uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
#elif BASE_FAST_LEAST == 1
using i8 =  int_fast8_t;
using i16 = int_fast16_t;
using i32 = int_fast32_t;
using i64 = int_fast64_t;
using u8 =  uint_fast8_t;
using u16 = uint_fast16_t;
using u32 = uint_fast32_t;
using u64 = uint_fast64_t;
#else 
using i8 =  int_least8_t;
using i16 = int_least16_t;
using i32 = int_least32_t;
using i64 = int_least64_t;
using u8 =  uint_least8_t;
using u16 = uint_least16_t;
using u32 = uint_least32_t;
using u64 = uint_least64_t;
#endif

using f32 = float;
using f64 = double;
using f128 = long double;

using uchar = unsigned char;

#endif