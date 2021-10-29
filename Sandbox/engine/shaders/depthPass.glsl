#ShaderSpec
Name: depthPass
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

out Out
{
	vec2 texCoords;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;

void main()
{
	v_Out.texCoords = a_TexCoords;

	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440

layout (location = 0) out vec4 o_Normal;

in Out
{
	vec2 texCoords;
	mat3 TBN;
} v_In;

struct Material
{
	sampler2D albedo;
	sampler2D normal;
	sampler2D mro;
};

uniform Material u_Material;

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Material.normal, v_In.texCoords).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

void main()
{
	o_Normal.rgb = CalculateNormal();
}