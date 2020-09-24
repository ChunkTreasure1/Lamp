//None
#version 440 core
layout (location = 0) in vec3 a_Position;

out vec3 v_NearPoint;
out vec3 v_FarPoint;

uniform mat4 u_View;
uniform mat4 u_Projection;

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection)
{
	mat4 viewInv = inverse(view);
	mat4 projInv = inverse(projection);

	vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
	return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
	vec3 p = a_Position.xyz;

	v_NearPoint = UnprojectPoint(a_Position.x, a_Position.y, 0.0, u_View, u_Projection);
	v_FarPoint = UnprojectPoint(a_Position.x, a_Position.y, 1.0, u_View, u_Projection);
	
	gl_Position = vec4(p, 1.0);
}
