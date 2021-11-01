#ShaderSpec
Name: ssao
TextureCount: 0
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

uniform float u_AspectRatio;
uniform float u_TanHalfFOV;

out vec2 v_TexCoords;
out vec2 v_ViewRay;

void main()
{
    v_TexCoords = a_TexCoords;
    v_ViewRay.x = a_Position.x * u_AspectRatio * u_TanHalfFOV;
    v_ViewRay.y = a_Position.y * u_TanHalfFOV;

    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 440
layout (location = 0) out float FragColor;

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
    float u_T;
};

in vec2 v_TexCoords;
in vec2 v_ViewRay;

uniform sampler2D u_NormalMap;
uniform sampler2D u_DepthMap;

uniform sampler2D u_Noise;
uniform vec2 u_BufferSize;

vec3 CalculateWorld(vec2 coords)
{
    float depth = texture(u_DepthMap, coords).x;
    float z = depth * 2.0 - 1.0;

    vec4 screenPos = vec4(coords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpace = inverse(u_Projection) * screenPos;

    viewSpace /= viewSpace.w;

    vec4 worldSpace = inverse(u_View) * viewSpace;
    return worldSpace.xyz;
}

void main()
{
    vec3 pos = CalculateWorld(v_TexCoords);
    vec2 noiseScale = vec2(u_BufferSize.x / 4.0, u_BufferSize.y / 4.0);

    vec3 normal = normalize(texture(u_NormalMap, v_TexCoords).rgb);
    vec3 randomVec = normalize(texture(u_Noise, v_TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < u_KernelSize; i++)
    {
        vec3 samplePos = TBN * u_KernelSamples[i].xyz;
        samplePos = pos + samplePos * u_Radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = u_Projection * offset;

        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec3 offsetPos = CalculateWorld(offset.xy);
        float sampleDepth =  offsetPos.z;

        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(pos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + u_Bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / u_KernelSize);

    FragColor = occlusion;
}