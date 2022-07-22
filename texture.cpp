#include "texture.h"
#include "ysl_stb_image.h"
//-------------------------------
//			Texture
//-------------------------------
texture::texture()
	: m_v(), m_type(texture_type::uninitialized)
{}

texture::texture(const texture &t)
	: m_v(t.m_v), m_type(t.m_type)
{}
texture &texture::operator=(const texture &t)
{
	if (this != &t)
	{
		m_v = t.m_v;
		m_type = t.m_type;
	}

	return *this;
}

texture::~texture()
{
	switch (m_type)
	{
		case texture_type::solid_color:
			std::get<tex_solid_color>(m_v).destroy();
			break;
		case texture_type::checker:
			std::get<tex_checker>(m_v).destroy();
			break;
		case texture_type::image:
			std::get<tex_image>(m_v).destroy();
			break;
		default:
			assert(false);
	}
}

ysl::vec3 texture::sample(const ysl::vec2 &uv, const ysl::point3 &p) const
{
	switch (m_type)
	{
		case texture_type::solid_color:
			return std::get<tex_solid_color>(m_v).sample();
		case texture_type::checker:
			return std::get<tex_checker>(m_v).sample(uv, p);
		case texture_type::image:
			return std::get<tex_image>(m_v).sample(uv);
		default:
			assert(false);
	}
	return ysl::zero_vec3;
}

//-------------------------------
//			Static creates
//-------------------------------
texture texture::solid_color(const ysl::vec3 &c)
{
	texture tex{};
	tex.m_type = texture_type::solid_color;
	tex.m_v = tex_solid_color::create(c);
	return tex;
}

texture texture::checker(const ysl::vec3 &c1, const ysl::vec3 &c2)
{
	texture tex{};
	tex.m_type = texture_type::checker;
	tex.m_v = tex_checker::create(c1, c2);
	return tex;
}

texture texture::image(const char *const filename)
{
	texture tex{};
	tex.m_type = texture_type::image;
	tex.m_v = tex_image::create(filename);
	return tex;
}

//-------------------------------
//			solid color
//-------------------------------
tex_solid_color tex_solid_color::create(ysl::vec3 c)
{
	tex_solid_color tex;
	tex.color_value = c;
	return tex;
}

void tex_solid_color::destroy()
{
	color_value = ysl::zero_vec3;
}

ysl::vec3 tex_solid_color::sample() const
{
	return color_value;
}

//-------------------------------
//			checker
//-------------------------------
tex_checker tex_checker::create(const ysl::vec3 &c1, const ysl::vec3 &c2)
{
	tex_checker tex;
	tex.odd = std::make_shared<texture>(texture::solid_color(c1));
	tex.even = std::make_shared<texture>(texture::solid_color(c2));
	return tex;
}

void tex_checker::destroy()
{
	odd = nullptr;
	even = nullptr;
}

ysl::vec3 tex_checker::sample(const ysl::vec2 &uv, const ysl::point3 &p) const
{
	f32 sines = ysl::sin(10.0f * p.x()) * ysl::sin(10.0f * p.y()) * ysl::sin(10.0f * p.z());
	if (sines < 0.0f)
	{
		return odd->sample(uv, p);
	}

	return even->sample (uv, p);
}

//-------------------------------
//			image
//-------------------------------
tex_image::tex_image(const tex_image &t)
	: width(t.width), height(t.height), bytes_per_scanline(t.bytes_per_scanline), size(t.size)
{
	data = new unsigned char[size];
	memcpy(data, t.data, size);
}

tex_image &tex_image::operator=(const tex_image &t)
{
	if (this != &t)
	{
		data = new unsigned char[size];
		memcpy(data, t.data, size);
		width = t.width;
		height = t.height;
		bytes_per_scanline = t.bytes_per_scanline;
		size = t.size;
	}

	return *this;
}
tex_image tex_image::create(const char *const filename)
{
	tex_image tex{};
	auto components_per_pixel = 3;
	tex.data = stbi_load(filename, &tex.width, &tex.height, &components_per_pixel, 3);
	tex.size = (tex.width * tex.height * 3);
	if (!tex.data)
	{
		std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
		tex.width = tex.height = 0;
	}
	// A technique to use texture coordinates instead of image pixel coordinates
	// because of scaling issues
	tex.bytes_per_scanline = bytes_per_pixel * tex.width;
	return tex;
}

void tex_image::destroy()
{
	stbi_image_free(data);
	width = 0;
	height = 0;
	bytes_per_scanline = 0;
	size = 0;
}

ysl::vec3 tex_image::sample(const ysl::vec2 &uv) const
{
	// If we have no texture data, then return solid cyan as a debugging aid.
	if (data == nullptr)
		return ysl::vec3(0, 1, 1);

	// Clamp input texture coordinates to [0,1] x [1,0]
	f32 u2 = ysl::util::clamp(uv[0], 0.0f, 1.0f);
	f32 v2 = 1.0f - ysl::util::clamp(uv[1], 0.0f, 1.0f);  // Flip V to image coordinates

	auto i = static_cast<int>(u2 * width);
	auto j = static_cast<int>(v2 * height);

	// Clamp integer mapping, since actual coordinates should be less than 1.0
	if (i >= width)
		i = width - 1;
	if (j >= height)
		j = height - 1;

	const auto color_scale = 1.0f / 255.0f;
	auto pixel = data + j * (3 * width) + i * 3;

	return ysl::vec3(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
}