#ShaderSpec
Name: deferred;
TextureCount: 3;
TextureNames
{
albedo
normal
mro
}

#version 440 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

struct GBuffer
{
	sampler2D positionAO;
	sampler2D normalMetallic;
	sampler2D albedoRoughness;
};

uniform GBuffer u_GBuffer;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

uniform vec3 u_CameraPosition;
uniform float u_Exposure;

const float PI = 3.14159265359;

float distributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotH * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return a2 / max(denom, 0.0000001);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 baseReflectivity, float roughness)
{
	return baseReflectivity + (max(vec3(1.0 - roughness), baseReflectivity) - baseReflectivity) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
	vec4 positionAO = texture(u_GBuffer.positionAO, v_TexCoords);
	vec4 normalMetallic = texture(u_GBuffer.normalMetallic, v_TexCoords);
	vec4 albedoRoughness = texture(u_GBuffer.albedoRoughness, v_TexCoords);

	vec3 fragPos = positionAO.rbg;
	vec3 normal = normalMetallic.rbg;
	vec3 albedo = pow(albedoRoughness.rbg, vec3(2.2));

	float metallic = normalMetallic.a;
	float roughness = albedoRoughness.a;
	float ao = positionAO.a;

	vec3 V = normalize(u_CameraPosition - fragPos);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0);

	vec3 R = reflect(-V, normal);

	const float maxReflectionLOD = 4.0;
	vec3 prefilterColor = textureLod(u_PrefilterMap, R, roughness * maxReflectionLOD).rgb;
	vec3 F = fresnelSchlickRoughness(max(dot(normal, V), 0.0), baseReflectivity, roughness);

	vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(normal, V), 0.0), roughness)).rg;
	vec3 specular = prefilterColor * (F * envBRDF.x + envBRDF.y);

	vec3 kS = fresnelSchlickRoughness(max(dot(normal, V), 0.0), baseReflectivity, roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
	vec3 diffuse = irradiance * albedo;
	vec3 ambient = (kD * diffuse + specular) * ao;

	vec3 color = ambient + Lo;

	//HDR tonemapping
	color = vec3(1.0) - exp(-color * u_Exposure);

	//Gamma correction
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}