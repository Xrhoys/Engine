#include "shared.hlsl"

sampler samLinear: register(s0);
Texture1D randomTex: register(t0);

float3 RandUnitVec3(float offset)
{
	float u = (frame_game_time + offset);

	float3 v = randomTex.SampleLevel(samLinear, u, 0).xyz;
	return normalize(v);
}

struct particle
{
	float3 initialPos : POSITION;
	float3 initialVel : VELOCITY;
	float2 size       : SIZE;
	float  age        : AGE;
	uint   type       : TYPE;
};

particle vs_so(particle input)
{
	return input;
}

[maxvertexcount(2)]
void gs_so(point particle gin[1], inout PointStream<particle> ptStream)
{
	gin[0].age += frame_time_step;

	if(gin[0].type == PT_EMITTER)
	{
		if(gin[0].age > 0.005f)
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 2.5f;
			vRandom.z *= 2.5f;

			particle p;
			p.initialPos = frame_emit_pos.xyz;
			p.initialVel = 4.0f * vRandom;
			p.size       = float2(vRandom.y, vRandom.y);
			p.age        = 0.0f;
			p.type       = PT_FLARE;


			ptStream.Append(p);

			gin[0].age = 0.0f;
		}

		ptStream.Append(gin[0]);
	}
	else
	{
		if(gin[0].age <= 1.5f)
		{
			ptStream.Append(gin[0]);
		}
	}
}
