/////Structs/////
struct DirectionalLight
{
	vec4 direction;
	vec4 colorIntensity;
	bool castShadows;
};

struct PointLight
{
    vec4 position;
    vec4 color;

    float intensity;
    float radius;
    float falloff;
    float farPlane;
};

struct LightIndex
{
    int index;
};

/////Uniform buffers/////
layout(std140, binding = 0) uniform CameraDataBuffer
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
    uint pointLightCount;

} u_DirectionalLights;

layout (std140, binding = 2) uniform SSAODataBuffer
{
    vec4 kernelSamples[256];
    vec4 sizeBiasRadiusStrength;
    
} u_SSAOData;

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;
    uint xScreenTiles;

} u_ScreenData;

layout(std140, binding = 4) uniform DirectionalLightData
{
    mat4 viewProjections[10];
    uint count;
} u_DirectionalLightData;

layout (std430, binding = 12) readonly buffer LightBuffer
{
    PointLight lights[1024];
} u_LightBuffer;

layout (std430, binding = 13) readonly buffer VisibleLightsBuffer
{
    LightIndex data[];
} u_VisibleIndices;