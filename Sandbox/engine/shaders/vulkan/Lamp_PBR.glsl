#ShaderSpecBegin
shader:
  name: pbrForward
  internal: true
  textures:
  - texture: u_Albedo
    name: Albedo
    internal: false
  - texture: u_Normal
    name: Normal
    internal: false
  - texture: u_MRO
    name: MRO
    internal: false
  - texture: u_DetailNormal
    name: DetailNormal
    internal: false
#ShaderSpecEnd

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

#include "Common/Lamp_Common.glsl"

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;

} u_MeshData;

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
    for (uint i = 0; i < u_DirectionalLightData.count; i++)
    {
        v_Out.shadowCoords[i] = u_DirectionalLightData.viewProjections[i] * u_MeshData.model * vec4(a_Position, 1.0);
    }

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out uint o_Id;

#include "Common/Lamp_Common.glsl"

layout (push_constant) uniform MeshDataBuffer
{
    layout(offset = 64) vec4 albedoColor;
    vec3 normalColor;
    uint useDetailNormal;

    vec2 blendingUseBlending;
    vec2 metalRoughness;

    uint useAlbedo;
    uint useNormal;
    uint useMRO;
    uint id;

} u_MeshData;

layout (location = 0) in Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;
    vec4 shadowCoords[10];
} v_In;

//Per pass
layout (set = 0, binding = 5) uniform samplerCube u_IrradianceMap;
layout (set = 0, binding = 6) uniform samplerCube u_PrefilterMap;
layout (set = 0, binding = 7) uniform sampler2D u_BRDFLUT;

layout (set = 0, binding = 11) uniform sampler2DShadow u_DirShadowMaps[1];

//Per object
layout (set = 1, binding = 8) uniform sampler2D u_Albedo;
layout (set = 1, binding = 9) uniform sampler2D u_Normal;
layout (set = 1, binding = 10) uniform sampler2D u_MRO;
layout (set = 1, binding = 14) uniform sampler2D u_DetailNormal;

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

float InterleavedGradientNoise(vec2 vec)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * dot(vec, magic.xy));
}

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
    const float goldenAngle = 2.4;

    float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
    float theta = sampleIndex * goldenAngle * phi;

    return r * vec2(cos(theta), sin(theta));
}

float CalculateDirectionalShadow(uint lightIndex)
{
    vec4 pos = v_In.shadowCoords[lightIndex];

    vec3 projCoords = pos.xyz / pos.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = -projCoords.y;

    vec2 texelSize = 1.0 / textureSize(u_DirShadowMaps[lightIndex], 0);
    float shadow = 0.0;

    const float noise = InterleavedGradientNoise(gl_FragCoord.xy);

    for (int i = 0; i < 16; ++i)
    {
        vec2 sampleOffset = VogelDiskSample(i, 16, noise);
        shadow += texture(u_DirShadowMaps[lightIndex], vec3(projCoords.xy + sampleOffset * texelSize * 5.5, projCoords.z - 0.001)).x * (1.0 / 16.0);
    }

    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness, uint lightIndex)
{
    float shadow = 0.0;
    if (light.castShadows)
    {
        shadow = CalculateDirectionalShadow(lightIndex);
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

    vec3 lightStrength = (shadow * (kD * albedo / PI + specular) * vec3(1.0) * NdotL) * light.colorIntensity.w * light.colorIntensity.xyz;
    return lightStrength;
}

vec3 CalculatePointLight(PointLight light, vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, float metallic, float roughness, vec3 albedo, vec3 fragPos)
{

    //Radiance
    vec3 dirToLight = normalize(light.position.xyz - fragPos);
    float distance = length(light.position.xyz - fragPos);
    vec3 H = normalize(dirToCamera + dirToLight);

    float attenuation = clamp(1.0 - (distance * distance) / (light.radius * light.radius), 0.0, 1.0);
    attenuation *= mix(attenuation, 1.0, light.falloff);

    vec3 radiance = (light.color.xyz * light.intensity) * attenuation;

    //Cook-Torrance BRDF
    float NdotV = max(dot(normal, dirToCamera), 0.0000001);
    float NdotL = max(dot(normal, dirToLight), 0.0000001);
    float HdotV = max(dot(H, dirToCamera), 0.0);
    float NdotH = max(dot(normal, H), 0.0);

    float d = distributionGGX(NdotH, roughness);
	float g = geometrySmith(NdotV, NdotL, roughness);
	vec3 f = fresnelSchlick(HdotV, baseReflectivity);

    //Energy conservation
    vec3 kD = (vec3(1.0) - f) * (1.0 - metallic);

    vec3 specular = (d * g * f) / max(4.0 * NdotV * NdotL, 1e-6);
    specular = clamp(specular, vec3(0.0), vec3(10.0));

    vec3 diffuse = kD * albedo;

    return (diffuse + specular) * radiance * NdotL;
}

int GetLightBufferIndex(int i)
{
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileId = location / ivec2(16, 16);
    uint index = tileId.y * u_ScreenData.xScreenTiles + tileId.x;

    uint offset = index * 1024;

    return u_VisibleIndices.data[offset + i].index;
}

vec3 CalculatePointLights(vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
    vec3 result = vec3(0.0);
    for (int i = 0; i < u_DirectionalLights.pointLightCount; ++i)
    {
        uint lightIndex = GetLightBufferIndex(i);
        if (lightIndex == -1)
        {
            break;
        }

        result += CalculatePointLight(u_LightBuffer.lights[lightIndex], dirToCamera, normal, baseReflectivity, metallic, roughness, albedo, v_In.fragPos);
    }

    return result;
}

vec3 CalculateDirectionalLights(vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
    vec3 lightAccumulation = vec3(0.0);

    for(int i = 0; i < u_DirectionalLights.count; ++i)
    {
        lightAccumulation += CalculateDirectionalLight(u_DirectionalLights.lights[i], dirToCamera, normal, baseReflectivity, albedo, metallic, roughness, i);
    }

    return lightAccumulation;
}

vec3 CalculateAmbience(vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
    vec3 diffuse = albedo.xyz;
    vec3 specular;

    vec3 F = fresnelSchlickRoughness(max(dot(normal, dirToCamera), 0.0), baseReflectivity, roughness);

    vec3 reflectVec = reflect(-dirToCamera, normal);
    int maxReflectionLOD = textureQueryLevels(u_PrefilterMap);
    vec3 prefilterColor = textureLod(u_PrefilterMap, reflectVec, roughness * float(maxReflectionLOD)).rgb;

    vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(normal, dirToCamera), 0.0), roughness)).rg;
    vec3 irradiance = texture(u_IrradianceMap, normal).rgb;

    specular = prefilterColor * (F * envBRDF.x + envBRDF.y);
    diffuse *= irradiance;
   
    vec3 kS = fresnelSchlickRoughness(max(dot(normal, dirToCamera), 0.0), baseReflectivity, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 ambient = (kD * diffuse + specular) * u_CameraData.ambienceExposure.x;

    return ambient;
}

vec3 CalculateNormal(vec3 normalVec)
{
    vec3 tangentNormal = normalVec * 2.0 - 1.0;
    return normalize(v_In.TBN * tangentNormal);
}

vec3 BlendNormal(vec3 n1, vec3 n2)
{
	vec3 t = n1 + vec3(0.0, 0.0, 1.0);
	vec3 u = n2 * vec3(-1.0, -1.0, 1.0);
	vec3 r = (t / t.z) * dot(t, u) - u;
	return r;
}

void main()
{
    //Pre calculations
    vec4 albedo;
    vec3 normal;

    float metallic;
    float roughness;
    float translucency = 0.0;

    if (bool(u_MeshData.useAlbedo))
    {
        albedo = texture(u_Albedo, v_In.texCoord);
    }
    else
    {
        albedo = u_MeshData.albedoColor;
    }

    if (bool(u_MeshData.useNormal))
    {
        normal = CalculateNormal(texture(u_Normal, v_In.texCoord).rgb);
    }
    else
    {
        normal = normalize(u_MeshData.normalColor.rgb);
    }

    if (bool(u_MeshData.useDetailNormal))
    {
        // const vec3 tangentSpaceNormal = texture(u_DetailNormal, v_In.texCoord).xyz * 2.0 - 1.0; // add scale
        // normal = BlendNormal(normal, normalize(tangentSpaceNormal));
    }

    if (bool(u_MeshData.useMRO))
    {
        vec3 mro = texture(u_MRO, v_In.texCoord).rgb;
        metallic = mro.r;
        roughness = mro.g;
        translucency = mro.b;
    }
    else
    {
        metallic = u_MeshData.metalRoughness.x;
        roughness = u_MeshData.metalRoughness.y;
    }

    vec3 dirToCamera = normalize(u_CameraData.position.xyz - v_In.fragPos);
    vec3 baseReflectivity = mix(globalDielectricBase, albedo.xyz, metallic);
    vec3 lightAccumulation = vec3(0.0);

    lightAccumulation += CalculateDirectionalLights(dirToCamera, normal, baseReflectivity, albedo.xyz, metallic, roughness);
    lightAccumulation += CalculatePointLights(dirToCamera, normal, baseReflectivity, albedo.xyz, metallic, roughness);
    lightAccumulation += CalculateAmbience(dirToCamera, normal, baseReflectivity, albedo.xyz, metallic, roughness);
    
    vec3 color = lightAccumulation;
    color *= u_CameraData.ambienceExposure.y;
    color = ACESTonemap(color);

    float blendingVal = bool(u_MeshData.blendingUseBlending.y) ? albedo.a * u_MeshData.blendingUseBlending.x : 1.0;

    o_Color = vec4(0.0, 0.0, 0.0, 1.0);
    o_Id = u_MeshData.id;
}