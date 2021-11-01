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

uniform sampler2D u_Ambient;
uniform sampler2D u_Light;
uniform sampler2D u_SSAOMap;
uniform float u_Exposure;

void main()
{
    vec3 ambient = texture(u_Ambient, v_TexCoords).rgb;
    vec3 light = texture(u_Light, v_TexCoords).rgb;
    float ssao = texture(u_SSAOMap, v_TexCoords).r;

    vec3 color = ambient * 0.5 + light;
    color *= u_Exposure;

    o_FinalColor = vec4(ambient * ssao, 1.0);
}