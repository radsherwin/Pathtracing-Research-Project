#include "ysl_window.h"
#include <iostream>

ysl_window::~ysl_window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

ysl_window::ysl_window(i32 w, i32 h, const char *const title)
	: width(w), height(h)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (m_window == nullptr)
	{
		glfwTerminate();
		std::cerr << "Window is nullptr" << std::flush;
	}
	glfwMakeContextCurrent(m_window);

	// initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::flush;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(m_window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	
	//(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

}

bool ysl_window::should_close() const
{
	return glfwWindowShouldClose(m_window);
}

void ysl_window::begin_frame() const
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ysl_window::end_frame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	poll_and_swap_events();
}

// 00000000
void ysl_window::setup_imgui_contents(u8 mask) const
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui::SetNextWindowPos({0,0});
	//ImGui::SetNextWindowSize(io.DisplaySize);
	
	ImGui::Begin("Window text");
	ImGui::SetWindowFontScale(1.8f);
	ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
}