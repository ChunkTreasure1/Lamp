#ShaderSpec
Name: ssaoCompute
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type compute
#version 450

layout (set = 0, binding = 4) uniform sampler2D u_NormalTexture;
layout (set = 0, binding = 5) uniform sampler2D u_DepthTexture;
layout (set = 0, binding = 6) uniform sampler2D u_NoiseTexture;

layout (binding = 7, rg32f) restrict writeonly uniform image2D o_Color;

layout(std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 1) uniform SSAODataBuffer
{
    vec4 kernelSamples[256];
    vec4 sizeBiasRadiusStrength;
    
} u_SSAOData;

layout(std140, binding = 2) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;

} u_ScreenData;

vec3 CalculateWorldCoords(vec2 coords)
{
    float depth = texture(u_DepthTexture, coords).x;

    vec4 screenPos = vec4(coords * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpace = inverse(u_CameraData.projection) * screenPos;

    viewSpace /= viewSpace.w;

    vec4 worldSpace = inverse(u_CameraData.view) * viewSpace;
    return worldSpace.xyz;
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    vec2 texPos = gl_GlobalInvocationID.xy;
    vec2 uv = texPos / u_ScreenData.screenSize;

    vec3 pos = CalculateWorldCoords(uv);
    vec2 noiseScale = vec2(u_ScreenData.screenSize.x / 4.0, u_ScreenData.screenSize.y / 4.0);

    vec3 normal = normalize(texture(u_NormalTexture, uv).rgb);
    vec3 randomVec = normalize(texture(u_NoiseTexture, uv * noiseScale).rgb);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 1.0;
    float positionDepth = (u_CameraData.view * vec4(pos, 1.0)).z;
    for(int i = 0; i < int(u_SSAOData.sizeBiasRadiusStrength.x); ++i)
    {
        vec4 samplePos = u_CameraData.view * vec4(pos + TBN * u_SSAOData.kernelSamples[i].xyz * u_SSAOData.sizeBiasRadiusStrength.z, 1.0);
    
        vec4 offset = u_CameraData.projection * samplePos;
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        float sampleDepth = (u_CameraData.view * vec4(CalculateWorldCoords(offset.xy), 1.0)).z;
        float rangeCheck = smoothstep(0.0, 1.0, u_SSAOData.sizeBiasRadiusStrength.z / abs(positionDepth - sampleDepth));

        occlusion -= samplePos.z + u_SSAOData.sizeBiasRadiusStrength.y < sampleDepth ? rangeCheck / int(u_SSAOData.sizeBiasRadiusStrength.x) : 0.0;
    }

    imageStore(o_Color, ivec2(texPos), vec4(uv.x, uv.y, 0, 1));
}