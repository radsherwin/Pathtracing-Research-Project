#ifndef YSL_RT_TRACER_H
#define YSL_RT_TRACER_H

#include "ysl_math/math_lib.h"
#include "renderable.h"
#include "material_man.h"
#include "scene.h"
#include "ysl_window.h"
#include "shader_man.h"

#include <iostream>
//#include <glad/glad.h> // included in window which ysl_engine is including both this and ysl_window

constexpr i32 max_depth = 3;

struct renderer
{
	u32 *image_data;
	i32 width;
	i32 height;
};

class ysl_tracer
{
public:
	ysl_tracer();
	ysl_tracer(const ysl_tracer &) = default;
	ysl_tracer &operator=(const ysl_tracer &) = default;
	~ysl_tracer()
	{
		destroy();
	}

	void init(std::pair<i32, i32> extents);
	void destroy();
	void create_texture();
	void run();
	void gpu_run();
	
	ysl::vec3 ray_trace(const ray &r, const ysl::vec3 &background, const render_world &objects, const std::optional<render_world> &lights, i32 depth);
	u32 get_color(const ysl::vec3 &pixel_color) const;

	int &set_spp();
	bool &set_denoise();
	bool &pause_light();
	bool &set_metal_and_glass();
	
	// rendering
	
	void write_to_file(const char *const file);
	void render_imgui_image();
	f32 render_progress() const;
	bool is_running() const;
	f64 get_runtime() const;
	bool is_done() const;
	bool should_gpu_run() const;

	void load_scene(i32 scene_idx);
	void launch_thread();
	void launch_gpu(i32 width, i32 height);
	void end_gpu();

	GLuint m_texture_data;

private:
	camera m_cam;
	scene m_sc;
	scene_world m_world;
	shader_man m_shader;
	renderer m_data;
	std::atomic<f64> m_runtime;
	std::atomic<f32> m_progress;
	GLuint m_framebuffer;
	std::atomic_bool m_is_running;
	std::atomic_bool m_is_done;
	bool m_should_gpu_run;

	i32  m_samples_per_pixel;
	bool m_denoise_enabled;
	bool m_pause_light;
	bool m_metal_and_glass;
};

#endif