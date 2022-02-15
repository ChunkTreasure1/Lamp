#ShaderSpec
Name: composite
TextureCount: 0
InternalShader: true
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
layout (location = 1) out uint o_ObjectId;

in vec2 v_TexCoords;

uniform sampler2D u_Color;
uniform usampler2D u_ObjectId;
uniform sampler2D u_SSAOMap;

void main()
{
    vec4 color = texture(u_Color, v_TexCoords);
    uvec4 objTex = texture(u_ObjectId, v_TexCoords);

    float ssao = 1 - texture(u_SSAOMap, v_TexCoords).r;

    o_FinalColor = vec4(color.rgb - color.rgb * ssao, 1.0);

    o_ObjectId = objTex.r;
}