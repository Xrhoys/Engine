#include "shared.hlsl"

struct particle
{
	float3 initialPos : POSITION;
	float3 initialVel : VELOCITY;
	float2 size       : SIZE;
	float  age        : AGE;
	uint   type       : TYPE;
};

struct ps_input
{
	float3 pos   : POSITION;
	float2 size  : SIZE;
	float4 color : COLOR;
	uint type    : TYPE;
};

ps_input vs(particle input)
{
	static float3 particle_accel = { 0.0f, 5.0f, 0.0f };
	
	ps_input output;

	float t = input.age;

	output.pos = 0.5f * t * t * particle_accel + t * input.initialVel + input.initialPos;

	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);

	output.color = float4(1.0f, 1.0f, 1.0f, opacity);

	output.size = input.size;
	output.type = input.type;

	return output;
}

struct gs_out
{
	float4 posH  : SV_POSITION;
	float4 color : COLOR;
	float2 tex   : TEXCOORD;
	uint primID  : SV_PrimitiveID;
};

[maxvertexcount(4)]
void gs(point ps_input gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<gs_out> triStream)
{
	static float2 texCoords[] = 
	{
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f },
	};

	if(gin[0].type != PT_EMITTER)
	{
		float3 look  = normalize(frame_eyes_pos - gin[0].pos);
		float3 right = normalize(cross(float3(0,1,0), look));
		float3 up    = cross(look, right);

		float halfWidth = 0.5f * gin[0].size.x;
		float halfHeight = 0.5f * gin[0].size.y;

		float4 v[4];
		v[0] = float4(gin[0].pos + halfWidth * right - halfHeight * up, 1.0f);
		v[1] = float4(gin[0].pos + halfWidth * right + halfHeight * up, 1.0f);
		v[2] = float4(gin[0].pos - halfWidth * right - halfHeight * up, 1.0f);
		v[3] = float4(gin[0].pos - halfWidth * right + halfHeight * up, 1.0f);

		gs_out gout;
		[unroll]
		for(int index = 0;
			index < 4;
			++index)
		{
			gout.posH    = mul(v[index], frame_wvp);
			gout.color   = gin[0].color;
			gout.tex     = texCoords[index];
			gout.primID  = primID;
			triStream.Append(gout);
		}	
	}
}

sampler sampler0: register(ps, s0);
Texture2DArray<float4> texture0 : register(t0);

float4 ps(gs_out input): SV_TARGET
{
	return texture0.Sample(sampler0, float3(input.tex, 0)) * input.color;
}
