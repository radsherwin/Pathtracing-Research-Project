#ifndef YSL_RT_ENGINE_H
#define YSL_RT_ENGINE_H

#include "ysl_window.h"
#include "ysl_tracer.h"

class ysl_engine
{
public:
	ysl_engine() : m_window{}, m_tracer{} {};
	ysl_engine(const ysl_engine &) = delete;
	ysl_engine &operator=(const ysl_engine &) = delete;
	~ysl_engine() = default;

	ysl_engine(i32 w, i32 h, const char *const title);

	void run();

private:
	void begin_frame() const;
	void end_frame();
private:	
	ysl_window m_window;
	ysl_tracer m_tracer;
};

#endif