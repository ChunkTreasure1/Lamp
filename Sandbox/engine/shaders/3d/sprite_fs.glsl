#ShaderSpec
Name: Sprite;
TextureCount: 0;
TextureNames
{
}
#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int Color2;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;
uniform int u_ObjectId;

void main()
{
	FragColor = texture(u_Texture, v_TexCoords);
	Color2 = u_ObjectId;
}