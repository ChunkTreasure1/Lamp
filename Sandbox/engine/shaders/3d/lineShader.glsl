#ShaderSpec
Name: Line
TextureCount: 0
TextureNames:
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

out vec4 v_Color;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec3 u_CameraPosition;
};

void main()
{
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
	v_Color = a_Color;
}

#type fragment
#version 440 core
out vec4 FragColor;

in vec4 v_Color;

void main()
{
	FragColor = v_Color;
}