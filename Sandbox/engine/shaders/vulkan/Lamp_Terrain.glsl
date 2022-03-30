#ShaderSpecBegin
shader:
  name: terrain
  internal: true
#ShaderSpecEnd

//Based on https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/glsl/terraintessellation
#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out vec2 o_TexCoords;
layout (location = 1) out vec3 o_Normal;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
    o_TexCoords = a_TexCoords;
    o_Normal = a_Normal;
}

#type tessellationControl
#version 450

layout (vertices = 4) out;

layout (location = 0) in vec2 a_TexCoords[];
layout (location = 1) in vec3 a_Normal[];

layout (location = 0) out vec2 o_TexCoords[4];
layout (location = 1) out vec3 o_Normal[4];

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;
    uint xScreenTiles;
} u_ScreenData;

layout (std140, binding = 1) uniform TerrainDataBuffer
{
    vec4 frustumPlanes[6];
    float tessellatedEdgeSize;
    float tessellationFactor;
    float displacementFactor;

} u_TerrainData;

layout (set = 0, binding = 2) uniform sampler2D u_HeightMap;

float ScreenSpaceTessellationFactor(vec4 p0, vec4 p1)
{
    vec4 midPoint = 0.5 * (p0 + p1);
    float radius = distance(p0, p1) / 2.0;

    //View space
    vec4 v0 = u_CameraData.view * midPoint;

    //Clip space
    vec4 clip0 = (u_CameraData.projection * (v0 - vec4(radius, vec3(0.0))));
    vec4 clip1 = (u_CameraData.projection * (v0 + vec4(radius, vec3(0.0))));

    //NDC
    clip0 /= clip0.w;
    clip1 /= clip1.w;

    //Viewport
    clip0.xy *= u_ScreenData.screenSize;
    clip1.xy *= u_ScreenData.screenSize;

    return clamp(distance(clip0, clip1) / u_TerrainData.tessellatedEdgeSize * u_TerrainData.tessellationFactor, 1.0, 64.0);
}

bool FrustumCheck()
{
    const float radius = 8.0;
    vec4 pos = gl_in[gl_InvocationID].gl_Position;
    pos.y += textureLod(u_HeightMap, a_TexCoords[0], 0.0).r * u_TerrainData.displacementFactor;

    for (int i = 0; i < 6; i++)
    {
        if (dot(pos, u_TerrainData.frustumPlanes[i]) + radius < 0.0)
        {
            return false;
        }
    }

    return true;
}

void main()
{
    if(gl_InvocationID == 0)
    {
        if (!FrustumCheck())
        {
            gl_TessLevelOuter[0] = 1;
            gl_TessLevelOuter[1] = 1;
            gl_TessLevelOuter[2] = 1;
            gl_TessLevelOuter[3] = 1;

            gl_TessLevelInner[0] = 1;
            gl_TessLevelInner[1] = 1;
        }
        else
        {
            if (u_TerrainData.tessellationFactor > 0)
            {
                gl_TessLevelOuter[0] = ScreenSpaceTessellationFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
                gl_TessLevelOuter[1] = ScreenSpaceTessellationFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
                gl_TessLevelOuter[2] = ScreenSpaceTessellationFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
                gl_TessLevelOuter[3] = ScreenSpaceTessellationFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);

                gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
                gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
            }
            else
            {
                gl_TessLevelOuter[0] = 1;
                gl_TessLevelOuter[1] = 1;
                gl_TessLevelOuter[2] = 1;
                gl_TessLevelOuter[3] = 1;

                gl_TessLevelInner[0] = 1;
                gl_TessLevelInner[1] = 1;
            }
        }
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    o_TexCoords[gl_InvocationID] = a_TexCoords[gl_InvocationID];
    o_Normal[gl_InvocationID] = a_Normal[gl_InvocationID];
}

#type tessellationEvaluation
#version 450

layout (quads, equal_spacing, ccw) in;

layout (location = 0) in vec2 a_TexCoords[];
layout (location = 1) in vec3 a_Normal[];

layout (location = 0) out vec3 o_Normal;
layout (location = 1) out vec2 o_TexCoords;
layout (location = 2) out vec3 o_Position;
layout (location = 3) out vec4 o_ShadowCoords[10];

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 1) uniform TerrainDataBuffer
{
    vec4 frustumPlanes[6];
    float tessellatedEdgeSize;
    float tessellationFactor;
    float displacementFactor;

} u_TerrainData;

layout(std140, binding = 4) uniform DirectionalLightData
{
    mat4 viewProjections[10];
    uint count;
} u_DirectionalLightData;

layout (set = 0, binding = 2) uniform sampler2D u_HeightMap;

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;

} u_MeshData;

void main()
{
    //Interpolate UVs
    vec2 uv1 = mix(a_TexCoords[0], a_TexCoords[1], gl_TessCoord.x);
    vec2 uv2 = mix(a_TexCoords[3], a_TexCoords[2], gl_TessCoord.x);
    o_TexCoords = mix(uv1, uv2, gl_TessCoord.y);

    //Interpolate normals
    vec3 n1 = mix(a_Normal[0], a_Normal[1], gl_TessCoord.x);
    vec3 n2 = mix(a_Normal[3], a_Normal[2], gl_TessCoord.x);
    o_Normal = mix(n1, n2, gl_TessCoord.y);

    //Interpolate positions
    vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

    //Displace
    pos.y += textureLod(u_HeightMap, o_TexCoords, 0.0).r * u_TerrainData.displacementFactor;

    for (uint i = 0; i < u_DirectionalLightData.count; i++)
    {
        o_ShadowCoords[i] = u_DirectionalLightData.viewProjections[i] * pos;
    }

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * pos;
    o_Position = pos.xyz;
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 o_Normal;
layout(location = 1) in vec2 o_TexCoords;
layout(location = 2) in vec3 o_Position;
layout(location = 3) in vec4 o_ShadowCoords[10];

struct DirectionalLight
{
	vec4 direction;
	vec4 colorIntensity;
	bool castShadows;
};

layout (std140, binding = 1) uniform DirectionalLightBuffer
{
    DirectionalLight lights[1];
    uint count;
    uint pointLightCount;

} u_DirectionalLights;

//Per pass
layout (set = 0, binding = 5) uniform samplerCube u_IrradianceMap;
layout (set = 0, binding = 6) uniform samplerCube u_PrefilterMap;
layout (set = 0, binding = 7) uniform sampler2D u_BRDFLUT;

layout (set = 0, binding = 11) uniform sampler2DShadow u_DirShadowMaps[1];

void main()
{
    o_Color = vec4(o_Position.y, o_Position.y, o_Position.y, 1.0);
}