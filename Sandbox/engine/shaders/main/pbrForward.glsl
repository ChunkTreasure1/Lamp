#ShaderSpec
Name: pbrForward
TextureCount: 3
InternalShader: false
TextureNames
{
albedo
normal
mro
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 view;
	mat4 projection;
	vec4 position;
} u_Camera;

layout(std140, binding = 4) uniform DirLightData
{
	mat4 dirLightVPs[10];
	int count;
} u_LightData;

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord[10];
	vec3 Normal;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;

void main()
{
	v_Out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.TexCoord = a_TexCoords;
	v_Out.Normal = a_Normal;


	//TBN creation
    vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	//Shadow calculation
	for(int i = 0; i < u_LightData.count; i++)
	{
		v_Out.ShadowCoord[i] = u_LightData.dirLightVPs[i] * u_Model * vec4(a_Position, 1.0);
	}

	gl_Position = u_Camera.projection * u_Camera.view * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectId;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord[10];
	vec3 Normal;
	mat3 TBN;
} v_In;

#include ../CommonDataStructures.h
#include ../UniformBlocks.h

uniform Material u_Material;
uniform int u_ObjectId;

uniform sampler2D u_DirShadowMaps[10];

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

uniform float u_Exposure;
uniform float u_Gamma;
uniform int u_TilesX;

const float PI = 3.14159265359;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

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

float PointShadowCalculation(vec3 fragPos, PointLight light, float distance)
{
	vec3 fragToLight = fragPos - light.position.xyz;

	float currentDepth = length(fragToLight);

	float bias = 0.05;
	float shadow = 0.0;
	int samples = 20;
	float offset = 0.1;

	//float diskRadius = (1.0 + (distance / light.farPlane)) / 25.0;

	for(int i = 0; i < samples; ++i)
    {
        float closestDepth = 0.0; //texture(light.shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
       // closestDepth *= light.farPlane;   
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

	shadow /= float(samples);

	return shadow;
}

vec3 CalculatePointLight(PointLight light, vec3 V, vec3 N, vec3 baseReflectivity, float metallic, float roughness, vec3 albedo, vec3 fragPos)
{
	float distance = length(light.position.xyz - fragPos);
	float shadow = 0.0; //PointShadowCalculation(fragPos, light, distance);

	if(distance > light.radius)
	{
		return vec3(0.0);
	}

	//Radiance
	vec3 L = normalize(light.position.xyz - fragPos);
	vec3 H = normalize(V + L);

	//Change to work with a falloff value
	float attenuation = clamp(1.0 - distance / light.radius, 0.0, 1.0);
	attenuation *= attenuation;

	vec3 radiance = (light.color.xyz * light.intensity) * attenuation;

	// Cook-Torrance BRDF
	float NdotV = max(dot(N, V), 0.0000001);
	float NdotL = max(dot(N, L), 0.0000001);
	float HdotV	= max(dot(H, V), 0.0);
	float NdotH = max(dot(N, H), 0.0);

	float d = distributionGGX(NdotH, roughness);
	float g = geometrySmith(NdotV, NdotL, roughness);
	vec3 f = fresnelSchlick(HdotV, baseReflectivity);

	vec3 specular = d * g * f;
	specular /= 4.0 * NdotV * NdotL;

	//Energy conservation
	vec3 kD = vec3(1.0) - f;
	kD *= 1.0 - metallic;

	vec3 lightStrength = ((1.0 - shadow) * (kD * albedo / PI + specular)) * radiance * NdotL;
	return lightStrength;	
}

float DirectionalShadowCalculation(vec4 pos, uint samplerId)
{
	vec3 projCoords = pos.xyz / pos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(u_DirShadowMaps[samplerId], projCoords.xy).r;
	float currentDepth = projCoords.z;

	const float bias = 0.05;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_DirShadowMaps[samplerId], 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(u_DirShadowMaps[samplerId], projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	if (projCoords.z > 1.0)
	{
		shadow = 0;
	}

	return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 V, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness, vec4 shadowCoord, uint samplerId)
{
	float shadow = DirectionalShadowCalculation(shadowCoord, samplerId);

	vec3 L = normalize(light.direction.xyz);
	vec3 H = normalize(V + L);

	// Cook-Torrance BRDF
	float NdotV = max(dot(normal, V), 0.0000001);
	float NdotL = max(dot(normal, L), 0.0000001);
	float HdotV	= max(dot(H, V), 0.0);
	float NdotH = max(dot(normal, H), 0.0);

	float d = distributionGGX(NdotH, roughness);
	float g = geometrySmith(NdotV, NdotL, roughness);
	vec3 f = fresnelSchlick(HdotV, baseReflectivity);

	vec3 specular = d * g * f;
	specular /= 4.0 * NdotV * NdotL;

	//Energy conservation
	vec3 kD = vec3(1.0) - f;
	kD *= 1.0 - metallic;

	vec3 lightStrength = ((1.0 - shadow) * (kD * albedo / PI + specular) * vec3(1.0) * NdotL) * light.colorIntensity.w * light.colorIntensity.xyz;
	return lightStrength; //SSSSTransmittance(translucency, u_SSSWidth, fragPos, normal, L, u_DirShadowMaps[samplerId], mat4(1), 1000);
}

vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 SSSSTransmittance(float translucency, float sssWidth, vec3 fragPos, vec3 normal, vec3 lightVector, sampler2D shadowMap, mat4 lightViewProjection, float lightFarPlane)
{
	float scale = 8.25 * (1.0 - translucency) / sssWidth;
	vec4 shrinkedPos = vec4(fragPos - 0.005 * normal, 1.0);

	vec4 shadowPos = lightViewProjection * shrinkedPos;
	float d1 = texture(shadowMap, shadowPos.xy / shadowPos.w).r;
	float d2 = shadowPos.z;

	d1 *= lightFarPlane;
	float d = scale * abs(d1 - d2);

	//Calculate color using the thickness
	float dd = -d * d;
	vec3 profile = vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
                     vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
                     vec3(0.118, 0.198, 0.0)   * exp(dd / 0.187)  +
                     vec3(0.113, 0.007, 0.007) * exp(dd / 0.567)  +
                     vec3(0.358, 0.004, 0.0)   * exp(dd / 1.99)   +
                     vec3(0.078, 0.0,   0.0)   * exp(dd / 7.41);

	return profile * clamp(0.3 + dot(lightVector, -normal), 0.0, 1.0);
}

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

void main()
{
	vec3 albedo = pow(texture(u_Material.albedo, v_In.TexCoord).rgb, vec3(u_Gamma));
	float metallic = texture(u_Material.mro, v_In.TexCoord).r;
	float roughness = texture(u_Material.mro, v_In.TexCoord).g;
	float translucency = texture(u_Material.mro, v_In.TexCoord).b;

	vec3 N = normalize(CalculateNormal());
	vec3 V = normalize(u_Camera.position.xyz - v_In.FragPos);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0);

	for (uint i = 0; i < u_DirectionalLights.count; i++)
	{
		Lo += CalculateDirectionalLight(u_DirectionalLights.lights[i], V, N, baseReflectivity, albedo, metallic, roughness, v_In.ShadowCoord[i], i);
	}

	//Calculate light indices
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileId = location / ivec2(16, 16);
	uint index = tileId.y * u_TilesX + tileId.x;
	
	uint offset = index * 1024;
	for(int i = 0; i < 1024 && u_VisibleIndices.data[offset + i].index != -1; ++i)
	{
		uint index = u_VisibleIndices.data[offset + i].index;
		Lo += CalculatePointLight(u_PointLights.lights[index], V, N, baseReflectivity, metallic, roughness, albedo, v_In.FragPos);
	}

	vec3 R = reflect(-V, N);
	
	const float maxReflectionLOD = 4.0;
	vec3 prefilterColor = textureLod(u_PrefilterMap, R, roughness * maxReflectionLOD).rgb;
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), baseReflectivity, roughness);
	vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilterColor * (F * envBRDF.x + envBRDF.y);
	
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), baseReflectivity, roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(u_IrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo;
	vec3 ambient = (kD * diffuse + specular);

	vec3 color = ambient + Lo;
	color *= u_Exposure;
	
	color = ACESTonemap(color);

	//Gamma correction
	color = pow(color, vec3(1.0 / u_Gamma));

	o_Color = vec4(color, 1.0);
	o_ObjectId = u_ObjectId;
}