#ShaderSpec
Name : Pbr;
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

struct DirectionalLight
{
	vec3 direction;
	vec3 color;

	float intensity;
	float bias;
};

struct Environment
{
	vec3 globalAmbient;
};

struct PointLight
{
	vec3 position;
	vec3 color;

	float intensity;
	float radius;
	float bias;
};

uniform Material u_Material;
uniform Environment u_Environment;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLight[12];
uniform int u_LightCount;
uniform int u_UsingParallax = 0;

//Testing
uniform vec3 u_CameraPosition;

const float PI = 3.14159265359;

vec3 FreshnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;

	return normalize(v_In.TBN * tangentNormal);
}

vec3 CalcDirLight(DirectionalLight light, vec3 V, vec3 N, vec3 F0, vec3 albedo_color, float metallic_color, float roughness_color)
{
	float shadow = 0;

	vec3 L = normalize(light.direction);
	vec3 H = normalize(V + L);

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness_color);
	float G = GeometrySmith(N, V, L, roughness_color);
	vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 nominator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = nominator / max(denominator, 0.001);

	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallic_color;
	float NdotL = max(dot(N, L), 0.0);

	return ((1.0 - shadow) * (kD * albedo_color / PI + specular) * vec3(1.0) * NdotL) * light.intensity * 5 * light.color;
}

vec3 CalcPointLight(PointLight light, vec3 V, vec3 N, vec3 F0, vec3 albedo, float metallic, float roughness)
{
	float dist = length(light.position - v_In.FragPos);

	if (dist > light.radius) return vec3(0);

	/////Per light radiance/////
	vec3 L = normalize(light.position - v_In.FragPos);
	vec3 H = normalize(V + L);

	float attenuation = clamp(1.0 - dist / light.radius, 0.0, 1.0); attenuation *= attenuation;
	vec3 radiance = vec3(1.0) * attenuation * light.intensity;
	////////////////////////////

	/////Cook-Torrance BRDF/////
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = FreshnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
	vec3 specular = numerator / denominator;
	////////////////////////////

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;

	float NdotL = max(dot(N, L), 0.0);
	return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
	vec3 albedo = pow(texture(u_Material.albedo, v_In.TexCoord).rgb, vec3(2.2));
	float metallic = texture(u_Material.mro, v_In.TexCoord).r;
	float roughness = texture(u_Material.mro, v_In.TexCoord).g;
	float ao = texture(u_Material.mro, v_In.TexCoord).b;

	vec3 N = CalculateNormal();
	vec3 V = normalize(u_CameraPosition - v_In.FragPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	Lo += CalcDirLight(u_DirectionalLight, V, N, F0, albedo, metallic, roughness);

	for (int i = 0; i < u_LightCount; i++)
	{
		Lo += CalcPointLight(u_PointLight[i], V, N, F0, albedo, metallic, roughness);
	}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	//HDR tonemapping
	color = color / (color + vec3(1.0));

	//gamma correction
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}