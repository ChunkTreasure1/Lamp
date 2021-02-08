//t
#version 440 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_LocalPos;

void main()
{
	v_LocalPos = a_Position;

	mat4 rotView = mat4(mat3(u_View));
	vec4 clipPos = u_Projection * rotView * vec4(a_Position, 1.0);
	
	gl_Position = clipPos.xyww;
}