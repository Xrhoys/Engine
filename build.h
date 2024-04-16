/* date = February 16th 2024 2:48 am */

#ifndef BUILD_H
#define BUILD_H

#define internal static 
#define local static 
#define global static

#include "asset.h"
#include "build.h"
#include "font.h"
#include "build.h"

#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define CANVAS_SIZE 512

/*///////////////////////////////
Data structure of asset pack
-- File start
PACK_DATA_HEADER: 12 bytes
PACK_ITEM_DESC[]: numblock * sizeof(pack_item_desc)
DATA: 4 bytes aligned data
-- File end

NOTE: upon loading a virtual directory (n-ary tree) will be created to handle relations between assets
For instance: model + animation + texture + shadow maps etc.

This allows non-continious indexing, but breaks sequential reading ...
Welp, we could just re-arrange stuff do be as packed as possible
///////////////////////////////*/

struct pack_data_header
{
	char reserved[4];
	char serial[4];
	u32  numblock;
};

struct pack_item_desc
{
	char       path[MAX_PATH_LENGTH];
	u64        offset;
	u64        size;
	asset_type type;
};

struct pack_item_shader
{
};

struct pack_item_texture
{
};

struct pack_item_model
{
};

struct pack_item_animation
{
};

struct pack_item_font
{
};

struct pack_item_material
{
};

#endif //BUILD_H
