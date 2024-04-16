#include "font.h"

internal void
FontLoadAsset(u8 *data, font_map *map)
{
	font_asset *header = (font_asset*)data;
	memcpy(map->label, header->label, FONT_TEXT_MAX_SIZE);
}
