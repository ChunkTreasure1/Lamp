//None
#version 460 core
layout (location = 0) in vec3 a_Position;

out vec3 v_TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	v_TexCoords = a_Position;
	vec4 pos = u_Projection * u_View * vec4(a_Position, 1.0);
	gl_Position = pos.xyww;
}