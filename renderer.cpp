ID3D11SamplerState          *cubemapSampler;
ID3D11ShaderResourceView    *cubemapView;
game_camera cubeCamera      = {};
ID3D11Buffer                *cubeMapVertexBuffer;
ID3D11Buffer                *cubeMapIndexBuffer;
mesh_generation             sphereMesh;
matrix trWVP;

global render_debug_instance_data debug_InstanceData[RENDER_DEBUG_COUNT] = 
{
	{ 36, 0, 0 }, // CUBE
};

// NOTE(Ecy): this is globlal within the render thread and should not be accessed from simulation thread
// TOOD(Ecy): The arena in the struct should be initialized from engine memory interface
global render_context g_renderContext = {};

inline render_entity*
render_RegisterRenderEntity(entity *gameEntity)
{
	render_entity *rend = &g_renderContext.renderEntities[g_renderContext.renderEntityCount++];
	
	if(gameEntity)
	{
		rend->gameEntity = gameEntity;
		rend->world = GenerateTransform(&gameEntity->position, &gameEntity->scale, &gameEntity->rotation);;
		rend->worldInverse = MatrixInverse(&rend->world);
	}
	else
	{
		if(g_renderContext.toggleDebug)
		{
			v3 center = collision_GetCenterWorldSpace(gameEntity);
			rend->boundingWorld = GenerateTransform(&center, &gameEntity->collision.stride, 
													&gameEntity->collision.rotation);
		}
	}
	
	return rend;
}

inline render_visibility*
render_RegisterVisibility(render_entity *rend)
{
	render_visibility *vis = &g_renderContext.visibilityEntities[g_renderContext.visibilityEntityCount++];
	vis->renderEntity = rend;
	
	return vis;
}

internal u32
render_GenerateDebugRenderData(render_command command, vertex_debug_instanced *output)
{
	u32 count = 0;
	for(u32 index = 0;
		index < command.count;
		++index)
	{
		render_visibility *vis = command.nodes[index].visibility;
		render_entity *renderEntity = vis->renderEntity;
		switch(vis->bounding.type)
		{
			case COLLISION_AABB:
			{
				q4 defaultRotation = Q4Identity();
				
				vertex_debug_instanced *v = &output[count++];
				v->color = V4(0.0f, .5f, .5f, .3f);
				
				if(renderEntity->gameEntity)
				{
					v3 center = collision_GetCenterWorldSpace(renderEntity->gameEntity);
					v->world = GenerateTransform(&center, &renderEntity->gameEntity->collision.stride, 
												 &renderEntity->gameEntity->collision.rotation);
				}
				else
				{
					v->world = renderEntity->boundingWorld;
				}
			}break;
			
			default:
			{
				continue;
			}break;
		}
	}

}

// TODO(Ecy): Get d3dCompiler out of runtiem
internal void
LoadVertexShader(render_context *state, LPCWSTR fileName,
				 D3D11_INPUT_ELEMENT_DESC *layout,
				 u32 layoutSize, ID3D11VertexShader **shader, ID3D11InputLayout **layoutRef)
{
	ID3DBlob *blob;
	
	if(D3DReadFileToBlob(fileName, &blob) != S_OK)
	{
		// TODO: logging
	}
	
	void* data = (void *)blob->GetBufferPointer();
	u32 size = blob->GetBufferSize();
	
	if(state->device->CreateVertexShader(data, size, 0, shader) != S_OK)
	{
		// TODO: logging
	}
	
	if(state->device->CreateInputLayout(layout, layoutSize, data, size, layoutRef));
	{
		// TODO: logging
	}
	
	blob->Release();
}

// TODO(Ecy): To refactor later
internal void
LoadPixelShader(render_context *state, LPCWSTR fileName, ID3D11PixelShader **shader)
{
	ID3DBlob *blob;
	
	if (D3DReadFileToBlob(fileName, &blob) != S_OK)
	{
		// TODO: logging
	}
	
	void* data = (void *)blob->GetBufferPointer();
	u32 size = blob->GetBufferSize();
	
	if (state->device->CreatePixelShader(data, size, 0, shader) != S_OK)
	{
		// TODO: logging
	}
}

internal void
LoadGeometryShader(render_context *state, LPCWSTR fileName, ID3D11GeometryShader **shader)
{
	ID3DBlob *blob;
	
	if (D3DReadFileToBlob(fileName, &blob) != S_OK)
	{
		// TODO: logging
	}
	
	void* data = (void *)blob->GetBufferPointer();
	u32 size = blob->GetBufferSize();
	
	if (state->device->CreateGeometryShader(data, size, 0, shader) != S_OK)
	{
		// TODO: logging
	}
}

internal void
render_LoadShaderResources(render_context *context)
{
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(vertex, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		render_group *group = &context->staticGroup;
		LoadVertexShader(context, L"shader_vs.fxc", layout, ArrayCount(layout), &group->vs, &group->layout);
		LoadPixelShader(context, L"shader_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilState;
		group->blendState        = context->blendingEnable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		group->vsConstants[group->vsContantsCount++] = context->frameBuffer;
		group->vsConstants[group->vsContantsCount++] = context->entityBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex_skinned, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex_skinned, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(vertex_skinned, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(vertex_skinned, bones), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex_skinned, weights), 
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		render_group *group = &context->skinnedGroup;
		LoadVertexShader(context, L"skeleton_vs.fxc", layout, ArrayCount(layout), 
						 &group->vs, &group->layout);
		LoadPixelShader(context, L"skeleton_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilState;
		group->blendState        = context->blendingEnable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		group->vsConstants[group->vsContantsCount++] = context->frameBuffer;
		group->vsConstants[group->vsContantsCount++] = context->entityBuffer;
		group->vsConstants[group->vsContantsCount++] = context->skinnedBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "p0_dst", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "p1_dst", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "p0_src", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "p1_src", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "color", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "color", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "color", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "corner_radius", 0, DXGI_FORMAT_R32_FLOAT, 0, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "edge_softness", 0, DXGI_FORMAT_R32_FLOAT, 0, 100, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "border_thickness", 0, DXGI_FORMAT_R32_FLOAT, 0, 104, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		
		render_group *group = &context->uiGroup;
		LoadVertexShader(context, L"font_vs.fxc", layout, ArrayCount(layout), 
						 &group->vs, &group->layout);
		LoadPixelShader(context, L"font_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilStateDisabled;
		group->blendState        = context->blendingEnable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		group->vsConstants[group->vsContantsCount++] = context->frameBuffer;
		group->psConstants[group->psContantsCount++] = context->frameBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		render_group *group = &context->skyboxGroup;
		LoadVertexShader(context, L"skybox_vs.fxc", layout, ArrayCount(layout), 
						 &group->vs, &group->layout);
		LoadPixelShader(context, L"skybox_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilStateDisabled;
		group->blendState        = context->blendingDisable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		group->vsConstants[group->vsContantsCount++] = context->frameBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
		
		render_group *group = &context->debugGroup;
		LoadVertexShader(context, L"debug_vs.fxc", layout, ArrayCount(layout), 
						 &group->vs, &group->layout);
		LoadPixelShader(context, L"debug_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilState;
		group->blendState        = context->blendingEnable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		group->vsConstants[group->vsContantsCount++] = context->frameBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
#if 0	
	{
		D3D11_INPUT_ELEMENT_DESC billboardLayout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex_billboard, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(vertex_billboard, size), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		LoadVertexShader(context, L"billboard_vs.fxc", billboardLayout, ArrayCount(billboardLayout), &context->billboardVS);
		LoadPixelShader(context, L"billboard_ps.fxc", &context->billboardPS);
		LoadGeometryShader(context, L"billboard_gs.fxc", &context->billboardGS);
		
	}
#endif
	
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex_particle, initialPos), 
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(vertex_particle, initialVel), 
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(vertex_particle, size), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(vertex_particle, age), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(vertex_particle, type), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		render_group *group = &context->particleGroup;
		LoadVertexShader(context, L"particle_vs.fxc", layout, ArrayCount(layout), 
						 &group->vs, &group->layout);
		LoadGeometryShader(context, L"particle_gs.fxc", &group->gs);
		LoadPixelShader(context, L"particle_ps.fxc", &group->ps);
		
		group->depthStencilState = context->depthStencilStateDisabled;
		group->blendState        = context->blendingEnable;
		group->sampleMask        = 0xffffffff;
		group->topology          = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		group->gsConstants[group->gsContantsCount++] = context->frameBuffer;
		
		r32 blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		memcpy(group->blendFactor, blendFactor, sizeof(r32) * 4);
		
	}
	
	{
		ID3DBlob *blob;
		if(D3DReadFileToBlob(L"fire_vs_so.fxc", &blob) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		void* data = (void *)blob->GetBufferPointer();
		u32 size = blob->GetBufferSize();
		
		if(context->device->CreateVertexShader(data, size, 0, &context->particleSOVS) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		blob->Release();
		blob = NULL;
		
		if(D3DReadFileToBlob(L"fire_gs_so.fxc", &blob) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		data = (void *)blob->GetBufferPointer();
		size = blob->GetBufferSize();
		
		D3D11_SO_DECLARATION_ENTRY dec[] = 
		{
			{ 0, "POSITION", 0, 0, 3, 0, },
			{ 0, "VELOCITY", 0, 0, 3, 0, },
			{ 0, "SIZE",     0, 0, 2, 0, },
			{ 0, "AGE",      0, 0, 1, 0, },
			{ 0, "TYPE",     0, 0, 1, 0, },
		};
		
		u32 stride = sizeof(vertex_particle);
		
		if(context->device->CreateGeometryShaderWithStreamOutput(data, size, dec, ArrayCount(dec), 
																 &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM,
																 NULL, &context->particleSOGS) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
	}
}

internal void
render_Initialize(HWND hwnd)
{
	// TODO(Ecy): to remove later
	render_context *context = &g_renderContext;
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = REFRESH_RATE; // TODO: query monitor refresh rate
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc = { 1, 0 };
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
	UINT createDeviceFlags = 0;
	
#if DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = 
	{ 
		D3D_FEATURE_LEVEL_11_0, 
		D3D_FEATURE_LEVEL_10_0, 
	};
	
	if(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 
									 createDeviceFlags, featureLevelArray, 2,
									 D3D11_SDK_VERSION, &sd, &context->swapChain, &context->device,
									 &featureLevel, &context->deviceContext) != S_OK)
	{
		// TODO(Ecy): log
		Assert(false);
	}
	
	ID3D11Device *device = context->device;
	ID3D11DeviceContext *deviceContext = context->deviceContext;
	
	ID3D11Texture2D* pBackBuffer;
	context->swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	device->CreateRenderTargetView(pBackBuffer, NULL, &context->mainRenderTarget);
	
	D3D11_TEXTURE2D_DESC desc;
	pBackBuffer->GetDesc(&desc);
	
	context->windowWidth  = desc.Width;
	context->windowHeight = desc.Height;
	
	// Depth stencil 
	{
		D3D11_TEXTURE2D_DESC dd;
		dd.Width = desc.Width;
		dd.Height = desc.Height;
		dd.MipLevels = 1;
		dd.ArraySize = 1;
		dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dd.SampleDesc = { 1, 0 };
		dd.Usage = D3D11_USAGE_DEFAULT;
		dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dd.CPUAccessFlags = 0;
		dd.MiscFlags = 0;
		
		ID3D11Texture2D* depthStencilBuffer = NULL;
		device->CreateTexture2D(&dd, NULL, &depthStencilBuffer);
		
		D3D11_DEPTH_STENCIL_VIEW_DESC dvd = {};
		dvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dvd.Texture2D.MipSlice = 0;
		
		device->CreateDepthStencilView(depthStencilBuffer, &dvd, &context->depthStencilView);
		
		D3D11_DEPTH_STENCIL_DESC dsd = {};
		dsd.DepthEnable = true;
		dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = D3D11_COMPARISON_LESS;
		
		dsd.StencilEnable = true;
		dsd.StencilReadMask = 0xFF;
		dsd.StencilWriteMask = 0xFF;
		
		// NOTE: Stencil operations if pixel is front-facing.
		dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		
		// NOTE: Stencil operations if pixel is back-facing.
		dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		
		device->CreateDepthStencilState(&dsd, &context->depthStencilState);
		
		dsd.DepthEnable = false;
		dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = D3D11_COMPARISON_LESS;
		dsd.StencilEnable = true;
		dsd.StencilReadMask = 0xFF;
		dsd.StencilWriteMask = 0xFF;
		dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		
		device->CreateDepthStencilState(&dsd, &context->depthStencilStateDisabled);
		
		dsd.StencilEnable = false;
		device->CreateDepthStencilState(&dsd, &context->depthStencilStateNoWrites);
		
		depthStencilBuffer->Release();
	}
	pBackBuffer->Release();
	
	// Viewport
	{
		D3D11_VIEWPORT *viewport = &context->mainViewport;
		viewport->TopLeftX = 0;
		viewport->TopLeftY = 0;
		viewport->Width = desc.Width;
		viewport->Height = desc.Height;
		viewport->MinDepth = 0.0f;
		viewport->MaxDepth = 1.0f;
		
		deviceContext->RSSetViewports(1, viewport);
	}
	
	// Constant buffers
	{
		D3D11_BUFFER_DESC desc = {};
		
		desc.Usage          = D3D11_USAGE_DEFAULT;
		desc.ByteWidth      = sizeof(render_frame_constant);
		desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		desc.MiscFlags      = 0;
		desc.CPUAccessFlags = 0;
		
		if(device->CreateBuffer(&desc, NULL, &context->frameBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		desc.ByteWidth      = sizeof(render_entity_constant);
		
		if(device->CreateBuffer(&desc, NULL, &context->entityBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
		
		desc.ByteWidth      = sizeof(matrix) * SKELETAL_MAX_BONES;
		
		if(device->CreateBuffer(&desc, NULL, &context->skinnedBuffer) != S_OK)
		{
			// TODO: logging
			Assert(false);
		}
	}
	
	// Blendings states
	{
		D3D11_BLEND_DESC blendDesc = {};
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = {};
		
		rtbd.BlendEnable           = TRUE;
		rtbd.SrcBlend              = D3D11_BLEND_SRC_COLOR;
		rtbd.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp               = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha         = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha        = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.RenderTarget[0] = rtbd;
		
		device->CreateBlendState(&blendDesc, &context->transparency);
		
		blendDesc = {};
		rtbd = {};
		
		rtbd.BlendEnable           = TRUE;
		rtbd.SrcBlend              = D3D11_BLEND_ONE;
		rtbd.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp               = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha         = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha        = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = 0x0f;
		
		blendDesc.RenderTarget[0] = rtbd;
		
		device->CreateBlendState(&blendDesc, &context->blendingEnable);
		
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		
		device->CreateBlendState(&blendDesc, &context->blendingDisable);
	}
	
#if 0	
	// Cube mapping resources
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = 256;
		texDesc.Height = 256;
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 6;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		ID3D11Texture2D* cubeTex = 0;
		if(device->CreateTexture2D(&texDesc, 0, &cubeTex))
		{
			Assert(false);
		}
		
		D3D11_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = texDesc.Format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.MipSlice = 0;
		
		for(u32 index = 0;
			index < 6;
			++index)
		{
			desc.Texture2DArray.FirstArraySlice = index;
			
			if(device->CreateRenderTargetView(cubeTex, &desc, 
											  &context->cubeMappingRenderTargetViews[index]) != S_OK)
			{
				Assert(false);
			}
		}
		cubeTex->Release();
		
		D3D11_TEXTURE2D_DESC dd = {};
		dd.Width = texDesc.Width;
		dd.Height = texDesc.Height;
		dd.MipLevels = 1;
		dd.ArraySize = 1;
		dd.Format = DXGI_FORMAT_D32_FLOAT;
		dd.SampleDesc = { 1, 0 };
		dd.Usage = D3D11_USAGE_DEFAULT;
		dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dd.CPUAccessFlags = 0;
		dd.MiscFlags = 0;
		
		ID3D11Texture2D *depthStencilBuffer = NULL;
		device->CreateTexture2D(&dd, NULL, &depthStencilBuffer);
		
		D3D11_DEPTH_STENCIL_VIEW_DESC ddesc = {};
		ddesc.Format = DXGI_FORMAT_D32_FLOAT;
		ddesc.Flags = 0;
		ddesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		ddesc.Texture2DArray.MipSlice = 0;
		
		if(device->CreateDepthStencilView(depthStencilBuffer, &ddesc, 
										  &context->cubeMappingDepthView) != S_OK)
		{
			Assert(false);
		}
		depthStencilBuffer->Release();
		
		D3D11_VIEWPORT *cubeViewport = &context->cubeMappingViewport;
		cubeViewport->TopLeftX = 0.0f;
		cubeViewport->TopLeftY = 0.0f;
		cubeViewport->Width = (r32)256;
		cubeViewport->Height = (r32)256;
		cubeViewport->MinDepth = 0.0f;
		cubeViewport->MaxDepth = 1.0f;
	}
#endif
	
	render_LoadShaderResources(context);
	
	// Allocate visibility and render commands array
	{
		context->renderEntities = (render_entity*)AllocArena(&context->arena, 
															 sizeof(render_entity) * MAX_RENDER_ENTITY_COUNT);
		context->visibilityEntities = (render_visibility*)AllocArena(&context->arena,
																	 sizeof(render_entity) * MAX_RENDER_ENTITY_COUNT);
	}
	
	
}

internal void
render_SetRenderState(render_group *group)
{
	ID3D11DeviceContext *dc = g_renderContext.deviceContext;
	
	// IA
	dc->IASetInputLayout(group->layout);
	
	// VS
	dc->VSSetShader(group->vs, 0, 0);
	if(group->vsConstants > 0)
	{
		dc->VSSetConstantBuffers(0, group->vsContantsCount, group->vsConstants);
	}
	
	// GS
	dc->GSSetShader(group->gs, 0, 0);
	if(group->gsConstants > 0)
	{
		dc->GSSetConstantBuffers(0, group->gsContantsCount, group->gsConstants);
	}
	
	// PS
	dc->PSSetShader(group->ps, 0, 0);
	if(group->psConstants > 0)
	{
		dc->PSSetConstantBuffers(0, group->psContantsCount, group->psConstants);
	}
	
	// Depth stencil
	dc->OMSetDepthStencilState(group->depthStencilState, NULL);
	
	// Blending
	dc->OMSetBlendState(group->blendState, group->blendFactor, group->sampleMask);
	
	// Topology
	dc->IASetPrimitiveTopology(group->topology);
}

internal void
render_CleanUp()
{
	// Device, deviceContext, render states, shaders, constant buffers
	// Go through asset stores
	return;
}

internal void
render_Draw(platform_engine *engine)
{
	game_state  *gameState    = &engine->game;
	game_camera *playerCamera = &gameState->playerCamera;
	
	render_context *context = &g_renderContext;
	
	u8 *memoryPool = AllocArena(&context->arena, Megabytes(2));
	arena_memory localArena = AllocArena(memoryPool, Megabytes(2));
	
	// TODO(Ecy): to move to local arena later
	render_node staticRenderNodes_[512];
	render_node skinnedRenderNodes_[64];
	render_node particleRenderNodes_[64];
	render_node uiRenderNodes_[16];
	
	render_command staticCommands   = { staticRenderNodes_, 0 };
	render_command skinnedCommands  = { skinnedRenderNodes_, 0 };
	render_command particleCommands = { particleRenderNodes_, 0 };
	render_command uiCommands       = { uiRenderNodes_, 0 };
	// NOTE(Ecy): this depends on the amount of type there is
	render_debug_command debugCommands[RENDER_DEBUG_COUNT];
		
	// 1 - Visibility test
	{
		// TODO(Ecy): frustum culling
		for(u32 index = 0;
			index < context->visibilityEntityCount;
			++index)
		{
			if(context->visibilityEntities[index].renderEntity->gameEntity->isSkinned)
			{
				render_node *node = &staticCommands.nodes[staticCommands.count++];
				node->visibility = &context->visibilityEntities[index];
			}
			else
			{
				render_node *node = &skinnedCommands.nodes[skinnedCommands.count++];
				node->visibility = &context->visibilityEntities[index];
			}
		}
		
		// Sort by render type 
	}
	
	// 2 - Extract render data from cache or generated every frame
	{
		
		for(u32 index = 0; 
			index < staticCommands.count;
			++index)
		{
			render_node *node = &staticCommands.nodes[index];
			render_entity *renderEntity = node->visibility->renderEntity;
			
			if(renderEntity->gameEntity)
			{
				entity *gameEntity = renderEntity->gameEntity;
				node->world = GenerateTransform(&gameEntity->position, &gameEntity->scale, &gameEntity->rotation);
				node->worldInverse = MatrixInverse(&node->world);
				
			}
			else
			{
				// Copy world data from cache
				node->world = renderEntity->world;
				node->worldInverse = renderEntity->worldInverse;
				
			}
		}
		
		for(u32 index = 0; 
			index < skinnedCommands.count;
			++index)
		{
			render_node *node = &skinnedCommands.nodes[index];
			render_entity *renderEntity = node->visibility->renderEntity;
			
			// NOTE(Ecy): skinned render commands are considered dynamic
			Assert(renderEntity->gameEntity);
			entity *gameEntity = renderEntity->gameEntity;
			node->world = GenerateTransform(&gameEntity->position, &gameEntity->scale, &gameEntity->rotation);
			node->worldInverse = MatrixInverse(&node->world);
			
			// TOOD(Ecy): skinned should not be part of the game code logic
			// later move this data to animation update layer
#if 0
			{
				Assert(ent->anim);
				skeletal_animation *anim = ent->anim;
				skeletal_key_frame *keyFrames = anim->keyFrames;
				
				// Update local transforms of each bone
				skeletal_key_frame *lastPoseRef, *nextPoseRef;
				// Find timestamp interval
				for(u32 index = 0;
					index < anim->keyFrameCount;
					++index)
				{
					if(anim->currentTime > (anim->keyFrameCount - 1) * (1.0f / anim->framerate)) 
					{
						anim->currentTime = 0.001f;
					}
					
					if(anim->currentTime >= keyFrames[index].timestamp && 
					   anim->currentTime < keyFrames[index + 1].timestamp)
					{
						lastPoseRef = &keyFrames[index];
						nextPoseRef = &keyFrames[index + 1];
						break;
					}
				}
				
				if(lastPoseRef && nextPoseRef)
				{
					skeletal_animation *animation = &gameState->animation;
					// Compute interpolation
					for (u32 index = 0;
						 index < ent->anim->boneCount;
						 ++index)
					{
						// Lerp position
						r64 t = (r64)(animation->currentTime - lastPoseRef->timestamp) / (r64)nextPoseRef->timestamp;
						v3 interpolatedPos = Lerp(lastPoseRef->position[index],
												  nextPoseRef->position[index],
												  t);
						v3 interpolatedScale = Lerp(lastPoseRef->scale[index],
													nextPoseRef->scale[index],
													t);
						// Slerp orientation
						q4 slerpedOrientation = SLerp(lastPoseRef->rotation[index], nextPoseRef->rotation[index], t);
						// Compute world transforms
						command.perObject.bones[index] = GenerateTransform(&interpolatedPos,
																		   &interpolatedScale,
																		   &slerpedOrientation);
					}
					
					anim->currentTime += clock->dt;
				}
			}
#endif
			
		}
		
	}
	
	// 2bis - Generate debug data
	if(context->toggleDebug)
	{
		render_debug_command debugCommands = { RENDER_DEBUG_CUBE, 0, 0 };
		
		vertex_debug_instanced staticBoundingData[512] = {};
		u32 count = render_GenerateDebugRenderData(staticCommands, staticBoundingData);
		
		vertex_debug_instanced skinnedBoundingData[512] = {};
		count = render_GenerateDebugRenderData(skinnedCommands, skinnedBoundingData);
	}
	
	// 3 - Unlock simulation
	{
		
	}
	
	// 4 - Generate per frame constant buffer data
	render_frame_constant constant = {};
	{
		matrix vp = playerCamera->view * playerCamera->projection3d;
		constant.viewProjection = Transpose(&vp);
		constant.lights[0] =
		{
			{ 0.3f, 0.3f, 0.3f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			
			{ -5.0f, 20.0f, -10.0f },
			2000.0f,
			
			{ 0, -1, 0 },
			10.0f,
			
			{ 0.001f, 0.001f, 0.001f },
			LT_SOURCE_SPOT_GGX,
		};
		constant.cameraPosition = playerCamera->position;
		constant.time = engine->clock.gameTime;
		constant.frametime = engine->clock.dt;
	}
	
	// 5 - Generate draw calls per render_group
	{
		ID3D11DeviceContext *dc = context->deviceContext;
		
		dc->OMSetRenderTargets(1, &context->mainRenderTarget, context->depthStencilView);
		dc->RSSetViewports(1, &context->mainViewport);
		
		r32 bgColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		dc->ClearRenderTargetView(context->mainRenderTarget, bgColor);
		dc->ClearDepthStencilView(context->depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		render_SetRenderState(&context->skyboxGroup);
		{
			dc->PSSetShaderResources(0, 1, &cubemapView);
			dc->PSSetSamplers(0, 1, &cubemapSampler);
			
			u32 stride = sizeof(v3);
			u32 offset = 0;
			
			dc->IASetVertexBuffers(0, 1, &cubeMapVertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(cubeMapIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			
			dc->DrawIndexed(sphereMesh.indexCount, 0, 0);
		}
		
		render_SetRenderState(&context->staticGroup);
		{
			for(u32 index = 0;
				index < staticCommands.count;
				++index)
			{
				render_node *node = &staticCommands.nodes[index];
				render_asset *asset = node->visibility->renderEntity->asset;
				dc->PSSetShaderResources(0, asset->textureCount, asset->textures);
				//dc->PSSetSamplers(0, 1, &asset->sampler);
				
				render_entity_constant constant = {};
				constant.world = node->world;
				constant.worldInverse = node->worldInverse;
				constant.material = asset->material;
				dc->UpdateSubresource(context->entityBuffer, 0, NULL, &constant, 0, 0);
				
				u32 stride = sizeof(vertex);
				u32 offset = 0;
				
				dc->IASetVertexBuffers(0, asset->vertexBufferCount, asset->vertexBuffers,
									   &stride, &offset);
				dc->IASetIndexBuffer(asset->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				
				dc->DrawIndexed(asset->indexCount, 0, 0);
			}
			
		}
		
		render_SetRenderState(&context->skinnedGroup);
		{
			for(u32 index = 0;
				index < skinnedCommands.count;
				++index)
			{
				render_node *node = &skinnedCommands.nodes[index];
				render_asset *asset = node->visibility->renderEntity->asset;
				dc->PSSetShaderResources(0, asset->textureCount, asset->textures);
				//dc->PSSetSamplers(0, 1, &textureAsset->sampler);
				
				render_entity_constant constant = {};
				constant.world = node->world;
				constant.worldInverse = node->worldInverse;
				constant.material = asset->material;
				dc->UpdateSubresource(context->entityBuffer, 0, NULL, &constant, 0, 0);
				
				dc->UpdateSubresource(context->skinnedBuffer, 0, NULL, node->skinnedData, 0, 0);
				
				u32 stride = sizeof(vertex_skinned);
				u32 offset = 0;
				
				dc->IASetVertexBuffers(0, asset->vertexBufferCount, asset->vertexBuffers,
									   &stride, &offset);
				dc->IASetIndexBuffer(asset->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
				
				dc->DrawIndexed(asset->indexCount, 0, 0);
			}
		}

#if 0		
		render_SetRenderState(&context->particleGroup);
		{
			dc->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
			dc->VSSetShader(context->particleSOVS, 0, 0);
			dc->GSSetShader(context->particleSOGS, 0, 0);
			dc->PSSetShader(NULL, NULL, 0);
			
			dc->GSSetConstantBuffers(0, 1, vsBuffers);
			
			u32 stride = sizeof(vertex_particle);
			u32 offset = 0;
			
			dc->SOSetTargets(1, &fireStreamOut, &offset);
			
			dc->GSSetSamplers(0, 1, &randomSampler);
			if(fireEffect.firstRun)
			{
				fireEffect.firstRun = false;
				
				dc->GSSetShaderResources(0, 1, &randomTex);
				
				dc->IASetVertexBuffers(0, 1, &fireInit, &stride, &offset);
				dc->Draw(1, 0);
			}
			else
			{
				dc->GSSetShaderResources(0, 1, &randomTex);
				
				dc->IASetVertexBuffers(0, 1, &fireDraw, &stride, &offset);
				dc->DrawAuto();
			}
			
			dc->VSSetShader(context->particleVS.shader, 0, 0);
			dc->GSSetShader(context->particleGS.shader, 0, 0);
			dc->PSSetShader(context->particlePS.shader, 0, 0);
			//dc->OMSetDepthStencilState(context->depthStencilState, NULL);
			dc->PSSetShaderResources(0, 1, &fireView);
			dc->PSSetSamplers(0, 1, &fireSampler);
			
			ID3D11Buffer *bufferArray[1] = {};
			dc->SOSetTargets(1, bufferArray, &offset);
			
			ID3D11Buffer *temp = fireDraw;
			fireDraw = fireStreamOut;
			fireStreamOut = temp;
			
			r32 localBlend[] = { 0, 0, 0, 0 };
			dc->OMSetBlendState(additiveBlending, localBlend, 0xFFFFFFFF);
			
			dc->IASetVertexBuffers(0, 1, &fireDraw, &stride, &offset);
			dc->DrawAuto();
		}
#endif

		// Render debug primitives
		render_SetRenderState(&context->debugGroup);
		{
			u32 strides[] = { sizeof(vertex_debug), sizeof(vertex_debug_instanced) };
			u32 offsets[] = { 0, 0 };
			
			//dc->IASetVertexBuffers(0, ArrayCount(buffers), buffers, strides, offsets);
			//dc->IASetIndexBuffer(context->debugIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			
			for(u32 index = 0;
				index < ArrayCount(debugCommands);
				++index)
			{
				render_debug_command *command = &debugCommands[index];
				
				render_debug_instance_data instanceData = debug_InstanceData[command->type];
				
				dc->DrawIndexedInstanced(instanceData.indexCount, command->count, 
										 instanceData.indexOffset, instanceData.vertexOffset,
										 instanceData.instanceOffset);
			}
		}

#if 0		
		// Render UI
		render_SetRenderState(&context->uiGroup);
		{
			u32 stride = sizeof(vertex_ui);
			u32 offset = 0;
			
			dc->IASetVertexBuffers(0, 1, &context->uiBuffer, &stride, &offset);
			
			for(u32 index = 0;
				index < context->uiGroup.count;
				++index)
			{
				render_draw_command_ui *command = &context->uiGroup.commands[index];
				
				render_asset *textureAsset;
				D3DGetAssetById(&renderContext, command->textureAssetId, &textureAsset);
				
				dc->VSSetShaderResources(0, 1, &textureAsset->view);
				dc->PSSetShaderResources(0, 1, &textureAsset->view);
				dc->PSSetSamplers(0, 1, &textureAsset->sampler);
				
				dc->DrawInstanced(4, command->drawCount, 0, command->drawStartIndex);
			}
		}
#endif

		context->swapChain->Present(0, 0);
		
		// NOTE(Ecy): time gpu time
	}
}
