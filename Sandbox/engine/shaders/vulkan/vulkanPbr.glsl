#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;    
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;
