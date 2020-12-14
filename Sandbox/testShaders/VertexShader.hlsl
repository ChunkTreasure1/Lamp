struct VSOut
{
	float3 color : COLOR;
	float4 pos : SV_POSITION;
};

cbuffer CBuffer
{
	matrix u_ModelProjection;
	matrix u_Projection;
};

VSOut main(float3 pos : POSITION, float3 color : COLOR)
{
	VSOut vso;
	vso.pos = mul(mul(u_Projection, u_Model), float4(pos, 1.f));
	vso.color = color;

	return vso;
}