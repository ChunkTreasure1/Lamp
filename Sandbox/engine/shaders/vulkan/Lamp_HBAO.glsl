#ShaderSpec
Name: hbao
TextureCount: 0
InternalShader: true
TextureNames
{
}

//Based on Hazel HBAO implementation
#type compute
#version 450

layout (std140, binding = 4) uniform HBAODataBuffer
{
    vec4 perspectiveInfo;
    vec2 invQuarterResolution;
    
    float radiusToScreen;
    float negInvR2;
    float NdotVBias;
    float aoMultiplier;
    float powExponent;
    
    bool isOrtho;

    vec4 float2Offsets[16];
    vec4 jitters[16];

} u_HBAOData;

layout (binding = 0) uniform sampler2DArray u_LinearDepthTexArray;
layout (binding = 1) uniform sampler2D u_ViewNormalsTex;
layout (binding = 2) uniform sampler2D u_ViewPositionTex;
layout (binding = 3, rg16f) restrict writeonly uniform image2DArray o_Color;

//For performance
#pragma optionNV(unroll all)

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{

}