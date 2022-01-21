#ShaderSpec
Name: terrain
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
    v_TexCoords = a_TexCoords;
}

#type tessellationControl
#version 450

layout (vertices = 4) out;

layout (location = 1) out vec2 v_TexCoords[];

layout (location = 0) in vec2 a_TexCoords[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    v_TexCoords[gl_InvocationID] = a_TexCoords[gl_InvocationID];

    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16;
        gl_TessLevelOuter[3] = 16;

        gl_TessLevelInner[0] = 16;
        gl_TessLevelInner[1] = 16;
    }
}

#type tessellationEvaluation
#version 450

layout (quads, fractional_odd_spacing, ccw) in;
layout (location = 1) in vec2 v_TexCoords[];

layout (location = 0) out float o_Height;


layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (set = 0, location = 1) uniform sampler2D u_HeightMap;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = v_TexCoords[0];
    vec2 t01 = v_TexCoords[1];
    vec2 t02 = v_TexCoords[2];
    vec2 t03 = v_TexCoords[3];
}