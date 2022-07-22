#ifndef YSL_RT_WINDOW_H
#define YSL_RT_WINDOW_H

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ysl_math/types.h"

#include <utility> // std::pair

class ysl_window
{
public:
	
	ysl_window() : m_window{nullptr}, width(0), height(0){}
	ysl_window(const ysl_window &) = delete;
	ysl_window &operator=(const ysl_window &) = delete;
	~ysl_window();

	ysl_window(i32 w, i32 h, const char *const title);

	bool should_close() const;
	inline void poll_and_swap_events()
	{
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	void begin_frame() const;
	void end_frame();
	void setup_imgui_contents(u8 mask) const;

	inline std::pair<i32, i32> extents() const
	{
		return {width, height};
	}
private:
	GLFWwindow *m_window;
	i32 width;
	i32 height;

};

#endif
