//None
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

out vec4 v_Color;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
	v_Color = a_Color;
}