#ShaderSpec
Name: deferred;
TextureCount: 0;
TextureNames
{
}

#version 440 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

struct GBuffer
{
	sampler2D position;
	sampler2D normal;
	sampler2D albedo;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 color;

	float intensity;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	
	float radius;
	float intensity;
	float falloff;
	float farPlane;

	samplerCube shadowMap;
};

uniform GBuffer u_GBuffer;
uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLights[12];
uniform int u_LightCount;

uniform sampler2D u_ShadowMap;
uniform mat4 u_ShadowVP;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;
uniform sampler2D u_SSAO;

uniform vec3 u_CameraPosition;
uniform float u_Exposure;
uniform float u_Gamma;

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

float DirectionalShadowCalculation(vec4 pos)
{
	vec3 projCoords = pos.xyz / pos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float currentDepth = projCoords.z;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow = (0.5 + (shadow / 18));

	if (projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}


vec3 CalculateDirectionalLight(DirectionalLight light, vec3 V, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness, vec4 shadowCoord)
{
	float shadow = DirectionalShadowCalculation(shadowCoord);

	vec3 L = normalize(light.direction);
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

	vec3 lightStrength = ((1.0 - shadow) * (kD * albedo / PI + specular) * vec3(1.0) * NdotL) * light.intensity * light.color;
	return lightStrength;
}

float PointShadowCalculation(vec3 fragPos, PointLight light, float distance)
{
	vec3 fragToLight = fragPos - light.position;

	float currentDepth = length(fragToLight);

	float bias = 0.05;
	float shadow = 0.0;
	int samples = 20;
	float offset = 0.1;

	float diskRadius = (1.0 + (distance / light.farPlane)) / 25.0;

	for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(light.shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= light.farPlane;   
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

	shadow /= float(samples);

	return shadow;
}

vec3 CalculatePointLight(PointLight light, vec3 V, vec3 N, vec3 baseReflectivity, float metallic, float roughness, vec3 albedo, vec3 fragPos)
{
	float distance = length(light.position - fragPos);
	float shadow = PointShadowCalculation(fragPos, light, distance);

	if(distance > light.radius)
	{
		return vec3(0.0);
	}

	//Radiance
	vec3 L = normalize(light.position - fragPos);
	vec3 H = normalize(V + L);

	//Change to work with a falloff value
	float attenuation = clamp(1.0 - distance / light.radius, 0.0, 1.0);
	attenuation *= attenuation;

	vec3 radiance = (light.color * light.intensity) * attenuation;

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