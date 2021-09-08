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
uniform mat4 u_ShadowVP;
uniform sampler2D u_SSAO;


void main()
{
	vec3 fragPos = texture(u_GBuffer.position, v_TexCoords).rgb;
	vec3 normal = texture(u_GBuffer.normal, v_TexCoords).rgb;
	vec3 albedo = pow(texture(u_GBuffer.albedo, v_TexCoords).rgb, vec3(u_Gamma));

	float metallic = texture(u_GBuffer.position, v_TexCoords).a;
	float roughness = texture(u_GBuffer.normal, v_TexCoords).a;
	float ao = texture(u_SSAO, v_TexCoords).r;

	vec3 V = normalize(u_CameraPosition - fragPos);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0);

	vec4 shadowCoord = u_ShadowVP * vec4(fragPos, 1.0);

	Lo += CalculateDirectionalLight(u_DirectionalLight, V, normal, baseReflectivity, albedo, metallic, roughness, shadowCoord);

	for(int i = 0; i < u_LightCount; ++i)
	{
		Lo += CalculatePointLight(u_PointLights[i], V, normal, baseReflectivity, metallic, roughness, albedo, fragPos);
	}

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
	color = pow(color, vec3(1.0 / u_Gamma));

	FragColor = vec4(color, 1.0);
}