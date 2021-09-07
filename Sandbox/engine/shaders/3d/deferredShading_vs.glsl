//t
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out vec2 v_TexCoords;
out vec4 v_ShadowCoords;

uniform mat4 u_SunShadowVP;

void main()
{
	v_TexCoords = a_TexCoords;
	v_ShadowCoords = u_SunShadowVP * vec4(a_Position, 1.0);
	gl_Position = vec4(a_Position, 1.0);
}