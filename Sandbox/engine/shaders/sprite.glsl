#ShaderSpec
Name: Sprite
TextureCount: 1
TextureNames
{
gizmo
}

#type vertex
#version 440 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Main
{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_ShadowVP;
	vec3 u_CameraPosition;
	float padding;
};

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main()
{
	v_TexCoords = a_TexCoords;

	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int ObjectId;

in vec2 v_TexCoords;

struct Material
{
	sampler2D gizmo;
};

uniform Material u_Material;
uniform int u_ObjectId;

void main()
{
	vec4 color = texture(u_Material.gizmo, v_TexCoords);
	if (color.a < 0.1)
	{
		discard;
	}
	
	FragColor = color;
	ObjectId = u_ObjectId;
}