#ifndef YSL_MATH_MATRIX_H
#define YSL_MATH_MATRIX_H

#include <cassert>
#include <cstring>
#include <iostream>
#include "vect.h"

namespace ysl
{

class alignas(16) mat4
{
public:
	mat4()
		: data{{ {},{},{},{} }}
	{}
	mat4(const mat4 &m)
		: data(m.data)
	{}

	mat4 &operator=(const mat4 &m)
	{
		if (this != &m)
		{
			data = m.data;
		}

		return *this;
	}
	~mat4() = default;

	static mat4 identity()
	{
		mat4 m{};
		m.data[0].set_x(1);
		m.data[1].set_y(1);
		m.data[2].set_z(1);
		m.data[3].set_w(1);

		return m;
	}

	static mat4 translation()
	{
		mat4 m{};
		m.data[0].set_x(1);
		m.data[1].set_y(1);
		m.data[2].set_z(1);
		m.data[3].set_w(1);

		return m;
	}

	static mat4 translation(const vec4 &v)
	{
		mat4 m{};
		m.data[0].set_x(1);
		m.data[1].set_y(1);
		m.data[2].set_z(1);
		m.data[3] = v;
		m.data[3].set_w(1);
		return m;
	}

	static mat4 translation(f32 x, f32 y, f32 z)
	{
		mat4 m{};
		m.data[0].set_x(1);
		m.data[1].set_y(1);
		m.data[2].set_z(1);
		m.data[3] = vec4{x,y,z, 1};
		return m;
	}

	static mat4 scale(f32 x, f32 y, f32 z)
	{
		mat4 m{};
		m.data[0].set_x(x);
		m.data[1].set_y(y);
		m.data[2].set_z(z);
		m.data[3].set_w(1);

		return m;
	}

	static mat4 scale(f32 s)
	{
		mat4 m{};
		m.data[0].set_x(s);
		m.data[1].set_y(s);
		m.data[2].set_z(s);
		m.data[3].set_w(1);

		return m;
	}

	static mat4 axis_angle(const vec4 &axis, f32 angle)
	{
		// replace this with quaternions
		const f32 angle_a = 0.5f * angle;
		const f32 cos_a = ysl::cos(angle_a);
		const f32 sin_a = ysl::sin(angle_a);
		vec4 q_vec = axis.get_norm();

		const f32 qx = q_vec.x();
		const f32 qy = q_vec.y();
		const f32 qz = q_vec.z();
		const f32 qw = cos_a;

		const f32 x2 = qx + qx;
		const f32 y2 = qy + qy;
		const f32 z2 = qz + qz;

		const f32 xx = qx * x2;
		const f32 xy = qx * y2;
		const f32 xz = qx * z2;

		const f32 yy = qy * y2;
		const f32 yz = qy * z2;
		const f32 zz = qz * z2;

		const f32 wx = qw*x2;
		const f32 wy = qw * y2;
		const f32 wz = qw * z2;

		return mat4{ {1.0f - (yy+zz), xy + wz, xz - wy, 0.0f},
					 {xy - wz, 1.0f - (xx +zz), yz+wx, 0.0f}, 
					 {xz+wy, yz-wx, 1.0f - (xx+yy), 0.0f},
					 {0,0,0,1.0f}};
	}

	static mat4 rot_x(f32 f)
	{
		return mat4 { {1, 0, 0, 0},
					  { 0, ysl::cos(f), ysl::sin(f), 0},
					  { 0, -ysl::sin(f), ysl::cos(f), 0},
					  {0,0,0,1}};
	}

	static mat4 rot_y(f32 f)
	{
		return mat4{{ysl::cos(f), 0, -ysl::sin(f), 0},
					  { 0, 1, 0, 0},
					  { ysl::sin(f), 0, ysl::cos(f), 0},
					  {0,0,0,1}};
	}

	static mat4 rot_z(f32 f)
	{
		return mat4{  {ysl::cos(f), ysl::sin(f), 0, 0},
					  { -ysl::sin(f), ysl::cos(f), 0, 0},
					  { 0, 0, 1, 0},
					  {0,0,0,1}};
	}

	mat4(const vec3 &v1, const vec3 &v2, const vec3 &v3)
	{
		data[0] = v1.xyzw();
		data[1] = v2.xyzw();
		data[2] = v3.xyzw();
		data[3] = ysl::identity_vec4;
	}
	mat4(const vec<4> &v1, const vec<4> &v2, const vec<4> &v3)
	{
		data[0] = v1;
		data[1] = v2;
		data[2] = v3;
		data[3] = ysl::identity_vec4;
	}

	mat4(const vec<4> &v1, const vec<4> &v2, const vec<4> &v3, const vec<4> &v4)
	{
		data[0] = v1;
		data[1] = v2;
		data[2] = v3;
		data[3] = v4;
	}

	mat4(f32 f)
	{
		for (i32 i = 0; i < 4; ++i)
		{
			data[i].set(f);
		}
	}

	// returns ROW at idx
	vec<4> &operator[] (const size_t &idx)
	{
		assert(idx >= 0 && idx < 4);
		return data[idx];
	}

	vec<4> operator[] (const size_t &idx) const
	{
		assert(idx >= 0 && idx < 4);
		return data[idx];
	}

	mat4 operator*(const mat4 &rhs) const
	{
		mat4 m_tmp;

		for (size_t i = 0; i < 4; ++i)
		{
			for (size_t j = 0; j < 4; ++j)
			{
				m_tmp[i].set(static_cast<i32>(j), ysl::dot(data[i], rhs.col(j)));
			}
		}

		return m_tmp;
	}

	vec<4> col(const size_t &idx) const
	{
		assert(idx >= 0 && idx < 4);
		vec<4> res;
		for (i32 i = 0; i < 4; ++i)
		{
			res.set(i, data[i][idx]);
		}

		return res;
	}

	void transpose()
	{
		ysl::transpose_matrix(data[0], data[1], data[2], data[3]);
	}

	mat4 get_transpose() const
	{
		/*vec4 r1 = data[0], r2 = data[1], r3 = data[2], r4 = data[3];
		ysl::transpose_matrix(r1, r2, r3, r4);
		return mat4{r1, r2, r3, r4};*/
		return mat4{col(0), col(1), col(2), col(3)};

	}

	vec<4> get_translation4() const
	{
		// return translation vector of size 4
		return vec4{data[3]};
	}

	vec<3> get_translation3() const
	{
		return vec3{data[3][0], data[3][1], data[3][2]};
	}

	mat4 get_translation_m() const
	{
		return mat4::translation(data[3]);
	}

	mat4 get_inv_translation_m() const
	{
		return mat4::translation(data[3].flip_sign());
	}

	mat4 get_rotation_m() const
	{
		// row 0 = side vector
		// row 1 = up vector
		// row 2 = forward vector
		// row 3 = translation vector
		// normalize because we don't want scale
		return mat4(data[0].get_norm(), data[1].get_norm(), data[2].get_norm());
	}

	vec3 get_scale() const
	{
		// returns vec3(sideScale, upScale, forwardScale)
		return vec3(data[0].mag(), data[1].mag(), data[2].mag());
	}

	mat4 get_scale_m() const
	{
		return mat4::scale(data[0].mag(), data[1].mag(), data[2].mag());
	}

	mat4 get_inv_scale_m() const
	{
		return mat4::scale(1 /data[0].mag(), 1/data[0].mag(), 1/data[0].mag());
	}

	void set_translation(const vec4 &v)
	{
		data[3] = v;
	}

	void set_translation(const vec3 &v)
	{
		data[3] = v.xyzw(1);
	}

	void set_scale_x(f32 f)
	{
		data[0] *= ysl::vec4{f,1,1,1};
	}

	void set_scale_y(f32 f)
	{
		data[1] *= ysl::vec4{1,f,1,1};
	}

	void set_scale_z(f32 f)
	{
		data[2] *= ysl::vec4{1,1,f,1};
	}

	void inv()
	{
		// normal = SRT
		// inverser = iTiRiS
		const mat4 r = get_rotation_m().get_transpose();
		const mat4 s = get_inv_scale_m();
		const mat4 t = get_inv_translation_m();
		
		*this = t * r * s;

	}

	mat4 get_inv() const
	{
		const mat4 r = get_rotation_m().get_transpose();
		const mat4 s = get_inv_scale_m();
		const mat4 t = get_inv_translation_m();

		mat4 m{};
		m = t * r * s;

		return m;
	}

	std::array<f32, 16> get_as_float() const
	{
		std::array<f32, 16> res;
		auto x = data[0].as_floats();
		auto y = data[1].as_floats();
		auto z = data[2].as_floats();
		auto w = data[3].as_floats();
		memcpy(&res[0],  &x, 4 * sizeof(f32));
		memcpy(&res[4],  &y, 4 * sizeof(f32));
		memcpy(&res[8],  &z, 4 * sizeof(f32));
		memcpy(&res[12], &w, 4 * sizeof(f32));

		return res;
	}

	void print() const
	{
		for (size_t i = 0; i < 4; ++i)
		{
			data[i].print();
		}
	}

	friend vec<4> operator*(const vec<4> &lhs, const mat4 &rhs);
	//friend vec<4> operator*=(const vec<4> &lhs, const mat4 &rhs);

private:
	std::array<vec4, 4> data;
};

inline vec<4> operator *(const vec<4> &lhs, const mat4 &rhs)
{
	vec<4> res;
	for (i32 i = 0; i < 4; ++i)
	{
		res.set(i, dot(lhs, rhs.col(i)));
	}

	return res;
}

inline vec<3> operator *(const vec<3> &lhs, const mat4 &rhs)
{
	vec<4> res;
	vec<4> lhs2 = lhs.xyzw();
	for (i32 i = 0; i < 4; ++i)
	{
		res.set(i, dot(lhs2, rhs.col(i)));
	}
	res.set_w(0);
	return res.xyz();
}

inline vec<3> operator *=(vec<3> &lhs, const mat4 &rhs)
{
	vec<4> res;
	vec<4> lhs2 = lhs.xyzw();
	for (i32 i = 0; i < 4; ++i)
	{
		res.set(i, dot(lhs2, rhs.col(i)));
	}
	res.set_w(0);
	lhs = res.xyz();
	return lhs;
}

} // namespace

#endif //YSL_MATH__MAT_H_

/*
//------------------------------------------------------------------------------------------------------
//                                  Base Matrix
//------------------------------------------------------------------------------------------------------
template<size_t nrows, size_t ncols>
struct mat;

//------------------------------------------------------------------------------------------------------
//                                  Operator Overloading
//------------------------------------------------------------------------------------------------------

template<size_t nrows, size_t ncols>
constexpr vec<ncols> operator *(const vec<nrows> &lhs, const mat<nrows, ncols> &rhs)
{
	vec<ncols> res;
	for (size_t i = 0; i < nrows; ++i)
	{
		res[i] = dot(lhs, rhs[i]);
	}

	return res;

}


//------------------------------------------------------------------------------------------------------
//                                  Custom Mat Types
//------------------------------------------------------------------------------------------------------
template<size_t nrows, size_t ncols>
struct mat
{
	mat() = default;
	mat(const mat &) = default;
	mat &operator=(const mat &) = default;
	~mat() = default;

	mat(const vec3 &v1, const vec3 &v2, const vec3 &v3) requires (nrows >= 3 && ncols >= 3)
	{
		v_row[0] = v1;
		v_row[1] = v2;
		v_row[2] = v3;
	}
	/*mat(const vec<4> &v1, const vec<4> &v2, const vec<4> &v3) requires (nrows == 3 && ncols == 3)
	{
		v_row[0] = v1.xyz();
		v_row[1] = v2.xyz();
		v_row[2] = v3.xyz();
	}
mat(const vec<ncols> &v1, const vec<ncols> &v2, const vec<ncols> &v3, const vec<ncols> &v4) requires (nrows == 4 && ncols == 4)
{
	v_row[0] = v1;
	v_row[1] = v2;
	v_row[2] = v3;
	v_row[3] = v4;
}
mat(f32 f)
{
	for (size_t i = 0; i < nrows; i++)
	{
		v_row[i] = vec<ncols>{f};
	}
}

// returns ROW at idx
vec<ncols> &operator[] (const size_t &idx)
{
	assert(idx >= 0 && idx < nrows);
	return v_row[idx];
}

vec<ncols> operator[] (const size_t &idx) const
{
	assert(idx >= 0 && idx < nrows);
	return v_row[idx];
}

constexpr mat<nrows, ncols> operator*(const mat<nrows, ncols> &rhs) const
{
	mat m_tmp;

	for (size_t i = 0; i < nrows; ++i)
	{
		for (size_t j = 0; j < ncols; ++j)
		{
			m_tmp[i].set(static_cast<int32_t>(j), ysl::dot(v_row[i], rhs.col(j)));
		}
	}

	return m_tmp;
}

static mat<nrows, ncols> identity()
{
	mat<nrows, ncols> m;
	for (size_t i = 0; i < nrows; ++i)
	{
		for (size_t j = 0; j < ncols; ++i)
		{
			m[i].set(j, (i == j));
		}
	}

	return m;
}


/*void transpose() requires (nrows == 4, ncols == 4)
{
	vec<4> r1;
	vec<4> r2;
	vec<4> r3;
	vec<4> r4;
	ysl::transpose_matrix(v_row[0], v_row[1], v_row[2], v_row[3]);
}*/

/*void transpose() const requires (nrows == 3, ncols == 3)
{

}

vec<nrows> col(const size_t &idx) const
{
	assert(idx >= 0 && idx < ncols);
	vec<nrows> res;
	for (int32_t i = 0; i < nrows; ++i)
	{
		res.set(i, v_row[i][idx]);
	}

	return res;
}

vec<4> get_translation4() const requires (ncols >= 3 && nrows == 4)
{
	// return translation vector of size 4
	return vec4{v_row[3]};
}

vec<3> get_translation3() const requires (ncols >= 3 && nrows == 4)
{
	return vec3{v_row[3][0], v_row[3][1], v_row[3][2]};
}

mat<3, 3> get_rotation() const requires (ncols >= 3 && nrows >= 3)
{
	// row 0 = side vector
	// row 1 = up vector
	// row 2 = forward vector
	// row 3 = translation vector
	// normalize because we don't want scale
	return mat<3, 3>(v_row[0].get_norm(), v_row[1].get_norm(), v_row[2].get_norm());
}

vec3 get_scale() const requires (ncols >= 3 && nrows >= 3)
{
	// returns vec3(sideScale, upScale, forwardScale)
	return vec3(v_row[0].mag(), v_row[1].mag(), v_row[2].mag());
}

constexpr void print() const
{
	for (size_t i = 0; i < nrows; ++i)
	{
		//printf("row %lu: ", i); 
		v_row[i].print();
	}
}

private:
	vec<ncols> v_row[nrows]; // returns the ROW at index "nrows"
};

//------------------------------------------------------------------------------------------------------
//                                  Static functions
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
//                                  typedef and defaults
//------------------------------------------------------------------------------------------------------

typedef mat<3, 3> mat3;
typedef mat<4, 4> mat4;
typedef mat<3, 4> mat34; */
