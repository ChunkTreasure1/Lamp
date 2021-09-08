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
layout(location = 0) out vec4 o_PositionMetal;
layout(location = 1) out vec4 o_NormalRough;
layout(location = 2) out vec4 o_AlbedoAO;

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

	o_PositionMetal.rgb = v_In.fragPos;
	o_PositionMetal.a = mro.r;

	o_NormalRough.rgb = CalculateNormal();
	o_NormalRough.a = mro.g;

	o_AlbedoAO.rgb = texture(u_Material.albedo, v_In.texCoords).rgb;
	o_AlbedoAO.a = mro.b;

}