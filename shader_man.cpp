#include "shader_man.h"

#include <fstream>
#include <sstream>

#include <GLFW/glfw3.h>
#include "scene.h"



shader_man::shader_man()
	: m_quad_program(0), m_ray_program(0), m_quad_vao(0), m_tex_out(0), m_framebuffer(0),
	  m_tex_w(0), m_tex_h(0),m_cam_ssbo(0), m_plane_ssbo(0), m_mat_ssbo(0),
	  m_delta_time(0.0f), m_last_frame(0.0f)
{}

std::string shader_man::load_shader(const char* const filename)
{
	std::stringstream c_s;
	std::ifstream c_f;
	c_f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	c_f.open(filename);

	c_s << c_f.rdbuf();
	c_f.close();

	return c_s.str();
}

void shader_man::create_vao()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	GLuint vbo = 0;
	f32 quad[] = { -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glGenVertexArrays(1, &m_quad_vao);
	glBindVertexArray(m_quad_vao);
	glEnableVertexAttribArray(0);
	GLintptr stride = 4 * sizeof(f32);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), nullptr);
	glEnableVertexAttribArray(1);
	GLintptr offset = 2 * sizeof(f32);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid *>(offset)); // st
}
void shader_man::create_quad_program(const char *const vs, const char *const fs)
{
	std::string vert = load_shader(vs);
	const char *v_data = vert.c_str();

	std::string frag = load_shader(fs);
	const char *f_data = frag.c_str();

	m_quad_program = glCreateProgram();

	// vertex
	GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v_shader, 1, &v_data, nullptr);
	glCompileShader(v_shader);
	check_shader_errors(v_shader);
	glAttachShader(m_quad_program, v_shader);

	// fragment
	GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f_shader, 1, &f_data, nullptr);
	glCompileShader(f_shader);
	check_shader_errors(f_shader);
	glAttachShader(m_quad_program, f_shader);
	glLinkProgram(m_quad_program);
	check_program_errors(m_quad_program);
	glDeleteShader(v_shader);
	glDeleteShader(f_shader);
}
void shader_man::create_compute(const char *const cs)
{
	std::string compute = load_shader(cs);
	const char *c_data = compute.c_str();

	GLuint ray_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(ray_shader, 1, &c_data, nullptr);
	glCompileShader(ray_shader);

	// cs program
	m_ray_program = glCreateProgram();
	glAttachShader(m_ray_program, ray_shader);
	glLinkProgram(m_ray_program);
	// cleanup
	glDeleteShader(ray_shader);
}

void shader_man::create_textures(u32 width, u32 height)
{
	m_tex_w = static_cast<GLuint>(width);
	m_tex_h = static_cast<GLuint>(height);
	glGenTextures(1, &m_tex_out);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex_out);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_tex_w, m_tex_h, 0, GL_RGBA, GL_FLOAT, nullptr);
	// bind to image
	glBindImageTexture(0, m_tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

// inside loop
void shader_man::prepare_compute(i32 spp, bool pause_light, bool metal_and_glass)
{
	glUseProgram(m_ray_program);
	const GLint loc = glGetUniformLocation(m_ray_program, "time");
	const f32 cur_frame = static_cast<f32>(glfwGetTime());
	m_delta_time = cur_frame - m_last_frame;
	m_last_frame = cur_frame;

	glUniform1f(loc, cur_frame);
	glUniform1i(glGetUniformLocation(m_ray_program, "spp"), spp);
	glUniform1i(glGetUniformLocation(m_ray_program, "pause_light"), pause_light);
	glUniform1i(glGetUniformLocation(m_ray_program, "show_more_mats"), metal_and_glass);
	
}
void shader_man::dispatch_compute()
{
	glDispatchCompute(m_tex_w/8 +1, m_tex_h/8 + 1, 1);
}

void shader_man::draw_compute(bool denoise)
{
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(m_quad_program);
	glUniform1i(glGetUniformLocation(m_quad_program, "denoise_enabled"), denoise);
	glUniform2f(glGetUniformLocation(m_quad_program, "dims"), static_cast<GLfloat>(m_tex_w), static_cast<GLfloat>(m_tex_h));
	glBindVertexArray(m_quad_vao);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex_out);
	/*
	// if I want to skip vs and fs and just output to window I can use framebuffers
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);

	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_out, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_tex_w, m_tex_h, 0, 0, m_tex_w, m_tex_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void shader_man::draw_fragment()
{
	glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(m_quad_program);
	const GLint loc = glGetUniformLocation(m_quad_program, "time");
	const f32 cur_frame = static_cast<f32>(glfwGetTime());
	m_delta_time = cur_frame - m_last_frame;
	m_last_frame = cur_frame;

	const GLint dims = glGetUniformLocation(m_quad_program, "dims"); 

	glUniform1f(loc, cur_frame);
	glUniform2f(dims, static_cast<GLfloat>(m_tex_w), static_cast<GLfloat>(m_tex_h));
	glBindVertexArray(m_quad_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex_out);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void shader_man::create_ssbo(gpu_world& world)
{
	glGenBuffers(1, &m_mat_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mat_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(sizeof(Material)*world.mats.size()), world.mats.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_plane_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_plane_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(sizeof(Plane) * world.planes.size()), world.planes.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_cam_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_cam_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(world.cam), &world.cam, GL_STATIC_DRAW);

	glUniform1i(glGetUniformLocation(m_ray_program, "mat_count"),   static_cast<GLint>(world.mats.size()));
	glUniform1i(glGetUniformLocation(m_ray_program, "plane_count"), static_cast<GLint>(world.planes.size()));
}

void shader_man::bind_ssbo()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_mat_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_plane_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_cam_ssbo);

	

}