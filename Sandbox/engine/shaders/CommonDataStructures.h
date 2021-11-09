struct DirectionalLight
{
	vec4 direction;
	vec4 colorIntensity;
};

struct PointLight
{
	vec4 position;
	vec4 color;
	
	float intensity;
	float radius;
	float falloff;
	float farPlane;
};

struct LightIndex
{
	int index;
};

struct Material
{
	sampler2D albedo;
	sampler2D normal;
	sampler2D mro;
};