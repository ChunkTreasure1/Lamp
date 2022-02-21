#ShaderSpec
Name: quad
TextureCount: 1
InternalShader: false
TextureNames
{
u_Texture
}

#type vertex
#version 440 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in int a_TexIndex;
layout(location = 4) in int a_Id;

layout(std140, binding = 0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 projection;
    vec4 position;
    vec2 ambienceExposure;
	
} u_CameraData;

layout (location = 0) out Out
{
	vec2 texCoord;
	vec4 color;
	flat int texIndex;
	flat int id;

} v_Out;

void main()
{
	v_Out.texCoord = a_TexCoords;
	v_Out.color = a_Color;
	v_Out.texIndex = a_TexIndex;
	v_Out.id = a_Id;

	gl_Position = u_CameraData.projection * u_CameraData.view * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int ObjectId;

layout (set = 0, binding = 1) uniform sampler2D u_Texture;

layout (location = 0) in Out
{
	vec2 texCoord;
	vec4 color;
	flat int texIndex;
	flat int id;

} v_In;

void main()
{
	FragColor = vec4(1.0, 1.0, 1.0, 1.0); //texture(u_Textures[int(v_In.texIndex)], v_In.texCoord) * v_In.color;
	ObjectId = v_In.id;
}