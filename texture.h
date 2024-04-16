/* date = January 30th 2024 10:18 pm */

#ifndef TEXTURE_H
#define TEXTURE_H

#pragma pack(push, 1)
struct tex_bitmap_header
{
	// header: 14 bytes
	char formatID[2];
	u32 size;
	u32 pad;
	u32 offset; // start of the data array
};

struct tex_bitmap_info_header
{
	u32 headerSize;
	i32 width;
	i32 height;
	u16 numPlanes;
	u16 bpp; // bits per pixel
	char compression[4];
	u32 imageSize;
	i32 hres;
	i32 vres;
	u32 numColors; // number of colors in the palette
	u32 numImpColors; // number of important colors used
};
#pragma pack(pop)

struct tex_bitmap
{
	tex_bitmap_header header;
	tex_bitmap_info_header dib; // assumes bitmap's dib header is of BM type (win nt, win 3.1)
	u8 *data;
};

#pragma pack(push, 1)
// NOTE: There's not really a use for the header right now, maybe later
struct dds_pixel_format
{
    u32 dwSize;
    u32 dwFlags;
    u32 dwFourCC;
    u32 dwRGBBitCount;
    u32 dwRBitMask;
    u32 dwGBitMask;
    u32 dwBBitMask;
    u32 dwABitMask;
};

struct dds_header
{
    u32              dwSize;
    u32              dwFlags;
    u32              dwHeight;
    u32              dwWidth;
    u32              dwPitchOrLinearSize;
    u32              dwDepth;
    u32              dwMipMapCount;
    u32              dwReserved1[11];
	dds_pixel_format ddspf;
    u32              dwCaps;
    u32              dwCaps2;
    u32              dwCaps3;
    u32              dwCaps4;
    u32              dwReserved2;
};

#pragma pack(pop)

struct dds_file
{
	dds_header header;
	u32  size;
	u32  pitch;
	void *data;
};

#endif //TEXTURE_H
