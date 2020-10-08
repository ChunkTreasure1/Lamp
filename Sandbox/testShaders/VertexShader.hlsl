struct VSOut
{
	float3 color : COLOR;
	float4 pos : SV_POSITION;
};

cbuffer CBuffer
{
	column_major matrix u_Model;
	column_major matrix u_Projection;
};

VSOut main(float3 pos : POSITION, float3 color : COLOR)
{
	VSOut vso;
	vso.pos = mul(mul(float4(pos, 1.f), u_Model), u_Projection);
	vso.color = color;

	return vso;
}