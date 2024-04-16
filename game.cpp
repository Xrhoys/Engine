#include "game.h"

#include "camera.cpp"
//#include "ui.cpp"
#include "texture.cpp"

//extern "C" GAME_INIT(GameInit)
internal void
GameInit(platform_engine *engine)
{
	memory_context *memory = &engine->memory;
	input_context  *input = &engine->input;
	timer_context  *clock = &engine->clock;
	game_state     *gameState = &engine->game;
	
	// Game states
	{
		u8* memoryStartAt = AllocArena(&memory->transientArena, Megabytes(32));
		gameState->arena = AllocArena(memoryStartAt, Megabytes(16));
		
		// NOTE(Ecy): totally arbitrary for now
		gameState->entities = (entity*)AllocArena(&gameState->arena, sizeof(entity) * 256);
	}
	
	InitCamera(&gameState->playerCamera, engine->windowWidth, engine->windowHeight, 70);
	//UIInit(platform);
	
	input->sensX = CAMERA_SENSITITY;
	input->sensY = CAMERA_SENSITITY;
	input->sensZ = CAMERA_SENSITITY;
	
#if 0	
	v3 axis = V3(0, 1, 0);
	// Skinned entities
	ent = EntityMake(gameContext);
	ent->assetId  = modelAsset->id;
	ent->isSkinned = true;
	ent->enablePhysics = true;
	ent->acceleration = V3(0.f, PHYSICS_GRAVITY, 0.f);
	//ent->skeleton = &characterBones;
	ent->position = V3(0, 50.0f, -5.0f);
	ent->scale    = V3(0.015f, 0.015f, 0.015f);
	ent->rotation = Q4Identity();
	ent->anim     = &animation;
	ent->material =
	{
		{ 0.1f, 0.1f, 0.1f, 1.0f },
		{ 0.4f, 0, 0, 1.0f },
		{ 1.000f, 0.782f, 0.344f, 0.3f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
	};
	ent->enableCollision = true;
	ent->collision = collision_Make(COLLISION_AABB);
	// NOTE(Ecy): most of the collision boxes are pre-computed anyway 
	ent->collision.center = ent->position;
	ent->collision.stride = V3(1.f, 3.f, 1.f);
#endif

	// Static entities
	gameState->floorEnt = game_MakeEntity(gameState);
	entity *floorEnt = gameState->floorEnt;
	floorEnt->position = V3(0, -2.0f, -10.0f);
	floorEnt->scale = V3(20.0f, 0.1f, 20.0f);
	floorEnt->isSkinned = false;
	//floorEnt->assetId = lightCube->id;
	floorEnt->rotation = Q4Identity();
	floorEnt->material = 
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 1.0f, 0.0f },
		{ 1.000f, 0.782f, 0.344f, 0.5f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
	};
	floorEnt->enableCollision = true;
	floorEnt->collision = collision_Make(COLLISION_AABB);
	// NOTE(Ecy): most of the collision boxes are pre-computed anyway 
	floorEnt->collision.center = floorEnt->position;
	floorEnt->collision.stride = V3(20.f, .1f, 20.f);
	
	r32 count = (r32)ArrayCount(gameState->testBounce);
	for(u32 i = 0;
		i < CUBE_FALLING_COUNT;
		++i)
	{
		r32 x = i * 3.f - 18.f;
		for(u32 j = 0;
			j < CUBE_FALLING_COUNT;
			++j)
		{
			r32 y = j * 3.0f - 30.0f;
			gameState->testBounce[i * 5 + j] = game_MakeEntity(gameState);
			entity *currentEnt = gameState->testBounce[i * 5 + j];
			
			currentEnt->position = V3(x, 10.f * (i * .04f + j * .04f), y);
			currentEnt->scale = V3(1.0f, 1.0f, 1.0f);
			currentEnt->isSkinned = false;
			currentEnt->enablePhysics = true;
			currentEnt->acceleration = V3(0.f, PHYSICS_GRAVITY, 0.f);
			//currentEnt->assetId = lightCube->id;
			currentEnt->rotation = Q4Identity();
			currentEnt->material = 
			{
				{ 0.0f, 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.2f, 0.5f, 1.0f },
				{ 0.1f, 0.1f, 0.344f, 0.7f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
			};
			currentEnt->enableCollision = true;
			currentEnt->collision = collision_Make(COLLISION_AABB);
			// NOTE(Ecy): most of the collision boxes are pre-computed anyway 
			currentEnt->collision.center = floorEnt->position;
			currentEnt->collision.stride = V3(1.0f, 1.0f, 1.0f);
			
		}
		
	}
}

//extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
internal void
GameUpdate(platform_engine *engine)
{
	memory_context *memory = &engine->memory;
	input_context  *input = &engine->input;
	timer_context  *clock = &engine->clock;
	game_state     *gameState = &engine->game;
	game_camera    *playerCamera = &gameState->playerCamera;
	
	r32 dt = clock->dt;
	
	// TODO(Ecy): attach to playerCamera/entity, non null script movement
	playerCamera->yaw = -input->dragY;
	playerCamera->pitch = -input->dragX;
	
	input_controller *keyboard = &input->controllers[0];
	r32 forward = 0.0f, 
	strife = 0.0f;
	if(keyboard->w.halfTransitionCount > 0 && keyboard->w.endedDown)
	{
		forward += CAMERA_MOVE_SPEED * dt;
	}
	if(keyboard->s.halfTransitionCount > 0 && keyboard->s.endedDown)
	{
		forward -= CAMERA_MOVE_SPEED * dt;
	}
	if(keyboard->a.halfTransitionCount > 0 && keyboard->a.endedDown)
	{
		strife += CAMERA_MOVE_SPEED * dt;
	}
	if(keyboard->d.halfTransitionCount > 0 && keyboard->d.endedDown)
	{
		strife -= CAMERA_MOVE_SPEED * dt;
	}
	
	v3 changes = playerCamera->target * forward + Cross(&playerCamera->target, &playerCamera->up) * strife;
	playerCamera->position += Normalize(&changes);
	
	UpdateCamera(playerCamera, playerCamera->position);
	
	//render_draw_command command = {};
	//camera.wvp = MatrixIdentity() * camera.view * camera.projection3d;
	//render->context->perFrameConstants.wvp = Transpose(&camera.wvp);
	gameState->lights[0] =
	{
		{ 0.3f, 0.3f, 0.3f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		
		{ -5.0f, 20.0f, -10.0f },
		2000.0f,
		
		{ 0, -1, 0 },
		10.0f,
		
		{ 0.001f, 0.001f, 0.001f },
		LT_SOURCE_SPOT_GGX,
	};
	
	// NOTE(Ecy): physics simulation experiment
	{
		for(u32 index = 0;
			index < gameState->count;
			++index)
		{
			entity *ent = &gameState->entities[index];
			
			if(ent->enablePhysics)
			{
				ent->velocity += ent->acceleration * dt;
				ent->position += ent->velocity * dt;
			}
		}
		
		entity *floorEnt = gameState->floorEnt;
		// NOTE(Ecy): collision with floor
		r32 floorLevel = floorEnt->position.y + floorEnt->collision.stride.y;
		for(u32 i = 0;
			i < gameState->count;
			++i)
		{
			entity *ent = &gameState->entities[i];
			
			if(ent != floorEnt)
			{
				if(ent->enableCollision)
				{
					b32 colliding = CollisionAABB(floorEnt->position, floorEnt->collision.stride, 
												  ent->position, ent->collision.stride);
					if(colliding)
					{
						if(floorLevel + ent->collision.stride.y - ent->position.y > FLT_EPSILON)
						{
							ent->position.y = floorLevel + ent->collision.stride.y;
							ent->velocity *= -PHYSICS_BOUNCE_RESTITUTION;
							
							// NOTE(Ecy): stickness is the absorption rate of the material and should set
							// a floor for not bouncing condition. Ideally it should be expressed in energy(J)
							// so f = 0.5*m*v2
							// For better accuray, also take into account area of contact
							// a bigger surface, leads to more energy absorbed
							if(fabs(ent->velocity.y) < PHYSICS_STIKINESS)
							{
								ent->velocity.y = 0.f;
							}
						}
					}
				}
			}
		}
	}
	
	for(u32 index = 0;
		index < gameState->count;
		++index)
	{
		entity *ent = &gameState->entities[index];
		
		//command.assetId = ent->assetId;
		//command.textureAssetId = whiteTex->id;
		//command.perObject.transform = GenerateTransform(&ent->position, &ent->scale, &ent->rotation);
		
		// TODO(Ecy): to remove once material is integrated with asset pipeline
		//command.perObject.material = ent->material;
		
		//matrix invert = MatrixInverse(&command.perObject.transform);
		//command.perObject.transformInv = invert;
		
		if(ent->isSkinned)
		{
			Assert(ent->anim);
			skeletal_animation *anim = ent->anim;
			skeletal_key_frame *keyFrames = anim->keyFrames;
			
			// Update local transforms of each bone
			skeletal_key_frame *lastPoseRef, *nextPoseRef;
			// Find timestamp interval
			for(u32 index = 0;
				index < anim->keyFrameCount;
				++index)
			{
				if(anim->currentTime > (anim->keyFrameCount - 1) * (1.0f / anim->framerate)) 
				{
					anim->currentTime = 0.001f;
				}
				
				if(anim->currentTime >= keyFrames[index].timestamp && 
				   anim->currentTime < keyFrames[index + 1].timestamp)
				{
					lastPoseRef = &keyFrames[index];
					nextPoseRef = &keyFrames[index + 1];
					break;
				}
			}
			
			if(lastPoseRef && nextPoseRef)
			{
				skeletal_animation *animation = &gameState->animation;
				// Compute interpolation
				for (u32 index = 0;
					 index < ent->anim->boneCount;
					 ++index)
				{
					// Lerp position
					r64 t = (r64)(animation->currentTime - lastPoseRef->timestamp) / (r64)nextPoseRef->timestamp;
					v3 interpolatedPos = Lerp(lastPoseRef->position[index],
											  nextPoseRef->position[index],
											  t);
					v3 interpolatedScale = Lerp(lastPoseRef->scale[index],
												nextPoseRef->scale[index],
												t);
					// Slerp orientation
					q4 slerpedOrientation = SLerp(lastPoseRef->rotation[index], nextPoseRef->rotation[index], t);
					// Compute world transforms
#if 0
					command.perObject.bones[index] = GenerateTransform(&interpolatedPos,
																	   &interpolatedScale,
																	   &slerpedOrientation);
#endif
				}
				
				anim->currentTime += clock->dt;
			}
			//render->queueDrawCommand(render->context->skinnedGroup, &command);
		}
		else
		{
			// TODO(Ecy): ok that is not good, should not be referencing to platform specific implementation ...
			//render->queueDrawCommand(render->context->staticGroup, &command);
		}
	}
	
#if 0
	{
		// TODO(Ecy): update the update API into 2 different calls (Map/Unmap)
		vertex_debug_instanced vertices[256];
		u32 vertexCount = 0;
		
		render_draw_debug_command debugCommand = { RENDER_DEBUG_CUBE, 0 };
		
		q4 defaultRotation = Q4Identity();
		for(u32 index = 0;
			index < gameContext->entitiesCount;
			++index)
		{
			entity *ent = &gameContext->entities[index];
			
			if(ent->enableCollision)
			{
				
				if(ent->collision.type == COLLISION_AABB)
				{
					matrix transform = GenerateTransform(&ent->position, &ent->collision.stride, &defaultRotation);
					vertices[vertexCount++] = { transform, V4(0.0f, .5f, .5f, .3f) };
					debugCommand.count++;
				}
			}
		}
		
		render->updateDebug(render->context, vertices, vertexCount);
		render->queueDebugCommand(&render->context->debugGroup, &debugCommand);
	}
#endif

#if 0	
	// UI
	UIBegin(platform);
	{
		// TODO(Ecy): Just put bump allocator here lol
		char buffer[64];
		u32 count = _snprintf_s(buffer, sizeof(buffer), sizeof(buffer), 
								"Mouse position: %.0f;%.0f", uiContext.mouseEvent.pos.x, uiContext.mouseEvent.pos.y);
		string mousePos = { buffer, count };
		
		char timerBuffer[64];
		count = _snprintf_s(timerBuffer, sizeof(timerBuffer), sizeof(timerBuffer), 
							"Time: %.2f", clock->frameStart);
		string timer = { timerBuffer, count };
		
		char frameTime[32];
		count = _snprintf_s(frameTime, sizeof(frameTime), sizeof(frameTime), 
							"fps: %.2f", 1.0f / clock->now());
		string frameTimeStr = { frameTime, count };
		
		char cameraPosition[32];
		count = _snprintf_s(cameraPosition, sizeof(cameraPosition), sizeof(cameraPosition), 
							"Cam: %.2f, %.2f, %.2f", camera.position.x, camera.position.y, camera.position.z);
		string camPos = { cameraPosition, count };

#if 0		
		char entStats[64];
		count = _snprintf_s(entStats, sizeof(entStats), sizeof(entStats), 
							"Char: %.2f, %.2f, %.2f", ent->velocity.x, ent->velocity.y, ent->velocity.z);
		string entStatsString = { entStats, count };
#endif

		ui_style style = {};
		style.size[UI_AXIS2_X] = { UI_SIZEKIND_PIXELS, 400.f, 0 };
		style.size[UI_AXIS2_Y] = { UI_SIZEKIND_PERCENTOFPARENT, 0.75f, 0 };
		style.layoutFlags = UI_LAYOUTFLAG_EXPAND_V;
		
		ui_Parent()
		{
			ui_Style(&style)
			{
				ui_Frame(string_MakeFromChar("Data"))
				{
					ui_Text(mousePos);
					ui_Text(camPos);
					ui_Text(timer);
					ui_Text(frameTimeStr);
					//ui_Text(entStatsString);
				}
			}
		}
	}
	UIUpdateAndRender(platform);
#endif
}
