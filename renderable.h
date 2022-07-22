#ifndef RT_RENDERABLE_H
#define RT_RENDERABLE_H

#include <memory>
#include <vector>

#include "ysl_math/vect.h"
#include "ysl_math/matrix.h"
#include "ray.h"
#include "hit_result.h"

class render_world;
struct aabb;

class renderable
{
public:
	renderable() : flip_face(false){}
	renderable(const renderable &) = default;
	renderable &operator=(const renderable &) = default;
	virtual ~renderable() = default;

	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const = 0;
	virtual bool bounding_box(aabb &box_out) const
	{
		(void)box_out;
		return false;
	}

	virtual f32 pdf_value(const ysl::point3 &origin, const ysl::vec3 &v) const
	{ 
		return 0.0f;
	}

	virtual ysl::vec3 random_direction(const ysl::point3 &origin) const
	{ 
		return ysl::vec3{1,0,0}; 
	}

	void flip_render_face(const bool &b = true)
	{
		flip_face = b;
	}

protected:
	bool flip_face;
};

struct sphere : renderable
{
	sphere();
	sphere(const sphere &);
	sphere &operator=(const sphere &);
	~sphere() = default;

	sphere(const ysl::point3 &c, f32 r, i32 mat_idx);

	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const override;
	virtual bool bounding_box(aabb &box_out) const override;

	virtual f32 pdf_value(const ysl::point3 &origin, const ysl::vec3 &v) const override;
	virtual ysl::vec3 random_direction(const ysl::point3 &origin) const override;

private:
	static void get_sphere_uv(const ysl::point3 &p, ysl::vec2 &uv_out);


	ysl::point3 m_center;
	f32 m_radius;
	i32 m_mat;
};

struct aabb : renderable
{
	aabb();
	aabb(const aabb &);
	aabb &operator=(const aabb &);
	~aabb() = default;

	aabb(const ysl::point3 &a, const ysl::point3 &b);

	static aabb surrounding_box(const aabb &box1, const aabb &box2);

	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const override;	

	inline ysl::point3 min() const {return m_bounds[0]; }
	inline ysl::point3 max() const {return m_bounds[1]; }

private:
	std::array<ysl::point3, 2> m_bounds;

};

struct bvh_node : renderable
{
	bvh_node();
	bvh_node(const bvh_node &);
	bvh_node &operator=(const bvh_node &);
	~bvh_node();

	bvh_node(const std::shared_ptr<render_world> &world, f32 t0, f32 t1);
	bvh_node(const std::vector<std::shared_ptr<renderable>> &src_objects, const size_t &start, const size_t &end, f32 t0, f32 t1);

	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const override;
	virtual bool bounding_box(aabb &box_out) const override;

private:
	
	bool box_compare(const std::shared_ptr<renderable> &a, const std::shared_ptr<renderable> &b, i32 axis);
private:
	aabb m_bbox;
	std::shared_ptr<renderable> m_left;
	std::shared_ptr<renderable> m_right;
};

struct rect : renderable
{
	enum class rect_type : u32
	{
		yz = 0,
		xz = 1,
		xy = 2,
		none
	};

	rect();
	rect(const rect &);
	rect &operator=(const rect &);
	~rect() = default;

	rect(rect_type rt, i32 mat_idx, const ysl::vec2 &side1, const ysl::vec2 &side2, f32 k);

	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const override;
	virtual bool bounding_box(aabb &box_out) const override;
	virtual f32 pdf_value(const ysl::point3 &origin, const ysl::vec3 &v) const override;
	virtual ysl::vec3 random_direction(const ysl::point3 &origin) const override;

private:
	ysl::vec4 m_corners;
	f32 m_k;
	i32 m_mat;
	u32 m_type_idx;
};

struct obox : renderable
{
	obox();
	obox(const obox &);
	obox &operator=(const obox &);
	~obox() = default;

	//obox(i32 mat_idx, const ysl::point3 &p0, const ysl::point3 &p1);
	obox(i32 mat_idx, const ysl::mat4 &matrix, const ysl::point3 &p0, const ysl::point3 &p1);
	virtual bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const override;
	virtual bool bounding_box(aabb &box_out) const override;

private:
	//std::array<rect, 6> m_sides;
	ysl::mat4 m_matrix;
	ysl::mat4 m_inv_mat; // testing
	ysl::point3 m_box_min;
	ysl::point3 m_box_max;
	i32 m_mat;
};



#endif