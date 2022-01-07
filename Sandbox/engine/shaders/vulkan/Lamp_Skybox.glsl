#ShaderSpec
Name: skybox
TextureCount: 0
InternalShader: true
TextureNames:
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (location = 0) out vec3 v_LocalPos;

void main()
{
	v_LocalPos = a_Position;

	mat4 rotView = mat4(mat3(u_CameraData.view));
	vec4 clipPos = u_CameraData.projection * rotView * vec4(a_Position, 1.0);
	
	gl_Position = clipPos;
}

#type fragment
#version 440 core
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 v_LocalPos;

layout (binding = 1) uniform samplerCube u_EnvironmentMap;

layout (push_constant) uniform Uniforms
{
    float environmentLod;
    float environmentMultiplier;

} u_Uniforms;

void main()
{		
    vec4 envColor = textureLod(u_EnvironmentMap, v_LocalPos, float(u_Uniforms.environmentLod)) * u_Uniforms.environmentMultiplier;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec4(1.0));
    
    FragColor = vec4(envColor.xyz, 1.0);
}