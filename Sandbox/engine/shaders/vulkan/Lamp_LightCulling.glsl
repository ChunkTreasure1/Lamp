#ShaderSpec
Name: lightCulling
TextureCount: 0
InternalShader: true
TextureNames
{
}

//Based on https://github.com/bcrusco/Forward-Plus-Renderer
#type compute
#version 450

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

layout(std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;
} u_CameraData;

layout(std140, binding = 1) uniform LightCullingBuffer
{
    vec2 screenSize;
    uint lightCount;
} u_LightCullingBuffer;

layout(std430, binding = 2) readonly buffer LightBuffer
{
    PointLight pointLights[];
} u_LightBuffer;

layout(std430, binding = 3) writeonly buffer VisibleLightsBuffer
{
    LightIndex data[];
} u_VisibleIndices;

layout(set = 0, binding = 4) uniform sampler2D u_DepthMap;

//Shared values between threads in group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];

//Shared local storage for visible indices
shared int visibleLightIndices[1024];
shared mat4 viewProjection;

#define TILE_SIZE 16

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
    ivec2 itemId = ivec2(gl_LocalInvocationID.xy);
    ivec2 tileId = ivec2(gl_WorkGroupID.xy);
    ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);

    uint index = tileId.y * tileNumber.x + tileId.x;

    if (gl_LocalInvocationIndex == 0)
    {
        minDepthInt = 0xFFFFFFFF;
        maxDepthInt = 0;

        visibleLightCount = 0;
        viewProjection = u_CameraData.projection * u_CameraData.view;
    }

    barrier();

    //Calculate min and max depth values
    vec2 texCoords = vec2(location) / u_LightCullingBuffer.screenSize;

    float depth = texture(u_DepthMap, texCoords).r;

    //Linearize depth value
    depth = u_CameraData.projection[3][2] / (depth + u_CameraData.projection[2][2]);

    //Convert to uint to enable atomic comparisons between threads
    uint depthInt = floatBitsToUint(depth);
    atomicMin(minDepthInt, depthInt);
    atomicMax(maxDepthInt, depthInt);

    barrier();

    //Calculate frustum planes on one thread
    if (gl_LocalInvocationIndex == 0)
    {
        float minDepth = uintBitsToFloat(minDepthInt);
        float maxDepth = uintBitsToFloat(maxDepthInt);

        vec2 negativeStep = (2.0 * vec2(tileId)) / vec2(tileNumber);
        vec2 positiveStep = (2.0 * vec2(tileId + ivec2(1, 1))) / vec2(tileNumber);

        //Setup frustum planes 
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

        //Transform LRTB planes
        for (uint i = 0; i < 4; i++)
        {
            frustumPlanes[i] *= viewProjection;
            frustumPlanes[i] /= length(frustumPlanes[i].xyz);
        }

        //Transform depth planes
        frustumPlanes[4] *= u_CameraData.view;
        frustumPlanes[4] /= length(frustumPlanes[4].xyz);

        frustumPlanes[5] *= u_CameraData.view;
        frustumPlanes[5] /= length(frustumPlanes[5].xyz);
    }

    barrier();

    //Cull lights using multiple threads
    uint threadCount = TILE_SIZE * TILE_SIZE;
    uint passCount = (u_LightCullingBuffer.lightCount + threadCount - 1) / threadCount;
    for (uint i = 0; i < passCount; i++)
    {
        uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
        if (lightIndex >= u_LightCullingBuffer.lightCount)
        {
            break;
        }

        vec4 position = u_LightBuffer.pointLights[lightIndex].position;
        float radius = u_LightBuffer.pointLights[lightIndex].radius;
        radius += radius * 0.3;

        //Check if light is in frustum
        float distance = 0.0;
        for (uint frustum = 0; frustum < 6; frustum++)
        {
            distance = dot(position, frustumPlanes[frustum]) + radius;

            if (distance <= 0.0)
            {
                break;
            }
        }

        //Light is visible add to buffer
        if (distance > 0.0)
        {
            uint offset = atomicAdd(visibleLightCount, 1);
            visibleLightIndices[offset] = int(lightIndex);
        }
    }

    barrier();

    //Fill light buffer, done by one thread
    if (gl_LocalInvocationIndex == 0)
    {
        uint offset = index * 1024;
        for(uint i = 0; i < visibleLightCount; i++)
        {
            u_VisibleIndices.data[offset + i].index = visibleLightIndices[i];
        }

        if (visibleLightCount != 1024)
        {
            u_VisibleIndices.data[offset + visibleLightCount].index = -1;
        }
    }
}