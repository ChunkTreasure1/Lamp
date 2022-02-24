 #ShaderSpec
Name: ssdoMain
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out Out
{
    vec2 texCoords;

} v_Out;

void main()
{
    v_Out.texCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;

layout (location = 0) in Out
{
    vec2 texCoords;
    
} v_In;

layout(std140, binding = 3) uniform ScreenDataBuffer
{
    vec2 screenSize;
    float aspectRatio;
    uint xScreenTiles;

} u_ScreenData;

layout (set = 1, binding = 8) uniform sampler2D u_NormalRoughness;

float m_weights[9] =
{
	0.013519569015984728,
	0.047662179108871855,
	0.11723004402070096,
	0.20116755999375591,
	0.240841295721373,
	0.20116755999375591,
	0.11723004402070096,
	0.047662179108871855,
	0.013519569015984728
};

float m_indices[9] =
{
    -4,
    -3,
    -2,
    -1,
    0,
    1,
    2,
    3,
    4
};

void main()
{
    vec2 step = vec2(0.0, 1.0) / u_ScreenData.xy;
    vec3 normal[9];

    normal[0] = texture(u_NormalRoughness, v_In.texCoords + m_indices[0] * step).xyz;
    normal[1] = texture(u_NormalRoughness, v_In.texCoords + m_indices[1] * step).xyz;
    normal[2] = texture(u_NormalRoughness, v_In.texCoords + m_indices[2] * step).xyz;
    normal[3] = texture(u_NormalRoughness, v_In.texCoords + m_indices[3] * step).xyz;
    normal[4] = texture(u_NormalRoughness, v_In.texCoords + m_indices[4] * step).xyz;
    normal[5] = texture(u_NormalRoughness, v_In.texCoords + m_indices[5] * step).xyz;
    normal[6] = texture(u_NormalRoughness, v_In.texCoords + m_indices[6] * step).xyz;
    normal[7] = texture(u_NormalRoughness, v_In.texCoords + m_indices[7] * step).xyz;
    normal[8] = texture(u_NormalRoughness, v_In.texCoords + m_indices[8] * step).xyz;

    float totalWeight = 1.0;
    float discardThreshold = 0.85;

    int i;

    for (i = 0; i < 9; ++i)
    {
        
    }
}