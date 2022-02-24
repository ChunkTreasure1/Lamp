#ShaderSpec
Name: ssdoMain
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out Out
{
    vec2 texCoords;

} v_Out;

void main()
{
    v_Out.texCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
#pragma optionNV(unroll all)

layout (location = 0) out vec4 o_Color;

layout (location = 0) in Out
{
    vec2 texCoords;
    
} v_In;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 2) uniform SSDODataBuffer
{
    vec4 kernelSamples[256];
    vec4 sizeBiasRadiusStrength;
    
} u_SSDOData;

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;
    uint xScreenTiles;

} u_ScreenData;

layout (set = 1, binding = 8) uniform sampler2D u_PositionMetallic;
layout (set = 1, binding = 9) uniform sampler2D u_NormalRoughness;
layout (set = 1, binding = 10) uniform sampler2D u_NoiseTexture;

const vec3 m_points[32] =
{
    vec3(-0.134, 0.044, -0.825),
    vec3(0.045, -0.431, -0.529),
    vec3(-0.537, 0.195, -0.371),
    vec3(0.525, -0.397, 0.713),
    vec3(0.895, 0.302, 0.139),
    vec3(-0.613, -0.408, -0.141),
    vec3(0.307, 0.822, 0.169),
    vec3(-0.819, 0.037, -0.388),
    vec3(0.376, 0.009, 0.193),
    vec3(-0.006, -0.103, -0.035),
    vec3(0.098, 0.393, 0.019),
    vec3(0.542, -0.218, -0.593),
    vec3(0.526, -0.183, 0.424),
    vec3(-0.529, -0.178, 0.684),
    vec3(0.066, -0.657, -0.570),
    vec3(-0.214, 0.288, 0.188),
    vec3(-0.689, -0.222, -0.192),
    vec3(-0.008, -0.212, -0.721),
    vec3(0.053, -0.863, 0.054),
    vec3(0.639, -0.558, 0.289),
    vec3(-0.255, 0.958, 0.099),
    vec3(-0.488, 0.473, -0.381),
    vec3(-0.592, -0.332, 0.137),
    vec3(0.080, 0.756, -0.494),
    vec3(-0.638, 0.319, 0.686),
    vec3(-0.663, 0.230, -0.634),
    vec3(0.235, -0.547, 0.664),
    vec3(0.164, -0.710, 0.086),
    vec3(-0.009, 0.493, -0.038),
    vec3(-0.322, 0.147, -0.105),
    vec3(-0.554, -0.725, 0.289),
    vec3(0.534, 0.157, -0.250),
};

const int m_numSamples = 32;

void main()
{
    vec2 noiseTextureSize = vec2(4, 4);
    vec3 centerPos = texture(u_PositionMetallic, v_In.texCoords).xyz;
    vec3 eyePos = u_CameraData.position.xyz;

    float centerDepth = distance(eyePos, centerPos);
    float radius = u_SSDOData.sizeBiasRadiusStrength.z / centerDepth;
    float maxDistanceInv = 1.0 / u_SSDOData.sizeBiasRadiusStrength.w;

    float attenuationAngleThreshold = 0.1;
    
    vec3 noise = texture(u_NoiseTexture, v_In.texCoords * u_ScreenData.screenSize.xy / noiseTextureSize).xyz * 2.0 - 1.0;
    vec3 centerNormal = texture(u_NormalRoughness, v_In.texCoords).xyz;

    vec4 sh2 = vec4(0.0);

    const float fudgeFactorL0 = 2.0;
    const float fudgeFactorL1 = 10.0;

    const float sh2WeightLO = fudgeFactorL0 * 0.28209;
    const vec3 sh2WeightL1 = vec3(fudgeFactorL1 * 0.48860);

    const vec4 sh2Weight = vec4(sh2WeightL1, sh2WeightLO) / m_numSamples;

    for (int i = 0; i < m_numSamples; ++i)
    {
        vec2 texOffset = reflect(m_points[i].xy, noise.xy).xy * radius;
        vec2 sampleTex = v_In.texCoords + texOffset;
        vec3 samplePos = textureLod(u_PositionMetallic, sampleTex, 0).xyz;
        vec3 centerToSample = samplePos - centerPos;
        
        float dist = length(centerToSample);
        vec3 centerToSampleNormalized = centerToSample / dist;
        float attenuation = 1.0 - clamp(dist * maxDistanceInv, 0.0, 1.0);
        float dp = dot(centerNormal, centerToSampleNormalized);

        attenuation = attenuation * attenuation * step(attenuationAngleThreshold, dp);
        sh2 += attenuation * sh2Weight * vec4(centerToSampleNormalized, 1.0);
    }

    o_Color = sh2;
}