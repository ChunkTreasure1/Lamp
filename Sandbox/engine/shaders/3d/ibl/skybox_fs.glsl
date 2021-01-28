#ShaderSpec
Name: Skybox;
TextureCount: 0;
TextureNames:
{
}
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