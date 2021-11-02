#ShaderSpec
Name: lightCulling
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type compute
#version 430

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

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

layout(std430, binding = 2) readonly buffer LightBuffer
{
    PointLight pointLights[];

} lightBuffer;

layout(std430, binding = 3) writeonly buffer VisibleLightsBuffer
{
    LightIndex data[];
	
} visibleIndices;

uniform sampler2D u_DepthMap;
uniform vec2 u_BufferSize;
uniform int u_LightCount;

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
        viewProjection = u_Projection * u_View;
    }

    barrier();

    //Calculate min and max depth values
    float maxDepth, minDepth;
    vec2 text = vec2(location) / u_BufferSize;

    float depth = texture(u_DepthMap, text).r;

    //Linearize depth value
    depth = (0.5 * u_Projection[3][2] / (depth + 0.5 * u_Projection[2][2] - 0.5));

    //Convert to uint to enable atomic comparisons between threads
    uint depthInt = floatBitsToUint(depth);
    atomicMin(minDepthInt, depthInt);
    atomicMax(maxDepthInt, depthInt);

    barrier();

    //Calculate frustum planes on one thread
    if (gl_LocalInvocationIndex == 0)
    {
        minDepth = uintBitsToFloat(minDepthInt);
        maxDepth = uintBitsToFloat(maxDepthInt);

        vec2 negativeStep = (2.0 * vec2(tileId)) / vec2(tileNumber);
        vec2 positiveStep = (2.0 * vec2(tileId + ivec2(1, 1))) / vec2(tileNumber);
        
         //Setup frustum planes
         frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); //Left
         frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); //Right
         frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); //Bottom
         frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); //Top
         frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); //Near
         frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); //Far

         //Transform LRBT planes
         for (uint i = 0; i < 4; i++)
         {
             frustumPlanes[i] *= viewProjection;
             frustumPlanes[i] /= length(frustumPlanes[i].xyz);
         }

         //Transform depth planes
         frustumPlanes[4] *= u_View;
         frustumPlanes[4] /= length(frustumPlanes[4].xyz);

         frustumPlanes[5] *= u_View;
         frustumPlanes[5] /= length(frustumPlanes[5].xyz);
    }

    barrier();

    //Cull lights using multiple threads
    uint threadCount = TILE_SIZE * TILE_SIZE;
    uint passCount = (1 + threadCount - 1) / threadCount;
    for (uint i = 0; i < passCount; i++)
    {
        //Get light index
        uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
        if (lightIndex >= u_LightCount)
        {
            break;
        }

        vec4 position = lightBuffer.pointLights[lightIndex].position;
        float radius = lightBuffer.pointLights[lightIndex].radius;

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
        for (uint i = 0; i < visibleLightCount; i++)
        {
            visibleIndices.data[offset + i].index = 1;//visibleLightIndices[i];
        }

        if (visibleLightCount != 1024)
        {
            visibleIndices.data[offset + visibleLightCount].index = -1;
        }
    }
}