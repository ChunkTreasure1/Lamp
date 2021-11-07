#ShaderSpec
Name: ssaoCompute
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type compute
#version 440

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

layout(std140, binding = 6) uniform SSAO
{
    vec4 u_KernelSamples[256];
    int u_KernelSize;
    float u_Bias;
    float u_Radius;
    float u_Strength;
};

uniform sampler2D u_NormalMap;
uniform sampler2D u_DepthMap;
uniform sampler2D u_Noise;

uniform vec2 u_BufferSize;

#define TILE_SIZE 16

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{

}