//t
#version 440 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * view * vec4(aPos, 1.0);

	gl_Position = clipPos;
}