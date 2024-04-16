// TODO(Ecy): to refactor later
inline dds_file 
LoadDdsImage(debug_read_file_result file)
{
	dds_file output = {};
    Assert(*(u32 *)file.contents == 0x20534444); // NOTE: dds magic word
    
    void* Data = file.contents;
    
    dds_header *Header = (dds_header *)((u8 *)Data + sizeof(u32));
    
    if(Header->ddspf.dwFlags == 0x4 && Header->ddspf.dwFourCC == 'DX10') // DDPF_FOURCC
    {
        // TODO: not surpported
		return output;
    }
    
    u32 Pitch;
    //if(Header->ddspf.dwFourCC == '1TXD')
    {
        //Pitch = max(1, ((Header->dwWidth + 3)/4))*8;
        Pitch = (Header->dwWidth * 32 + 7)/8;
    }
    
    output.header = *Header;
    output.pitch = Pitch;
    output.size = Header->dwHeight*output.pitch;
    output.data = (u8 *)Data + sizeof(dds_header) + 4;
    
    return output;
}

internal tex_bitmap
TextureParseBitmapInfo(debug_read_file_result fileHandle)
{
	tex_bitmap map = {};
	
	u8 *cursor = (u8*)fileHandle.contents;
	tex_bitmap_header *bmHeader = (tex_bitmap_header*)cursor;
	u8 *data = cursor + bmHeader->offset;
	cursor += sizeof(tex_bitmap_header);
	tex_bitmap_info_header *info = (tex_bitmap_info_header*)cursor;
	
	memcpy(&map.header, bmHeader, sizeof(tex_bitmap_header));
	memcpy(&map.dib, info, sizeof(tex_bitmap_info_header));
	map.data = data;
	
	return map;
}
