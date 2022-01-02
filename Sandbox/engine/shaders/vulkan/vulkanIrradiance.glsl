#ShaderSpec
Name: irradianceCube
TextureCount: 0
InternalShader: true
TextureNames:
{
}

#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out vec3 v_Position;

layout (std140, binding = 4) uniform CubeBuffer
{
    mat4 modelViewProjection;
    vec2 phiTheta;

} u_CubeBuffer;

void main()
{
    v_Position = a_Position;
    gl_Position = u_CubeBuffer.modelViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;
layout (location = 0) in vec3 v_Position;

layout (std140, binding = 4) uniform CubeBuffer
{
    mat4 modelViewProjection;
    vec2 phiTheta;

} u_CubeBuffer;

layout (binding = 0) uniform samplerCube u_EnvironmentMap;

const float PI = 3.1415926535897932384626433832795;

void main()
{
    // vec3 normal = normalize(v_Position);
    // vec3 up = vec3(0.0, 1.0, 0.0);
    // vec3 right = normalize(cross(up, normal));

    // up = cross(normal, right);

    // const float twoPI = PI * 2.0;
    // const float halfPI = PI * 0.5;

    // vec3 color = vec3(0.0);
    // uint sampleCount = 0u;

    // for (float phi = 0.0; phi < twoPI; phi += u_CubeBuffer.phiTheta.x)
    // {
    //         for (float theta = 0.0; theta < twoPI; theta += u_CubeBuffer.phiTheta.y)
    //         {
    //             vec3 tempVec = cos(phi) * right + sin(phi) * up;
    //             vec3 sampleVec = cos(theta) * normal + sin(theta) * tempVec;

    //             color += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
    //             sampleCount++;
    //         }
    // }

    vec3 color = texture(u_EnvironmentMap, vec3(0.0)).rgb;
    o_Color = vec4(color, 1.0);
}