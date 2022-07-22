#include "ysl_engine.h"

ysl_engine::ysl_engine(i32 w, i32 h, const char *const title)
	: m_window{w, h, title}, m_tracer{}
{} 
int spp = 0;
void ysl_engine::run()
{
	while (!m_window.should_close())
	{
		m_window.begin_frame();

		m_window.setup_imgui_contents(0);

		if (ImGui::Button("Generate CPU"))
		{
			m_tracer.destroy();
			m_tracer.load_scene(3);
			m_tracer.launch_thread();
		}

		if (ImGui::Button("Generate GPU"))
		{	
			m_tracer.destroy();
			m_tracer.launch_gpu(1000, 1000);
		}

		ImGui::InputInt("SPP", &m_tracer.set_spp());
		ImGui::Checkbox("Denoise?", &m_tracer.set_denoise());
		ImGui::Checkbox("Metal and Glass", &m_tracer.set_metal_and_glass());

		if (m_tracer.should_gpu_run())
		{
			ImGui::Checkbox("Pause light", &m_tracer.pause_light());
			if (ImGui::Button("Stop GPU"))
			{
				m_tracer.end_gpu();
			}
			
			m_tracer.gpu_run();
		}

		if (m_tracer.is_running())
		{
			ImGui::ProgressBar(m_tracer.render_progress());
		}

		if (m_tracer.is_done())
		{
			ImGui::Text("Total rendering time: %.2fs", m_tracer.get_runtime());
			m_tracer.render_imgui_image();
		}
		
		ImGui::End();
		m_window.end_frame();
	}

}