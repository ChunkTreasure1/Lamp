float4 main(float3 color : COLOR) : SV_Target
{
	return float4(color, 1.f);
}