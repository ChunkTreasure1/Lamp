#ShaderSpec
Name: pbrEditor
TextureCount: 3
InternalShader: false
TextureNames
{
u_Albedo
u_Normal
u_MRO
}

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;

} u_MeshData;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;

} u_CameraData;

layout (location = 1) out Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;    
} v_Out;

void main()
{
    v_Out.fragPos = vec3(u_MeshData.model * vec4(a_Position, 1.0));
    v_Out.texCoord = a_TexCoords;
    v_Out.normal = a_Normal;

    //TBN creation
    vec3 T = normalize(vec3(u_MeshData.model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_MeshData.model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_MeshData.model * vec4(a_Normal, 0.0)));

    v_Out.TBN = mat3(T, B, N);

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * vec4(a_Position, 1.0);
    gl_Position.y = -gl_Position.y;
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    vec4 position;
    mat4 view;
    mat4 projection;

} u_CameraData;

layout (std140, binding = 1) uniform DirectionalLightBuffer
{
    vec4 direction;
    vec4 colorIntensity;
} u_DirectionalLight;

layout (location = 1) in Out
{
    vec3 fragPos;
    vec2 texCoord;
    vec3 normal;
    mat3 TBN;    
} v_In;

layout (set = 0, binding = 2) uniform sampler2D u_Albedo;
layout (set = 0, binding = 3) uniform sampler2D u_Normal;
layout (set = 0, binding = 4) uniform sampler2D u_MRO;

const vec3 globalDielectricBase = vec3(0.04);
const float PI = 3.14159265359;

float distributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotH * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return a2 / max(denom, 0.0000001);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

	return ggx1 * ggx2;
}


vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

vec3 CalculateDirectionalLight(vec3 dirToCamera, vec3 normal, vec3 baseReflectivity, vec3 albedo, float metallic, float roughness)
{
    float shadow = 0.0;

    vec3 lightDir = normalize(u_DirectionalLight.direction.xyz);
    vec3 H = normalize(dirToCamera + lightDir);

    // Cook-Torrance BRDF
	float NdotV = max(dot(normal, dirToCamera), 0.0000001);
	float NdotL = max(dot(normal, lightDir), 0.0000001);
	float HdotV	= max(dot(H, dirToCamera), 0.0);
	float NdotH = max(dot(normal, H), 0.0);

	float d = distributionGGX(NdotH, roughness);
	float g = geometrySmith(NdotV, NdotL, roughness);
	vec3 f = fresnelSchlick(HdotV, baseReflectivity);

    vec3 specular = d * g * f;
	specular /= 4.0 * NdotV * NdotL;

	//Energy conservation
	vec3 kD = vec3(1.0) - f;
	kD *= 1.0 - metallic;

    vec3 lightStrength = ((1.0 - shadow) * (kD * albedo / PI + specular) * vec3(1.0) * NdotL) * u_DirectionalLight.colorIntensity.w * u_DirectionalLight.colorIntensity.xyz;
    return lightStrength;
}

vec3 CalculateNormal()
{
    vec3 tangentNormal = texture(u_Normal, v_In.texCoord).xyz * 2.0 - 1.0;
    return normalize(v_In.TBN * tangentNormal);
}

void main()
{
    vec4 albedoTex = texture(u_Albedo, v_In.texCoord);
    vec3 albedo = albedoTex.rgb;

    vec3 mro = texture(u_MRO, v_In.texCoord).rgb;
    float metallic = mro.x;
    float roughness = mro.y;
    float translucency = mro.z;

    vec3 normal = normalize(CalculateNormal());
    vec3 dirToCamera = normalize(u_CameraData.position.xyz - v_In.fragPos);

    vec3 baseReflectivity = mix(globalDielectricBase, albedo, metallic);
    vec3 lightAccumulation = vec3(0.0);

    lightAccumulation += CalculateDirectionalLight(dirToCamera, normal, baseReflectivity, albedo, metallic, roughness);

    vec3 color = lightAccumulation;

    color = pow(abs(color), vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}