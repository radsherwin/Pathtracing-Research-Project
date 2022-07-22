#ifndef YSL_RT_MATH_COMMON_H
#define YSL_RT_MATH_COMMON_H

#include "vect.h"

namespace ysl
{
inline vec<3> random_in_unit_sphere()
{
	while (true)
	{
		auto p = ysl::vec<3>::random(-1.0f, 1.0f);
		if (p.mag_squared() >= 1) continue;

		return p;
	}
}

// correct rendering of lambertian spheres
inline vec<3> random_unit_vector()
{
	return random_in_unit_sphere().get_norm();
}

// an alternative diffuse formulation
// hemispherical scattering
inline vec<3> random_in_hemisphere(const vec<3> &normal)
{
	vec<3> in_unit_sphere = random_in_unit_sphere();
	if (dot(in_unit_sphere, normal) > 0.0f)
	{
		return in_unit_sphere;
	}

	return -in_unit_sphere;
}

// for defocus blur. The disk around the look_from on camera
inline vec<3> random_in_unit_disk()
{
	while (true)
	{
		auto p = vec<3>(util::random_f32(-1, 1), util::random_f32(-1, 1), 0);
		if (p.mag_squared() >= 1) continue;
		return p;
	}
}

// generate random vectors
inline vec3 random_cosine_direction(const ysl::vec3 &n)
{
	const f32 x = ysl::util::random_f32();
	const f32 y = ysl::util::random_f32();

	ysl::vec3 uu = ysl::cross(n, ysl::vec3(0,1,1)).get_norm();
	ysl::vec3 vv = ysl::cross(uu, n);

	f32 ra = ysl::sqrt(y);
	f32 rx = ra*ysl::cos(6.2831f*x);
	f32 ry = ra*ysl::sin(6.2831f*x);

	f32 rz = ysl::sqrt(1.0f-y);
	ysl::vec3 rr = ysl::vec3(rx*uu + ry*vv + rz*n);

	return rr.get_norm();
}

inline ysl::vec3 construct_onb(const ysl::vec3 &vec, const ysl::vec3 &dir)
{
	ysl::vec3 axis[3];
	axis[2] = vec.get_norm();
	const ysl::vec3 a = (ysl::abs(axis[2].x()) > 0.9f) ? ysl::vec3{0,1,0} : ysl::vec3{1,0,0};
	axis[1] = ysl::cross(axis[2], a).get_norm();
	axis[0] = ysl::cross(axis[2], axis[1]);

	return (dir.x() * axis[0] + dir.y() * axis[1] + dir.z() * axis[2]);
}

inline ysl::vec3 random_to_sphere(f32 radius, f32 dist_sqrd)
{
	const f32 r1 = ysl::util::random_f32();
	const f32 r2 = ysl::util::random_f32();
	const f32 z = 1 + r2*(ysl::sqrt(1-radius*radius/dist_sqrd) - 1);

	const f32 phi = 2 * ysl::pi * r1;
	const f32 x = ysl::cos(phi) * ysl::sqrt(1-z*z);
	const f32 y = ysl::sin(phi) * ysl::sqrt(1-z*z);

	return ysl::vec3{x,y,z};
}

}

#endif