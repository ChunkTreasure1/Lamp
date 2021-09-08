#ShaderSpec
Name: SSAOMain
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
layout(location = 0) out float FragColor;

in vec2 v_TexCoords;
in vec4 v_ShadowCoords;

struct GBuffer
{
	sampler2D position;
	sampler2D normal;
};

uniform GBuffer u_GBuffer;
uniform sampler2D u_Noise;

uniform vec3 u_Samples[256];
uniform mat4 u_Projection;
uniform mat4 u_View;
uniform vec2 u_BufferSize;

uniform int u_KernelSize;
uniform float u_Radius;
uniform float u_Bias;

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
	for (int i = 0; i < u_KernelSize; i++)
	{
		vec4 samplePos = u_View * vec4(fragPos + TBN * u_Samples[i] * u_Radius, 1.0);

		vec4 offset = u_Projection * samplePos;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = (u_View * vec4(texture(u_GBuffer.position, offset.xy).xyz, 1.0)).z;

		float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(positionDepth - sampleDepth));
		occlusion -= samplePos.z + u_Bias < sampleDepth ? rangeCheck / u_KernelSize : 0.0;
	}

	FragColor = occlusion;
}
