#include "material.h"

#include "ysl_math/color.h"
#include "ysl_math/common.h"
#include "hit_result.h"
#include "texture.h"
#include "pdf.h"

//-------------------------------
//			Material
//-------------------------------
material::material()
	: tex(nullptr), diffuse{1,1,1}, fuzz_ir(0.0f), type(material_type::uninitialized)
{}

material::material(const material &m)
	: tex(m.tex), diffuse(m.diffuse), fuzz_ir(m.fuzz_ir), type(m.type)
{}

material &material::operator=(const material &m)
{
	if (this != &m)
	{
		tex = m.tex;
		diffuse = m.diffuse;
		fuzz_ir = m.fuzz_ir;
		type = m.type;
	}

	return *this;
}

material::~material()
{	
	//tex = std::nullopt;
	diffuse = ysl::zero_vec3;
	fuzz_ir = 0.0f;
	type = material_type::uninitialized;
}

ysl::vec3 material::get_albedo(const ysl::vec2 &uv, const ysl::point3 &p) const
{
	if (tex)
	{
		return diffuse * tex->sample(uv, p);
	}

	return diffuse;
}


bool material::scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const
{
	switch (type)
	{
		case material_type::dielectric:
			return dielectric_scatter(r_in, hit, scatter_out);
		case material_type::metal:
			return metal_scatter(r_in, hit, scatter_out);
		case material_type::lambertian:
			return lambertian_scatter(r_in, hit, scatter_out);
		case material_type::light_diffuse:
			return false;
		default:
			assert(false);
	}

	return false;
}

f32 material::scatter_pdf(const ray &r_in, const hit_result &hit, const ray &scattered) const
{
	switch (type)
	{
		case material_type::lambertian:
			return lamb_scatter_pdf(r_in, hit, scattered);
		default:
			return 0.0f;
	}

	return 0.0f;
}

ysl::vec3 material::emitted(const ray &r_in, const ysl::vec2 &uv, const ysl::point3 &p, const hit_result &hit) const
{
	if (type == material_type::light_diffuse)
	{
		if (hit.front_face)
		{
			return get_albedo(uv, p);
		}
	}

	return ysl::color::black;
}

//-------------------------------
//			Static creates
//-------------------------------
material material::dielectric(f32 ir)
{
	material mat{};
	mat.type = material_type::dielectric;
	mat.fuzz_ir = ir;

	return mat;
}

material material::metal(const ysl::vec3 &_albedo, f32 _fuzz)
{
	material mat{};
	mat.type = material_type::metal;
	mat.diffuse = _albedo;
	mat.fuzz_ir = _fuzz;

	return mat;
}

material material::lambertian(const std::shared_ptr<texture> &_tex)
{
	material mat{};
	mat.type = material_type::lambertian;
	mat.tex = _tex;

	return mat;
}

material material::lambertian(const ysl::vec3 &albedo)
{
	material mat{};
	mat.type = material_type::lambertian;
	mat.diffuse = albedo;

	return mat;
}

material material::light_diffuse(const std::shared_ptr<texture> &a)
{
	material mat{};
	mat.type = material_type::light_diffuse;
	mat.tex = a;

	return mat;
}

material material::light_diffuse(const ysl::vec3 &color)
{
	material mat{};
	mat.type = material_type::light_diffuse;
	mat.diffuse = color;

	return mat;
}

//-------------------------------
//			Scatter functions
//-------------------------------
bool material::dielectric_scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const
{
	scatter_out.is_specular = true;
	scatter_out.attenuation = ysl::color::white;

	const f32 refraction_ratio = hit.front_face ? (1.0f / fuzz_ir) : fuzz_ir;

	const ysl::vec3 unit_direction = r_in.direction().get_norm();
	const f32 cos_theta = ysl::min(ysl::dot(-unit_direction, hit.normal), 1.0f);
	const f32 sin_theta = ysl::sqrt(1.0f - cos_theta * cos_theta);

	const bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
	ysl::vec3 direction{};
	// Schlick approximation
	if (cannot_refract || priv_reflectance(cos_theta, refraction_ratio) > ysl::util::random_f32())
	{
		direction = ysl::reflect(unit_direction, hit.normal);
	}
	else
	{
		direction = ysl::refract(unit_direction, hit.normal, refraction_ratio);
	}

	scatter_out.out_ray = ray(hit.p, direction, r_in.time());

	return true;
}

f32 material::priv_reflectance(f32 cos, f32 ref_idx) const
{
	f32 r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * ysl::pow((1.0f - cos), 5.0f);
}

bool material::metal_scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const
{
	/*const ysl::vec3 reflected = ysl::reflect(r_in.direction().get_norm(), hit.normal);
	scattered_out = ray(hit.p, reflected + fuzz_ir * ysl::random_in_unit_sphere(), r_in.time());
	albedo_out = diffuse;
	return ysl::dot(scattered_out.direction(), hit.normal) > 0;*/


	const ysl::vec3 reflected = ysl::reflect(r_in.direction().get_norm(), hit.normal);
	scatter_out = scatter_result{
		.out_ray = ray(hit.p, reflected+fuzz_ir*ysl::random_in_unit_sphere(), r_in.time()),
		.attenuation = diffuse,
		.is_specular = true,
	};

	return true;
}

bool material::lambertian_scatter(const ray &r_in, const hit_result &hit, scatter_result &s_out) const
{
	s_out = scatter_result{
		.out_ray = ray(hit.p, ysl::random_cosine_direction(hit.normal)),
		.attenuation = get_albedo(hit.uv, hit.p),
		.is_specular = false
	};
	//const ysl::vec3 scatter_direction = ysl::construct_onb(hit.normal, ysl::random_cosine_direction());

	//scatter_out.pdf_ray = ray(hit.p, scatter_direction.get_norm(), r_in.time());
	//scatter_out.pdf_value = pdf::cosine_pdf(hit.p, r_in.time(), hit.normal, scatter_out.pdf_ray);
	

	return true;
}

//-------------------------------
//			Scatter pdf
//-------------------------------
f32 material::lamb_scatter_pdf(const ray &r_in, const hit_result &hit, const ray &scattered) const
{
	const auto cosine = ysl::dot(scattered.direction().get_norm(), hit.normal);
	return cosine < 0 ? 0 : cosine/ysl::pi;
}

