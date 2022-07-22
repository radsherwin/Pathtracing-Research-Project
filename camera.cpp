#include "camera.h"
#include "ysl_math/common.h"

camera::camera()
{
    const f32 aspect_ratio = 16.0f / 9.0f;
    const f32 viewport_height = 2.0f;
    const f32 viewport_width = aspect_ratio * viewport_height;
    const f32 focal_length = 1.0f;

    origin = ysl::zero_vec3;
    horizontal = ysl::vec3(viewport_width, 0, 0);
    vertical = ysl::vec3(0, viewport_height, 0);
    lower_left_corner = origin - horizontal / 2.0f - vertical / 2.0f - ysl::vec3(0, 0, focal_length);
}

camera::camera(const ysl::point3 &look_from, const ysl::point3 &look_at, const ysl::vec3 &v_up_world,
               f32 vertical_fov, f32 aspect_ratio,
               f32 aperture, f32 focus_dist)
{
    const f32 theta = ysl::util::degrees_to_radians(vertical_fov);
    const f32 h = ysl::tan(theta / 2.0f); // half of the vertical height
    const f32 viewport_height = 2.0f * h;
    const f32 viewport_width = aspect_ratio * viewport_height;

    v_fwd = (look_from - look_at).get_norm(); // forward
    v_side = ysl::cross(v_up_world, v_fwd).get_norm();
    v_up = ysl::cross(v_fwd, v_side);

    origin = look_from;
    horizontal = focus_dist * viewport_width * v_side;
    vertical = focus_dist * viewport_height * v_up;
    lower_left_corner = origin - horizontal / 2.0f - vertical / 2.0f - focus_dist * v_fwd;

    lens_radius = aperture / 2.0f;
}

ray camera::get_ray(f32 s, f32 t) const
{
    ysl::vec3 rd = lens_radius * ysl::random_in_unit_disk();
    ysl::vec3 offset = v_side * rd[0] + v_up * rd[1];

    /*return {origin + offset,
             lower_left_corner + s * horizontal + t * vertical - origin - offset,
             ysl::util::random_f32(tm0, tm1)};*/
    return {origin + offset,
             lower_left_corner + s * horizontal + t * vertical - origin - offset};
}