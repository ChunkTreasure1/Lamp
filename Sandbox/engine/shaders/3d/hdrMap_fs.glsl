#ShaderSpec
Name: EqCubeHdr;
TextureCount: 1;
TextureNames:
{
hdrMap
}
#version 440 core
out vec4 FragColor;

in vec3 v_TexCoords;
uniform sampler2D u_HdrMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(v_TexCoords));
	vec3 color = texture(u_HdrMap, uv).rgb;

	FragColor = vec4(color, 1.0);
}