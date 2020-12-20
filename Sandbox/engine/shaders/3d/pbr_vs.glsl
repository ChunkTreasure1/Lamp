//t
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_LightViewProjection;

void main()
{
	v_Out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.TexCoord = a_TexCoords;
	
	//TBN creation
    vec3 T = normalize(mat3(u_Model) * a_Tangent);
	vec3 B = normalize(mat3(u_Model) * a_Bitangent);
    vec3 N = normalize(mat3(u_Model) * a_Normal);

	v_Out.TBN = mat3(T, B, N);

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}