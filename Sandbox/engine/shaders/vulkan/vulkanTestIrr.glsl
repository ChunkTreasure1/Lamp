#ShaderSpec
Name: vulkanTestIrr
TextureCount: 0
InternalShader: true
TextureNames:
{
}

#type vertex
#version 450
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;


void main()
{
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450
layout (location = 0) out vec4 o_Color;

void main()
{		
    o_Color = vec4(1.0);
}