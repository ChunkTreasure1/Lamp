#ShaderSpecBegin
shader:
  name: depthPrePass
  internal: true
#ShaderSpecEnd

#type vertex
#version 440
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

#include "Common/Lamp_Common.glsl"

layout (push_constant) uniform MeshDataBuffer
{
    mat4 model;
    vec2 blendingUseBlending;

} u_MeshData;

void main()
{
	gl_Position = u_CameraData.projection * u_CameraData.view * u_MeshData.model * vec4(a_Position, 1.0);
}

#type fragment
#version 440

layout (location = 0) out float o_Color;

void main()
{
	o_Color = 1.0;
}