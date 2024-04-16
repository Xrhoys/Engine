/* date = October 9th 2022 2:55 am */

#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

#define DRAW_COMMAND_MAX_SIZE 512
#define REFRESH_RATE 60.0f
#define MAX_DEBUG_INSTANCE_SIZE 4096

#define MEMORY_BLOCK_SIZE Megabytes(1)
#define MEMORY_BLOCK_MAX_INDEX 128

struct memory_context
{
	b32 isInit;
	
	// For house keeping
	u64   permanentStorageSize;
	void* permanentStorage;
	
	u64   transientStorageSize;
	void* transientStorage;
	
	// Actual interface
	arena_memory permanentArena;
	arena_memory transientArena;
	
	debug_platform_read_entire_file  *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
	debug_platform_free_file_memory  *DEBUGPlatformFreeFileMemory;
};

struct timer_context
{
	r64 gameTime;
	r32 dt;
};

struct platform_engine
{
	b32 running;
	u32 windowWidth, windowHeight;
	
	audio_context  audio;
	memory_context memory;
	input_context  input;
	timer_context  clock;
	
	asset_context asset;
	
	game_state    game;
	
	// NOTE(Ecy): dedicated thread handles
	thread_context renderThread;
	thread_context gameThread;
};

#endif //WIN32_MAIN_H
