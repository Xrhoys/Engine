/* date = October 9th 2022 2:04 am */

#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
    
#define internal static 
#define local static 
#define global static
	
#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
	
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))
#define Clamp(value, low, high) ((value) < (high)) ? (((value) > (low)) ? (value) : (low)) : (high) 
#define Abs(x) (x) > 0 ? (x) : -(x)
	
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
	
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
	
    typedef int8_t   i8;
    typedef int16_t  i16;
    typedef int32_t  i32;
    typedef int64_t  i64;
    typedef int32_t  b32;
    typedef int8_t   b8;
    
    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    
    typedef float    r32;
    typedef double   r64;
	
	enum thread_state
	{
		THREAD_STATE_READY,
		THREAD_STATE_LOCK,
		
		THREAD_STATE_COUNT,
	};
	
    typedef struct thread_context
	{
		thread_state state;
		u32          threadID;
	} thread_context;
	
#if DEBUG
	
	typedef struct debug_read_file_result
	{
		u32 contentSize;
		void *contents;
	} debug_read_file_result;
	
	// NOTE: there's nothing in thread context yet, thus debug
	// TODO(Ecy): this is stupid, you should pass the file handle instead of raw pointer
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *memory)
	typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *thread, char *filename)
	typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(thread_context *thread, char *filename, u32 memorySize, void* memory)
	typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
	
#endif
	
#define GAME_UPDATE_AND_RENDER(name) void name()
	typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
	
#define GAME_INIT(name) void name()
	typedef GAME_INIT(game_init);
	
#ifdef __cplusplus
}
#endif

#endif //PLATFORM_H
