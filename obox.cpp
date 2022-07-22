#include "renderable.h"
#include "collision.h"

obox::obox()
	: m_matrix{}, m_inv_mat{}, m_box_min{}, m_box_max{}, m_mat(0)
{}

obox::obox(const obox &o)
	: m_matrix(o.m_matrix), m_inv_mat(o.m_inv_mat), m_box_min(o.m_box_min), m_box_max(o.m_box_max), m_mat(o.m_mat)
{}

obox &obox::operator=(const obox &o)
{
	if (this != &o)
	{
		//m_sides = o.m_sides;
		m_matrix = o.m_matrix;
		m_inv_mat = o.m_inv_mat;
		m_box_min = o.m_box_min;
		m_box_max = o.m_box_max;
		m_mat = o.m_mat;
	}

	return *this;
}

obox::obox(i32 mat_idx, const ysl::mat4 &world_matrix, const ysl::point3 &p0, const ysl::point3 &p1)
	: m_matrix(world_matrix), m_inv_mat(world_matrix.get_inv()), m_box_min(p0), m_box_max(p1), m_mat(mat_idx)
{
	/*const f32 radians = ysl::util::degrees_to_radians(-12.0f);
	const f32 cos_theta = ysl::cos(radians);
	const f32 sin_theta = ysl::sin(radians);

	ysl::point3 min{ysl::infinity}, max{-ysl::infinity};

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				auto x = i * m_box_max.x() + (1 - i) * m_box_min.x();
				auto y = j * m_box_max.y() + (1 - j) * m_box_min.y();
				auto z = k * m_box_max.z() + (1 - k) * m_box_min.z();

				auto newx = cos_theta * x + sin_theta * z;
				auto newz = -sin_theta * x + cos_theta * z;

				ysl::vec3 tester(newx, y, newz);

				for (int c = 0; c < 3; c++)
				{
					min.set(c, fmin(min[c], tester[c]));
					max.set(c, fmax(max[c], tester[c]));
				}
			}
		}
	}

	m_box_min = min;
	m_box_max = max;*/
}

bool obox::hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const
{
	// ray transformation
	const ysl::point3 r_origin = (r_in.origin().xyzw(1) * m_inv_mat).xyz();
	const ysl::point3 r_dir = (r_in.direction() * m_inv_mat);

	const ray new_r{r_origin, r_dir};

	const bool result = intersect::ray_aabb(new_r, m_box_min, m_box_max, t_min, t_max, hit_out);
	if (!result) return false;
	ysl::vec4 p = hit_out.p.xyzw(1);
	hit_out.p = (p * m_matrix).xyz(); // transform point back to world

	/*const auto time_test = (hit_out.p - r_in.origin()) / r_in.direction();
	const auto t_x = r_in.at(time_test.x());
	const auto t_y = r_in.at(time_test.y());
	const auto t_z = r_in.at(time_test.z());*/
	ysl::mat4 nm = m_inv_mat;
	//nm.set_translation( {0,0,0,1}); // avoid translating the normals

	const auto norm = hit_out.normal * nm.get_transpose(); //global_normal

	hit_out.normal = norm.get_norm();
	hit_out.mat_idx = m_mat;

	return true;
}

bool obox::bounding_box(aabb &box_out) const
{
	box_out = aabb(m_box_min, m_box_max);
	return true;
}