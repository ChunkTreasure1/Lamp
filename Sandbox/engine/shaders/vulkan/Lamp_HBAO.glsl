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

#define PI 3.14159265
#define AO_RANDOM_TEX_SIZE 4

const float NUM_STEPS = 4;
const float NUM_DIRECTIONS = 8;

vec3 GetQuarterCoord(vec2 UV)
{
    return vec3(UV, float(gl_GlobalInvocationID.z));
}

vec3 UVToView(vec2 uv, float eye_z)
{
    return vec3((uv * u_HBAOData.perspectiveInfo.xy + u_HBAOData.perspectiveInfo.zw) * (u_HBAOData.isOrtho ? 1.0 : eye_z), eye_z);
}

vec3 FetchQuarterResViewPos(vec2 UV)
{
    float viewDepth = textureLod(u_LinearDepthTexArray, GetQuarterCoord(UV), 0).x;
    return UVToView(UV, viewDepth);
}

vec2 RotateDirection(vec2 dir, vec2 cosSin)
{
    return vec2(dir.x * cosSin.x - dir.y * cosSin.y, dir.x * cosSin.y + dir.y * cosSin.x);
}

vec4 GetJitter()
{
    return u_HBAOData.jitters[gl_GlobalInvocationID.z];
}

float Falloff(float distanceSquare)
{
    return distanceSquare * u_HBAOData.negInvR2 + 1.0;
}

// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
float ComputeAO(vec3 P, vec3 N, vec3 S)
{
    vec3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * 1.0 / sqrt(VdotV);

    return clamp(NdotV - u_HBAOData.NdotVBias, 0, 1) * clamp(Falloff(VdotV), 0, 1);
}

float ComputeCourseAO(vec2 fullResUv, float radiusPixel, vec4 rand, vec3 viewPos, vec3 viewNormal)
{
    radiusPixel /= 4.0;

    float stepSizePixels = radiusPixel / (NUM_STEPS + 1);

    const float alpha = 2.0 * PI / NUM_DIRECTIONS;
    float ao = 0;

    for (float directionIndex = 0; directionIndex < NUM_DIRECTIONS; ++directionIndex)
    {
        float angle = alpha * directionIndex;

        vec2 direction = RotateDirection(vec2(cos(angle), sin(angle)), rand.xy);

        float rayPixels = (rand.z * stepSizePixels + 1.0);
        for (float stepIndex = 0; stepIndex < NUM_STEPS; ++stepIndex)
        {
            vec2 snappedUV = round(rayPixels * direction) * u_HBAOData.invQuarterResolution + fullResUv;
            vec3 S = FetchQuarterResViewPos(snappedUV);
            rayPixels += stepSizePixels;

            ao += ComputeAO(viewPos, viewNormal, S);
        }
    }

    ao *= u_HBAOData.aoMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
    return clamp(1.0 - ao * 2.0, 0, 1);
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    vec2 float2Offset = u_HBAOData.float2Offsets[gl_GlobalInvocationID.z].xy;
    vec2 base = gl_GlobalInvocationID.xy * 4 + float2Offset;
    vec2 uv = base * u_HBAOData.invQuarterResolution / 4;

    vec3 viewPos = texelFetch(u_ViewPositionTex, ivec2(base), 0).xyz;
    viewPos.z = -viewPos.z;

    vec3 normalAndAO = texelFetch(u_ViewNormalsTex, ivec2(base), 0).xyz;
    vec3 viewNormal = -(normalAndAO.xyz * 2.0 - 1.0);

    float radiusPixel = u_HBAOData.radiusToScreen / (u_HBAOData.isOrtho ? 1.0 : viewPos.z);

    vec4 rand = GetJitter();

    float ao = ComputeCourseAO(uv, radiusPixel, rand, viewPos, viewNormal);

    imageStore(o_Color, ivec3(gl_GlobalInvocationID), vec4(pow(ao, u_HBAOData.powExponent), viewPos.z, 0.0, 1.0));
}