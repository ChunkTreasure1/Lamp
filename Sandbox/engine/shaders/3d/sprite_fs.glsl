#ShaderSpec
Name: Sprite;
TextureCount: 0;
TextureNames
{
}
#version 440 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
	FragColor = texture(u_Texture, v_TexCoords);
}