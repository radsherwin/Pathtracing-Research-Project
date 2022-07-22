#ifndef YSL_MATH_VECT_H
#define YSL_MATH_VECT_H

#include "trig.h"
#include "util.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <array>
#include <bit>

struct mat4;
namespace ysl
{

//------------------------------------------------------------------------------------------------------
//                                  Base Vect
//------------------------------------------------------------------------------------------------------
template<size_t size>
struct vec;

template<size_t size>
vec<size> cross(const vec<size> &v1, const vec<size> &v2);

template<size_t size>
f32 dot(const vec<size> &v1, const vec<size> &v2);

template<size_t size>
vec<size> operator*(f32 a, const vec<size> &rhs);

template<size_t size>
vec<size> operator-(vec<size> &lhs);

template<size_t size>
vec<size> vmin(const vec<size> &a, const vec<size> &b);

template<size_t size>
vec<size> vmax(const vec<size> &a, const vec<size> &b);

void transpose_matrix(vec<4> &v1, vec<4> &v2, vec<4> &v3, vec<4> &v4);


//------------------------------------------------------------------------------------------------------
//                                  Vect classes
//------------------------------------------------------------------------------------------------------
template<size_t size>
struct vec
{
	vec() : _m(_mm_setzero_ps()) {}; 
	vec(const vec &) = default;
	~vec() = default;

	vec &operator=(const vec &v)
	{
		if (this != &v)
		{
			_m = v._m;
		}

		return *this;
	}

	explicit vec(std::array<f32, 4> a)
		: _m(std::bit_cast<__m128>(a))
	{}

	explicit vec(std::array<f32, 3> a)
		: _m(_mm_setr_ps(a[0], a[1], a[2], 0.0f))
	{}
	explicit vec(std::array<i32, 3> a)
		: _m(_mm_setr_ps(static_cast<f32>(a[0]), static_cast<f32>(a[1]), static_cast<f32>(a[2]), 0.0f))
	{}

	explicit vec(__m128 m)
		: _m(m)//_m(std::move(m))
	{}

	vec(const vec<2> &a, const vec<2> &b) requires (size == 4)
		: _m(_mm_setr_ps(a.x(), a.y(), b.x(), b.y()))
	{}

	vec(const vec<3> &a, f32 s) requires (size == 4)
		: _m(_mm_setr_ps(a.x(), a.y(), a.z(), s))
	{}

	explicit vec(f32 val)
		: _m(_mm_set1_ps(val))
	{}

	vec(f32 x, f32 y)
		: _m(_mm_setr_ps(x, y, 0.0f, 0.0f))
	{}

	vec(f32 x, f32 y, f32 z) requires(size > 2)
		: _m(_mm_setr_ps(x, y, z, 0.0f))
	{}

	vec(f32 x, f32 y, f32 z, f32 w) requires(size == 4)
		: _m(_mm_setr_ps(x, y, z, w))
	{}

	inline f32 operator[](const size_t &i) const
	{
		assert(i >= 0 && i < size);
		return std::bit_cast<std::array<f32, 4>>(_m)[i];
	}

	std::ostream &operator<<(std::ostream &out)
	{
		auto m_f32 = std::bit_cast<std::array<f32, 4>>(_m);
		for (size_t i = 0; i < size; ++i)
		{
			out << m_f32[i];
			if (i + 1 < size) out << ' ';
		}

		return out;
	}


	inline vec operator *(const vec &v2) const
	{
		const __m128 m = _mm_mul_ps(_m, v2._m);
		return vec{m};
	}

	inline vec operator *=(const vec &v2)
	{
		_m = _mm_mul_ps(_m, v2._m);
		return *this;
	}

	inline vec operator *(f32 s) const
	{
		const __m128 m = _mm_mul_ps(_m, _mm_set1_ps(s));

		return vec{m};
	}

	inline vec operator *=(f32 s)
	{
		_m = _mm_mul_ps(_m, _mm_set1_ps(s));

		return *this;
	}

	inline vec operator +(const vec &v2) const
	{
		const __m128 m = _mm_add_ps(_m, v2._m);
		return vec{m};
	}

	inline vec operator +=(const vec &v2)
	{
		_m = _mm_add_ps(_m, v2._m);
		return *this;
	}

	inline vec operator -(const vec &v2) const
	{
		const __m128 m = _mm_sub_ps(_m, v2._m);
		return vec{m};
	}

	inline vec operator -=(const vec &v2)
	{
		_m = _mm_sub_ps(_m, v2._m);
		return *this;
	}

	inline vec operator /(f32 s) const
	{
		__m128 m = _mm_rcp_ps(_mm_set1_ps(s));
		m = _mm_mul_ps(_m, m);
		return vec{m};
	}

	constexpr vec operator /(const vec &v2) const
	{
		__m128 m = _mm_rcp_ps(v2._m);
		m = _mm_mul_ps(_m, m);
		return vec{m};
	}

	inline vec operator /=(const vec &v2)
	{
		const __m128 m = _mm_rcp_ps(v2._m);
		_m = _mm_mul_ps(_m, m);
		return *this;
	}

	inline vec operator /=(f32 s)
	{

		_m = _mm_mul_ps(_m, _mm_rcp_ps(_mm_set1_ps(s)));
		return *this;
	}

	inline vec operator-() const
	{
		const __m128 m = _mm_xor_ps(_m, _mm_set1_ps(-0.f));
		return vec{m};
	}

	// Static functions
	static vec random()
	{
		const __m128 m = _mm_set_ps(0.0f, ysl::util::random_f32(), ysl::util::random_f32(), ysl::util::random_f32());

		return vec{m};
	}

	static vec<size> random(f32 min, f32 max)
	{
		const __m128 m = _mm_set_ps(0.0f, ysl::util::random_f32(min, max), ysl::util::random_f32(min, max), ysl::util::random_f32(min, max));

		return vec{m};
	}

	inline void rcp()
	{

		_m = _mm_rcp_ps(_m);
	}

	inline vec get_rcp() const
	{
		__m128 m = _mm_rcp_ps(_m);
		m = _mm_insert_ps(m, _mm_set_ss(_mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(3, 3, 3, 3)))), 0x30);
		return vec{m};
	}	

	inline vec<4> flip_sign() const
	{
		const __m128 m  = _mm_mul_ps(_m, _mm_set1_ps(-1.0f));
		return vec<4>{m};
	}

	inline bool near_zero() const
	{
		const auto s = 1e-8;
		for (size_t i = 0; i < size; ++i)
		{
			if (ysl::abs(std::bit_cast<std::array<f32, 4>>(_m)[i]) > s) return false;
		}

		return true;
	}

	[[nodiscard]] inline f32 x() const { return _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(0, 0, 0, 0))); }
	[[nodiscard]] inline f32 y() const { return _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(1, 1, 1, 1))); }//
	[[nodiscard]] inline f32 z() const requires (size > 2) { return _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(2, 2, 2, 2))); }
	[[nodiscard]] inline f32 w() const requires (size > 3) { return _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(3, 3, 3, 3))); }

	[[nodiscard]] inline ysl::vec<2> xy() const requires (size == 4) { return vec<2>{_mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(0, 0, 0, 0))), _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(1, 1, 1, 1)))};}
	[[nodiscard]] inline ysl::vec<2> zw() const requires (size == 4) { return vec<2>{_mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(2, 2, 2, 2))), _mm_cvtss_f32(_mm_shuffle_ps(_m, _m, _MM_SHUFFLE(3, 3, 3, 3)))}; }
	
	[[nodiscard]] inline ysl::vec<3> yzx() const requires(size >= 3)
	{
		// TODO: use shuffle instead
		return ysl::vec<3>{y(), z(), x()};
	}

	[[nodiscard]] inline ysl::vec<3> zxy() const requires(size >= 3)
	{
		// TODO: use shuffle instead
		return ysl::vec<3>{z(), x(), y()};
	}

	[[nodiscard]]  inline vec<3> xyz() const // trunc3
	{
		return vec<3>{_m};
	}

	[[nodiscard]] inline vec<4> xyzw() const // to4
	{
		const __m128 m = _mm_insert_ps(_m, _mm_set_ss(0.0f), 0x30);
		return vec<4>{m};
	}

	[[nodiscard]] inline vec<4> xyzw(f32 f) const // to4
	{
		const __m128 m = _mm_insert_ps(_m, _mm_set_ss(f), 0x30);
		return vec<4>{m};
	}

	std::array<f32, 4> as_floats() const
	{
		const auto x = std::bit_cast<std::array<f32, 4>>(_m);
		return x;
	}

	// Hack but oh well
	inline void set(i32 idx, f32 s)
	{
		switch (idx)
		{
			case 0:
				_m = _mm_move_ss(_m, _mm_set_ss(s));
				break;
			case 1:
				_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x10);
				break;
			case 2:
				_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x20);
				break;
			case 3:
				_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x30);
				break;
			default:
				assert(false);
		}
	}

	inline void set(f32 s)
	{
		_m = _mm_set1_ps(s);
	}

	inline void set_x(f32 s)
	{
		_m = _mm_move_ss(_m, _mm_set_ss(s));
	}

	inline void set_y(f32 s)
	{
		_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x10);
	}

	inline void set_z(f32 s)
	{
		_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x20);
	}

	inline void set_w(f32 s)
	{
		_m = _mm_insert_ps(_m, _mm_set_ss(s), 0x30);
	}

	// Math functions
	[[nodiscard]] inline f32 mag() const
	{
		__m128 m = _mm_mul_ps(_m, _m);
		m = _mm_hadd_ps(m, m);
		m = _mm_hadd_ps(m, m);
		m = _mm_sqrt_ps(m);
		return _mm_cvtss_f32(m);

		//// Slow mag
		//std::array<f32, 4> a = std::bit_cast<std::array<f32, 4>>(_m);
		//f32 m = a[0]*a[0] + a[1]*a[1]+a[2]*a[2]+a[3]*a[3];
		//m = sqrtf(m);
		//return m;
	}

	[[nodiscard]] inline f32 mag_squared() const
	{
		__m128 m = _mm_mul_ps(_m, _m);
		m = _mm_hadd_ps(m, m);
		m = _mm_hadd_ps(m, m);
		return _mm_cvtss_f32(m);
	}

	inline void norm()
	{
		const f32 _mag = mag();
		/*__m128 m = _mm_rcp_ps(_mm_set1_ps(_mag));
		_m = _mm_mul_ps(_m, m);*/
		_m = _mm_div_ps(_m, _mm_set1_ps(_mag));
	}

	[[nodiscard]] inline vec get_norm() const
	{
		const f32 _mag = mag();
		/*__m128 m = _mm_rcp_ps(_mm_set1_ps(_mag));
		m = _mm_mul_ps(_m, m);*/
		__m128 m = _mm_div_ps(_m, _mm_set1_ps(_mag));
		return vec{m};
	}

	[[nodiscard]] inline f32 dot(const vec &v2)
	{
		__m128 m = _mm_mul_ps(_m, v2._m2);
		m = _mm_hadd_ps(m, m);
		m = _mm_hadd_ps(m, m);
		return _mm_cvtss_f32(m);;
	}

	[[nodiscard]] inline vec cross(const vec &v) const
	{
		__m128 tmp0 = _mm_shuffle_ps(_m, _m, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 tmp1 = _mm_shuffle_ps(v._m, v._m, _MM_SHUFFLE(3, 1, 0, 2));
		__m128 tmp2 = _mm_mul_ps(tmp0, v._m);
		const __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
		tmp2 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
		tmp0 = _mm_sub_ps(tmp3, tmp2);

		return vec{tmp0};
	}

	inline void print() const
	{
		printf("vec%zu: ( ", size);
		for (size_t i = 0; i < size; ++i)
		{
			printf("%f ", std::bit_cast<std::array<f32, 4>>(_m)[i]);
		}
		printf(")\n");
	}

private:
	template<size_t size> friend vec<size> cross(const vec<size> &v1, const vec<size> &v2);
	template<size_t size> friend f32 dot(const vec<size> &v1, const vec<size> &v2);
	template<size_t size> friend vec<size> operator*(f32 a, const vec<size> &rhs);
	template<size_t size> friend vec<size> operator-(vec<size> &lhs);
	template<size_t size> friend vec<size> vmin(const vec<size> &a, const vec<size> &b);
	template<size_t size> friend vec<size> vmax(const vec<size> &a, const vec<size> &b);
	friend void transpose_matrix(vec<4> &v1, vec<4> &v2, vec<4> &v3, vec<4> &v4);

public:
	__m128 _m;
};

//------------------------------------------------------------------------------------------------------
//                                  Operator Overloading
//------------------------------------------------------------------------------------------------------

template<size_t size>
inline vec<size> operator*(f32 a, const vec<size> &rhs)
{
	__m128 m = _mm_mul_ps(rhs._m, _mm_set1_ps(a));

	return vec<size>{m};
}

template<size_t size>
inline vec<size> operator-(vec<size> &lhs)
{
	lhs._m = _mm_mul_ps(lhs._m, _mm_set1_ps(-1.0f));
	return lhs;
}


//------------------------------------------------------------------------------------------------------
//                                  Friends
//------------------------------------------------------------------------------------------------------

// Cross
template<size_t size>
inline vec<size> cross(const vec<size> &v1, const vec<size> &v2)
{
	__m128 tmp0 = _mm_shuffle_ps(v1._m, v1._m, _MM_SHUFFLE(3, 0, 2, 1));
	const __m128 tmp1 = _mm_shuffle_ps(v2._m, v2._m, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp2 = _mm_mul_ps(tmp0, v2._m);
	const __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
	tmp2 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
	tmp0 = _mm_sub_ps(tmp3, tmp2);
	return vec<size>{tmp0};
}

template<size_t size>
inline f32 dot(const vec<size> &v1, const vec<size> &v2)
{
	__m128 m = _mm_mul_ps(v1._m, v2._m);
	m = _mm_hadd_ps(m, m);
	m = _mm_hadd_ps(m, m);
	return _mm_cvtss_f32(m);
}

template<size_t size>
inline vec<size> vmin(const vec<size> &a, const vec<size> &b)
{
	return vec<size>{ysl::min(a[0], b[0]), ysl::min(a[1], b[1]), ysl::min(a[2], b[2])};
}

template<size_t size>
inline vec<size> vmax(const vec<size> &a, const vec<size> &b)
{
	return vec<size>{ysl::max(a[0], b[0]), ysl::max(a[1], b[1]), ysl::max(a[2], b[2])};
}

inline void transpose_matrix(vec<4> &v1, vec<4> &v2, vec<4> &v3, vec<4> &v4)
{
	const __m128 r1 = _mm_shuffle_ps(v1._m, v2._m, _MM_SHUFFLE(1, 0, 1, 0));
	const __m128 r2 = _mm_shuffle_ps(v1._m, v2._m, _MM_SHUFFLE(3, 2, 3, 2));
	const __m128 r3 = _mm_shuffle_ps(v3._m, v4._m, _MM_SHUFFLE(1, 0, 1, 0));
	const __m128 r4 = _mm_shuffle_ps(v3._m, v4._m, _MM_SHUFFLE(3, 2, 3, 2));
	v1._m = _mm_shuffle_ps(r1, r3, _MM_SHUFFLE(2, 0, 2, 0));
	v2._m = _mm_shuffle_ps(r1, r3, _MM_SHUFFLE(3, 1, 3, 1));
	v3._m = _mm_shuffle_ps(r2, r4, _MM_SHUFFLE(2, 0, 2, 0));
	v4._m = _mm_shuffle_ps(r2, r4, _MM_SHUFFLE(3, 1, 3, 1));
}

//------------------------------------------------------------------------------------------------------
//                                  Publics
//------------------------------------------------------------------------------------------------------

inline vec<3> reflect(const vec<3> &v, const vec<3> &n)
{
	return v - 2 * dot(v, n) * n;
}

inline vec<4> reflect(const vec<4> &v, const vec<4> &n)
{
	vec<4> new_v = v - 2 * dot(v, n) * n;
	return new_v;
}

template<size_t size>
inline vec<size> refract(const vec<size> &uv, const vec<size> &n, f32 etai_over_etat)
{
	const f32 cos_theta = ysl::min(dot(-uv, n), 1.0f);
	const vec<size> r_out_perp = etai_over_etat * (uv + cos_theta * n);
	const vec<size> r_out_parallel = -ysl::sqrt(ysl::abs(1.0f - r_out_perp.mag_squared())) * n;
	return r_out_perp + r_out_parallel;
}

//------------------------------------------------------------------------------------------------------
//                                  typedef and defaults
//------------------------------------------------------------------------------------------------------

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

// custom typedef for raytracing
typedef vec<3> point3;
typedef vec<4> point4;

// defaults
const vec<4> zero_vec4 = vec<4>{};
const vec<3> zero_vec3 = vec<3>{};
const vec<2> zero_vec2 = vec<2>{};
const vec<3> identity_vec3 = vec<3>(1);
const vec<4> identity_vec4 = vec<4>(0,0,0,1);

} // namespace ysl

#endif //YVESMATH__VECT_H_
