#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoords;

layout(location = 1) out vec2 v_TexCoords;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 projection;
} u_UBO;

void main()
{
	gl_Position = u_UBO.projection * u_UBO.view * u_UBO.model * vec4(a_Position, 1.0);
	gl_Position.y = -gl_Position.y;
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 1) in vec2 v_TexCoords;
layout(binding = 1) uniform sampler2D u_Sampler;

void main()
{
    o_Color = texture(u_Sampler, v_TexCoords);
}