cbuffer g_buffer : register(b0)
{
	float4x4 wvp;
};

struct vs_input
{
	float3 posL: POSITION;
};

struct ps_input
{
	float4 pos      : SV_POSITION;
	float3 texcoord : TEXCOORD;
};

ps_input vs(vs_input input)
{
	ps_input output;

	output.texcoord = input.posL;

	output.pos = mul(float4(input.posL, 1.0f), wvp).xyww;

	return output;
}

TextureCube  texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 ps(ps_input input) :  SV_TARGET
{
	return texture0.Sample(sampler0, input.texcoord); 
}