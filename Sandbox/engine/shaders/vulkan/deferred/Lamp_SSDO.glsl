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

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;
    uint xScreenTiles;

} u_ScreenData;

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
layout (set = 1, binding = 11) uniform sampler2D u_DepthTexture;

void main()
{
    const float attenuationFactor = 1e-5;

    vec2 st = v_In.texCoords * u_ScreenData.screenSize;
    
    vec4 t1 = texture
}