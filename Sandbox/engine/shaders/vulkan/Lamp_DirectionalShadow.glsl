#ShaderSpec
Name: directionalShadow
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type vertex
#version 440

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;
    vec4 albedoColor;
    vec4 normalColor;

    vec2 blendingUseBlending;
    vec2 metalRoughness;

    bool useAlbedo;
    bool useNormal;
    bool useMRO;
    bool useSkybox;

} u_MeshData;

layout(std140, binding = 0) uniform DirectionalShadowBuffer
{
	mat4 viewProjection;

} u_CameraData;

void main()
{
	gl_Position = u_CameraData.viewProjection * u_MeshData.model * vec4(a_Position, 1.0);
}

#type fragment
#version 440

layout (location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(0.0, 0.0, 0.0, 1.0);
}