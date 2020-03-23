#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
	v_TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
}