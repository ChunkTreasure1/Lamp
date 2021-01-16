#ShaderSpec
Name : testPbr;
TextureCount : 3;
TextureNames
{
albedo
normal
mro
}
#version 440 core
out vec4 FragColor;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
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
uniform samplerCube u_IrradianceMap;

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

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 V, vec3 N, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
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

	vec3 lightStrength = (kD * albedo / PI + specular) * vec3(1.0) * NdotL * light.intensity * light.color;
	return lightStrength;
}

vec3 CalculatePointLight(PointLight light, vec3 V, vec3 N, vec3 baseReflectivity, float metallic, float roughness, vec3 albedo)
{
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

	vec3 lightStrength = (kD * albedo / PI + specular) * radiance * NdotL;

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

	for(int i= 0; i < u_LightCount; ++i)
	{
		Lo += CalculatePointLight(u_PointLights[i], V, N, baseReflectivity, metallic, roughness, albedo);
	}

	//IBL
//	float NdotV = max(dot(N, V), 0.0000001);
//	vec3 F = fresnelSchlick(NdotV, baseReflectivity);
//	vec3 kD = (1.0 - F) * (1.0 - metallic);
//	vec3 diffuse = texture(u_IrradianceMap, N).rgb * albedo * kD;
	
	vec3 ambient = vec3(0.04) * albedo;
	vec3 color = ambient + Lo;


	//HDR tonemapping
	color = color / (color + vec3(1.0));

	//Gamma correction
	color = pow(color, vec3(1.0 / 2.2));
	FragColor = vec4(color, 1.0);
}