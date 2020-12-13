#ShaderSpec
Name: Grid;
TextureCount: 0;
TextureNames:
{
}
#version 440 core
out vec4 FragColor;

in vec3 v_NearPoint;
in vec3 v_FarPoint;

vec4 grid(vec3 r, float scale)
{
	vec2 coords = r.xz * scale;
	vec2 derivative = fwidth(coords);
	vec2 grid = abs(fract(coords - 0.5) - 0.5) / derivative;
	
	float line = min(grid.x, grid.y);
	float minimumz = min(derivative.y, 1);
	float minimumx = min(derivative.x, 1);

	vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

	if(r.x > -0.1 * minimumx && r.x < 0.1 * minimumx)
	{
		color.z = 1.0;
	}

	if(r.z > -0.1 * minimumz && r.z < 0.1 * minimumz)
	{
		color.x = 1.0;
	}

	return color;
}

void main()
{
    float t = -v_NearPoint.y / (v_FarPoint.y - v_NearPoint.y);
    FragColor = vec4(1.0, 0.0, 0.0, 1.0 * float(t > 0)); // opacity = 1 when t > 0, opacity = 0 otherwise
}