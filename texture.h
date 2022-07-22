#ifndef RT_TEXTURE_H
#define RT_TEXTURE_H

#include "ysl_math/vect.h"
#include <memory>
#include <variant>

class texture;

struct tex_solid_color
{
	tex_solid_color() = default;
	tex_solid_color(const tex_solid_color&) =default;
	tex_solid_color &operator=(const tex_solid_color &) = default;
	~tex_solid_color() = default;

	static tex_solid_color create(ysl::vec3 c);
	void destroy();

	ysl::vec3 sample() const;

private:
	ysl::vec3 color_value;
};

struct tex_checker
{
	tex_checker() = default;
	tex_checker(const tex_checker &) = default;
	tex_checker &operator=(const tex_checker &) = default;
	~tex_checker() = default;

	static tex_checker create(const ysl::vec3 &c1, const ysl::vec3 &c2);
	void destroy();

	ysl::vec3 sample(const ysl::vec2 &uv, const ysl::point3 &p) const;

private:
	std::shared_ptr<texture> even;
	std::shared_ptr<texture> odd;
};

//struct tex_noise
//{
//	static tex_noise create(f32 s);
//	void destroy();
//
//	ysl::vec3 tex_coord(const ysl::point3 &p) const;
//
//private:
//	perlin perlin_noise;
//	f32 scale;
//};

struct tex_image
{
	const static i32 bytes_per_pixel = 3;

	tex_image() = default;
	tex_image(const tex_image &);
	tex_image &operator=(const tex_image &);
	~tex_image() = default;

	static tex_image create(const char *const filename);
	void destroy();

	ysl::vec3 sample(const ysl::vec2 &uv) const;

private:
	unsigned char *data;
	i32 width;
	i32 height;
	i32 bytes_per_scanline;
	i32 size;
};

class texture
{
public:
	enum class texture_type : u8
	{
		solid_color = 0,
		checker,
		noise,
		image,
		uninitialized
	};

	texture();
	texture(const texture &t);
	texture &operator=(const texture &);
	~texture();

	static texture solid_color(const ysl::vec3 &c);
	static texture checker(const ysl::vec3 &c1, const ysl::vec3 &c2);
	static texture image(const char *const filename);
	static texture gl_tex();

	ysl::vec3 sample(const ysl::vec2 &uv, const ysl::point3 &p) const;

public:
	std::variant<tex_solid_color, tex_checker, tex_image> m_v;
	texture_type m_type;
};

#endif