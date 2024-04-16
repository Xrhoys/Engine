struct vs_input
{
	float2 dst_p0: P0_DST;
	float2 dst_p1: P1_DST;
	float2 src_p0: P0_SRC;
	float2 src_p1: P1_SRC;
	float4 color[4]: COLOR;
	float corner_radius: CORNER_RADIUS;
	float edge_softness: EDGE_SOFTNESS;
	float border_thickness: BORDER_THICKNESS;
	uint vertex_id: SV_VertexID;
};

struct ps_input
{
	float4 vertex: SV_POSITION;
	float2 uv: TEXCOORD;
	float2 dst_pos: DST_POS;
	float2 dst_center: DST_CENTER;
	float2 dst_half_size: DST_HALF_SIZE;
	float corner_radius: CORNER_RADIUS; 
	float edge_softness: EDGE_SOFTNESS;  
	float border_thickness: BORDER_THICKNESS;
	float4 color: COLOR;
};

struct c_globals
{
	float2 res;
	sampler sampler0;
	Texture2D<float4> texture0;
};

c_globals globals;

float RoundedRectSDF(float2 sample_pos,
                     float2 rect_center,
                     float2 rect_half_size,
                     float r)
{
	float2 d2 = (abs(rect_center - sample_pos) - rect_half_size + float2(r, r));
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

ps_input vs(vs_input input)
{
	static float2 vertices[] = 
	{
    	{-1, +1},
		{-1, -1},
    	{+1, +1},
    	{+1, -1},
	};

	static uint texWidth, texHeight;
	globals.texture0.GetDimensions(texWidth, texHeight);

	// "dst" => "destination" (on screen)
	float2 dst_half_size = (input.dst_p1 - input.dst_p0) / 2;
	float2 dst_center = (input.dst_p1 + input.dst_p0) / 2;
	float2 dst_pos = (vertices[input.vertex_id] * dst_half_size + dst_center);
	
	// "src" => "source" (on texture)
	float2 src_half_size = (input.src_p1 - input.src_p0) / 2;
	float2 src_center = (input.src_p1 + input.src_p0) / 2;
	float2 src_pos = (vertices[input.vertex_id] * src_half_size + src_center);
	
	// package output
	ps_input output;
	output.dst_pos       = dst_pos;
	output.dst_center    = dst_center;
	output.dst_half_size = dst_half_size;
	output.vertex = float4(2 * dst_pos.x / globals.res.x - 1,
						   - 2 * dst_pos.y / globals.res.y + 1,
						   0,
						   1);
	output.color = input.color[input.vertex_id];
	output.uv = float2(src_pos.x / texWidth,
                       src_pos.y / texHeight);
	output.corner_radius = input.corner_radius;
	output.edge_softness = input.edge_softness;
	output.border_thickness = input.border_thickness;
	return output;
}

float4 ps(ps_input input): SV_TARGET
{
	float softness = input.edge_softness;
	float2 softness_padding = float2(max(0, softness*2-1), max(0, softness*2-1));

	// sample distance
	float dist = RoundedRectSDF(input.dst_pos,
                                input.dst_center,
                                input.dst_half_size - softness_padding,
                                input.corner_radius);

	// map distance => a blend factor
	float sdf_factor = 1.f - smoothstep(0, 2*softness, dist);

	// border factor
	float border_factor = 1.f;
	if(input.border_thickness != 0)
	{
		float2 interior_half_size = input.dst_half_size - float2(input.border_thickness, input.border_thickness);
		float interior_radius_reduce_f = min(interior_half_size.x / input.dst_half_size.x,
												interior_half_size.y / input.dst_half_size.y);
		float interior_corner_radius = (input.corner_radius * interior_radius_reduce_f * interior_radius_reduce_f);

		float inside_d = RoundedRectSDF(input.dst_pos,
                                  input.dst_center,
                                  interior_half_size-
                                  softness_padding,
                                  interior_corner_radius);
		float inside_f = smoothstep(0, 2*softness, inside_d);
		border_factor = inside_f;
	}

	float4 sample = globals.texture0.Sample(globals.sampler0, input.uv);
	float4 color = input.color * sample * sdf_factor * border_factor;

	return color;
}
