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
layout(location = 1) out vec4 o_NormalMetallic;
layout(location = 2) out vec4 o_AlbedoRoughness;

in Out
{
	vec3 fragPos;
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

	o_Position = vec4(v_In.fragPos, 1.0);

	o_NormalMetallic.rgb = CalculateNormal();
	o_NormalMetallic.a = mro.r;

	o_AlbedoRoughness.rgb = texture(u_Material.albedo, v_In.texCoords).rgb;
	o_AlbedoRoughness.a = mro.b;
}