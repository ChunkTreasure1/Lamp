#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;

// texture samplers
uniform sampler2D u_Texture;
uniform vec3 u_ObjColor;
uniform vec3 u_LightColor;

uniform vec3 u_LightPosition;
uniform vec3 u_CameraPosition;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * u_LightColor;

	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(u_LightPosition - v_FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_LightColor;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(u_CameraPosition - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * u_LightColor;

	vec3 result = (ambient + diffuse + specular) * u_ObjColor;
	FragColor = texture(u_Texture, v_TexCoord) * vec4(result, 1.0);
}