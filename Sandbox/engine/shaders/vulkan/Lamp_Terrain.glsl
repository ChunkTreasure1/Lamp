#ShaderSpec
Name: terrain
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

layout (location = 0) out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
    v_TexCoords = a_TexCoords;
}

#type tesselationControl
#version 450 core

layout (location = 0, vertices = 4) out;
layout (location = 1) out vec2 v_TexCoords;

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