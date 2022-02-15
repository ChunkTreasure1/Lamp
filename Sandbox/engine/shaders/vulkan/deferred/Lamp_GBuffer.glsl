#ShaderSpec
Name: gbuffer
TextureCount: 3
InternalShader: true
TextureNames
{
u_Albedo
u_Normal
u_MRO
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
    vec2 ambienceExposure;

} u_CameraData;

layout(std140, binding = 4) uniform DirectionalLightData
{
    mat4 viewProjections[10];
    uint count;
} u_DirectionalLightData;

layout (location = 0) out Out
{
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    mat3 TBN;
    vec4 shadowCoords;
} v_Out;

void main()
{
    vec4 worldPos = u_MeshData.model * vec4(a_Position, 1.0);
    v_Out.fragPos = worldPos.xyz;
    v_Out.texCoords = a_TexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(u_MeshData.model)));
    v_Out.normal = normalMatrix * a_Normal;

    //TBN creation
    vec3 T = normalize(vec3(u_MeshData.model * vec4(a_Tangent, 0.0)));
	vec3 B = normalize(vec3(u_MeshData.model * vec4(a_Bitangent, 0.0)));
    vec3 N = normalize(vec3(u_MeshData.model * vec4(a_Normal, 0.0)));

    v_Out.TBN = mat3(T, B, N);

    //Shadow calculation
    for (uint i = 0; i < u_DirectionalLightData.count; i++)
    {
        v_Out.shadowCoords = u_DirectionalLightData.viewProjections[i] * u_MeshData.model * vec4(a_Position, 1.0);
    }

    gl_Position = u_CameraData.projection * u_CameraData.view * worldPos;
}

#type fragment
#version 440

layout (location = 0) out vec4 o_PositionMetallic;
layout (location = 1) out vec4 o_Albedo;
layout (location = 2) out vec4 o_NormalRoughness;
layout (location = 3) out vec4 o_ShadowCoords;

//Per object
layout (set = 1, binding = 8) uniform sampler2D u_Albedo;
layout (set = 1, binding = 9) uniform sampler2D u_Normal;
layout (set = 1, binding = 10) uniform sampler2D u_MRO;

layout (push_constant) uniform MeshDataBuffer
{
    layout(offset = 64) vec4 albedoColor;
    vec4 normalColor;

    vec2 blendingUseBlending;
    vec2 metalRoughness;

    uint useAlbedo;
    uint useNormal;
    uint useMRO;
    uint useSkybox;

} u_MeshData;

layout (location = 0) in Out
{
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    mat3 TBN;
    vec4 shadowCoords;
} v_In;

vec3 CalculateNormal(vec3 normalVec)
{
    vec3 tangentNormal = normalVec * 2.0 - 1.0;
    return normalize(v_In.TBN * tangentNormal);
}

void main()
{
    vec4 albedo;
    vec3 normal;
    vec3 mro;
 
    if (bool(u_MeshData.useAlbedo))
    {
        albedo = texture(u_Albedo, v_In.texCoords); 
    }   
    else
    {
        albedo = u_MeshData.albedoColor;
    } 

    if (bool(u_MeshData.useNormal))
    {
        normal = CalculateNormal(texture(u_Normal, v_In.texCoords).rgb);
    }
    else
    {
        normal = u_MeshData.normalColor.rgb;
    }

    if (bool(u_MeshData.useMRO))
    {
        mro = texture(u_MRO, v_In.texCoords).rgb;
    }
    else
    {
        mro.x = u_MeshData.metalRoughness.x;
        mro.y = u_MeshData.metalRoughness.y;
    }

    o_PositionMetallic.xyz = v_In.fragPos;
    o_PositionMetallic.w = mro.x;

    o_Albedo = albedo;

    o_NormalRoughness.xyz = normal;
    o_NormalRoughness.w = mro.y;

    o_ShadowCoords = v_In.shadowCoords;
}