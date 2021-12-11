#ShaderSpec
Name: depthPrePass
TextureCount: 0
InternalShader: true
TextureNames
{
u_Normal
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

} u_MeshData;

layout(std140, binding = 0) uniform CameraDataBuffer
{
	mat4 view;
	mat4 projection;
	vec4 position;

} u_CameraData;

layout (location = 0) out Out
{
	vec2 texCoords;
	mat3 TBN;

} v_Out;

void main()
{
	v_Out.texCoords = a_TexCoords;

	vec3 T = normalize(vec3(u_MeshData.model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_MeshData.model * vec4(a_Bitangent, 0.0)));
	vec3 N = normalize(vec3(u_MeshData.model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * vec4(a_Position, 1.0);
	gl_Position.y = -gl_Position.y;
}

#type fragment
#version 440

layout (location = 0) out vec4 o_Normal;

layout (location = 0) in Out
{
	vec2 texCoords;
	mat3 TBN;
} v_In;

layout (set = 0, binding = 5) uniform sampler2D u_Normal;

vec3 CalculateNormal()
{
	vec3 tangentNormal = texture(u_Normal, v_In.texCoords).xyz * 2.0 - 1.0;
	return normalize(v_In.TBN * tangentNormal);
}

void main()
{
	o_Normal.rgb = CalculateNormal();
}