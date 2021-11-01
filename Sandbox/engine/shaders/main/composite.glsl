#ShaderSpec
Name: composite
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout (location = 0) out vec4 o_FinalColor;

in vec2 v_TexCoords;

uniform sampler2D u_Color;
uniform sampler2D u_SSAOMap;
uniform float u_Exposure;

void main()
{
    vec3 color = texture(u_Color, v_TexCoords).rgb;
    float ssao = texture(u_SSAOMap, v_TexCoords).r;

    color = color - (vec4(0.5, 0.5, 0.5, 0.0) * ssao).rgb;
    color *= u_Exposure;

    color = pow(color, vec3(1.0 / 2.2));

    o_FinalColor = vec4(ssao, ssao, ssao, 1.0);
}