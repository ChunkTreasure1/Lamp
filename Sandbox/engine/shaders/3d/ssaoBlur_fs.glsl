#ShaderSpec
Name: SSAOBlur;
TextureCount: 0;
TextureNames
{
}

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