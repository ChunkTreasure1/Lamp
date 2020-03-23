#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;

// texture samplers
uniform sampler2D u_Texture;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = texture(u_Texture, v_TexCoord) * vec4(1.0, 1.0, 1.0, 1.0);
}