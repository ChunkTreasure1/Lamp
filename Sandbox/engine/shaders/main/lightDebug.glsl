#ShaderSpec
Name: lightDebug
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec4 u_CameraPosition;
};

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord;
	vec3 Normal;
	mat3 TBN;
} v_Out;

uniform mat4 u_Model;

void main()
{
	v_Out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.TexCoord = a_TexCoords;
	v_Out.Normal = a_Normal;

	//TBN creation
    vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	v_Out.TBN = mat3(T, B, N);

	//Shadow calculation
	v_Out.ShadowCoord = u_ShadowVP * u_Model * vec4(a_Position, 1.0);

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core

layout(location = 0) out vec4 FragColor;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec4 ShadowCoord;
	vec3 Normal;
	mat3 TBN;
} v_In;

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

layout(std430, binding = 2) readonly buffer LightBuffer
{
    PointLight pointLights[1024];
    uint lightCount;

} lightBuffer;

layout(std430, binding = 3) readonly buffer VisibleLightsBuffer
{
    LightIndex data[];

} visibleIndices;

uniform int u_TotalX;
uniform int u_LightCount;

void main()
{
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileId = location / ivec2(16, 16);
	uint index = tileId.y * u_TotalX + tileId.x;
	
	uint offset = index * 1024;
	uint i;
	for (i = 0; i < 1024 && visibleIndices.data[offset + i].index != -1; i++);
	
	float ratio = float(i) / float(20);
	FragColor = vec4(vec3(ratio, ratio, ratio), 1.0);
}