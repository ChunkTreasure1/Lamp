#version 440 core
out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 u_CameraPosition;
uniform Material u_Material;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLight[12];
uniform int u_LightCount;

vec3 CalculateDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

	vec3 ambient = light.ambient * vec3(texture(u_Material.diffuse, v_TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(u_Material.diffuse, v_TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(u_Material.specular, v_TexCoord));

	return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * vec3(texture(u_Material.diffuse, v_TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(u_Material.diffuse, v_TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(u_Material.specular, v_TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(v_Normal);
	vec3 viewDir = normalize(u_CameraPosition - v_FragPos);

	vec3 result = vec3(0, 0, 0);
	
	for(int i = 0; i < u_LightCount; i++)
	{
		result += CalculatePointLight(u_PointLight[i], norm, v_FragPos, viewDir);
	}

	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}