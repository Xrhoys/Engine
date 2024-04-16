/* date = February 15th 2024 10:01 pm */

#ifndef ASSET_H
#define ASSET_H

#define ASSET_SIZE 128

/*////////////////////////////////
Asset/Rendere v2
The notion of asset is seperate from files.
The assumptions is that currently all the files will be loaded at once and asset will be pulled to gpu
whenever necessairy

===

Types of asset handled
- static scene element
- skinned models
- animation (pre-rendered keyframe)
- compiled shader
////////////////////////////////*/

struct render_buffer_load_desc
{
	u32 flags, stride;
	u8 *vertexData, *indexData;
	u32 vertexCount, indexCount;
	b8 isDynamic;
};

enum asset_type
{
	ASSET_TYPE_RAW,
	ASSET_TYPE_MODEL,
	ASSET_TYPE_TEXTURE,
	ASSET_TYPE_MATERIAL,
	ASSET_TYPE_FONT,
	ASSET_TYPE_ANIMATION,
	ASSET_TYPE_SHADER,
	
	ASSET_COUNT,
};

struct asset_item
{
	i32 id;
	u32 flags;
	u8 *memory_;
	
	asset_type type;
};

struct asset_group
{
	asset_item assets[ASSET_SIZE_PER_GROUP];
	u32 flags;
};

struct asset_make_desc
{
	void *vertices, *indices;
	u32 vertexCount, indexCount;
};

struct asset_context
{
	render_asset               assets[ASSET_SIZE];
	u32                        assetIds_;
};

inline render_asset*
AssetAlloc(asset_context *context)
{
	for(u32 index = 0;
		index < ASSET_SIZE;
		++index)
	{
		if(!context->assets[index].isLoaded)
		{
			context->assets[index].isLoaded = true;
			context->assets[index].id = ++context->assetIds_;
			
			return &context->assets[index];
		}
	}
	
	return 0;
}

#endif //ASSET_H
