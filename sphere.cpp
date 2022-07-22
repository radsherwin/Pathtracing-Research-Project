#include "renderable.h"
#include "ysl_math/common.h"

sphere::sphere()
	: m_center{}, m_radius(0.0f), m_mat(0)
{}

sphere::sphere(const sphere &s)
	: m_center(s.m_center), m_radius(s.m_radius), m_mat(s.m_mat)
{}

sphere &sphere::operator=(const sphere &s)
{
	if (this != &s)
	{
		m_center = s.m_center;
		m_radius = s.m_radius;
		m_mat = s.m_mat;
	}

	return *this;
}

sphere::sphere(const ysl::point3 &c, f32 r, i32 mat_idx)
	: m_center(c), m_radius(r), m_mat(mat_idx)
{}

bool sphere::hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const
{
	const ysl::vec3 oc = r_in.origin() - m_center;
	const ysl::vec3 r_dir = r_in.direction();
	const f32 a = r_dir.mag_squared();
	const f32 half_b = ysl::dot(oc, r_dir);
	const f32 c = oc.mag_squared() - m_radius * m_radius;

	const f32 discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;

	const f32 sqrtd = ysl::sqrt(discriminant);

	// find the nearest root that lies in the acceptable range
	f32 root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root)
	{
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
		{
			return false;
		}
	}

	hit_out.t = root;
	hit_out.p = r_in.at(hit_out.t);
	ysl::vec3 outward_normal = (hit_out.p - m_center) / m_radius;
	set_face_normal(r_in, outward_normal, hit_out.front_face, hit_out.normal);
	get_sphere_uv(outward_normal, hit_out.uv); // get UV coords
	hit_out.mat_idx = m_mat;

	return true;
}

bool sphere::bounding_box(aabb &box_out) const
{
	box_out = aabb(m_center - ysl::vec3{m_radius},
					   m_center + ysl::vec3{m_radius});
	return true;
}

void sphere::get_sphere_uv(const ysl::point3 &p, ysl::vec2 &uv_out)
{
	// Spherical coordinates
	// p: a given point on the sphere of radius one, centered at the origin
	// u: returned value [0,1] of angle around the Y axis from X=-1
	// v: returned value [0,1] o fangle from Y=-1 to Y=+1
	//      <1 0 0> yields <0.50 0.50>      <-1  0  0> yields <0.00, 0.50>
	//      <0 1 0> yields <0.50 1.00>      < 0 -1  0> yields <0.50, 0.00>
	//      <0 0 1> yields <0.25 0.50>      < 0  0 -1> yields <0.75, 0.50>

	const f32 theta = ysl::acos(-p.y());
	const f32 phi = ysl::atan2(-p.z(), p.x()) + ysl::pi;

	uv_out.set_x(phi / (2 * ysl::pi));
	uv_out.set_y(theta / ysl::pi);
}

f32 sphere::pdf_value(const ysl::point3 &origin, const ysl::vec3 &v) const
{
	hit_result hit_out;
	if (!hit(ray(origin, v), 0.001f, ysl::infinity, hit_out))
	{
		return 0.0f;
	}

	const f32 cos_theta_max = ysl::sqrt(1 - m_radius*m_radius / (m_center-origin).mag_squared());
	const f32 solid_angle = 2 * ysl::pi * (1 - cos_theta_max);

	return 1 / solid_angle;
}
ysl::vec3 sphere::random_direction(const ysl::point3 &origin) const
{
	const ysl::vec3 direction = m_center - origin;
	const f32 dist_squared = direction.mag_squared();
	return ysl::construct_onb(direction, ysl::random_to_sphere(m_radius, dist_squared));
}