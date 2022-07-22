#ifndef YSL_RT_COLLISION_H
#define YSL_RT_COLLISION_H

#include "renderable.h"

namespace intersect
{

inline bool ray_aabb(const ray &r_in, const ysl::point3 &p_min, const ysl::point3 &p_max, f32 t0, f32 t1, hit_result &hit_out)
{
	const ysl::point3 orig = r_in.origin();
	const ysl::vec3 inv_dir = r_in.inv_direction();
	const std::array<i32, 3> sign = r_in.signs();

	const f32 orig_x = orig.x();
	const f32 inv_dir_x = inv_dir.x();

	const ysl::point3 m_bounds[2] = {p_min, p_max};

	f32 tmin = (m_bounds[sign[0]].x() - orig_x) * inv_dir_x;
	f32 tmax = (m_bounds[1 - sign[0]].x() - orig_x) * inv_dir_x;

	const f32 orig_y = orig.y();
	const f32 inv_dir_y = inv_dir.y();
	const f32 tymin = (m_bounds[sign[1]].y() - orig_y) * inv_dir_y;
	const f32 tymax = (m_bounds[1 - sign[1]].y() - orig_y) * inv_dir_y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;

	tmin = std::max(tymin, tmin);
	tmax = std::min(tymax, tmax);

	const f32 orig_z = orig.z();
	const f32 inv_dir_z = inv_dir.z();
	const f32 tzmin = (m_bounds[sign[2]].z() - orig_z) * inv_dir_z;
	const f32 tzmax = (m_bounds[1 - sign[2]].z() - orig_z) * inv_dir_z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	tmin = std::max(tzmin, tmin);
	tmax = std::min(tzmax, tmax);

	//return ((tmin < t1) && (tmax > t0));

	if(!((tmin < t1) && (tmax > t0))) return false;

	//return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;

	const ysl::vec3 r_at = r_in.at(tmin);
	hit_out.p = r_at;
	ysl::vec3 normal{};
	
	f32 t_dist[6];
	t_dist[0] = ysl::abs(r_at.x() - p_min.x());
	t_dist[1] = ysl::abs(r_at.y() - p_min.y());
	t_dist[2] = ysl::abs(r_at.z() - p_min.z());
	t_dist[3] = ysl::abs(r_at.x() - p_max.x());
	t_dist[4] = ysl::abs(r_at.y() - p_max.y());
	t_dist[5] = ysl::abs(r_at.z() - p_max.z());

	i32 best = 0;
	f32 b_dist = t_dist[0];
	for (i32 i = 1; i < 6; ++i) if (t_dist[i] < b_dist)
	{
		b_dist = t_dist[i];
		best = i;
	}
	if (best == 0) normal.set_x(-1); //hit_out.front_face = false;
	if (best == 1) normal.set_y(-1); //hit_out.front_face = false;
	if (best == 2) normal.set_z(-1); //hit_out.front_face = false;
	if (best == 3) normal.set_x(1);  //hit_out.front_face = true;
	if (best == 4) normal.set_y(1);  //hit_out.front_face = true;
	if (best == 5) normal.set_z(1);  //hit_out.front_face = true;

	hit_out.normal = normal;
	//hit_out.set_face_normal(r_in, normal);
	hit_out.front_face = ysl::dot(r_in.direction(), normal) < 0.0f;
	hit_out.t = tmin;

	return true;	
}

} // namespace intersect

#endif