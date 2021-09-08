//t
#version 440 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoords;

out Out
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	v_Out.fragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.texCoords = a_TexCoords;
	v_Out.normal = a_Normal;

	//TBN creation
    vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}