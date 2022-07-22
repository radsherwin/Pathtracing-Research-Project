#include "scene.h"

scene_world scene::get_scene(camera &cam, i32 idx)
{
	scene_world world;
	switch (idx)
	{
		case 0:
			world = basic_scene();
			background = ysl::color::black;
			look_from = ysl::point3(26, 3, 6);
			look_at = ysl::point3(0, 2, 0);
			image_width = 400;
			break;
		case 1:
			world = basic_light();
			background = ysl::color::black;
			look_from = ysl::point3(26, 3, 6);
			look_at = ysl::point3(0, 2, 0);
			break;
		case 2:
			world = orig_scene();
			background = ysl::color::dark_gray;
			break;
		case 3:
			world = cornell_box();
			background = ysl::color::black;
			look_from = ysl::point3{278, 278, -800};
			look_at = ysl::point3{278, 278, 0};
			aspect_ratio = 1.0f;
			image_width = 1000;
			v_fov = 40.0f;
			aperture = 0.0f;
			break;
		case 4: 
			world = glsl_test();
			background = ysl::color::black;
			aspect_ratio = 1.0f;
			image_width = 512;
			break;
		default:
			background = ysl::color::black;
	}

	image_height = static_cast<i32>(image_width / aspect_ratio);
	cam = camera(look_from, look_at, v_up_world, v_fov, aspect_ratio, aperture, dist_to_focus);

	return world;
}

void scene::get_gpu_scene(gpu_world& world)
{
	// Material
	Material red{.albedo ={0.65f, 0.05f, 0.05f}, .fuzz_ir = 0.0f, .type = 0};
	Material green{ .albedo = {0.12f, 0.45f, 0.15f}, .fuzz_ir = 0.0f, .type = 0 };
	Material white{ .albedo = {0.73f, 0.73f, 0.73f}, .fuzz_ir = 0.0f, .type = 0 };
	Material aluminum{ .albedo = {0.8f, 0.85f, 0.88f}, .fuzz_ir = 0.0f, .type = 2 };
	Material glass{ .albedo ={0.0f, 0.0f, 0.0f}, .fuzz_ir = 0.7f, .type = 1 };
	Material blue{.albedo = {0.05f, 0.05f, 0.65f}, .fuzz_ir = 0.0f, .type = 0};
	Material yellow{.albedo = {0.02f, 0.62f, 0.62f}, .fuzz_ir = 0.0f, .type = 0};
	std::vector<Material> mats = {red, green, white, aluminum, glass, blue, yellow};
	world.mats = mats;

	const int plane_yz = 0;
	const int plane_zx = 1;
	const int plane_xy = 2;

	// Planes
	Plane right_plane{.mat = green, .corners = {0.0f, 555.0f, 0.0f, 555.0f}, .k = 555.0f, .type = plane_yz, .flip_face = false};
	Plane left_plane{ .mat = red, .corners = {0.0f, 555.0f, 0.0f, 555.0f}, .k = 0.0f, .type = plane_yz, .flip_face = false };
	Plane bottom_plane{ .mat = white, .corners = {0.0f, 555.0f, 0.0f, 555.0f}, .k = 0.0f, .type = plane_zx, .flip_face = false };
	Plane top_plane{ .mat = white, .corners = {0.0f, 555.0f, 0.0f, 555.0f}, .k = 555.0f, .type = plane_zx, .flip_face = false };
	Plane back_plane{ .mat = white, .corners = {0.0f, 555.0f, 0.0f, 555.0f}, .k = 555.0f, .type = plane_xy, .flip_face = false };
	std::vector<Plane> planes = {right_plane, left_plane, bottom_plane, top_plane, back_plane};
	world.planes = planes;

	camera cam;
	get_scene(cam, 3);
	world.cam = cam;

}

//-------------------------------
//			Scenes
//-------------------------------
scene_world scene::basic_scene()
{
	scene_world world;
	//render_world lights;
	//m_mats.reset();

	//std::shared_ptr<render_world> bvh = std::make_shared<render_world>();
	//auto red_m = std::make_shared<material>(material::lambertian({0.65f, 0.05f, 0.05f}));

	//const i32 red = m_mats.add(red_m);
	////ysl::mat4 m = ysl::mat4::translation(2,3,-8);
	////m.set_scale(0.2f, 0.2f, 0.2f);
	///*ysl::mat4 m{ {1.0f, 0.0f, 0.0f, 0.0f},
	//			 {0.0f, 1.0f, 0.0f, 0.0f},
	//			 {0.0f, 0.0f, 1.0f, 0.0f},
	//			 {2.0f, 3.0f, -8.0f, 1.0f}};*/
	//ysl::mat4 m = ysl::mat4::identity();
	////ysl::mat4 m = ysl::mat4::rot_z(ysl::util::degrees_to_radians(12.0f));
	////m.set_translation({-13,0,0, 1.0f});
	////m.set_scale_z(3.0f);
	////ysl::mat4 m = ysl::mat4::identity();
	//world.objects.add(std::make_shared<obox>(red, m, ysl::vec3{-1.0f}, ysl::vec3{1.0f}));
	////bvh->add(std::make_shared<aabb>(red, ysl::vec3{-0.5f}, ysl::vec3{0.5f}));

	////const i32 mat_2 = m_mats.add(std::make_shared<material>(material::lambertian(ysl::color::pink)));
	////bvh->add(std::make_shared<sphere>(ysl::point3(0.0f, 2.0f, 0.0f), 2.0f, mat_2));

	//const i32 diff_light = m_mats.add(std::make_shared<material>(material::light_diffuse(ysl::vec3(4.0f, 4.0f, 4.0f))));
	//const auto light_obj = std::make_shared<rect>(rect::rect_type::xy, diff_light, ysl::vec2{3.0f,5.0f}, ysl::vec2{1.0f,3.0f}, -2.0f);
	//bvh->add(light_obj);
	//lights.add(light_obj);

	//world.lights = lights;
	//world.objects.add(std::make_shared<bvh_node>(bvh, 0.0f, 0.0f));

	return world;
}

scene_world scene::basic_light()
{
	scene_world world;
	/*render_world lights;
	m_mats.reset();

	const i32 mat_ground = m_mats.add(std::make_shared<material>(material::lambertian(ysl::color::orange)));
	world.objects.add(std::make_shared<sphere>(ysl::point3(0.0f, -1000.0f, 0.0f), 1000.0f, mat_ground));

	std::shared_ptr<render_world> bvh = std::make_shared<render_world>();

	const i32 mat_2 = m_mats.add(std::make_shared<material>(material::lambertian(ysl::color::pink)));
	bvh->add(std::make_shared<sphere>(ysl::point3(0.0f, 2.0f, 0.0f), 2.0f, mat_2));

	const i32 diff_light = m_mats.add(std::make_shared<material>(material::light_diffuse(ysl::vec3(4.0f, 4.0f, 4.0f))));

	const auto light_obj = std::make_shared<rect>(rect::rect_type::xy, diff_light, ysl::vec2{3.0f,5.0f}, ysl::vec2{1.0f,3.0f}, -2.0f);

	bvh->add(light_obj);
	lights.add(light_obj);

	world.objects.add(std::make_shared<bvh_node>(bvh, 0.0f, 0.0f));
	world.lights = lights;*/

	return world;
}

scene_world scene::orig_scene()
{
	scene_world world;
	m_mats.reset();

	std::shared_ptr<render_world> bvh = std::make_shared<render_world>();
	const i32 ground_material = m_mats.add(new material(material::lambertian({0.5f, 0.5f, 0.5f})));
	bvh->add(std::make_shared<sphere>(ysl::point3(0.0f, -1000.0f, 0.0f), 1000.0f, ground_material));

	for (i32 a = -11; a < 11; a++)
	{
		for (i32 b = -11; b < 11; b++)
		{
			auto choose_mat = ysl::util::random_f32();
			ysl::point3 center(a + 0.9f * ysl::util::random_f32(), 0.2f, b + 0.9f * ysl::util::random_f32());

			if ((center - ysl::point3(4.0f, 0.2f, 0.0f)).mag() > 0.9f)
			{
				i32 sphere_material;

				if (choose_mat < 0.8f)
				{
					// diffuse
					const auto albedo = ysl::vec3::random() * ysl::vec3::random();
					sphere_material = m_mats.add(new material(material::lambertian(albedo)));
					bvh->add(std::make_shared<sphere>(center, 0.2f, sphere_material));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					auto albedo = ysl::vec3::random(0.5f, 1.0f);
					auto fuzz = ysl::util::random_f32(0.0f, 0.5f);
					sphere_material = m_mats.add(new material(material::metal(albedo, fuzz)));
					bvh->add(std::make_shared<sphere>(center, 0.2f, sphere_material));
				}
				else
				{
					// glass
					sphere_material = m_mats.add(new material(material::dielectric(1.5f)));
					bvh->add(std::make_shared<sphere>(center, 0.2f, sphere_material));
				}
			}
		}
	}

	const i32 material1 = m_mats.add(new material(material::dielectric(1.5f)));
	bvh->add(std::make_shared<sphere>(ysl::point3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	const i32 material2 = m_mats.add(new material(material::lambertian({0.4f, 0.2f, 0.1f})));
	bvh->add(std::make_shared<sphere>(ysl::point3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	const i32 material3 = m_mats.add(new material(material::metal({0.7f, 0.6f, 0.5f}, 0.0f)));
	bvh->add(std::make_shared<sphere>(ysl::point3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	const i32 diff_light = m_mats.add(new material(material::light_diffuse(ysl::vec3(6.0f, 6.0f, 6.0f))));
	bvh->add(std::make_shared<rect>(rect::rect_type::xy, diff_light, ysl::vec2{3.0f,5.0f}, ysl::vec2{1.0f,3.0f}, -2.0f));

	world.objects.add(std::make_shared<bvh_node>(bvh, 0.0f, 0.0f));

	return world;
}

scene_world scene::cornell_box()
{
	scene_world world;
	render_world lights;

	m_mats.reset();

	std::shared_ptr<render_world> bvh = std::make_shared<render_world>();
	auto red_m = new material(material::lambertian({0.65f, 0.05f, 0.05f}));
	auto white_m = new material(material::lambertian({0.73f, 0.73f, 0.73f}));
	auto green_m = new material(material::lambertian({0.12f, 0.45f, 0.15f}));
	auto light_m = new material(material::light_diffuse({15.0f, 15.0f, 15.0f}));

	const i32 red = m_mats.add(red_m);
	const i32 white = m_mats.add(white_m);
	const i32 green = m_mats.add(green_m);
	const i32 light = m_mats.add(light_m);
	const i32 aluminum = m_mats.add(new material(material::metal({0.8f, 0.85f, 0.88f}, 0.0f)));
	const i32 glass = m_mats.add(new material(material::dielectric(.7f)));

	auto light_obj = std::make_shared<rect>(rect::rect_type::xz, light, ysl::vec2{213.0f, 343.0f}, ysl::vec2{227.0f, 332.0f}, 554.0f);
	light_obj->flip_render_face();

	lights.add(light_obj);  // add to light array

	bvh->add(std::make_shared<rect>(rect::rect_type::yz, green, ysl::vec2{0.0f, 555.0f},   ysl::vec2{0.0f, 555.0f}, 555.0f));
	bvh->add(std::make_shared<rect>(rect::rect_type::yz, red,   ysl::vec2{0.0f, 555.0f},   ysl::vec2{0.0f, 555.0f}, 0.0f));
	world.objects.add(light_obj);
	bvh->add(std::make_shared<rect>(rect::rect_type::xz, white, ysl::vec2{0.0f, 555.0f},   ysl::vec2{0.0f, 555.0f}, 0.0f));
	bvh->add(std::make_shared<rect>(rect::rect_type::xz, white, ysl::vec2{0.0f, 555.0f},   ysl::vec2{0.0f, 555.0f}, 555.0f));
	bvh->add(std::make_shared<rect>(rect::rect_type::xy, white, ysl::vec2{0.0f, 555.0f},   ysl::vec2{0.0f, 555.0f}, 555.0f));
	//ysl::mat4 m = ysl::mat4::identity(); 
	ysl::mat4 m = ysl::mat4::rot_y(ysl::util::degrees_to_radians(-18.0f)); // -18
	m.set_translation( ysl::vec4{20,0,-20,1});
	ysl::mat4 m2 = ysl::mat4::rot_y(ysl::util::degrees_to_radians(19.0f));
	m2.set_translation(ysl::vec4{-85.0f,0,60,1});
	
	world.objects.add(std::make_shared<obox>(white, m, ysl::vec3{130.0f, 0.0f, 65.0f}, ysl::vec3{295.0f, 165.0f, 230.0f}));
	//auto sphere_obj = std::make_shared<sphere>(ysl::point3{190.0f, 90.0f, 190.0f}, 90.0f, glass);
	//world.objects.add(sphere_obj);
	/*lights.add(sphere_obj); */
	//world.objects.add();
	world.objects.add(std::make_shared<obox>(aluminum, m2, ysl::vec3{265.0f, 0.0f, 295.0f}, ysl::vec3{430.0f, 330.0f, 460.0f}));  // tall box
	world.objects.add(std::make_shared<sphere>(ysl::vec3{210.0f, 210.0f, 100.0f}, 35.0f, glass));
	world.objects.add(std::make_shared<bvh_node>(bvh, 0.0f, 0.0f));

	world.lights = lights;

	return world;
}

scene_world scene::glsl_test()
{
	scene_world world;
	render_world lights;

	m_mats.reset();

	const i32 ground_material = m_mats.add(new material(material::lambertian(ysl::color::orange)));
	world.objects.add(std::make_shared<sphere>(ysl::point3(0.0f, -1001.0f, 0.0f), 1000.0f, ground_material));

	const i32 sphere_mat = m_mats.add(new material(material::lambertian({0.65f, 0.05f, 0.05f})));
	world.objects.add(std::make_shared<sphere>(ysl::point3(-1.0f, 0.0f, 0.0f), 0.6f, sphere_mat));

	const i32 box_mat = m_mats.add(new material(material::light_diffuse(ysl::vec3(5.0f, 5.0f, 5.0f))));
	ysl::mat4 m = ysl::mat4::identity();
	m.set_translation(ysl::vec4{3.0f, 1.0f, 0.0f,1.0f});
	auto light_obj = std::make_shared<obox>(box_mat, m, ysl::vec3{-0.25f}, ysl::vec3{0.25f});
	world.objects.add(light_obj);
	lights.add(light_obj);

	world.lights = lights;

	return world;

}