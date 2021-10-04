#ShaderSpec
Name: dirShadow
TextureCount: 3
TextureNames
{
}

#type vertex
#version 440
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec3 u_CameraPosition;
	float padding;
};

uniform mat4 u_Model;

void main()
{
	gl_Position = u_ShadowVP * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440

void main()
{}