internal void
InitCamera(game_camera *camera, u32 width, u32 height, r32 fov)
{
	camera->position = V3(0.0f, 10.0f, 15.0f);
    camera->target   = V3(0.0f, -0.5f, -1.0f);
    camera->up       = V3(0.0f, 1.0f,  0.0f);
    camera->mode     = CAMERA_FPS;
    camera->fov      = Pi * fov / 180;
    camera->ratio    = (r32)height / (r32)width;
    
    CameraLookAtLH(&camera->view, camera->position, camera->target, camera->up);
    PerspectiveFovLH(&camera->projection3d, camera->fov, camera->ratio, 0.1f, 1000.0f);
    PerspectiveOrthLH(&camera->projection2d, width, height, 0.1f, 3.0f);
    
    camera->invProjection2d = Transpose(&camera->projection2d);
}

internal void
UpdateCamera(game_camera *camera, v3 position)
{
	// NOTE(Ecy): This is done once per frame, the hit is not that big, so let it live for now
    camera->target.x = cos(camera->yaw) * cos(camera->pitch);
    camera->target.y = sin(camera->pitch);
    camera->target.z = sin(camera->yaw) * cos(camera->pitch);
    
    if(camera->mode == CAMERA_ARC)
    {
        // r32 radius = 10.0f;
        camera->position = position - camera->target * camera->radius;
    }
	else
	{
		 camera->position = position;
	}
    
    // Update camera position and view direction
    v3 result = camera->position + camera->target;
    CameraLookAtLH(&camera->view, camera->position, result, camera->up);
}

internal void
BuildCubeMapCameras(game_camera *camArray, v3 position)
{
	game_camera cubeMapCameras[6] = {};
	r32 x = position.x;
	r32 y = position.y;
	r32 z = position.z;
	
	v3 targets[6] =
	{
		V3(x + 1.0f, y, z),
		V3(x - 1.0f, y, z),
		V3(x, y + 1.0f, z),
		V3(x, y - 1.0f, z),
		V3(x, y, z + 1.0f),
		V3(x, y, z - 1.0f),
	};
	
	v3 ups[6] = 
	{
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 0.0f, -1.0f),
		V3(0.0f, 0.0f, 1.0f),
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f),
	};
	
	for(u32 index = 0;
		index < 6;
		++index)
	{
		game_camera *cam = &camArray[index];
		cam->position = position;
		cam->target = targets[index];
		cam->up     = ups[index];
		cam->fov    = 0.5f * Pi;
		cam->ratio  = 1.0f;
		
		CameraLookAtLH(&cam->view, cam->position, cam->target, cam->up);
		PerspectiveFovLH(&cam->projection3d, cam->fov, cam->ratio, 0.1f, 1000.0f);
	}
}
