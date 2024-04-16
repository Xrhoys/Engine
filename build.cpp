#include "platform.h"

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>

#define STB_TRUETYPE_IMPLEMENTATION 1
#include <stb_truetype.h>

#include "build.h"

// TOOD(Ecy): merge this later with runtime OS layer
internal
DEBUG_PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
	debug_read_file_result File = {};
    
    void *Result = 0;
    DWORD BytesRead;
    u32 FileSize32;
    HANDLE FileHandle = CreateFileA(filename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            // NOTE: Will cause problem for 64bits
            FileSize32 = (u32)FileSize.QuadPart;
            Result = VirtualAlloc(0, FileSize.QuadPart, 
                                  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            if(Result)
            {
                if(ReadFile(FileHandle, Result, FileSize32, &BytesRead, 0))
                {
                    File.contents = Result;
                    File.contentSize = FileSize32;
                }
                else
                {
                    VirtualFree(Result, 0, MEM_RELEASE);
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }
    
    return File;
}

internal
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(Win32WriteEntireFile)
{
	b32 result = false;
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if(WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
        {
            result = (bytesWritten == memorySize);
        }
        else
        {
            // TODO: Logging
        }
		
        CloseHandle(fileHandle);
    }
    else
    {
        // TODO: Logging
    }
	
    return(result);
}

internal
DEBUG_PLATFORM_FREE_FILE_MEMORY(Win32FreeFileMemory)
{
	VirtualFree(memory, 0, MEM_RELEASE);
}

static void
BuildFontData()
{
	debug_read_file_result ttfFile = Win32ReadEntireFile(NULL, "arial.ttf");
	
	stbtt_fontinfo font;
	stbtt_InitFont(&font, (u8*)ttfFile.contents, stbtt_GetFontOffsetForIndex((u8*)ttfFile.contents, 0));		
	
	// NOTE(Ecy): allocate enough memory for output buffer
	u8 *pixelBuffer = (u8*)VirtualAlloc(0, CANVAS_SIZE * CANVAS_SIZE * sizeof(u32),
										MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	u8 *writeBuffer = (u8*)VirtualAlloc(0, 10 * 1024 * 1024,
										MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	stbtt_packedchar bakedChar[256] = {};
	
	u8 *cursor = pixelBuffer;
	
	//i32 res = stbtt_BakeFontBitmap((u8*)ttfFile.contents, 0, 20, writeBuffer, CANVAS_SIZE, CANVAS_SIZE, 0, 256, bakedChar);
	stbtt_pack_context context = {};
	i32 res = stbtt_PackBegin(&context, cursor, CANVAS_SIZE, CANVAS_SIZE, 0, 1, NULL);
	stbtt_PackSetOversampling(&context, 2, 2);
	res = stbtt_PackFontRange(&context, (u8*)ttfFile.contents, 0, STBTT_POINT_SIZE(20), 0, 256, bakedChar);
	
	stbtt_PackEnd(&context);
	
	u32 *outputBuffer = (u32*)writeBuffer;
	for(u32 index = 0;
		index < CANVAS_SIZE;
		++index)
	{
		for(u32 j = 0;
			j < CANVAS_SIZE;
			++j)
		{
			u8 pixel = pixelBuffer[index * CANVAS_SIZE + j];
			outputBuffer[index * CANVAS_SIZE + j] = ((pixel << 24) |
													 (pixel << 16) |
													 (pixel <<  8) |
													 (pixel <<  0) );
		}
		
	}
	
	Win32WriteEntireFile(NULL, "bakedMap.tex", CANVAS_SIZE * CANVAS_SIZE * sizeof(u32), writeBuffer);
	
	cursor = pixelBuffer;
	s
	font_asset asset = {};
	asset.baseSize = 20.0f;
	asset.numChars = 256;
	asset.numOffset = 0;
	asset.tWidth = CANVAS_SIZE;
	asset.tHeight = CANVAS_SIZE;
	asset.glyphDataOffset = sizeof(font_asset);
	asset.textureDataOffset = sizeof(bakedChar);
	memcpy(cursor, &asset, sizeof(asset));
	cursor += sizeof(font_asset);
	memcpy(cursor, bakedChar, sizeof(bakedChar));
	cursor += sizeof(bakedChar);
	
	Win32WriteEntireFile(NULL, "bakedMap.data", cursor - pixelBuffer, pixelBuffer);
	
#if 0	
	u32 maxHeight = 0;
	u32 totalWidth = 0;
	
	for(char index = 0;
		index < 256;
		++index)
	{
		i32 width, height, xoff, yoff;
		u8 *bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 16), 
											  index, &width, &height, &xoff, &yoff);
		
		//memcpy(cursor, bitmap, width * height);
		
		asset_font_glyph *currentGlyph = &fontData.glyphs[index - FONT_BASE_OFFSET];
		currentGlyph->glyph   = index;
		currentGlyph->_offset  = cursor - writeBuffer;
		currentGlyph->width   = width;
		currentGlyph->height  = height;
		currentGlyph->ratio   = (r32)width / (r32)height;
		currentGlyph->xoffset = xoff;
		currentGlyph->yoffset = yoff;
		
		totalWidth += width;
		if(height > maxHeight) maxHeight = height;
		
		cursor += width * height;
		
		stbtt_FreeBitmap(bitmap, 0);
	}
	
	fontData.width = totalWidth;
	fontData.height = maxHeight;
	
	u32 currentLineWidth = 0;
	r32 bitmapWidth = (r32)totalWidth;
	r32 bitmapHeight = (r32)maxHeight;
	for(u32 index = 0;
		index < MAX_ASCII_SUPPORTED_GLYPH;
		++index)
	{
		asset_font_glyph *currentGlyph = &fontData.glyphs[index];
		
		r32 offsetWidth = (r32)currentLineWidth;
		
		currentGlyph->u = offsetWidth / bitmapWidth;
		currentGlyph->v = (r32)currentGlyph->height / bitmapHeight;
		
		currentLineWidth += currentGlyph->width;
	}
	
	u8 *pixelMapBuffer = (u8*)malloc(10*1024*1024);
	memcpy(pixelMapBuffer, &fontData, sizeof(asset_font));
	
	cursor = pixelMapBuffer + sizeof(asset_font);
	
	for(u32 line = 0;
		line < maxHeight;
		++line)
	{
		for(u32 index = 0;
			index < MAX_ASCII_SUPPORTED_GLYPH;
			++index)
		{
			asset_font_glyph *currentGlyph = &fontData.glyphs[index];
			if (line >= fontData.glyphs[index].height)
			{
				cursor += currentGlyph->width * 4;
			}
			else
			{
				u8 *writeBufferOffset = writeBuffer + currentGlyph->_offset + currentGlyph->width * line;
				
				u32 *dest = (u32*)cursor;
				for(u32 pixelIndex = 0;
					pixelIndex < currentGlyph->width;
					++pixelIndex)
				{
					u8 pixel = writeBufferOffset[pixelIndex];
					*dest++ = ((pixel << 24) |
							   (pixel << 16) |
							   (pixel <<  8) |
							   (pixel <<  0) );
					cursor += 4;
				}
				
			}
			
		}
	}
	
	WriteEntireFile(NULL, "asset_data", fontData.width * fontData.height * 4 + sizeof(asset_font), pixelMapBuffer);
#endif
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
	BuildFontData();
	
	return 0;
}