#ShaderSpec
Name: selection;
TextureCount: 0;
TextureNames:
{
}
#version 440 core
layout(location = 0) out int o_ObjectId;

uniform int u_ObjectId;

void main()
{
	o_ObjectId = u_ObjectId;
}