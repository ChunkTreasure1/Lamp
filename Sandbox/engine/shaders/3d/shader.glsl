#ShaderSpec
Name: Illumn;
TextureCount: 4;
TextureNames
{
diffuse
specular
normal
depth
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 InvTBN;
} v_Out;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_LightViewProjection;
uniform vec3 u_CameraPosition;

void main()
{
	v_Out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Out.TexCoord = a_TexCoords;

	//TBN creation
    vec3 T = normalize(mat3(u_Model) * a_Tangent);
	vec3 B = normalize(mat3(u_Model) * a_Bitangent);
    vec3 N = normalize(mat3(u_Model) * a_Normal);

	v_Out.InvTBN = transpose(mat3(T, B, N));

	v_Out.TangentViewPos = v_Out.InvTBN * u_CameraPosition;
	v_Out.TangentFragPos = v_Out.InvTBN * vec3(u_Model * vec4(a_Position, 1.0));

	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
out vec4 FragColor;

in Out
{
	vec3 FragPos;
	vec2 TexCoord;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 InvTBN;
} v_In;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normal;
	sampler2D depth;
	float depthScale;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 position;

	vec3 diffuse;
	vec3 specular;
};

struct Environment
{
	vec3 globalAmbient;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 diffuse;
	vec3 specular;
};

uniform Material u_Material;
uniform Environment u_Environment;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLight[12];
uniform int u_LightCount;

uniform int u_UsingParallax = 0;

vec3 CalculateDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(v_In.InvTBN * light.position - v_In.TangentFragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

	vec3 ambient = u_Environment.globalAmbient * vec3(texture(u_Material.diffuse, v_In.TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(u_Material.diffuse, v_In.TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(u_Material.specular, v_In.TexCoord));

	return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightPos = v_In.InvTBN * light.position;

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = u_Environment.globalAmbient * vec3(texture(u_Material.diffuse, v_In.TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(u_Material.diffuse, v_In.TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(u_Material.specular, v_In.TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(u_Material.depth, texCoords).r;     
    return texCoords - viewDir.xy * (height * u_Material.depthScale); 
}

void main()
{
	vec3 viewDir = normalize(v_In.TangentViewPos - v_In.TangentFragPos);
	vec2 texCoords = v_In.TexCoord;

	//texCoords = ParallaxMapping(v_In.TexCoord, viewDir);

	vec3 norm = texture(u_Material.normal, texCoords).rgb;
	norm = normalize(norm * 2.0 - 1.0);

	vec3 result = vec3(0, 0, 0);
	for(int i = 0; i < u_LightCount; i++)
	{
		result += CalculatePointLight(u_PointLight[i], norm, v_In.TangentFragPos, v_In.TangentViewPos);
	}

	result += CalculateDirLight(u_DirectionalLight, norm, v_In.TangentViewPos);
	float shadow = 0.0;

	vec3 ambient = u_Environment.globalAmbient * vec3(texture(u_Material.diffuse, texCoords));
	vec3 lighting = result + ambient;

	FragColor = vec4(lighting, 1.0);
}