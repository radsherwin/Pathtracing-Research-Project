#ifndef YSL_RT_SHADER_MAN_H
#define YSL_RT_SHADER_MAN_H

#include <string>
#include <glad/glad.h>

#include "ysl_math/types.h"

class shader_man
{
public:
	shader_man();
	~shader_man() = default;
	std::string load_shader(const char* const filename);

	void create_vao();
	void create_quad_program(const char* const vs, const char* const fs);
	void create_compute(const char* const cs);
	 
	void create_textures(u32 width, u32 height);

	void prepare_compute(i32 spp, bool pause_light, bool metal_and_glass);
	void dispatch_compute();
	void draw_compute(bool denoise);
	void draw_fragment();

	void create_ssbo(struct gpu_world &world);
	void bind_ssbo();

private:
	inline void print_shader_info_log(GLuint shader);
	inline void print_program_info_log(GLuint program);
	inline bool check_shader_errors(GLuint shader);
	inline bool check_program_errors(GLuint program);

	GLuint m_quad_program;
	GLuint m_ray_program;

	// vao
	GLuint m_quad_vao;

	// output
	GLuint m_tex_out;
	GLuint m_framebuffer;

	// texture width_height
	GLuint m_tex_w;
	GLuint m_tex_h;

	// SSBO
	GLuint m_cam_ssbo;
	GLuint m_plane_ssbo;
	GLuint m_mat_ssbo;

	// timer
	f32 m_delta_time;
	f32 m_last_frame;

};

inline void shader_man::print_shader_info_log(GLuint shader)
{
	int max_length = 4096;
	int actual_length = 0;
	char slog[4096];
	glGetShaderInfoLog(shader, max_length, &actual_length, slog);
	fprintf(stderr, "shader info log for GL index %u\n%s\n", shader, slog);
}

inline void shader_man::print_program_info_log(GLuint program)
{
	int max_length = 4096;
	int actual_length = 0;
	char plog[4096];
	glGetProgramInfoLog(program, max_length, &actual_length, plog);
	fprintf(stderr, "program info log for GL index %u\n%s\n", program, plog);
}
inline bool shader_man::check_shader_errors(GLuint shader)
{
	GLint params = -1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: shader %u did not compile\n", shader);
		print_shader_info_log(shader);
		return false;
	}
	return true;
}

inline bool shader_man::check_program_errors(GLuint program)
{
	GLint params = -1;
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: program %u did not link\n", program);
		print_program_info_log(program);
		return false;
	}
	return true;
}

#endif