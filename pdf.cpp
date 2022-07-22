#include "pdf.h"

#include "ysl_math/common.h"
#include "renderable.h"


f32 pdf::mixture_pdf(const ysl::point3 &r_origin, f32 r_time, const ysl::vec3 &normal, const std::shared_ptr<renderable> &r_ptr, ray &ray_out)
{
    ysl::vec3 dir{};

    if (ysl::util::random_f32() < 0.5f)
    {
        // cosine
        dir = ysl::random_cosine_direction(normal); 
    }
    else
    {
        // renderable
        dir = r_ptr->random_direction(r_origin);
    }

    ray_out = ray(r_origin, dir, r_time);
    const f32 cosine = ysl::dot(dir.get_norm(), normal);
 
    return 0.5f * ((cosine <= 0.0f) ? 0.0f : cosine / ysl::pi) + 0.5f * r_ptr->pdf_value(r_origin, dir);
}

f32 pdf::mixture_pdf(const std::shared_ptr<renderable> &r_ptr, const ysl::vec3 &normal, ray &ray_out)
{
    if (ysl::util::random_f32() >= 0.5f)
    {
        ray_out.set_direction(r_ptr->random_direction(ray_out.origin()));
    }

    const f32 cosine = ysl::dot(ray_out.direction().get_norm(), normal);
    return 0.5f * ((cosine <= 0.0f) ? 0.0f : cosine / ysl::pi) + 0.5f * r_ptr->pdf_value(ray_out.origin(), ray_out.direction());

}

f32 pdf::cosine_pdf(const ysl::point3 r_origin, f32 r_time, const ysl::vec3 &normal, ray &ray_out)
{
    const ysl::vec3 dir = ysl::random_cosine_direction(normal);
    ray_out = ray(r_origin, dir, r_time);
    const f32 cosine = ysl::dot(dir.get_norm(), normal);
    return (cosine <= 0.0f) ? 0.0f : cosine / ysl::pi;
}

f32 pdf::hittable_pdf(const ysl::point3 &r_origin, f32 r_time, const std::shared_ptr<renderable> &r_ptr, ray &ray_out)
{
    const ysl::vec3 dir = r_ptr->random_direction(r_origin);
    ray_out = ray(r_origin, dir, r_time);
    return r_ptr->pdf_value(r_origin, dir);
}


