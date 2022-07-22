#include "renderable.h"

rect::rect()
	: m_corners{}, m_k(0.0f), m_mat(0), m_type_idx(static_cast<u32>(rect_type::none))
{}

rect::rect(const rect &o)
	: m_corners{o.m_corners}, m_k(o.m_k), m_mat(o.m_mat), m_type_idx(o.m_type_idx)
{}

rect &rect::operator=(const rect &o)
{
	if (this != &o)
	{
		m_corners = o.m_corners;
		m_k = o.m_k;
		m_mat = o.m_mat;
		m_type_idx = o.m_type_idx;
	}

	return *this;
}

// Move the static cast to construction to save runtime performance
rect::rect(rect_type rt, i32 mat_idx, const ysl::vec2 &side1, const ysl::vec2 &side2, f32 thickness)
	: m_corners{side1, side2}, m_k(thickness), m_mat(mat_idx), m_type_idx(static_cast<u32>(rt))
{}

bool rect::hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const
{
	const u8 s3_idx = m_type_idx;
	const ysl::vec3 orig = r_in.origin(); // r_in.origin() * transform_matrix
	const ysl::vec3 dir = r_in.direction(); // r_in.direction() * transform_matrix

	const f32 t = (m_k - orig[s3_idx]) / dir[s3_idx];

	if (t < t_min || t> t_max)
	{
		return false;
	}

	//const u8 s2_idx = (type_idx + 2) % 3;
	//const u8 s1_idx = (type_idx + 1) % 3;

	const ysl::vec3 r_at = r_in.at(t);
	hit_out.p = r_at;

	const auto s1 = m_corners.xy();
	const auto s2 = m_corners.zw();

	const u8 s1_idx = m_type_idx == 0 ? 1 : 0;
	const u8 s2_idx = m_type_idx == 2 ? 1 : 2;

	// if hit location is not within min and max bounds of rectangle
	if (r_at[s1_idx] < s1[0] || r_at[s1_idx] > s1[1] || r_at[s2_idx] < s2[0] || r_at[s2_idx] > s2[1])
	{
		return false;
	}

	hit_out.uv.set_x((r_at[s1_idx] - s1[0]) / (s1[1] - s1[0]));
	hit_out.uv.set_y((r_at[s2_idx] - s2[0]) / (s2[1] - s2[0]));
	hit_out.t = t;
	ysl::vec3 normal{};
	normal.set(s3_idx, 1);
	set_face_normal(r_in, normal, hit_out.front_face, hit_out.normal);
	
	if(flip_face) hit_out.front_face = !hit_out.front_face; // flipping face for lights

	hit_out.mat_idx = m_mat;

	return true;
}

bool rect::bounding_box(aabb &box_out) const
{
	const u8 s3_idx = m_type_idx;
	/*const u8 s2_idx = (m_type_idx + 2) % 3;
	const u8 s1_idx = (m_type_idx + 1) % 3;*/
	const u8 s1_idx = m_type_idx == 0 ? 1 : 0;
	const u8 s2_idx = m_type_idx == 2 ? 1 : 2;
	constexpr f32 pad = 0.0001f;
	ysl::point3 small{};
	const auto s1 = m_corners.xy();
	const auto s2 = m_corners.zw();
	small.set(s1_idx, s1[0]);
	small.set(s2_idx, s2[0]);
	small.set(s3_idx, m_k - pad);

	ysl::point3 big{};
	big.set(s1_idx, s1[1]);
	big.set(s2_idx, s2[1]);
	big.set(s3_idx, m_k + pad);

	box_out = aabb(small, big);
	return true;
}

f32 rect::pdf_value(const ysl::point3 &origin, const ysl::vec3 &v) const
{
	hit_result hit_temp;
	if (!hit(ray(origin, v), 0.001f, ysl::infinity, hit_temp))
	{
		return 0;
	}

	const ysl::vec2 s1 = m_corners.xy();
	const ysl::vec2 s2 = m_corners.zw();

	const auto area = (s1.y() - s1.x()) * (s2.y() - s2.x());
	const auto dist_squared = hit_temp.t * hit_temp.t * v.mag_squared();
	const auto cosine = ysl::abs(ysl::dot(v, hit_temp.normal) / v.mag());

	return dist_squared / (cosine * area);

}
ysl::vec3 rect::random_direction(const ysl::point3 &origin) const
{
	const auto s1 = m_corners.xy();
	const auto s2 = m_corners.zw();
	const auto random_point = ysl::point3{ysl::util::random_f32(s1[0], s1[1]), m_k, ysl::util::random_f32(s2[0], s2[1])};
	return random_point - origin;
}