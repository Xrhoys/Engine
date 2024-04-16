#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 100
#define MAX_LIGHTS 8

#define LT_SOURCE_POINT 0
#define LT_SOURCE_DIRECTION 1
#define LT_SOURCE_SPOT 2
#define LT_SOURCE_SPOT_GGX 3

#define PI 3.14159265f
#define INVPI 0.318309886f
#define v1 float3(1, 1, 1)
#define FSMOOTH_FACTOR 1.05f

#define PT_EMITTER 0
#define PT_FLARE 1

struct lt_source
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	
	float3 position;
	float  range;

	float3 direction;
	float  spot;

	float3 att;
	int    type;
};

struct lt_material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 reflect;
};

struct lt_material_ggx
{
	float3 f0;
	float  roughness;
};

cbuffer g_buffer_perFrame : register(b0)
{
	float4x4       frame_wvp;
	lt_source      frame_lights[MAX_LIGHTS];
	float3         frame_eyes_pos;
	float          frame_game_time;
	float3         frame_emit_pos;
	float          frame_time_step;
};

void ggx_spot_light(lt_material mat, lt_source light,
						float3 pos, float3 normal, float3 toEye,
						out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float3 light_v = light.position - pos;

	float d = length(light_v);

	if(d > light.range)
		return;

	light_v /= d;

	ambient = mat.ambient * light.ambient;

	// Half vector
	float3 h = normalize(light_v + toEye);
		
	float diffuse_factor = dot(light_v, h);

	{
		float3 v = reflect(-light_v, normal);
		// float spec_factor = pow(max(dot(toEye, v), 0.0f), mat.specular.w);
		
		// diffuse = diffuse_factor * mat.diffuse * light.diffuse;

		float nl   = dot(normal, light_v);
		float nv   = dot(normal, v);
		float absNL = abs(nl);
		float absNV = abs(nv);
		float nm   = dot(normal, h);
		float ag2  = mat.specular.w * mat.specular.w;
		float ndfD = (1 + nm * nm * (ag2 - 1));

		float3 fresnelTerm = mat.specular.xyz + (v1 - mat.specular.xyz) * pow(1 - max(dot(h, light_v), 0.0f), 5);		
		float3 visibility  = 0.5f / lerp(2 * absNL * absNV, absNL + absNV, mat.specular.w);
		
		float3 ndf         = float3(0, 0, 0);
		[flatten]
		if(nm > 0.0f)
		{
			ndf = ag2 / (PI * ndfD * ndfD);
		}

		spec = float4(ndf * fresnelTerm * visibility, 1.0f) * light.specular;

		// Diffuse
		[flatten]
		if(nl > 0.0f && nv > 0.0f)
		{
			float  kfacing = 0.5f + 0.5f * dot(light_v, v);
			float3 fsmooth = FSMOOTH_FACTOR * (v1 - mat.specular.xyz) * (1 - pow(1 - nl, 5)) * (1 - pow(1 - nv, 5));
			float3 frough  = kfacing * (0.9f - 0.4f * kfacing) * ((0.5f + nm) / nm);
			float3 fmulti  = 0.3641f * mat.specular.w;
			diffuse = float4(INVPI * mat.diffuse.xyz * ((1 - mat.specular.w) * fsmooth 
					+ mat.specular.w * frough 
					+ mat.diffuse.xyz * fmulti), mat.diffuse.w);
		}
	}

	float3 direction = normalize(light.direction);
	float spot = pow(max(dot(-light_v, direction), 1.0f), light.spot);

	float att = spot / dot(light.att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

void compute_point_light(lt_material mat, lt_source light,
						float3 pos, float3 normal, float3 toEye,
						out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float3 light_v = light.position - pos;

	float d = length(light_v);

	if(d > light.range)
		return;

	light_v /= d;
	
	ambient = mat.ambient * light.ambient;

	float diffuse_factor = dot(light_v, normal);

	[flatten]
	if(diffuse_factor > 0.0f)
	{
		float3 v = reflect(-light_v, normal);
		float spec_factor = pow(max(dot(toEye, v), 0.0f), mat.specular.w);
		
		diffuse = diffuse_factor * mat.diffuse * light.diffuse;
		spec = spec_factor * mat.specular * light.specular;
	}

	float att = 1.0f / dot(light.att, float3(1.0f, d, d*d));

	diffuse *= att;
	spec *= att;
}

void compute_directional_light(lt_material mat, lt_source light,
								float3 pos, float3 normal, float3 toEye,
								out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 light_v = -light.direction;

	ambient = mat.ambient * light.ambient;

	float diffuse_factor = dot(light_v, normal);

	[flatten]
	if(diffuse_factor > 0.0f)
	{
		float3 v = reflect(-light_v, normal);
		float spec_factor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

		diffuse = diffuse_factor * mat.diffuse * light.diffuse;
		spec = spec_factor * mat.specular * light.specular;
	}
}

void compute_spot_light(lt_material mat, lt_source light,
						float3 pos, float3 normal, float3 toEye,
						out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float3 light_v = light.position - pos;

	float d = length(light_v);

	if(d > light.range)
		return;

	light_v /= d;

	ambient = mat.ambient * light.ambient;

	float diffuse_factor = dot(light_v, normal);

	[flatten]
	if(diffuse_factor > 0.0f)
	{
		float3 v = reflect(-light_v, normal);
		float spec_factor = pow(max(dot(toEye, v), 0.0f), mat.specular.w);
		
		diffuse = diffuse_factor * mat.diffuse * light.diffuse;
		spec = spec_factor * mat.specular * light.specular;
	}

	float3 direction = normalize(light.direction);
	float spot = pow(max(dot(-light_v, direction), 0.0f), light.spot);

	float att = spot / dot(light.att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

void vs()
{
	return;
}

void ps()
{
	return;
}
