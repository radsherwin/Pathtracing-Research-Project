#ifndef RT_SCENE_H
#define RT_SCENE_H

#include "camera.h"
#include "ysl_math/color.h"
#include "render_world.h"
#include "texture.h"
#include "material.h"
#include "material_man.h"

#include <memory>
#include <optional>


/*GPU stuff*/
struct Material
{
	alignas(16) ysl::vec4 albedo;
	f32 fuzz_ir;
	i32 type;
};

struct Plane
{
	Material mat;
	f32 corners[4];
	f32 k;
	i32 type;
	bool flip_face;
};

struct Camera
{
	f32 origin[3];
	f32 lower_left_corner[3];
	f32 horizontal[3];
	f32 vertical[3];
	f32 v_fwd[3];
	f32 v_side[3];
	f32 v_up[3];
	f32 lens_radius;
};

struct gpu_world
{
	std::vector<Material> mats;
	std::vector<Plane>planes;
	camera cam;
};

struct scene_world
{
	render_world objects;
	std::optional<render_world> lights;
};

class scene
{
public:
	scene()
		: m_mats(), background(ysl::color::black), look_from{13,2,3}, look_at{0,0,0}, v_up_world{0,1,0},
		  aspect_ratio(16.0f/9.0f), image_width(800), image_height(static_cast<i32>(image_width / aspect_ratio)),
		  dist_to_focus(10.0f), aperture(0.1f), v_fov(20.0f), pad{}
	{}
	scene(const scene &) = delete;
	scene &operator=(const scene &) = delete;
	~scene() = default;

	// Scenes
	scene_world basic_scene();
	scene_world basic_light();
	scene_world orig_scene();
	scene_world cornell_box();
	scene_world glsl_test();
	
	scene_world get_scene(camera &cam, i32 idx);
	void get_gpu_scene(gpu_world &world);

public:
	material_man m_mats; // 32
	ysl::vec3 background;
	ysl::point3 look_from;
	ysl::point3 look_at;
	ysl::vec3 v_up_world;

	f32 aspect_ratio;
	i32 image_width;
	i32 image_height;
	f32 dist_to_focus;

	f32 aperture;
	f32 v_fov;
	f32 pad[2];	

};

#endif