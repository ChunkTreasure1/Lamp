#ShaderSpecBegin
shader:
  name: sssBlur
  internal: true
#ShaderSpecEnd

#type compute
#version 450 core

layout (set = 0, binding = 0) uniform sampler2D u_DiffuseTexture;
layout (set = 0, binding = 1) uniform sampler2D u_DepthTexture;
layout (set = 0, binding = 2, rgba16f) uniform writeonly image2D o_Result;

layout (push_constant) uniform SSSData
{
    vec2 screenSize;
    vec2 direction;
    float width;

} u_SSSData;

vec4 m_kernel[] =
{
   	vec4(0.530605, 0.613514, 0.739601, 0),
	vec4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
	vec4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
	vec4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
	vec4(0.00700976, 0.00049366, 0.000151938, -1.6875),
	vec4(0.0094389, 0.00139119, 0.000416598, -1.33333),
	vec4(0.0128496, 0.00356329, 0.00132016, -1.02083),
	vec4(0.017924, 0.00711691, 0.00347194, -0.75),
	vec4(0.0263642, 0.0119715, 0.00684598, -0.520833),
	vec4(0.0410172, 0.0199899, 0.0118481, -0.333333),
	vec4(0.0493588, 0.0367726, 0.0219485, -0.1875),
	vec4(0.0402784, 0.0657244, 0.04631, -0.0833333),
	vec4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
	vec4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
	vec4(0.0402784, 0.0657244, 0.04631, 0.0833333),
	vec4(0.0493588, 0.0367726, 0.0219485, 0.1875),
	vec4(0.0410172, 0.0199899, 0.0118481, 0.333333),
	vec4(0.0263642, 0.0119715, 0.00684598, 0.520833),
	vec4(0.017924, 0.00711691, 0.00347194, 0.75),
	vec4(0.0128496, 0.00356329, 0.00132016, 1.02083),
	vec4(0.0094389, 0.00139119, 0.000416598, 1.33333),
	vec4(0.00700976, 0.00049366, 0.000151938, 1.6875),
	vec4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
	vec4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
	vec4(0.000973794, 1.11862e-005, 9.43437e-007, 3), 
};

float LinearizeDepth(float z)
{
    float n = 0.01;
    float f = 50;
    return (n * f) / (f - z * (f - n));
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    vec4 color = texelFetch(u_DiffuseTexture, ivec2(gl_GlobalInvocationID.xy), 0);

    //Don't blur non SSS pixels
    if (color.a == 0.0)
    {
        imageStore(o_Result, ivec2(gl_GlobalInvocationID.xy), color);
        return;
    }

    float depth = LinearizeDepth(texelFetch(u_DepthTexture, ivec2(gl_GlobalInvocationID.xy), 0).x);
    float rayRadiusUV = 0.5 * u_SSSData.width / depth;

    //Skip if footprint is less than a pixel
    if (rayRadiusUV <= u_SSSData.screenSize.x)
    {
        imageStore(o_Result, ivec2(gl_GlobalInvocationID.xy), color);
        return;
    }

    //Calculate final step to fetch surronding pixels
    vec2 finalStep = rayRadiusUV * u_SSSData.direction;
    finalStep *= color.a;
    finalStep *= 1.0 / 3.0;

    vec4 colorBlurred = color;
    colorBlurred.rgb *= m_kernel[0].rgb;

    vec2 texCoord = (vec2(gl_GlobalInvocationID.xy) + vec2(0.5)) * u_SSSData.screenSize;
    
    for (int i = 1; i < 25; ++i)
    {
        //Fetch sample info
        vec2 offset = texCoord + m_kernel[i].a * finalStep;
        vec4 diffCol = textureLod(u_DiffuseTexture, offset, 0.0);
        float newDepth = LinearizeDepth(textureLod(u_DepthTexture, offset, 0.0).x);

        float maxDepthDiff = 0.01;
        float alpha = min(distance(newDepth, depth) / maxDepthDiff, maxDepthDiff);

        alpha *= 1.0 - diffCol.a;
        
        diffCol.rgb = mix(diffCol.rgb, color.rgb, alpha);
        colorBlurred.rgb += m_kernel[i].rgb * diffCol.rgb;
    }

    imageStore(o_Result, ivec2(gl_GlobalInvocationID.xy), colorBlurred);
}