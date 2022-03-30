#ShaderSpecBegin
shader:
  name: composite
  internal: true
#ShaderSpecEnd

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
    v_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout (location = 0) out vec4 o_FinalColor;

layout (location = 0) in vec2 v_TexCoords;

layout (set = 0, binding = 5) uniform sampler2D u_Color;

void main()
{
    vec4 color = texture(u_Color, v_TexCoords);

    o_FinalColor = vec4(color.rgb, 1.0);
}
