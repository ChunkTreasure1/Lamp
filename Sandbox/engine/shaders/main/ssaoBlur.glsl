#ShaderSpec
Name: SSAOBlur
TextureCount: 0
TextureNames
{
}

#type vertex
#version 440 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Bitangent;
layout (location = 4) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 440 core
layout(location = 0) out float FragColor;

in vec2 v_TexCoords;
in vec4 v_ShadowCoords;

uniform sampler2D u_SSAO;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(u_SSAO, 0));
	float result = 0.0;
	for (int x = -2; x < 2; x++)
	{
		for (int y = -2; y < 2; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(u_SSAO, v_TexCoords + offset).r;
		}
	}

	FragColor = result / (4.0 * 4.0);
}