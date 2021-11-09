#ShaderSpec
Name: quad
TextureCount: 1
InternalShader: false
TextureNames
{
gizmo
}

#type vertex
#version 440 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_Id;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosition;
};

out Out
{
	vec2 texCoord;
	vec4 color;
	float texIndex;
	float id;

} v_Out;

void main()
{
	v_Out.texCoord = a_TexCoords;
	v_Out.color = a_Color;
	v_Out.texIndex = a_TexIndex;
	v_Out.id = a_Id;

	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int ObjectId;

in Out
{
	vec2 texCoord;
	vec4 color;
	float texIndex;
	float id;

} v_In;

uniform sampler2D u_Textures[32];

void main()
{
	FragColor = texture(u_Textures[int(v_In.texIndex)], v_In.texCoord) * v_In.color;
	ObjectId = int(v_In.id);
}