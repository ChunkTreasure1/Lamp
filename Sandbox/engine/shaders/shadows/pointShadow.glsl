#ShaderSpec
Name: pointShadow
TextureCount: 0
TextureNames
{
}
#type vertex
#version 440 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoords;

uniform mat4 u_Model;

void main()
{
	gl_Position = u_Model * vec4(a_Position, 1.0);
}

#type geometry
#version 440 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

out vec4 FragPos;

uniform mat4 u_Transforms[6];

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; i++)
		{
			FragPos = gl_in[i].gl_Position;
			gl_Position = u_Transforms[face] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#type fragment
#version 440 core

layout(location = 2) in vec4 FragPos;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

void main()
{
	float dist = length(FragPos.xyz - u_LightPosition);
	dist = dist / u_FarPlane;

	gl_FragDepth = dist;
}