//t
#version 440
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

uniform mat4 u_ShadowMVP;

void main()
{
	gl_Position = u_ShadowMVP * vec4(a_Position, 1.0);
}
