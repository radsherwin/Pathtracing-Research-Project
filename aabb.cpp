#include "renderable.h"


aabb::aabb()
	: m_bounds{}
{}

aabb::aabb(const aabb &b)
	: m_bounds{b.m_bounds}
{}

aabb &aabb::operator=(const aabb &b)
{
	if (this != &b)
	{
		m_bounds = b.m_bounds;
	}

	return *this;
}

aabb::aabb(const ysl::point3 &a, const ysl::point3 &b)
	: m_bounds{a,b}
{}

aabb aabb::surrounding_box(const aabb &box0, const aabb &box1)
{
	const ysl::point3 b0_min = box0.min();
	const ysl::point3 b1_min = box1.min();
	
	const ysl::point3 small{ysl::min(b0_min[0], b1_min[0]),
							ysl::min(b0_min[1], b1_min[1]),
							ysl::min(b0_min[2], b1_min[2])};

	const ysl::point3 b0_max = box0.max();
	const ysl::point3 b1_max = box1.max();

	const ysl::point3 big{ ysl::max(b0_max[0], b1_max[0]),
						   ysl::max(b0_max[1], b1_max[1]),
						   ysl::max(b0_max[2], b1_max[2])};

	return aabb(small, big);
}

// AABB-Ray intersection from: https://www.researchgate.net/publication/220494140_An_Efficient_and_Robust_Ray-Box_Intersection_Algorithm
bool aabb::hit(const ray &r_in, f32 t0, f32 t1, hit_result &hit_out) const
{
	const ysl::point3 orig = r_in.origin();
	const ysl::vec3 inv_dir = r_in.inv_direction();
	const std::array<i32, 3> sign = r_in.signs();

	const f32 orig_x = orig.x();
	const f32 inv_dir_x = inv_dir.x();

	// testing
	/*const ysl::vec3 vmin = (m_bounds[0] - orig) * inv_dir;
	const ysl::vec3 vmax = (m_bounds[1] - orig) * inv_dir;*/

	f32 tmin = (m_bounds[sign[0]].x() - orig_x) * inv_dir_x;
	f32 tmax = (m_bounds[1 - sign[0]].x() - orig_x) * inv_dir_x;

	const f32 orig_y = orig.y();
	const f32 inv_dir_y = inv_dir.y();
	f32 tymin = (m_bounds[sign[1]].y() - orig_y) * inv_dir_y;
	f32 tymax = (m_bounds[1 - sign[1]].y() - orig_y) * inv_dir_y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;

	tmin = std::max(tymin, tmin);
	tmax = std::min(tymax, tmax);

	const f32 orig_z = orig.z();
	const f32 inv_dir_z = inv_dir.z();
	f32 tzmin = (m_bounds[sign[2]].z() - orig_z) * inv_dir_z;
	f32 tzmax = (m_bounds[1 - sign[2]].z() - orig_z) * inv_dir_z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	tmin = std::max(tzmin, tmin);
	tmax = std::min(tzmax, tmax);

	return ((tmin < t1) && (tmax > t0));
}

//bool aabb::hit(const ray &r_in, f32 t_min, f32 t_max) const
//{
//	const ysl::point3 orig = r_in.origin();
//	const ysl::vec3 inv_dir = r_in.inv_direction();
//	const ysl::vec3 vmin = (m_bounds[0] - orig) * inv_dir;
//	const ysl::vec3 vmax = (m_bounds[1] - orig) * inv_dir;
//
//	f32 tmin1 = std::max(vmin[1], std::max(vmin[2], vmin[0]));
//	f32 tmax1 = std::min(vmax[1], std::min(vmin[2],vmax[0]));
//
//	bool result = ((tmin1 < t1)) && (tmax1 > t0));
//}