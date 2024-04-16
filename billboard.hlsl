#include "shared.hlsl"

cbuffer g_buffer_perObject : register(b1)
{
	float4x4       g_world;
	float4x4       g_worldInv;
	lt_material    g_material;
};

struct vs_input
{
	float3 pos  : POSITION;
	float2 size : SIZE;
};

struct gs_input
{
	float3 center : POSITION;
	float2 size   : SIZE;
};

struct ps_input
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
	float3 normal  : NORMAL;
	float2 tex     : TEXCOORD;
	uint   primID  : SV_PrimitiveID;
};

gs_input vs(vs_input input)
{
	gs_input output;

	output.center = input.pos;
	output.size   = input.size;

	return output;
}

[maxvertexcount(4)]
void gs(point gs_input gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<ps_input> triStream)
{
	static float2 texCoords[] = 
	{
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f },
	};

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = frame_eyes_pos - gin[0].center;
	look.y = 0.0f;
	look = normalize(look);
	float3 right = cross(up, look);

	float halfWidth = 0.5f * gin[0].size.x;
	float halfHeight = 0.5f * gin[0].size.y;

	float4 v[4];
	v[0] = float4(gin[0].center + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(gin[0].center + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(gin[0].center - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(gin[0].center - halfWidth * right + halfHeight * up, 1.0f);

	ps_input gout;
	[unroll]
	for(int index = 0;
		index < 4;
		++index)
	{
		gout.posH = mul(v[index], frame_wvp);
		gout.posW = v[index].xyz;
		gout.normal = look;
		gout.tex     = texCoords[index];
		gout.primID  = primID;
		triStream.Append(gout);
	}
}

sampler sampler0;
Texture2DArray<float4> texture0;

float4 ps(ps_input input): SV_TARGET
{
	input.normal = normalize(input.normal);
	
	float3 to_eye_w = normalize(frame_eyes_pos - input.posW);

	// Sample texture
	float3 uvw = float3(input.tex, input.primID % 4);
	float4 texColor = texture0.Sample(sampler0, uvw);
	// clip alpha (make it optional later)
	clip(texColor.a - 0.05f);

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

	float4 lit_color = texColor * (ambient + diffuse) + spec;
	lit_color.a = g_material.diffuse.a * texColor.a;

	return lit_color;
}
