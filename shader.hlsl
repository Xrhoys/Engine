#include "shared.hlsl"

cbuffer g_buffer_perObject : register(b1)
{
	float4x4       g_world;
	float4x4       g_worldInv;
	lt_material    g_material;
};

struct vs_input
{
	float3 pos   	: POSITION;
	float3 normal    : NORMAL;
	float2 tex   	: TEXCOORD;
};

struct ps_input
{
	float4 pos     : SV_POSITION;
	float4 normal  : NORMAL;
	float3 posW    : POSITION;
	float2 tex     : TEXCOORD;
};

Texture2D<float4> texture0 : register(t0);
sampler           sampler0 : register(s0);

ps_input vs(vs_input input)
{
	ps_input output;

	output.posW       = mul(float4(input.pos, 1.0f), g_world).xyz;
	output.pos        = mul(float4(output.posW, 1.0f), frame_wvp);
	output.normal     = float4(normalize(mul(input.normal, (float3x3)g_worldInv)), 1.0f);
	output.tex        = input.tex;

	return output;
}

float4 ps(ps_input input): SV_TARGET
{
	float4 texel = texture0.Sample(sampler0, input.tex);

	float3 to_eye_w = normalize(frame_eyes_pos - input.posW);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 a, d, s;

	[unroll]
	for(int index = 0;
		index < MAX_LIGHTS;
		++index)
	{
		switch(frame_lights[index].type)
		{
			case LT_SOURCE_POINT:
			{
				compute_point_light(g_material, frame_lights[index], input.posW, input.normal.xyz, to_eye_w, a, d, s);
				ambient += a;
				diffuse += d;
				spec    += s;
			}break;

			case LT_SOURCE_DIRECTION:
			{
				compute_directional_light(g_material, frame_lights[index], input.posW, input.normal.xyz, to_eye_w, a, d, s);
				ambient += a;
				diffuse += d;
				spec    += s;
			}break;

			case LT_SOURCE_SPOT:
			{
				compute_spot_light(g_material, frame_lights[index], input.posW, input.normal.xyz, to_eye_w, a, d, s);
				ambient += a;
				diffuse += d;
				spec    += s;
			}break;

			case LT_SOURCE_SPOT_GGX:
			{
				ggx_spot_light(g_material, frame_lights[index], input.posW, input.normal.xyz, to_eye_w, a, d, s);
				ambient += a;
				diffuse += d;
				spec    += s;
			}break;

			default:
			{

			}break;
		}
	}

	float4 lit_color = ambient + diffuse + spec;
	lit_color.a = g_material.diffuse.a;

	return lit_color;
}
