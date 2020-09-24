//None
#version 440 core
layout (location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;

vec3 gridPlane[6] = vec3[]
(
	vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main()
{
	gl_Position = u_ViewProjection * vec4(gridPlane[gl_VertexID].xyz, 1.0);
}
