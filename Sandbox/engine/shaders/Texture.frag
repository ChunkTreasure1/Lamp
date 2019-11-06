#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_ColorShade;

void main()
{
	color = texture(u_Texture, v_TexCoord) * u_ColorShade;
}