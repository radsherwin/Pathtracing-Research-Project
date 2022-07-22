#include "renderable.h"
#include "render_world.h"

bvh_node::bvh_node()
	: m_bbox(), m_left(nullptr), m_right(nullptr)
{}

bvh_node::bvh_node(const bvh_node &b)
	: m_bbox(b.m_bbox), m_left(b.m_left), m_right(b.m_right)
{}

bvh_node &bvh_node::operator=(const bvh_node &b)
{
	if (this != &b)
	{
		m_bbox = b.m_bbox;
		m_left = b.m_left;
		m_right = b.m_right;
	}

	return *this;
}

bvh_node::~bvh_node()
{
	m_left = nullptr;
	m_right = nullptr;
}

bvh_node::bvh_node(const std::shared_ptr<render_world> &world, f32 t0, f32 t1)
	: bvh_node(world->objects(), 0, world->size(), t0, t1)
{}

bvh_node::bvh_node(const std::vector<std::shared_ptr<renderable>> &src_objects,
				   const size_t &start, const size_t &end,
				   f32 t0, f32 t1)
{
	const i32 axis = ysl::util::random_i32(0, 2);

	auto comp = [this, &axis](const std::shared_ptr<renderable> &ab, const std::shared_ptr<renderable> &bb)
	{
		return box_compare(ab, bb, axis);
	};

	std::vector<std::shared_ptr<renderable>> objects = src_objects;
	const size_t obj_span = end - start;
	if (obj_span == 1)
	{
		m_left = m_right = objects[start];
	}
	else if (obj_span == 2)
	{
		if (comp(objects[start], objects[start + 1]))
		{
			m_left = objects[start];
			m_right = objects[start + 1];
		}
		else
		{
			m_left = objects[start + 1];
			m_right = objects[start];
		}
	}
	else
	{
		std::sort(objects.begin() + start, objects.begin() + end, comp);
		const size_t mid = start + obj_span / 2;
		m_left = std::make_shared<bvh_node>(objects, start, mid, t0, t1);
		m_right = std::make_shared<bvh_node>(objects, mid, end, t1, t1);
	}

	aabb box_left;
	aabb box_right;
	if (!m_left->bounding_box(box_left) || !m_right->bounding_box(box_right))
	{
		std::cerr << "111 No bounding box in bvh_node constructor.\n";
	}

	m_bbox = aabb::surrounding_box(box_left, box_right);
}

bool bvh_node::box_compare(const std::shared_ptr<renderable> &a, const std::shared_ptr<renderable> &b, i32 axis)
{
	aabb box_a;
	aabb box_b;
	if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
	{
		std::cerr << "222 No bounding box in bvh_node constructor.\n";
	}

	return box_a.min()[axis] < box_b.min()[axis];
}

bool bvh_node::hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const
{
	if (!m_bbox.hit(r_in, t_min, t_max, hit_out))
		return false;

	const bool hit_left = m_left->hit(r_in, t_min, t_max, hit_out);
	const bool hit_right = m_right->hit(r_in, t_min, hit_left ? hit_out.t : t_max, hit_out);

	return hit_left || hit_right;
}

bool bvh_node::bounding_box(aabb &box_out) const
{
	box_out = m_bbox;
	return true;
}