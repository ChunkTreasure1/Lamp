#ShaderSpec
Name : gbuffer;
TextureCount : 3;
TextureNames
{
albedo
normal
mro
}

#version 440 core
layout(location = 0) out vec4 o_Position;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Albedo;
layout(location = 3) out vec4 o_MRO;

in Out
{
	vec3 fragPos;
	vec3 normal;
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
	o_MRO.rgb = texture(u_Material.mro, v_In.texCoords).rgb;
	o_MRO.a = 1.0;

	o_Position.r = v_In.fragPos.x;
	o_Position.g = v_In.fragPos.y;
	o_Position.b = v_In.fragPos.z;
	o_Position.a = 1.0;

	o_Normal.rgb = CalculateNormal();
	o_Normal.a = 1.0;

	o_Albedo.rgb = texture(u_Material.albedo, v_In.texCoords).rgb;
	o_Albedo.a = 1.0;

}