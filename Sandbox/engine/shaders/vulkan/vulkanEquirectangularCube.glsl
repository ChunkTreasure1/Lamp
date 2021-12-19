#ShaderSpec
Name: equirectangularCube
TextureCount: 0
InternalShader: true
TextureNames:
{
}

#type vertex
#version 450
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

layout (std140, binding = 4) uniform CubeBuffer
{
    mat4 modelViewProjection;
    vec2 phiTheta;

} u_CubeBuffer;

layout(location = 0) out vec3 v_Position;

void main()
{
    v_Position = a_Position;
    gl_Position =  u_CubeBuffer.modelViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450
layout (location = 0) out vec4 o_Color;
layout (location = 0) in vec3 v_Position;

layout (binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(v_Position));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;
    
    o_Color = vec4(color, 1.0);
}