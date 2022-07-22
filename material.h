#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include <stdint.h>

#include "ysl_math/vect.h"

#include "ray.h"
#include "hit_result.h"

class texture;
//struct hit_result;
class material
{
public:
	enum class material_type : u8
	{
		dielectric = 0,
		metal,
		lambertian,

		light_diffuse,

		uninitialized
	};

	material();
	material(const material &);
	material &operator=(const material &);
	~material();

	static material dielectric(f32 ir);
	static material metal(const ysl::vec3 &_albedo, f32 _fuzz);
	static material lambertian(const std::shared_ptr<texture> &_tex);
	static material lambertian(const ysl::vec3 &albedo);

	// light
	static material light_diffuse(const std::shared_ptr<texture> &a);
	static material light_diffuse(const ysl::vec3 &color);

	bool scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const;
	f32 scatter_pdf(const ray &r_in, const hit_result &hit, const ray &scattered) const;
	ysl::vec3 emitted(const ray &r_in, const ysl::vec2 &uv, const ysl::point3 &p, const hit_result &hit) const;

	ysl::vec3 get_albedo(const ysl::vec2 &uv, const ysl::point3 &p) const;

private:
	bool dielectric_scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const;
	bool metal_scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const;
	bool lambertian_scatter(const ray &r_in, const hit_result &hit, scatter_result &scatter_out) const;

	f32 lamb_scatter_pdf(const ray &r_in, const hit_result &hit, const ray &scattered) const;

	f32 priv_reflectance(f32 cos, f32 ref_ifx) const;

	std::shared_ptr<texture> tex; // 16
	ysl::vec3 diffuse; // 16
	f32 fuzz_ir; // 4
	material_type type; // 1

};

#endif