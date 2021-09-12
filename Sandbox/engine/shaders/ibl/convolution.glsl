#ShaderSpec
Name: Convolution
TextureCount: 0
TextureNames:
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;

out vec3 v_WorldPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    v_WorldPos = a_Position;
    gl_Position =  u_Projection * u_View * vec4(v_WorldPos, 1.0);
}

#type fragment
#version 440 core

out vec4 FragColor;

in vec3 v_WorldPos;

uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 normal = normalize(v_WorldPos);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.025;
	float samples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			samples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(samples));

	FragColor = vec4(irradiance, 1.0);
}