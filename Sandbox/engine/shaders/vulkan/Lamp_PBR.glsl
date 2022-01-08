#ShaderSpec
Name: pbrForward
TextureCount: 3
InternalShader: false
TextureNames
{
u_Albedo
u_Normal
u_MRO
}

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;

} u_MeshData;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout(std140, binding = 4) uniform DirectionalLightData
{
    mat4 directionalLightVPs[10];
    uint count;
} u_DirectionalLightVPs;

layout (location = 0) out Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;    
    vec4 shadowCoords[10];
} v_Out;

void main()
{
    v_Out.fragPos = vec3(u_MeshData.model * vec4(a_Position, 1.0));
    v_Out.texCoord = a_TexCoords;
    v_Out.normal = a_Normal;

    //TBN creation
    vec3 T = normalize(vec3(u_MeshData.model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_MeshData.model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_MeshData.model * vec4(a_Normal, 0.0)));

    v_Out.TBN = mat3(T, B, N);

    //Shadow calculation
    for (uint i = 0; i < u_DirectionalLightVPs.count; i++)
    {
        v_Out.shadowCoords[i] = u_DirectionalLightVPs[i] * u_Model * vec4(a_Position, 1.0);
    }

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;

struct DirectionalLight
{
	vec4 direction;
	vec4 colorIntensity;
	bool castShadows;
};

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 1) uniform DirectionalLightBuffer
{
    DirectionalLight lights[1];
    uint count;

} u_DirectionalLights;

layout (push_constant) uniform MeshDataBuffer
{
    layout(offset = 64) vec2 blendingUseBlending;

} u_MeshData;


layout (location = 0) in Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;
    vec4 shadowCoords[10];    
} v_In;

layout (set = 0, binding = 5) uniform sampler2D u_Albedo;
layout (set = 0, binding = 6) uniform sampler2D u_Normal;
layout (set = 0, binding = 7) uniform sampler2D u_MRO;

layout (set = 0, binding = 8) uniform samplerCube u_IrradianceMap;
layout (set = 0, binding = 9) uniform samplerCube u_PrefilterMap;
layout (set = 0, binding = 10) uniform sampler2D u_BRDFLUT;

layout (set = 0, binding = 11) uniform samplerCube u_DirShadowMaps;

const vec3 globalDielectricBase = vec3(0.04);
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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 baseReflectivity, float roughness)
{
	return baseReflectivity + (max(vec3(1.0 - roughness), baseReflectivity) - baseReflectivity) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

float CalculateDirectionalShadow(uint lightIndex)
{
    vec3 pos = v_In.shadowCoords[lightIndex];

    vec3 projCoords = pos.xyz / pos.w;
    
    float closestDepth = texture(u_DirShadowMaps[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;

    const float bias = 0.05;
    float shadow = 0.0;

    vec2 texelSize = 1.0 / textureSize(u_DirShadowMaps[lightIndex], 0);
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
        
        }        
    }

}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness, uint lightIndex)
{
    float shadow = 0.0;
    if (light.castShadows)
    {
        shadow = 
    }

    vec3 lightDir = normalize(light.direction.xyz);
    vec3 H = normalize(dirToCamera + lightDir);

    // Cook-Torrance BRDF
	float NdotV = max(dot(normal, dirToCamera), 0.0000001);
	float NdotL = max(dot(normal, lightDir), 0.0000001);
	float HdotV	= max(dot(H, dirToCamera), 0.0);
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

vec3 CalculateNormal()
{
    vec3 tangentNormal = texture(u_Normal, v_In.texCoord).xyz * 2.0 - 1.0;
    return normalize(v_In.TBN * tangentNormal);
}

void main()
{
    vec4 albedoTex = texture(u_Albedo, v_In.texCoord);
    vec3 albedo = albedoTex.rgb;

    vec3 mro = texture(u_MRO, v_In.texCoord).rgb;
    float metallic = mro.x;
    float roughness = mro.y;
    float translucency = mro.z;

    vec3 normal = normalize(CalculateNormal());
    vec3 dirToCamera = normalize(u_CameraData.position.xyz - v_In.fragPos);

    vec3 baseReflectivity = mix(globalDielectricBase, albedo, metallic);
    vec3 lightAccumulation = vec3(0.0);

    lightAccumulation += CalculateDirectionalLight(u_DirectionalLights.lights[0], dirToCamera, normal, baseReflectivity, albedo, metallic, roughness, 0); //TODO: Setup using multiple instead

    vec3 reflectVec = reflect(-dirToCamera, normal);

    int maxReflectionLOD = textureQueryLevels(u_PrefilterMap);
    vec3 prefilterColor = textureLod(u_PrefilterMap, reflectVec, roughness * float(maxReflectionLOD)).rgb;
    vec3 F = fresnelSchlickRoughness(max(dot(normal, dirToCamera), 0.0), baseReflectivity, roughness);
    
    vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(normal, dirToCamera), 0.0), roughness)).rg;
    vec3 specular = prefilterColor * (F * envBRDF.x + envBRDF.y);

    vec3 kS = fresnelSchlickRoughness(max(dot(normal, dirToCamera), 0.0), baseReflectivity, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse + specular) * u_CameraData.ambienceExposure.x;

    vec3 color = ambient + lightAccumulation;
    color *= u_CameraData.ambienceExposure.y;

    color = ACESTonemap(color);

    float blendingVal = bool(u_MeshData.blendingUseBlending.y) ? albedoTex.a * u_MeshData.blendingUseBlending.x : 1.0;
    o_Color = vec4(color, blendingVal);
}