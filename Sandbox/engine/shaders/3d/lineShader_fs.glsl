#ShaderSpec
Name: Line;
TextureCount: 0;
TextureNames:
{
}
#version 440 core
out vec4 FragColor;

in vec4 v_Color;

void main()
{
	FragColor = v_Color;
}