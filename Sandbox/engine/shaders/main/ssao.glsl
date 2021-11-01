#ShaderSpec
Name: SSAOMain
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec4 u_CameraPosition;
};

layout(std140, binding = 1) uniform SSAO
{
	vec4 u_Samples[64];
	int u_KernelSize;
	float u_Radius;
	float u_Bias;
};

in vec2 v_TexCoords;
in vec4 v_ShadowCoords;

struct GBuffer
{
	sampler2D position;
	sampler2D normal;
};

uniform GBuffer u_GBuffer;
uniform sampler2D u_Noise;
uniform vec2 u_BufferSize;

vec2 noiseScale;

void main()
{
	noiseScale = vec2(u_BufferSize.x / 4.0, u_BufferSize.y / 4.0);

	vec3 fragPos = texture(u_GBuffer.position, v_TexCoords).xyz;

	vec3 normal = normalize(texture(u_GBuffer.normal, v_TexCoords).rgb);
	vec3 randomVec = normalize(texture(u_Noise, v_TexCoords * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 1.0;
	float positionDepth = (u_View * vec4(fragPos, 1.0)).z;
	vec4 samplePos = u_View * vec4(fragPos + TBN * u_Samples[0].xyz * u_Radius, 1.0);

	vec4 offset = u_Projection * samplePos;
	offset.xyz /= offset.w;
	offset.xyz = offset.xyz * 0.5 + 0.5;

	vec3 sampleDepth = (u_View * vec4(texture(u_GBuffer.position, offset.xy).xyz, 1.0)).xyz;

	FragColor = vec4(sampleDepth, 1.0);
}