/* date = January 4th 2023 8:02 pm */

#ifndef RENDERER_H
#define RENDERER_H

#define MAX_ASSET_BUFFER_COUNT 8

#define ASSET_SIZE_PER_GROUP 128
#define MAX_RENDER_QUEUE_SIZE 128
#define MAX_BONE_INFLUENCE 4
#define MAX_LIGHT_COUNT 8
#define MAX_RENDER_ENTITY_COUNT 512

struct vertex
{
	v3 pos;
	v3 normal;
	v2 texcoord;
};

struct vertex_skinned
{
	v3  pos;
	v3  normal;
	v2  texcoord;
	i32 bones[MAX_BONE_INFLUENCE];
	r32 weights[MAX_BONE_INFLUENCE];
};

struct vertex_font
{
	v3 pos;
	v2 texcoord;
	union
	{
		struct
		{
			u8 r;
			u8 g;
			u8 b;
			u8 a;
		};
		
		u8 v[4];
	} color;
};

struct vertex_ui
{
	v2 p0; // top left corner (screen position)
	v2 p1; // bottom right corner (screen position)
	v2 t0; // same, texture 
	v2 t1; // **
	v4 color[4];
	r32 cornerRadius;
	r32 edgeSoftness;
	r32 borderThickness;
};

struct vertex_billboard
{
	v3 pos;
	v2 size;
};

struct vertex_particle
{
	v3 initialPos;
	v3 initialVel;
	v2 size;
	r32 age;
	u32 type;
};

struct ui_globals
{
	v2 res[2];
};

struct vertex_debug
{
	v3 position;
};

struct vertex_debug_instanced
{
	matrix world;
	v4     color;
};

enum lt_type
{
	LT_SOURCE_POINT,
	LT_SOURCE_DIRECTION,
	LT_SOURCE_SPOT,
	LT_SOURCE_SPOT_GGX,
	
	LT_SOURCE_COUNT,
};

struct lt_material
{
	v4 ambient;
	v4 diffuse;
	v4 specular;
	v4 reflect;
};

struct lt_source
{
	v4   ambient;
	v4   diffuse;
	v4   specular;
	
	v3   position;
	r32  range;
	
	v3   direction;
	r32  spot;
	
	v3   att;
	lt_type type;
};

inline vertex
Vertex(r32 x, r32 y, r32 z, r32 nx, r32 ny, r32 nz, r32 u, r32 v)
{
	vertex vx;
	
	vx.pos.x = x;
	vx.pos.y = y;
	vx.pos.z = z;
	
	vx.normal.x = nx;
	vx.normal.y = ny;
	vx.normal.z = nz;
	
	vx.texcoord.x = u;
	vx.texcoord.y = v;
	
	return vx;
}

inline vertex
Vertex(v3 position, v3 normal, v2 uv)
{
	vertex v;
	
	v.pos = position;
	v.normal = normal;
	v.texcoord = uv;
	
	return v;
};

inline vertex
Vertex(r32 x, r32 y, r32 z)
{
	vertex vx = {};
	
	vx.pos.x = x;
	vx.pos.y = y;
	vx.pos.z = z;
	
	return vx;
}

inline vertex_font
VertexFont(r32 x, r32 y, r32 z, r32 a, r32 b, r32 c, r32 d, r32 u, r32 v)
{
	vertex_font vx;
	
	vx.pos.x = x;
	vx.pos.y = y;
	vx.pos.z = z;
	
	vx.color.r = a;
	vx.color.g = b;
	vx.color.b = c;
	vx.color.a = d;
	
	vx.texcoord.x = u;
	vx.texcoord.y = v;
	
	return vx;
}

inline vertex_font
VertexFont(r32 x, r32 y, r32 z, color textColor, r32 u, r32 v)
{
	vertex_font vx;
	
	vx.pos.x = x;
	vx.pos.y = y;
	vx.pos.z = z;
	
	vx.color.r = textColor.x;
	vx.color.g = textColor.y;
	vx.color.b = textColor.z;
	vx.color.a = textColor.w;
	
	vx.texcoord.x = u;
	vx.texcoord.y = v;
	
	return vx;
}

inline vertex_skinned
VertexSkinned(v3 pos, v2 tex)
{
	vertex_skinned v = {};
	v.pos = { pos.x, pos.y, pos.z };
	v.texcoord = { tex.u, tex.v };
	
	return v;
}

inline vertex_skinned
VertexSkinned(r32 x, r32 y, r32 z, r32 u, r32 v)
{
	vertex_skinned vx = {};
	vx.pos = { x, y, z };
	vx.texcoord = { u, v };
	
	return vx;
}

inline vertex_skinned
VertexSkinned(r32 x, r32 y, r32 z, r32 u, r32 v, u32 boneId)
{
	vertex_skinned vx = {};
	vx.pos = { x, y, z };
	vx.texcoord = { u, v };
	
	vx.bones[0] = boneId;
	vx.weights[0] = 1.0f;
	
	return vx;
}

inline void
AddBonesToVertex(vertex_skinned *v, u32 index, u32 boneId, r32 weight)
{
	Assert(index < MAX_BONE_INFLUENCE);
	v->bones[index] = boneId;
	v->weights[index] = weight;
}

struct mesh_generation
{
	u32 vertexCount;
	u32 indexCount;
};

inline mesh_generation
GenerateUVSphere(v3 *vertices, u32 *indices, u32 sliceCount, u32 stackCount)
{
	mesh_generation mesh = {};
	
	u32 verticeCount = 0;
	vertices[verticeCount++] =
	{
		0.0f, 1.0f, 0.0f
	};
	
	for(u32 index = 0;
		index < stackCount - 1;
		++index)
	{
		r32 phi = Pi * (r32)(index + 1) / (r32)(stackCount);
		
		for(u32 sliceIndex = 0;
			sliceIndex < sliceCount;
			++sliceIndex)
		{
			r32 theta = 2.0f * Pi * sliceIndex / (r32)(sliceCount);
			
			r32 cosPhi = cos(phi);
			r32 sinPhi = sin(phi);
			
			r32 x = sinPhi * cos(theta);
			r32 y = cosPhi;
			r32 z = sinPhi * sin(theta);
			
			vertices[verticeCount++] = { x, y, z };
		}
	}

	vertices[verticeCount++] = { 0.0f, -1.0f, 0.0f };
	
	u32 *cursor = indices;
	for(u32 index = 0;
		index < sliceCount;
		++index)
	{
		cursor[0] = 0;
		cursor[1] = index + 1;
		cursor[2] = (index + 1) % sliceCount + 1;
		
		cursor += 3;
	}
	
	u32 lastVertexIndex = verticeCount - 1;
	for(u32 index = 0;
		index < sliceCount;
		++index)
	{
		cursor[0] = lastVertexIndex;
		cursor[1] = lastVertexIndex - index - 1;
		cursor[2] = lastVertexIndex - ((index + 1) % sliceCount + 1);
		
		cursor += 3;
	}
	
	for(u32 index = 0;
		index < stackCount - 2;
		++index)
	{
		u32 i0 = index * sliceCount + 1;
		u32 i1 = (index + 1) * sliceCount + 1;
		for(u32 sliceIndex = 0;
			sliceIndex < sliceCount;
			++sliceIndex)
		{
			u32 j0 = i0 + sliceIndex;
			u32 j1 = i0 + (sliceIndex + 1) % sliceCount;
			u32 j2 = i1 + (sliceIndex + 1) % sliceCount;
			u32 j3 = i1 + sliceIndex;
			
			cursor[0] = j0;
			cursor[2] = j1;
			cursor[1] = j2;
			
			cursor[3] = j0;
			cursor[5] = j2;
			cursor[4] = j3;
			
			cursor += 6;
		}
	}
	
	mesh.vertexCount = verticeCount;
	mesh.indexCount  = cursor - indices;
	return mesh;
}

struct render_particle
{
	u32 maxParticleCount;
	b32 firstRun;
	
	// Copy of per frame constant buffer useful?
	r32 gameTime;
	r32 timeStep;
	r32 age;
	
	v3 eyePos;
	v3 emitPos;
	v3 emitDir;
};

inline void
render_ResetParticle(render_particle *sys)
{
	sys->firstRun = true;
	sys->age = .0f;
}

struct render_frame_constant
{
	matrix    viewProjection;
	lt_source lights[MAX_LIGHT_COUNT];
	v3        cameraPosition;
	r32       time;
	r32       frametime;
};

struct render_entity_constant
{
	matrix world;
	matrix worldInverse;
	lt_material material;
};

struct render_skinned_constant
{
	matrix *bones;
};

struct render_asset
{
	b32          isLoaded;
	u32          id;
	// Model
	ID3D11Buffer *vertexBuffers[MAX_ASSET_BUFFER_COUNT];
	ID3D11Buffer *indexBuffer;
	u32          vertexBufferCount;
	u32          indexCount;
	
	ID3D11ShaderResourceView *textures[MAX_ASSET_BUFFER_COUNT];
	u32                      textureCount;
	
	lt_material  material;
};

struct render_entity
{
	matrix world;
	matrix worldInverse;
	matrix boundingWorld;
	
	// reference to game object if dynamic (need to update)
	// for static objects, look up cache
	entity *gameEntity;
	
	render_asset *asset;
};

struct render_debug_instance_data
{
	u32 indexCount;
	u32 indexOffset;
	u32 vertexOffset;
	u32 instanceOffset;
};

struct render_visibility
{
	struct
	{
		collision_bounding_type type;
		
		v3 center;
		v3 stride;
		
		q4 rotation;
		
		r32 radius;
	}bounding;
	
	// distance to the camera
	render_entity *renderEntity;
};

struct render_node
{
	render_visibility *visibility;
	
	// Generated render data for the current frame
	matrix world;
	matrix worldInverse;
	render_skinned_constant *skinnedData;
};

enum render_debug_type
{
	RENDER_DEBUG_CUBE,
	
	RENDER_DEBUG_COUNT,
};

struct render_debug_command
{
	render_debug_type type;
	u32               offset, count;
};

struct render_command
{
	render_node *nodes;
	u32 count;
};

struct render_group
{
	ID3D11VertexShader       *vs;
	ID3D11InputLayout        *layout;
	ID3D11Buffer             *vsConstants[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	u32                      vsContantsCount;
	
	ID3D11GeometryShader     *gs;
	ID3D11Buffer             *gsConstants[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	u32                      gsContantsCount;
	
	ID3D11PixelShader        *ps;
	ID3D11Buffer             *psConstants[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	u32                      psContantsCount;
	
	ID3D11DepthStencilState  *depthStencilState;
	
	ID3D11BlendState         *blendState;
	r32                      blendFactor[4];
	u32                      sampleMask;
	
	D3D11_PRIMITIVE_TOPOLOGY topology;
	// NOTE(Ecy): didn't find a way to express draw call type, but not really important at this point
	// can always just throw a static array look up at it
	
	u32 strides[MAX_ASSET_BUFFER_COUNT];
	u32 offsets[MAX_ASSET_BUFFER_COUNT];
};

struct render_context
{
	u32 windowWidth, windowHeight;
	
	ID3D11Device                *device;
	ID3D11DeviceContext         *deviceContext;
	IDXGISwapChain              *swapChain;
	D3D11_VIEWPORT              mainViewport;
	
	ID3D11RenderTargetView      *mainRenderTarget;
	
	ID3D11DepthStencilView      *depthStencilView;
	ID3D11DepthStencilState     *depthStencilState;
	ID3D11DepthStencilState     *depthStencilStateDisabled;
	ID3D11DepthStencilState     *depthStencilStateNoWrites;
	
	ID3D11BlendState            *transparency;
	ID3D11BlendState            *blendingEnable;
	ID3D11BlendState            *blendingDisable;	
	
	ID3D11Buffer                *frameBuffer;
	ID3D11Buffer                *entityBuffer;
	ID3D11Buffer                *skinnedBuffer;
	
	// Asset system
	
	// Render groups
	render_group                staticGroup;
	render_group                skinnedGroup;
#ifdef DEBUG
	render_group                uiGroup;
	render_group                debugGroup;
#endif
	render_group                skyboxGroup;
	render_group                particleGroup;
	
	// Particle related
	ID3D11GeometryShader        *particleSOGS;
	ID3D11VertexShader          *particleSOVS;
	
	// Timer related
	u64                         lastTimer;
	
	// Memory
	arena_memory                arena;
	
	// Controls
	b32                         toggleDebug;
	
	// NOTE(Ecy): render entity cache
	render_entity               *renderEntities;
	render_visibility           *visibilityEntities;
	u32                         renderEntityCount;
	u32                         visibilityEntityCount;
};

//////////////////////////////// Interface
render_entity* render_RegisterRenderEntity(entity *gameEntity);
render_visibility* render_RegisterVisibility(render_entity *rend);

#endif //RENDERER_H
