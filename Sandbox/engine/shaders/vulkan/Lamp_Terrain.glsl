#ShaderSpec
Name: terrain
TextureCount: 0
InternalShader: true
TextureNames
{
}

//Base on https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/glsl/terraintessellation
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

// float ScreenSpaceTessellationFactor(vec4 p0, vec4 p1)
// {
//     vec4 midPoint = 0.5 * (p0 + p1);
//     float radius = distance(p0, p1) / 2.0;

//     vec4 v0 = u_CameraData.view * midPoint;

//     vec4 clip0 = (u_CameraData.projection * (v0 - vec4(radius, vec3(0.0))));
//     vec4 clip1 = (u_CameraData.projection * (v0 + vec4(radius, vec3(0.0))));

//     clip0 /= clip0.w;
//     clip1 /= clip1.w;


// }

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16; // move to set tesselation factor using UBO
        gl_TessLevelOuter[3] = 16;

        gl_TessLevelInner[0] = 16;
        gl_TessLevelInner[1] = 16;
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

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (std140, binding = 1) uniform TerrainDataBuffer
{
    float heightMultiplier; 

} u_TerrainData;

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

    //Interpolate positions
    vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

    //Displace
    pos.y += textureLod(u_HeightMap, o_TexCoords, 0.0).r * u_TerrainData.heightMultiplier;

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * pos;
    o_Position = pos.xyz;
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 o_Normal;
layout(location = 1) in vec2 o_TexCoords;
layout(location = 2) in vec3 o_Position;

void main()
{
    o_Color = vec4(o_Position.y, o_Position.y, o_Position.y, 1.0);
}