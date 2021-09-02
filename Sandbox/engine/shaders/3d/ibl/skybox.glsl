#ShaderSpec
Name: Skybox;
TextureCount: 0;
TextureNames:
{
}

#type vertex
#version 440 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_LocalPos;

void main()
{
	v_LocalPos = a_Position;

	mat4 rotView = mat4(mat3(u_View));
	vec4 clipPos = u_Projection * rotView * vec4(a_Position, 1.0);
	
	gl_Position = clipPos.xyww;
}

#type fragment
#version 440 core
out vec4 FragColor;

in vec3 v_LocalPos;

uniform samplerCube u_EnvironmentMap;

void main()
{		
    vec3 envColor = textureLod(u_EnvironmentMap, v_LocalPos, 1.2).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}