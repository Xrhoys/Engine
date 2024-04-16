/* date = October 16th 2022 11:20 pm */

#ifndef GAME_H
#define GAME_H

#define color v4
#define Color V4

// NOTE: Raylib color palette
#define LIGHTGRAY  Color(200, 200, 200, 255)        // Light Gray
#define GRAY       Color(130, 130, 130, 255)        // Gray
#define DARKGRAY   Color( 80,  80,  80, 255)        // Dark Gray
#define YELLOW     Color(253, 249,   0, 255)        // Yellow
#define GOLD       Color(255, 203,   0, 255)        // Gold
#define ORANGE     Color(255, 161,   0, 255)        // Orange
#define PINK       Color(255, 109, 194, 255)        // Pink
#define RED        Color(230,  41,  55, 255)        // Red
#define MAROON     Color(190,  33,  55, 255)        // Maroon
#define GREEN      Color(  0, 228,  48, 255)        // Green
#define LIME       Color(  0, 158,  47, 255)        // Lime
#define DARKGREEN  Color(  0, 117,  44, 255)        // Dark Green
#define SKYBLUE    Color(102, 191, 255, 255)        // Sky Blue
#define BLUE       Color(  0, 121, 241, 255)        // Blue
#define DARKBLUE   Color(  0,  82, 172, 255)        // Dark Blue
#define PURPLE     Color(200, 122, 255, 255)        // Purple
#define VIOLET     Color(135,  60, 190, 255)        // Violet
#define DARKPURPLE Color(112,  31, 126, 255)        // Dark Purple
#define BEIGE      Color(211, 176, 131, 255)        // Beige
#define BROWN      Color(127, 106,  79, 255)        // Brown
#define DARKBROWN  Color( 76,  63,  47, 255)        // Dark Brown
#define WHITE      Color(255, 255, 255, 255)        // White
#define BLACK      Color(  0,   0,   0, 255)        // Black
#define BLANK      Color(  0,   0,   0,   0)        // Transparent
#define MAGENTA    Color(255,   0, 255, 255)        // Magenta
#define RAYWHITE   Color(245, 245, 245, 255)        // Ray White

#define SCREEN_WIDTH  1600
#define SCREEN_HEIGHT 900

#define CAMERA_MOVE_SPEED 10.0f
#define CAMERA_SENSITITY 0.001f

#define PHYSICS_GRAVITY -9.81f
#define PHYSICS_BOUNCE_RESTITUTION 0.999f
#define PHYSICS_STIKINESS 2.0f

#define CUBE_FALLING_COUNT 12

struct entity;

#define ENTITY_BUCKET_SIZE 1024

#include <stdio.h> // NOTE: replace that later (_snprintf_s)
#include "math.h"
#include "collision.h"
#include "renderer.h"
#include "camera.h"
//#include "entity.h"
//#include "animation.h"
#include "font.h"
//#include "player.h"
#include "input.h"
#include "skeletal_animation.h"
#include "ui.h"
#include "texture.h"
#include "audio.h"
#include "asset.h"

struct game_memory_header
{
	u32 id;
	b32 isSet;
};

struct entity
{
	i32 assetId;
	
	b8 isSet;
	b8 isSkinned;
	b8 enablePhysics;
	b8 enableCollision;
	
	// Skinned
	
	// NOTE(Ecy): this could be useful for debug information
	// Would not support blending in this format :think:
	skeletal_def *skeleton;
	skeletal_animation *anim;
	
	v3 position;
	v3 scale;
	q4 rotation;
	v3 velocity;
	v3 acceleration;
	
	// Collision related stuff
	collision_node collision;
	
	// NOTE(Ecy): This is not great, the concept of rendering and game logics 
	lt_material material;
	
	// Rendering interface
	render_entity *renderEntity;
	render_visibility *visibility;
};

struct game_state
{
	arena_memory arena;
	
	// Scene data
	game_camera playerCamera;
	entity *entities;
	u32    count;
	lt_source lights[MAX_LIGHT_COUNT];
	
	// DEBUG TEMPORARY
	render_asset *whiteTex; 
	render_asset *lightCube; 
	render_asset *fontTex; 
	render_asset *modelAsset;
	entity *floorEnt, *ent;
	entity *testBounce[CUBE_FALLING_COUNT * CUBE_FALLING_COUNT];
	skeletal_animation animation;
	skeletal_animation cubeAnim;
};

inline entity*
game_MakeEntity(game_state *state, b32 isDynamic = false)
{
	for(u32 index = 0;
		index < state->count;
		++index)
	{
		if(!state->entities[index].isSet)
		{
			state->entities[index].isSet = true;
			return &state->entities[index];
		}
	}
	
	Assert(state->count < ENTITY_BUCKET_SIZE);
	entity *ent = &state->entities[state->count++];
	// TODO(Ecy): make hash table of entity instead
	ent->isSet = true;
	
	if(isDynamic)
	{
		ent->renderEntity = render_RegisterRenderEntity(ent);
	}
	else
	{
		ent->renderEntity = render_RegisterRenderEntity(NULL);
	}
	ent->visibility = render_RegisterVisibility(ent->renderEntity);
	
	return ent;
}

inline void
EntityFree(entity *ent)
{
	ent->isSet = false;
}

#endif //GAME_H
