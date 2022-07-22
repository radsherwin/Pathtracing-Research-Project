#ifndef YSL_RT_PDF_H
#define YSL_RT_PDF_H

#include "ysl_math/vect.h"
#include "ray.h"

class renderable;
namespace pdf
{

//struct pdf_result
//{
//	ray pdf_ray;
//	f32 pdf_value;
//};

f32 mixture_pdf(const ysl::point3 &r_origin, f32 r_time, const ysl::vec3 &normal, const std::shared_ptr<renderable> &r_ptr, ray &ray_out);
f32 mixture_pdf(const std::shared_ptr<renderable> &r_ptr, const ysl::vec3 &normal, ray &ray_out);
f32 cosine_pdf(const ysl::point3 r_origin, f32 r_time, const ysl::vec3 &normal, ray &ray_out);
f32 hittable_pdf(const ysl::point3 &r_origin, f32 r_time, const std::shared_ptr<renderable> &r_ptr, ray &ray_out);


}


#endif