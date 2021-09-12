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

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec3 u_CameraPosition;
};

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main()
{
	v_TexCoords = a_TexCoords;

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
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