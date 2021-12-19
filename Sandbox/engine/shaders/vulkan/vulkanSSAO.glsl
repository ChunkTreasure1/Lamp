#ShaderSpec
Name: ssaoMain
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
    vec2 viewRay;

} v_Out;

layout(std140, binding = 0) uniform CameraDataBuffer
{
	mat4 view;
	mat4 projection;
	vec4 positionAndTanHalfFOV;

} u_CameraData;

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;

} u_ScreenData;

void main()
{
    v_Out.texCoords = a_TexCoords;
    v_Out.viewRay.x = a_Position.x * u_ScreenData.aspectRatio * u_CameraData.positionAndTanHalfFOV.w;
    v_Out.viewRay.y = a_Position.y * u_CameraData.positionAndTanHalfFOV.w;

    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout (location = 0) out float o_Color;

layout(std140, binding = 0) uniform CameraDataBuffer
{
	mat4 view;
	mat4 projection;
	vec4 positionAndTanHalfFOV;

} u_CameraData;

layout (std140, binding = 2) uniform SSAODataBuffer
{
    vec4 kernelSamples[256];
    vec4 sizeBiasRadiusStrength;
    
} u_SSAOData;

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;

} u_ScreenData;

layout (location = 0) in Out
{
    vec2 texCoords;
    vec2 viewRay;
    
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
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = (u_CameraData.view * vec4(CalculateWorldCoords(offset.xy), 1.0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, u_SSAOData.sizeBiasRadiusStrength.z / abs(positionDepth - sampleDepth));
        occlusion -= samplePos.z + u_SSAOData.sizeBiasRadiusStrength.y < sampleDepth ? rangeCheck / int(u_SSAOData.sizeBiasRadiusStrength.x) : 0.0;
    }

    if(occlusion < 1.0)
    {
        float invStrength = 1.0 - u_SSAOData.sizeBiasRadiusStrength.w;
        occlusion += invStrength;
        occlusion = clamp(occlusion, 0.0, 1.0);
    }

    o_Color = occlusion;
}