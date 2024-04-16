/* date = March 6th 2023 3:10 am */

#ifndef FONT_H
#define FONT_H

#define FontListSize 95
#define ASCII_MAPPING_DEBUT_CURSOR 32
#define FONT_TEXT_MAX_SIZE 256
#define EOF (-1)

#define LABEL_MAX_SIZE 32

struct font_glyph
{
	u16 x0, y0, x1, y1; // coordinates of bbox in bitmap
	r32 xoff, yoff, xadvance;
	r32 xoff2, yoff2;
};

struct font_map
{
	char label[LABEL_MAX_SIZE];
	i32 textureAssetId;
	
	font_glyph glyphs[FONT_TEXT_MAX_SIZE];
	u32 characterOffset;
};

struct font_asset
{
	char word[4];
	char label[LABEL_MAX_SIZE];
	r32 baseSize;
	u32 numChars;
	u32 numOffset;
	
	u32 tWidth;
	u32 tHeight;
	
	u32 glyphDataOffset;
	u32 textureDataOffset;
};

#endif //FONT_H
