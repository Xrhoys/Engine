////////////////////////////////
/* 
Bounding debug boxes should be genrated with vertex shaders
 */
////////////////////////////////
internal collision_bounding
ComputeBoundingAABB(vertex *vertices, u32 count)
{
    v3 vMin = V3( INFINITY,  INFINITY,  INFINITY);
    v3 vMax = V3(-INFINITY, -INFINITY, -INFINITY);
    
    for(u32 index = 1;
         index < count;
        ++index)
    {
        v3 pos = vertices[index].pos;
        
        // VMin
        if(pos.x < vMin.x)
        {
            vMin.x = pos.x;
        }
        
        if(pos.y < vMin.y)
        {
            vMin.y = pos.y;
        }
        
        if(pos.z < vMin.z)
        {
            vMin.z = pos.z;
        }
        
        // vMax
        if(pos.x > vMax.x)
        {
            vMax.x = pos.x;
        }
        
        if(pos.y > vMax.y)
        {
            vMax.y = pos.y;
        }
        
        if(pos.z > vMax.z)
        {
            vMax.z = pos.z;
        }
    }
    
	collision_bounding result = { BOUNDING_AABB, (vMin + vMax) * 0.5f , (vMax - vMin) * 0.5 };
    return result;
}

internal void
GenerateBoundingAABB(render_buffer *buffer, color shapeColor = YELLOW)
{
	vertex vertices[] =
	{
		// Front
		Vertex( -0.5f, -0.5f, -0.5f, shapeColor ),
		Vertex( -0.5f, +0.5f, -0.5f, shapeColor ),
		Vertex( +0.5f, +0.5f, -0.5f, shapeColor ),
		Vertex( +0.5f, -0.5f, -0.5f, shapeColor ),
		// Back
		Vertex( -0.5f, -0.5f, +0.5f, shapeColor ),
		Vertex( -0.5f, +0.5f, +0.5f, shapeColor ),
		Vertex( +0.5f, +0.5f, +0.5f, shapeColor ),
		Vertex( +0.5f, -0.5f, +0.5f, shapeColor ),
	};
	
	u32 indices[] = 
	{
        0, 1, 2, 3, 0 ,3, 1, 2,
        4, 5, 6, 7, 4, 7, 5, 6,
        0, 4, 1, 5, 2, 6, 3, 7
	};
	
	render_target target = { vertices, 8, sizeof(vertex), indices, 24, 0 };
	*buffer = g_render->registerObject(&target);
}
