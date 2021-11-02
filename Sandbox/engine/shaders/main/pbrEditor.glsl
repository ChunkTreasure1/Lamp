#ShaderSpec
Name: pbrEditor
TextureCount: 3
InternalShader: true
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
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
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

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core

layout(location = 0) out vec4 FragColor;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec3 Normal;
	mat3 TBN;
} v_In;

struct Material
{
	sampler2D albedo;
	sampler2D normal;
	sampler2D mro;
};

uniform Material u_Material;

struct DirectionalLight
{
	vec4 direction;
	vec4 colorIntensity;
};

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

uniform DirectionalLight u_DirLight;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 V, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
	float shadow = 0;

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
	return lightStrength;
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

vec4 CalculateColor(vec3 albedo, float metallic, float roughness, float ao, vec3 fragPos, vec3 N)
{
	vec3 V = normalize(u_CameraPosition.xyz - fragPos);

	vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0);

	Lo += CalculateDirectionalLight(u_DirLight, V, N, baseReflectivity, albedo, metallic, roughness);

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

	vec3 color = ambient + Lo;
	color *= u_Exposure;
	
	color = ACESTonemap(color);

	//Gamma correction
	color = pow(color, vec3(1.0 / u_Gamma));

	return vec4(color, 1.0);
}

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

vec4 CalculateForward()
{
	vec3 albedo = pow(texture(u_Material.albedo, v_In.TexCoord).rgb, vec3(u_Gamma));
	float metallic = texture(u_Material.mro, v_In.TexCoord).r;
	float roughness = texture(u_Material.mro, v_In.TexCoord).g;
	float ao = 0.5;

	vec3 N = normalize(CalculateNormal());

	return CalculateColor(albedo, metallic, roughness, ao, v_In.FragPos, N);
}

void main()
{
	vec4 color = CalculateForward();

	FragColor = color;
}