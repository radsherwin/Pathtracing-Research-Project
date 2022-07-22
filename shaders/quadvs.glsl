#version 430 core

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec2 vt;

out vec2 st;

void main()
{
	st = vt;
	gl_Position = vec4(vert_pos, 0.0, 1.0);
}