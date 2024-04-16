internal void
asset_Init(platform_engine *engine)
{
#if 0
	{
		// texture cube
		vertex vertices[] = 
		{
			// Front when looking at the cube 
			Vertex({ -1.0f, -1.0f, -1.0f }, { 0, 0, -1 }, { 0.0f, 0.0f }),
			Vertex({ -1.0f, +1.0f, -1.0f }, { 0, 0, -1 }, { 0.0f, 1.0f }),
			Vertex({ +1.0f, +1.0f, -1.0f }, { 0, 0, -1 }, { 1.0f, 1.0f }),
			Vertex({ +1.0f, -1.0f, -1.0f }, { 0, 0, -1 }, { 1.0f, 0.0f }),
			
			// Back
			Vertex({ -1.0f, -1.0f, +1.0f }, { 0, 0, 1 }, { 0.0f, 0.0f }),
			Vertex({ -1.0f, +1.0f, +1.0f }, { 0, 0, 1 }, { 0.0f, 1.0f }),
			Vertex({ +1.0f, +1.0f, +1.0f }, { 0, 0, 1 }, { 1.0f, 1.0f }),
			Vertex({ +1.0f, -1.0f, +1.0f }, { 0, 0, 1 }, { 1.0f, 0.0f }),
			
			// Top
			Vertex({ -1.0f, +1.0f, -1.0f }, { 0, 1, 0 }, { 0.0f, 0.0f }),
			Vertex({ -1.0f, +1.0f, +1.0f }, { 0, 1, 0 }, { 0.0f, 1.0f }),
			Vertex({ +1.0f, +1.0f, +1.0f }, { 0, 1, 0 }, { 1.0f, 1.0f }),
			Vertex({ +1.0f, +1.0f, -1.0f }, { 0, 1, 0 }, { 1.0f, 0.0f }),
			
			// Bottomz
			Vertex({ -1.0f, -1.0f, -1.0f }, { 0, -1, 0 }, { 0.0f, 0.0f }),
			Vertex({ +1.0f, -1.0f, -1.0f }, { 0, -1, 0 }, { 1.0f, 0.0f }),
			Vertex({ +1.0f, -1.0f, +1.0f }, { 0, -1, 0 }, { 1.0f, 1.0f }),
			Vertex({ -1.0f, -1.0f, +1.0f }, { 0, -1, 0 }, { 0.0f, 1.0f }),
			
			// Left
			Vertex({ -1.0f, +1.0f, -1.0f }, { -1, 0, 0 }, { 1.0f, 0.0f }),
			Vertex({ -1.0f, -1.0f, -1.0f }, { -1, 0, 0 }, { 0.0f, 0.0f }),
			Vertex({ -1.0f, -1.0f, +1.0f }, { -1, 0, 0 }, { 0.0f, 1.0f }),
			Vertex({ -1.0f, +1.0f, +1.0f }, { -1, 0, 0 }, { 1.0f, 1.0f }),
			
			// Right
			Vertex({ +1.0f, -1.0f, -1.0f }, { 1, 0, 0 }, { 0.0f, 0.0f }),
			Vertex({ +1.0f, +1.0f, -1.0f }, { 1, 0, 0 }, { 1.0f, 0.0f }),
			Vertex({ +1.0f, +1.0f, +1.0f }, { 1, 0, 0 }, { 1.0f, 1.0f }),
			Vertex({ +1.0f, -1.0f, +1.0f }, { 1, 0, 0 }, { 0.0f, 1.0f }),
		};
		
		u32 indices[] = 
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			
			// back face
			4, 6, 5,
			4, 7, 6,
			
			// top face
			8, 9, 10,
			8, 10, 11,
			
			// bottom face
			12, 13, 14,
			12, 14, 15,
			
			// left face
			16, 17, 18,
			16, 18, 19,
			
			// right face
			20, 21, 22,
			20, 22, 23,
		};
		
		render_buffer_load_desc desc = {};
		desc.vertexData = (u8*)vertices;
		desc.vertexCount = ArrayCount(vertices);
		desc.stride = sizeof(vertex_skinned);
		desc.indexData = (u8*)indices;
		desc.indexCount = ArrayCount(indices);
		
		engine.gameState.lightCube = AssetLoadModel(render, &desc);
	}
	
	
	// Load custom model format
	{
		debug_read_file_result file = memory->DEBUGPlatformReadEntireFile(NULL, "model.mh");
		Assert(file.contents);
		
		u8 *cursor = (u8*)file.contents;
		mesh_data *data = (mesh_data*)cursor;
		
		cursor += sizeof(mesh_data);
		
		render_buffer_load_desc desc = {};
		while(cursor < (u8*)file.contents + file.contentSize)
		{
			asset_block_header header = {};
			header.type = (asset_block_type)(*cursor++);
			memcpy(header.name, cursor, sizeof(header.name));
			cursor += sizeof(header.name);
			
			switch(header.type)
			{
				case ASSET_BLOCK_MESH:
				{
					header.numVertices = *(u32*)cursor;
					cursor += sizeof(u32);
					header.numIndices = *(u32*)cursor;
					cursor += sizeof(u32);
					header.vOffset = *(u32*)cursor;
					cursor += sizeof(u32);
					header.iOffset = *(u32*)cursor;
					cursor += sizeof(u32);
					
					desc.vertexData = cursor;
					desc.vertexCount = header.numVertices;
					desc.stride = sizeof(vertex_skinned);
					cursor += sizeof(vertex_skinned) * header.numVertices;
					desc.indexData = cursor;
					desc.indexCount = header.numIndices;
					cursor += sizeof(u32) * header.numIndices;
					
					
				}break;
				case ASSET_BLOCK_POSE:
				{
#if 0
					header.numBones = *(u32*)cursor;
					cursor += sizeof(u32);
					
					for(u32 index = 0;
						index < header.numBones;
						++index)
					{
						skeletal_bone *bone = &characterBones.bones[index];
						
						memcpy(bone->name, cursor, sizeof(bone->name));
						cursor += sizeof(bone->name);
						bone->id = *(i32*)cursor;
						cursor += sizeof(i32);
						bone->parent = *(i32*)cursor;
						cursor += sizeof(i32);
					}
					
					characterBones.count = header.numBones;
#endif
					
				}break;
				case ASSET_BLOCK_ANIMATION:
				{
					header.numKeyframes = *(u32*)cursor;
					cursor += sizeof(u32);
					
					header.numBones = *(u32*)cursor;
					cursor += sizeof(u32);
					
					animation.keyFrames = (skeletal_key_frame*)AllocArena(&assetMemory, 
																		  header.numKeyframes * sizeof(skeletal_key_frame));
					animation.keyFrameCount = header.numKeyframes;
					animation.framerate = 30.0f;
					
					for(u32 index = 0;
						index < header.numKeyframes;
						++index)
					{
						skeletal_key_frame *frame = &animation.keyFrames[index];
						
						memcpy(frame->rotation, cursor, sizeof(frame->rotation));
						cursor += header.numBones * sizeof(q4);
						memcpy(frame->position, cursor, sizeof(frame->position));
						cursor += header.numBones * sizeof(v3);
						memcpy(frame->scale, cursor, sizeof(frame->scale));
						cursor += header.numBones * sizeof(v3);
						
						frame->timestamp = *(r64*)cursor;
						cursor += sizeof(r64);
					}
					
				}break;
				
				case ASSET_BLOCK_NONE:
				default:
				{
					Assert(false);
				}break;
			}
			
			//cursor += header->size * header->stride;
		}
		
		debug_read_file_result fileAnim = memory->DEBUGPlatformReadEntireFile(NULL, "anim.mh");
		Assert(fileAnim.contents);
		
		cursor = (u8*)fileAnim.contents;
		data = (mesh_data*)cursor;
		
		cursor += sizeof(mesh_data);
		
		asset_block_header header = {};
		header.type = (asset_block_type)(*cursor++);
		memcpy(header.name, cursor, sizeof(header.name));
		cursor += sizeof(header.name);
		
		header.numKeyframes = *(u32*)cursor;
		cursor += sizeof(u32);
		
		header.numBones = *(u32*)cursor;
		cursor += sizeof(u32);
		
		animation.keyFrames = (skeletal_key_frame*)malloc(header.numKeyframes * sizeof(skeletal_key_frame));
		animation.keyFrameCount = header.numKeyframes;
		animation.framerate = 30.0f;
		animation.boneCount = header.numBones;
		
		for(u32 index = 0;
			index < header.numKeyframes;
			++index)
		{
			skeletal_key_frame *frame = &animation.keyFrames[index];
			
			memcpy(frame->rotation, cursor, sizeof(frame->rotation));
			cursor += header.numBones * sizeof(q4);
			memcpy(frame->position, cursor, sizeof(frame->position));
			cursor += header.numBones * sizeof(v3);
			memcpy(frame->scale, cursor, sizeof(frame->scale));
			cursor += header.numBones * sizeof(v3);
			
			frame->timestamp = *(r64*)cursor;
			cursor += sizeof(r64);
		}
		
		modelAsset = AssetLoadModel(render, &desc);
	}
	
	// Load white texture
	{
		whiteTex = AssetAlloc(render->context);
		
		u32 texData[4096];
		memset(texData, 0xFFFFFFFF, sizeof(texData));
		
		render_texture_load_desc desc =
		{
			0, 64, 64, 64 * 4, 4096, (u8*)texData,
		};
		render->createTexture(render->context, &desc, whiteTex);
		
		debug_read_file_result textureFile = memory->DEBUGPlatformReadEntireFile(NULL, "hermit.bmp");
		tex_bitmap bmTex = TextureParseBitmapInfo(textureFile);
		
		fontTex = AssetAlloc(render->context);
		
		desc.width  = bmTex.dib.width;
		desc.height = bmTex.dib.height;
		desc.pitch  = bmTex.dib.width * 4;
		desc.data   = bmTex.data;
		
		render->createTexture(render->context, &desc, fontTex);
		
		memory->DEBUGPlatformFreeFileMemory(NULL, textureFile.contents);
	}
	
	{
		{
			debug_read_file_result textureFile = Win32ReadEntireFile(NULL, "bakedMap.tex");
			render_asset *texAsset = AssetAlloc(&renderContext);
			
			ID3D11Texture2D *texture;
			D3D11_TEXTURE2D_DESC tDesc;
			tDesc.Width = 512;
			tDesc.Height = 512;
			tDesc.MipLevels = 1; 
			tDesc.ArraySize = 1;
			// TODO(Ecy): more types :/
			tDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tDesc.SampleDesc.Count = 1;
			tDesc.SampleDesc.Quality = 0;
			tDesc.Usage = D3D11_USAGE_DEFAULT;
			tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			tDesc.MiscFlags = 0;
			
			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = textureFile.contents;
			initData.SysMemPitch = 512 * 4;
			initData.SysMemSlicePitch = 0;
			
			if(renderContext.device->CreateTexture2D(&tDesc, &initData, &texture) != S_OK)
			{
				Assert(false);
			}
			
			D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
			resourceDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
			resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
			resourceDesc.Texture2D.MipLevels = 1;
			
			if(renderContext.device->CreateShaderResourceView(texture, &resourceDesc, &texAsset->view) != S_OK)
			{
				Assert(false);
			}
			
			D3D11_SAMPLER_DESC sampDesc = {};
			sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampDesc.MinLOD         = 0;
			sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
			sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			
			if(renderContext.device->CreateSamplerState(&sampDesc, &texAsset->sampler) != S_OK)
			{
				Assert(false);
			}
			texture->Release();
			
			texAsset->type = RENDER_ASSET_TEXTURE;
			
			Win32FreeFileMemory(NULL, textureFile.contents);
		}
		
		// TODO(Ecy): Temporary, this will end up in game code,
		
		D3D11_BUFFER_DESC desc = {};
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth      = sizeof(vertex_ui) * UI_BUFFER_VERTICES_COUNT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags      = 0;
		
		if(renderContext.device->CreateBuffer(&desc, NULL, &renderContext.uiBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		desc.Usage          = D3D11_USAGE_DEFAULT;
		desc.ByteWidth      = sizeof(ui_globals);
		desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		desc.MiscFlags      = 0;
		desc.CPUAccessFlags = 0;
		
		if(renderContext.device->CreateBuffer(&desc, NULL, &renderContext.uiConstant) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
	}
	
	
	ID3D11SamplerState          *cubemapSampler;
	ID3D11ShaderResourceView    *cubemapView;
	game_camera cubeCamera      = {};
	ID3D11Buffer                *cubeMapVertexBuffer;
	ID3D11Buffer                *cubeMapIndexBuffer;
	mesh_generation             sphereMesh;
	matrix trWVP;
	{
		InitCamera(&cubeCamera, platform.width, platform.height, 70);
		//UpdateCamera(&cubeCamera, cubeCamera.position);
		
		// TODO(Ecy): We need camera translation here
		matrix world = MatrixTranslate(cubeCamera.position.x, cubeCamera.position.y, cubeCamera.position.z);
		cubeCamera.wvp = world * MatrixScale(1000.0f, 1000.0f, 1000.0f) * cubeCamera.view * cubeCamera.projection3d;
		trWVP = Transpose(&cubeCamera.wvp);
		renderContext.deviceContext->UpdateSubresource(renderContext.bufferSkybox, 0, NULL, 
													   &trWVP, 0, 0);
		
		debug_read_file_result cubemap = Win32ReadEntireFile(NULL, "skybox/CubeMap1.dds");
		dds_file            cubemapTex = LoadDdsImage(cubemap);
		
		ID3D11Texture2D *texture;
		D3D11_TEXTURE2D_DESC tDesc;
		tDesc.Width = cubemapTex.header.dwWidth;
		tDesc.Height = cubemapTex.header.dwHeight;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 6;
		tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		tDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		u8 *cursor = (u8*)cubemapTex.data;
		u32 size = cubemapTex.header.dwWidth * cubemapTex.header.dwHeight * 4;
		D3D11_SUBRESOURCE_DATA initData[6];
		
		initData[0].pSysMem = cursor;
		initData[0].SysMemPitch = cubemapTex.header.dwWidth * 4;
		cursor += size;
		
		initData[1].pSysMem = cursor;
		initData[1].SysMemPitch = cubemapTex.header.dwWidth * 4;
		cursor += size;
		
		initData[2].pSysMem = cursor;
		initData[2].SysMemPitch = cubemapTex.header.dwWidth * 4;
		cursor += size;
		
		initData[3].pSysMem = cursor;
		initData[3].SysMemPitch = cubemapTex.header.dwWidth * 4;
		cursor += size;
		
		initData[4].pSysMem = cursor;
		initData[4].SysMemPitch = cubemapTex.header.dwWidth * 4;
		cursor += size;
		
		initData[5].pSysMem = cursor;
		initData[5].SysMemPitch = cubemapTex.header.dwWidth * 4;
		
		if(renderContext.device->CreateTexture2D(&tDesc, initData, &texture) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
		resourceDesc.Format              = DXGI_FORMAT_B8G8R8A8_UNORM;
		resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURECUBE;
		resourceDesc.TextureCube.MipLevels = 1;
		
		if(renderContext.device->CreateShaderResourceView(texture, &resourceDesc, &cubemapView) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.MaxAnisotropy  = 1;
		sampDesc.MinLOD         = 0;
		sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		
		if(renderContext.device->CreateSamplerState(&sampDesc, &cubemapSampler) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		texture->Release();
		
		Win32FreeFileMemory(NULL, cubemap.contents);
		
		v3 vertices[9999];
		u32 indices[9999 * 3];
		sphereMesh = GenerateUVSphere(vertices, indices, 10, 10);
		
		D3D11_BUFFER_DESC bDesc = {};
		bDesc.Usage          = D3D11_USAGE_DEFAULT;
		bDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		bDesc.ByteWidth      = sizeof(v3) * sphereMesh.vertexCount;
		bDesc.CPUAccessFlags = 0;
		bDesc.MiscFlags      = 0;
		
		D3D11_SUBRESOURCE_DATA dataDesc = {};
		dataDesc.pSysMem = vertices;
		
		if(renderContext.device->CreateBuffer(&bDesc, &dataDesc, &cubeMapVertexBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		bDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
		bDesc.ByteWidth      = sizeof(u32) * sphereMesh.indexCount;
		dataDesc.pSysMem     = indices;
		
		if(renderContext.device->CreateBuffer(&bDesc, &dataDesc, &cubeMapIndexBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
	}
	
	{
		// NOTE(Ecy): register all debug shapes here
		vertex_debug vertices[] =
		{
			-1.0f, -1.0f, -1.0f,
			-1.0f, +1.0f, -1.0f,
			+1.0f, +1.0f, -1.0f,
			+1.0f, -1.0f, -1.0f,
			
			-1.0f, -1.0f, +1.0f,
			-1.0f, +1.0f, +1.0f,
			+1.0f, +1.0f, +1.0f,
			+1.0f, -1.0f, +1.0f,
		};
		
		u32 indices[] = 
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			
			// back face
			4, 6, 5,
			4, 7, 6,
			
			// left face
			4, 5, 1,
			4, 1, 0,
			
			// right face
			3, 2, 6,
			3, 6, 7,
			
			// top face
			1, 5, 6,
			1, 6, 2,
			
			// bottom face
			4, 0, 3, 
			4, 3, 7,
		};
		
		D3D11_BUFFER_DESC bDesc = {};
		bDesc.Usage          = D3D11_USAGE_DEFAULT;
		bDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		bDesc.ByteWidth      = sizeof(vertices);
		bDesc.CPUAccessFlags = 0;
		bDesc.MiscFlags      = 0;
		
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices;
		if(renderContext.device->CreateBuffer(&bDesc, &initData, &renderContext.debugVertexBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		bDesc.Usage          = D3D11_USAGE_DYNAMIC;
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bDesc.ByteWidth      = sizeof(vertex_debug_instanced) * MAX_DEBUG_INSTANCE_SIZE;
		if(renderContext.device->CreateBuffer(&bDesc, NULL, &renderContext.debugInstanceBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		bDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
		bDesc.ByteWidth      = sizeof(indices);
		initData.pSysMem     = indices;
		
		if(renderContext.device->CreateBuffer(&bDesc, &initData, &renderContext.debugIndexBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
	}
	
#if 0	
	// billboard debug data
	ID3D11Buffer                *billboardBuffer;
	ID3D11SamplerState          *billboardSampler;
	ID3D11ShaderResourceView    *billboardView;
	r32 rotOffset = .0f;
	{
		// load billboard texture array
		
		debug_read_file_result texFile = Win32ReadEntireFile(NULL, "billboard/circle_01.bmp");
		tex_bitmap billboardTex         = TextureParseBitmapInfo(texFile);
		
		ID3D11Texture2D *texture;
		D3D11_TEXTURE2D_DESC tDesc;
		tDesc.Width = billboardTex.dib.width;
		tDesc.Height = billboardTex.dib.height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;
		
		u32 size = billboardTex.dib.width * billboardTex.dib.height * 4;
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = billboardTex.data;
		initData.SysMemPitch = billboardTex.dib.width * 4;
		
		if(renderContext.device->CreateTexture2D(&tDesc, &initData, &texture) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
		resourceDesc.Format              = DXGI_FORMAT_B8G8R8A8_UNORM;
		resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		resourceDesc.Texture2DArray.MostDetailedMip = 0;
		resourceDesc.Texture2DArray.MipLevels = 1;
		resourceDesc.Texture2DArray.FirstArraySlice = 0;
		resourceDesc.Texture2DArray.ArraySize = 1;
		
		if(renderContext.device->CreateShaderResourceView(texture, &resourceDesc, &billboardView) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.MaxAnisotropy  = 1;
		sampDesc.MinLOD         = 0;
		sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		
		if(renderContext.device->CreateSamplerState(&sampDesc, &billboardSampler) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		texture->Release();
		
		Win32FreeFileMemory(NULL, texFile.contents);
		
		// create vertex buffer
		
		vertex_billboard vertices[256] = {};
		
		for(u32 i = 0;
			i < 16;
			++i)
		{
			r32 y = 10 * cos(Pi * i / 32);
			for(u32 j = 0;
				j < 16;
				++j)
			{
				r32 x = (i + 1)*sin(2 * Pi * j / 16);
				r32 z = (i + 1)*cos(2 * Pi * j / 16);
				vertex_billboard *v = &vertices[i * 16 + j];
				v->pos = V3(x, y, z);
				v->size = V2(.2f, .2f);
			}
		}
		
		D3D11_BUFFER_DESC bDesc = {};
		bDesc.Usage          = D3D11_USAGE_DYNAMIC;
		bDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		bDesc.ByteWidth      = sizeof(vertices);
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bDesc.MiscFlags      = 0;
		
		initData = {};
		initData.pSysMem = vertices;
		if(renderContext.device->CreateBuffer(&bDesc, &initData, &billboardBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
	}
#endif
	
	// particle system
	render_particle fireEffect = {};
	ID3D11Buffer                *fireInit;
	ID3D11Buffer                *fireDraw;
	ID3D11Buffer                *fireStreamOut;
	ID3D11SamplerState          *fireSampler;
	ID3D11SamplerState          *randomSampler;
	ID3D11ShaderResourceView    *fireView;
	ID3D11ShaderResourceView    *randomTex;
	ID3D11BlendState            *additiveBlending;
	{
		debug_read_file_result circle01 = Win32ReadEntireFile(NULL, "particle/circle_01.bmp");
		tex_bitmap circleTex01          = TextureParseBitmapInfo(circle01);
		debug_read_file_result circle02 = Win32ReadEntireFile(NULL, "particle/circle_02.bmp");
		tex_bitmap circleTex02          = TextureParseBitmapInfo(circle02);
		debug_read_file_result circle03 = Win32ReadEntireFile(NULL, "particle/circle_03.bmp");
		tex_bitmap circleTex03          = TextureParseBitmapInfo(circle03);
		debug_read_file_result circle04 = Win32ReadEntireFile(NULL, "particle/circle_04.bmp");
		tex_bitmap circleTex04          = TextureParseBitmapInfo(circle04);
		debug_read_file_result circle05 = Win32ReadEntireFile(NULL, "particle/circle_05.bmp");
		tex_bitmap circleTex05          = TextureParseBitmapInfo(circle05);
		
		ID3D11Texture2D *texture;
		D3D11_TEXTURE2D_DESC tDesc;
		tDesc.Width = circleTex01.dib.width;
		tDesc.Height = circleTex01.dib.height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 5;
		tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;
		
		u32 size = circleTex01.dib.width * circleTex01.dib.height * 4;
		D3D11_SUBRESOURCE_DATA initData[5] = {};
		initData[0].pSysMem     = circleTex01.data;
		initData[0].SysMemPitch = circleTex01.dib.width * 4;
		initData[1].pSysMem     = circleTex02.data;
		initData[1].SysMemPitch = circleTex02.dib.width * 4;
		initData[2].pSysMem     = circleTex03.data;
		initData[2].SysMemPitch = circleTex03.dib.width * 4;
		initData[3].pSysMem     = circleTex04.data;
		initData[3].SysMemPitch = circleTex04.dib.width * 4;
		initData[4].pSysMem     = circleTex05.data;
		initData[4].SysMemPitch = circleTex05.dib.width * 4;
		
		if(renderContext.device->CreateTexture2D(&tDesc, initData, &texture) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
		resourceDesc.Format              = DXGI_FORMAT_B8G8R8A8_UNORM;
		resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		resourceDesc.Texture2DArray.MostDetailedMip = 0;
		resourceDesc.Texture2DArray.MipLevels = 1;
		resourceDesc.Texture2DArray.FirstArraySlice = 0;
		resourceDesc.Texture2DArray.ArraySize = 5;
		
		if(renderContext.device->CreateShaderResourceView(texture, &resourceDesc, &fireView) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.MaxAnisotropy  = 1;
		sampDesc.MinLOD         = 0;
		sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		
		if(renderContext.device->CreateSamplerState(&sampDesc, &fireSampler) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		texture->Release();
		
		{
			Win32FreeFileMemory(NULL, circle01.contents);
		}
		
		// Random tex
		v3u8 randomData[256];
		
		for(u32 i = 0;
			i < ArrayCount(randomData);
			++i)
		{
			u32 seed = rand();
			randomData[i].x = seed % 255;
			seed = rand();
			randomData[i].y = seed % 255;
			seed = rand();
			randomData[i].z = seed % 255;
		}
		
		ID3D11Texture1D *randomTexture;
		D3D11_TEXTURE1D_DESC desc = {};
		desc.Width = 256;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		
		initData[0].pSysMem = randomData;
		
		if(renderContext.device->CreateTexture1D(&desc, &initData[0], &randomTexture) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		
		D3D11_SHADER_RESOURCE_VIEW_DESC randomResDesc = {};
		randomResDesc.Format              = desc.Format;
		randomResDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE1D;
		randomResDesc.Texture1D.MipLevels = 1;
		randomResDesc.Texture1D.MostDetailedMip = 0;
		
		if(renderContext.device->CreateShaderResourceView(randomTexture, &randomResDesc, &randomTex) != S_OK)
		{
			// TODO(Ecy): Log on failure
			Assert(false);
		}
		randomTexture->Release();
		
		sampDesc = {};
		sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MinLOD         = 0;
		sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		
		if(renderContext.device->CreateSamplerState(&sampDesc, &randomSampler) != S_OK)
		{
			Assert(false);
		}
		
		vertex_particle vertices[] =
		{
			{ V3(0.f, 0, 0.f), V3(0, 0.01f, 0), V2(10, 10), 1.0f, 0 },
		};
		
		D3D11_BUFFER_DESC bDesc = {};
		bDesc.Usage          = D3D11_USAGE_DEFAULT;
		bDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		bDesc.ByteWidth      = sizeof(vertices);
		bDesc.CPUAccessFlags = 0;
		bDesc.MiscFlags      = 0;
		
		initData[0] = {};
		initData[0].pSysMem = vertices;
		
		if(renderContext.device->CreateBuffer(&bDesc, &initData[0], &fireInit) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		bDesc.ByteWidth      = sizeof(vertex_particle) * 1000;
		bDesc.BindFlags      |= D3D11_BIND_STREAM_OUTPUT;
		if(renderContext.device->CreateBuffer(&bDesc, NULL, &fireDraw) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		bDesc.ByteWidth      = sizeof(vertex_particle) * 1000;
		if(renderContext.device->CreateBuffer(&bDesc, NULL, &fireStreamOut) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		// blending
		D3D11_BLEND_DESC blendDesc = {};
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = {};
		
		rtbd.BlendEnable           = TRUE;
		rtbd.SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend             = D3D11_BLEND_ONE;
		rtbd.BlendOp               = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha         = D3D11_BLEND_ZERO;
		rtbd.DestBlendAlpha        = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = 0x0F;
		
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.RenderTarget[0] = rtbd;
		
		renderContext.device->CreateBlendState(&blendDesc, &additiveBlending);
		
		fireEffect.firstRun = true;
	}
#endif
}
