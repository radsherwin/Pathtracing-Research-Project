#ifndef RT_CAMERA_H
#define RT_CAMERA_H

#include "ray.h"

class camera
{
public:
    camera();
    camera(const camera &) = default;
    camera &operator=(const camera &) = default;
    ~camera() = default;

    camera(const ysl::point3 &look_from, const ysl::point3 &look_at, const ysl::vec3 &v_up_world,
           f32 vertical_fov, f32 aspect_ratio,
           f32 aperture, f32 focus_dist);

    ray get_ray(f32 u, f32 v) const;

private:
    ysl::point3 origin;
    ysl::point3 lower_left_corner;
    ysl::vec3 horizontal;
    ysl::vec3 vertical;
    ysl::vec3 v_fwd;
    ysl::vec3 v_side;
    ysl::vec3 v_up;
    f32 lens_radius;
    f32 pad;

};

#endif