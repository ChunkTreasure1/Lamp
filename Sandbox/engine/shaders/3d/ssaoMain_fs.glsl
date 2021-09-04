#ShaderSpec
Name: ssaoMain;
TextureCount: 0;
TextureNames:
{
}
#version 440 core

layout(location = 0) out vec4 FragColor;

in Out
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
} v_In;

uniform sampler2D u_DepthMap;
uniform mat4 u_Projection;
uniform vec3 u_Samples[64];

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
	vec3 fragPos = v_In.position;
	vec3 normal = v_In.normal;

	mat3 TBN = v_In.TBN;

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; i++)
	{
		vec3 samplePos = TBN * u_Samples[i];
		samplePos = fragPos + samplePos * radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = u_Projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(u_DepthMap, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
}