#ShaderSpec
Name: deferred
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

#include pbrBase.ext

struct GBuffer
{
	sampler2D position;
	sampler2D normal;
	sampler2D albedo;
};

uniform GBuffer u_GBuffer;
uniform sampler2D u_SSAO;


void main()
{
	vec3 fragPos = texture(u_GBuffer.position, v_TexCoords).rgb;
	vec3 normal = texture(u_GBuffer.normal, v_TexCoords).rgb;
	vec3 albedo = pow(texture(u_GBuffer.albedo, v_TexCoords).rgb, vec3(u_Gamma));

	float metallic = texture(u_GBuffer.position, v_TexCoords).a;
	float roughness = texture(u_GBuffer.normal, v_TexCoords).a;
	float ao = texture(u_SSAO, v_TexCoords).r;

	vec4 shadowCoord = u_ShadowVP * vec4(fragPos, 1.0);

	FragColor = CalculateColor(albedo, metallic, roughness, ao, fragPos, shadowCoord, normal);
}