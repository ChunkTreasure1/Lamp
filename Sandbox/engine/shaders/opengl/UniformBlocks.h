layout(std140, binding = 0) uniform Main
{
	mat4 view;
	mat4 projection;
	vec4 position;
} u_Camera;

layout(std140, binding = 1) uniform DirectionalLights
{
	DirectionalLight lights[10];
	uint count;
} u_DirectionalLights;

layout(std430, binding = 2) readonly buffer LightBuffer
{
    PointLight lights[1024];
    uint count;

} u_PointLights;

layout(std430, binding = 3) readonly buffer VisibleLightsBuffer
{
    LightIndex data[];

} u_VisibleIndices;

layout(std140, binding = 4) uniform DirLightData
{
	mat4 dirLightVPs[10];
	int count;
} u_LightData;

layout(std140, binding = 5) uniform SSS
{
	float sssWidth;
} u_SSS;