#ifndef RT_HIT_RESULT_H
#define RT_HIT_RESULT_H

#include "ysl_math/vect.h"
#include <memory>
#include "ray.h"

struct hit_result
{
	ysl::point3 p{};
	ysl::vec3 normal{};
	ysl::vec2 uv{};
	i32 mat_idx = 0;
	f32 t = 0.0f;
	bool front_face = false;
};

struct scatter_result
{
	ray out_ray{};
	ysl::point3 attenuation{};
	bool is_specular = false;
};

inline void set_face_normal(const ray &r, const ysl::vec3 &outward_normal, bool &front_face_out, ysl::vec3 &norm_out)
{
	front_face_out = ysl::dot(r.direction(), outward_normal) < 0.0f;
	norm_out = front_face_out ? outward_normal : -outward_normal;
}

#endif