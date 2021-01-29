#ShaderSpec
Name: testPbr;
TextureCount: 3;
TextureNames
{
albedo
normal
mro
}
#version 440 core
out layout(location = 0) vec4 FragColor;
out layout(location = 1) vec4 Color2;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord;
	vec3 Normal;
	mat3 TBN;
} v_In;

struct Material
{
	sampler2D albedo;
	sampler2D normal;
	sampler2D mro;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	
	float radius;
	float intensity;
	float falloff;
	float farPlane;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 color;

	float intensity;
};

uniform Material u_Material;

uniform PointLight u_PointLights[1];
uniform int u_LightCount;
uniform DirectionalLight u_DirectionalLight;

uniform vec3 u_CameraPosition;

//Bind the shadowmap to slot 0, 1, 2, 3, 4
uniform sampler2D u_ShadowMap;
uniform samplerCube u_TestPointMap;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

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

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

float DirectionalShadowCalculation(vec4 pos)
{
	vec3 projCoords = pos.xyz / pos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(v_In.Normal);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth > closestDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	if (projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

float PointShadowCalculation(vec3 fragPos, PointLight light)
{
	vec3 fragToLight = fragPos - light.position;
	float closestDepth = texture(u_TestPointMap, fragToLight).r;
	closestDepth *= light.farPlane;

	float currentDepth = length(fragToLight);

	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 V, vec3 N, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
	float shadow = DirectionalShadowCalculation(v_In.ShadowCoord);

	vec3 L = normalize(light.direction);
	vec3 H = normalize(V + L);

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

	vec3 lightStrength = ((1.0 - shadow) * (kD * albedo / PI + specular) * vec3(1.0) * NdotL) * light.intensity * light.color;
	return lightStrength;
}

vec3 CalculatePointLight(PointLight light, vec3 V, vec3 N, vec3 baseReflectivity, float metallic, float roughness, vec3 albedo)
{
	float shadow = PointShadowCalculation(v_In.FragPos, light);
	float distance = length(light.position - v_In.FragPos);
	if(distance > light.radius)
	{
		return vec3(0.0);
	}

	//Radiance
	vec3 L = normalize(light.position - v_In.FragPos);
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
	vec3 albedo = pow(texture(u_Material.albedo, v_In.TexCoord).rgb, vec3(2.2));
	float metallic = texture(u_Material.mro, v_In.TexCoord).r;
	float roughness = texture(u_Material.mro, v_In.TexCoord).g;
	float ao = texture(u_Material.mro, v_In.TexCoord).b;

	vec3 N = normalize(CalculateNormal());
	vec3 V = normalize(u_CameraPosition - v_In.FragPos);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0);

	Lo += CalculateDirectionalLight(u_DirectionalLight, V, N, baseReflectivity, albedo, metallic, roughness);

	for(int i = 0; i < u_LightCount; ++i)
	{
		Lo += CalculatePointLight(u_PointLights[i], V, N, baseReflectivity, metallic, roughness, albedo);
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
	vec3 ambient = (kD * diffuse + specular) * ao;
	//vec3 ambient = vec3(0.04) * albedo;

	vec3 color = ambient + Lo;

	//HDR tonemapping
	color = color / (color + vec3(1.0));

	//Gamma correction
	color = pow(color, vec3(1.0 / 2.2));
	FragColor = vec4(color, 1.0);
	Color2 = vec4(1.0, 0.0, 0.0, 1.0);
}