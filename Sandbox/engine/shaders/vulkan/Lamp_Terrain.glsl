#ShaderSpec
Name: terrain
TextureCount: 0
InternalShader: true
TextureNames
{
}

#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (location = 0) out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
    v_TexCoords = a_TexCoords;
}

#type tessellationControl
#version 450

layout (vertices = 4) out;

layout (location = 1) out vec2 v_TexCoords[];

layout (location = 0) in vec2 a_TexCoords[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    v_TexCoords[gl_InvocationID] = a_TexCoords[gl_InvocationID];

    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16;
        gl_TessLevelOuter[3] = 16;

        gl_TessLevelInner[0] = 16;
        gl_TessLevelInner[1] = 16;
    }
}

#type tessellationEvaluation
#version 450

layout (quads, fractional_odd_spacing, ccw) in;
layout (location = 1) in vec2 v_TexCoords[];

layout (location = 0) out float o_Height;

layout (std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;

} u_CameraData;

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;

} u_MeshData;

layout (set = 0, binding = 1) uniform sampler2D u_HeightMap;

void main()
{
    //Get patch coordiantes
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    //Get control point texture coords
    vec2 t00 = v_TexCoords[0];
    vec2 t01 = v_TexCoords[1];
    vec2 t10 = v_TexCoords[2];
    vec2 t11 = v_TexCoords[3];

    //Bilinear filtering
    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    //Get y height
    o_Height = texture(u_HeightMap, texCoord).y * 64.0 - 16.0; // scale and shift, set from menu

    //Get control point position coords
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 0));

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    p += normal * o_Height;

    gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * p;
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in float o_Height;

void main()
{
    float h = (o_Height + 16) / 64;
    o_Color = vec4(h, h, h, 1.0);
}