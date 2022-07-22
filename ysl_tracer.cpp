#include "ysl_tracer.h"
#include "ysl_math/common.h"

#include "hit_result.h"
#include "scene.h"
#include "timer.h"
#include "pdf.h"
#include "shader_man.h"

#pragma warning(disable:4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

#include <random>
#include <thread>

ysl_tracer::ysl_tracer() : m_texture_data(0), m_cam(), m_sc(), m_world(), m_shader(), 
                           m_data(), m_runtime(0.0f), m_progress(0.0f), m_framebuffer(0), 
                           m_is_running(false), m_is_done(false), m_should_gpu_run(false),
                           m_samples_per_pixel(1), m_denoise_enabled(false), m_metal_and_glass(false)
{}

void ysl_tracer::init(std::pair<i32, i32> extents)
{
    m_data.width = extents.first;
    m_data.height = extents.second;
    m_data.image_data = new u32[m_data.width * m_data.height];

    glGenTextures(1, &m_texture_data);
    create_texture();
}

void ysl_tracer::destroy()
{
    m_is_done = false;
    m_is_running = false;
    delete[] m_data.image_data;
    m_data.image_data = nullptr;
    glDeleteTextures(1, &m_texture_data);
    glDeleteFramebuffers(1, &m_framebuffer);
    m_data.width = 0;
    m_data.height = 0;
    m_texture_data = 0;
}

void ysl_tracer::load_scene(i32 scene_idx)
{
    m_world = m_sc.get_scene(m_cam, scene_idx);
    init({m_sc.image_width, m_sc.image_height});
}

void ysl_tracer::launch_thread()
{
    std::thread t_tracer(&ysl_tracer::run, this);
    t_tracer.detach();
}

void ysl_tracer::gpu_run()
{
    // main loop
    m_shader.bind_ssbo();
    m_shader.prepare_compute(m_samples_per_pixel, m_pause_light, m_metal_and_glass);
    m_shader.dispatch_compute();
    m_shader.draw_compute(m_denoise_enabled);
    
    //m_shader.bind_ssbo();
    //m_shader.draw_fragment();
}

void ysl_tracer::launch_gpu(i32 width, i32 height)
{
    
    m_shader.create_vao();
    //m_shader.create_quad_program("./shaders/quadvs.glsl", "./shaders/rt_fs.glsl");
    m_shader.create_quad_program("./shaders/quadvs.glsl", "./shaders/quadfs.glsl");
    m_shader.create_compute("./shaders/ray_tracer.glsl");
    m_shader.create_textures(width, height);

    // load GPU scene
    //camera cam;
    //m_sc.get_scene(cam, 3);
    //m_shader.create_ssbo(cam);
    gpu_world world;
    m_sc.get_gpu_scene(world);
    m_shader.create_ssbo(world);

    m_should_gpu_run = true;
}

void ysl_tracer::end_gpu()
{
    m_should_gpu_run = false;
}

void ysl_tracer::run()
{
    srand((unsigned int)time(0));

    ysl::init_seed();
    m_is_running = true;

    // Render
    f32 u = 0.0f;
    f32 v = 0.0f;
    const f32 width1 = static_cast<f32>(m_sc.image_width - 1);
    const f32 height1 = static_cast<f32>(m_sc.image_height - 1);

    //ysl::util::init();

    timer sw;
    sw.tic(); 
    for (i32 j = m_sc.image_height - 1; j >= 0; --j)
    {
        //std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        m_progress = static_cast<f32>((m_sc.image_height-1-j)) / m_sc.image_height;

        const f32 fj = static_cast<f32>(j);
        for (i32 i = 0; i < m_sc.image_width; ++i)
        {
            const f32 fi = static_cast<f32>(i);
            ysl::vec3 pixel_color{};

            /*for (i32 s = 0; s < samples_per_pixel; s += 8)
            {
                f32 ran_f32[8]{};
                ysl::util::random_f32_8(&ran_f32[0]);

                for (i32 r = 0; r < 4; ++r)
                {
                    u = (fi + ran_f32[r]) / width1;
                    v = (fj + ran_f32[r + 4]) / height1;
                    pixel_color += ray_trace(m_cam.get_ray(u, v), m_sc.background, m_world.objects, m_world.lights, max_depth);
                }
            }*/
            for (i32 s = 0; s < m_samples_per_pixel; ++s)
            {
                    //const f32 r1 = ysl::util::random_f32();
                    //const f32 r2 = ysl::util::random_f32();
                    //const f32 r3 = dist(gen);
                    //const f32 r4 = dist(gen);
                    u = (fi + ysl::util::random_f32()) / width1;
                    v = (fj + ysl::util::random_f32()) / height1;
                    pixel_color += ray_trace(m_cam.get_ray(u, v), m_sc.background, m_world.objects, m_world.lights, max_depth);
            }
            // trick to flip image upside down since texture is seemingly read backwards
            //m_data.image_data[(m_sc.image_height-1-j) * m_sc.image_width + i] = get_color(pixel_color);
            m_data.image_data[j * m_sc.image_width + i] = get_color(pixel_color);
        }
    }
    sw.toc();
    sw.print_s();
    m_runtime.store(sw.elapsed_time());

    m_is_running = false;
    m_is_done = true;
}

ysl::vec3 ysl_tracer::ray_trace(const ray &r, const ysl::vec3 &background, const render_world &objects, const std::optional<render_world> &lights, i32 depth)
{
    if (depth <= 0)
    {
        return ysl::color::black;
    }

    hit_result hit_out;
    // 0.001 ==> fix shadow acne
    if (!objects.hit(r, 0.001f, ysl::infinity, hit_out))
    {
        return background;
    }
    
    // Get the material from the material manager via index
    const auto p_mat = m_sc.m_mats.get(hit_out.mat_idx);
    const ysl::vec3 emitted = p_mat->emitted(r, hit_out.uv, hit_out.p, hit_out);
   
    
    scatter_result s_out;
    // this is saying, we hit a light source, so there's no need to keep bouncing
    if (!p_mat->scatter(r, hit_out, s_out))
    {
        return emitted;
    }

    if(s_out.is_specular)
    {
        return s_out.attenuation * ray_trace(s_out.out_ray, background, objects, lights, depth-1);
    }

    f32 pdf_val = pdf::mixture_pdf(lights.value().objects()[0], hit_out.normal, s_out.out_ray);
    //f32 pdf_val = pdf::hittable_pdf(hit_out.p, r.time(), lights.value().objects()[0], s_out.out_ray);
    //f32 pdf_val = pdf::cosine_pdf(hit_out.p, r.time(), hit_out.normal, s_out.out_ray);
    return emitted + s_out.attenuation * p_mat->scatter_pdf(r, hit_out, s_out.out_ray) * ray_trace(s_out.out_ray, background, objects, lights, depth - 1) / pdf_val;
    //return emitted + albedo * ray_trace(scattered, background, objects, lights, depth - 1);
    //return ysl::color::white;
}


u32 ysl_tracer::get_color(const ysl::vec3 &pixel_color) const
{
    f32 x = pixel_color.x();
    f32 y = pixel_color.y();
    f32 z = pixel_color.z();

    if (std::isnan(x)) x = 0.0f;
    if (std::isnan(y)) y = 0.0f;
    if (std::isnan(z)) z = 0.0f;

    x = ysl::pow(ysl::util::clamp(x / static_cast<f32>(m_samples_per_pixel), 0.0f, 1.0f), 1.0f / 2.2f);
    y = ysl::pow(ysl::util::clamp(y / static_cast<f32>(m_samples_per_pixel), 0.0f, 1.0f), 1.0f / 2.2f);
    z = ysl::pow(ysl::util::clamp(z / static_cast<f32>(m_samples_per_pixel), 0.0f, 1.0f), 1.0f / 2.2f);

    const u8 r = static_cast<u8>(x * 255.0f);
    const u8 g = static_cast<u8>(y * 255.0f);
    const u8 b = static_cast<u8>(z * 255.0f);

    return (0xff << 24) | (b << 16) | (g << 8) | r;
}


void ysl_tracer::write_to_file(const char *const file)
{
    stbi_write_png(file, m_data.width, m_data.height, 4, m_data.image_data, m_data.width * sizeof(u32));
}

void ysl_tracer::create_texture()
{
    glBindTexture(GL_TEXTURE_2D, m_texture_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_data.width, m_data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data.image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ysl_tracer::render_imgui_image()
{
    create_texture();
    //ImGui::Image((void *)(intptr_t)m_texture_data, ImVec2((f32)m_data.width, (f32)m_data.height));
    // Render to GLFW window
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture_data, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0,  m_data.width, m_data.height, 0, 0, m_data.width, m_data.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    //GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    //glDrawBuffers(1, draw_buffers);

}

f32 ysl_tracer::render_progress() const
{
    return m_progress.load();
}

bool ysl_tracer::is_running() const
{
    return m_is_running.load();
}

bool ysl_tracer::is_done() const
{
    return m_is_done.load();
}

f64 ysl_tracer::get_runtime() const
{
    return m_runtime.load();
}

bool ysl_tracer::should_gpu_run() const
{
    return m_should_gpu_run;
}

int& ysl_tracer::set_spp()
{
    return m_samples_per_pixel;
}

bool& ysl_tracer::set_denoise()
{
    return m_denoise_enabled;
}

bool& ysl_tracer::pause_light()
{
    return m_pause_light;
}

bool& ysl_tracer::set_metal_and_glass()
{
    return m_metal_and_glass;
}