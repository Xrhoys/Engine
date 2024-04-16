/* date = February 16th 2024 1:10 am */

#ifndef MEMORY_H
#define MEMORY_H

#define MAX_PATH_LENGTH 64
#define PATH_SEPARATOR '/'

struct arena_memory
{
	void *data;
	u32  size;
	u8   *cursor;
}; 

inline void
Memset(void* src, void* dest, u32 size)
{
	u8* srcCursor  = (u8 *)src;
	u8* destCursor = (u8 *)dest;
	
	for (u32 index = 0;
		 index < size;
		 index++)
	{
		destCursor[index] = srcCursor[index];
	}
}

inline u8*
AllocArena(arena_memory *arena, u32 size)
{
	// Assert(arena->size > (UINT*)arena->cursor + size);
	
	u8* ptr = arena->cursor;
	arena->cursor += size;
	return ptr;
}

inline arena_memory
AllocArena(void *data, u32 size)
{
	arena_memory block;
	
	block.data = data;
	block.size = size;
	block.cursor = (u8*)data;
	
	return block;
}

inline void
PopArena(arena_memory *memory, u32 size)
{
	Assert(&(u8*)memory->cursor - size <= &(u8*)memory->data);
	memory->cursor -= size;
}

inline void
ZeroMem(void *memory, u32 size)
{
	u8  *cursor = (u8*)memory;
	u32 tempSize = size;
	while(tempSize--)
	{
		*cursor++ = 0;
	}
}

struct string
{
	char *contents;
	u32 size;
};

inline string
string_MakeFromChar(char *text)
{
	string str = { text, 0 };
	while(*text++ != '\0')
	{
		str.size++;
	}
	
	return str;
};

inline void
string_GeneratePath(string *array, u32 count, string *output)
{
	char *cursor = output->contents;
	for(u32 index = 0;
		index < count;
		++index)
	{
		memcpy(cursor, array[index].contents, array[index].size);
		cursor += array[index].size;
		*cursor++ = PATH_SEPARATOR;
	}
	
	output->size = cursor - output->contents - 1;
};

#endif //MEMORY_H
