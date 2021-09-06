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
layout(location = 0) out vec4 o_PositionAO;
layout(location = 1) out vec4 o_NormalMetallic;
layout(location = 2) out vec4 o_AlbedoRoughness;

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
	vec3 mro = texture(u_Material.mro, v_In.texCoords).rgb;

	o_PositionAO.rgb = v_In.fragPos;
	o_PositionAO.a = mro.g;

	o_NormalMetallic.rgb = CalculateNormal();
	o_NormalMetallic.a = mro.r;

	o_AlbedoRoughness.rgb = texture(u_Material.albedo, v_In.texCoords).rgb;
	o_AlbedoRoughness.a = mro.b;
}