#ifndef RT_RENDER_WORLD_H
#define RT_RENDER_WORLD_H

#include "renderable.h"

#include <vector>
#include <memory>

class render_world
{
public:
	render_world() : m_objects{} {}
	render_world(const render_world &r) : m_objects(r.m_objects) {}
	render_world &operator=(const render_world &r)
	{
		if (this != &r)
		{
			m_objects = r.m_objects;
		}

		return *this;
	}
	~render_world()
	{
		m_objects.clear();
	}

	inline render_world(const std::shared_ptr<renderable> &object) { m_objects.emplace_back(object); }

	inline void add(const std::shared_ptr<renderable> &object) { m_objects.emplace_back(object); }

	inline void clear() { m_objects.clear(); }

	inline auto objects() const { return m_objects; }

	inline size_t size() const { return m_objects.size(); }

	inline bool hit(const ray &r_in, f32 t_min, f32 t_max, hit_result &hit_out) const
	{
		hit_result temp_hit;
		bool hit_anything = false;
		f32 closest_so_far = t_max;

		for (const auto &object : m_objects)
		{
			if (object->hit(r_in, t_min, closest_so_far, temp_hit))
			{
				hit_anything = true;
				closest_so_far = temp_hit.t;
				hit_out = temp_hit;
			}
		}

		return hit_anything;
	}

private:
	std::vector<std::shared_ptr<renderable>> m_objects;
};

#endif
