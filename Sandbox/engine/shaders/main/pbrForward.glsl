#ShaderSpec
Name: pbrForward
TextureCount: 3
TextureNames
{
albedo
normal
mro
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec3 u_CameraPosition;
	float padding;
};

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord;
	vec3 Normal;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;

void main()
{
	v_Out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.TexCoord = a_TexCoords;
	v_Out.Normal = a_Normal;

	//TBN creation
    vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	//Shadow calculation
	v_Out.ShadowCoord = u_ShadowVP * u_Model * vec4(a_Position, 1.0);

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int ObjectId;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord;
	vec3 Normal;
	mat3 TBN;
} v_In;

struct Material
{
	sampler2D albedo;
	sampler2D normal;
	sampler2D mro;
};

uniform Material u_Material;
uniform int u_ObjectId;

#include pbrBase.ext

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.TexCoord).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

vec4 CalculateForward()
{
	vec3 albedo = pow(texture(u_Material.albedo, v_In.TexCoord).rgb, vec3(u_Gamma));
	float metallic = texture(u_Material.mro, v_In.TexCoord).r;
	float roughness = texture(u_Material.mro, v_In.TexCoord).g;
	float ao = 0.5;

	vec3 N = normalize(CalculateNormal());

	return CalculateColor(albedo, metallic, roughness, ao, v_In.FragPos, v_In.ShadowCoord, N);
}

void main()
{
	vec4 color = CalculateForward();

	FragColor = color;
	ObjectId = u_ObjectId;
}