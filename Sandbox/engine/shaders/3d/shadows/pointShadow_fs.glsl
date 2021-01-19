#ShaderSpec
Name : pointShadow;
TextureCount : 0;
TextureNames
{
}
#version 440 core
in vec4 FragPos;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

void main()
{
	float dist = length(FragPos.xyz - u_LightPos);
	dist = dist / u_FarPlane;

	gl_FragDepth = dist;
}