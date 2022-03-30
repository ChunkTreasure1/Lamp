#ShaderSpecBegin
shader:
  name: ssaoMain
  internal: true
#ShaderSpecEnd

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

#include "Common/Lamp_Common.glsl" 

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
layout (location = 0) out float o_Color;

#include "Common/Lamp_Common.glsl"

layout (location = 0) in Out
{
    vec2 texCoords;
    
} v_In;

layout (set = 0, binding = 4) uniform sampler2D u_NormalMap;
layout (set = 0, binding = 5) uniform sampler2D u_DepthMap;
layout (set = 0, binding = 6) uniform sampler2D u_Noise;

vec3 CalculateWorldCoords(vec2 coords)
{
    float depth = texture(u_DepthMap, coords).x;

    vec4 screenPos = vec4(coords * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpace = inverse(u_CameraData.projection) * screenPos;

    viewSpace /= viewSpace.w;

    vec4 worldSpace = inverse(u_CameraData.view) * viewSpace;
    return worldSpace.xyz;
}

void main()
{
    vec3 pos = CalculateWorldCoords(v_In.texCoords);
    vec2 noiseScale = vec2(u_ScreenData.screenSize.x / 4.0, u_ScreenData.screenSize.y / 4.0);

    vec3 normal = normalize(texture(u_NormalMap, v_In.texCoords).rgb);
    vec3 randomVec = normalize(texture(u_Noise, v_In.texCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 1.0;
    float positionDepth = (u_CameraData.view * vec4(pos, 1.0)).z;
    for (int i = 0; i < int(u_SSAOData.sizeBiasRadiusStrength.x); i++)
    {
        vec4 samplePos = u_CameraData.view * vec4(pos + TBN * u_SSAOData.kernelSamples[i].xyz * u_SSAOData.sizeBiasRadiusStrength.z, 1.0);

        vec4 offset = u_CameraData.projection * samplePos;
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        float sampleDepth = (u_CameraData.view * vec4(CalculateWorldCoords(offset.xy), 1.0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, u_SSAOData.sizeBiasRadiusStrength.z / abs(positionDepth - sampleDepth));
        occlusion -= samplePos.z + u_SSAOData.sizeBiasRadiusStrength.y < sampleDepth ? rangeCheck / int(u_SSAOData.sizeBiasRadiusStrength.x) : 0.0;
    }

    o_Color = occlusion;
}