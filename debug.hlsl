cbuffer g_buffer_perFrame : register(b0)
{
	float4x4       g_wvp;
};

struct vs_input
{
	float3 pos   : POSITION;
	float4x4 world : WORLD;
	float4 color : COLOR;
	uint instance_id : SV_InstanceID;
};

struct ps_input
{
	float4 pos   : SV_POSITION;
	float4 color : COLOR;
};

ps_input vs(vs_input input)
{
	ps_input output;
	output.color     = input.color;
	float3 posW      = mul(float4(input.pos, 1.0f), input.world).xyz;
	output.pos       = mul(float4(posW, 1.0f), g_wvp);
	
	return output;
}

float4 ps(ps_input input): SV_TARGET
{
	return input.color;
}
