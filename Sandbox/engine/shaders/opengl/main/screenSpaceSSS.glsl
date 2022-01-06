#ShaderSpec
Name: screenSpaceSSS
TextureCount: 0
InternalShader: true
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
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_DepthMap; //Scene depth map
uniform sampler2D u_ColorMap;
uniform float u_Width; //Width of the sss
uniform vec2 u_BlurDirection;
uniform bool u_InitializeStencil;
uniform float u_CameraFOV;

//25 samples
vec4 kernel[] = {
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

//Based on https://github.com/iryoku/separable-sss
void main()
{
    vec4 colorMap = texture(u_ColorMap, v_TexCoords);
    if (u_InitializeStencil)
    {
        if (colorMap.a == 0.0)
        {
            discard;
        }
    }

    float depthMap = texture(u_DepthMap, v_TexCoords).r;

    //calculate sss width scale
    float distToProjectionWindow = 1.0 / tan(0.5 * radians(u_CameraFOV));
    float scale = distToProjectionWindow / depthMap;

    //calculate final step to fetch surrounding pixles
    vec2 finalStep = u_Width * scale * u_BlurDirection;
    finalStep *= colorMap.a;
    finalStep *= 1.0 / 3.0;

    vec4 colorBlurred = colorMap;
    colorBlurred.rgb *= kernel[0].rgb;

    for (int i = 0; i < 25; i++)
    {
        vec2 offset = v_TexCoords + kernel[i].a * finalStep;
        vec4 color = texture(u_ColorMap, offset);

        //Expensive on memory
        float depth = texture(u_DepthMap, offset).r;
        float s = clamp(300.f * distToProjectionWindow * u_Width * abs(depthMap - depth), 0.0, 1.0);

        color.rgb = mix(color.rgb, colorMap.rgb, s);

        colorBlurred.rgb += kernel[i].rgb * color.rgb;
    }

    FragColor = colorBlurred;
}