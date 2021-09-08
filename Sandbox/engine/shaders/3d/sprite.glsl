#ShaderSpec
Name: Sprite
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	v_TexCoords = a_TexCoords;

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
	FragColor = texture(u_Texture, v_TexCoords);
}