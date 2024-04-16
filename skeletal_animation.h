/* date = October 15th 2023 8:25 pm */

#ifndef SKELETAL_ANIMATION_H
#define SKELETAL_ANIMATION_H

#define SKELETAL_MAX_BONES 100

/*
Structure:

Header - Fixed size:
===
4 - Magic Word
1 - Num meshes
1 - Num poses
1 - Num animations
1 - Reserved
===

# Ordered meshes, poses, animations

block type: None(0), Mesh(1), Pose(2), Animation(3)

=== Mesh data header
1        - Type : mesh
char[64] - Name
4        - Num vertices
4        - Num indices
4        - Vertices data offset
4        - Indices data offset
===

=== Pose data header
1        - Type : pose
char[64] - Name
4        - Num bones
=== 

=== Animation data header
1        - Type : animation
char[64] - Name
4        - Num bones
4        - Key frame count
===

*/

enum asset_block_type
{
	ASSET_BLOCK_NONE,
	
	ASSET_BLOCK_MESH,
	ASSET_BLOCK_POSE,
	ASSET_BLOCK_ANIMATION,
	
	ASSET_BLOCK_COUNT,
};

struct asset_block_header
{
	asset_block_type type;
	char name[64];
	
	// mesh
	u32 numVertices;
	u32 numIndices;
	u32 vOffset;
	u32 iOffset;
	
	// pose
	u32 numBones;
	// anim
	u32 numKeyframes;
};

struct mesh_data
{
	char mw[4];
	u8   numMeshes;
	u8   numPoses;
	u8   numAnims;
	u8   reserved;
};

struct skeletal_bone
{
	char name[64];
	i32  id;
	i32  parent;
};

struct skeletal_key_frame
{
	q4 rotation[SKELETAL_MAX_BONES];
	
	v3 position[SKELETAL_MAX_BONES];
	v3 scale[SKELETAL_MAX_BONES];
	
	r64 timestamp;
};

// NOTE(Ecy): A model has skeletal defintion and applies animation on it
struct skeletal_def
{
	skeletal_bone bones[SKELETAL_MAX_BONES];
	u32           count;
};

struct skeletal_animation
{
	// Persistent data
	skeletal_key_frame *keyFrames;
	u32                keyFrameCount;
	u32                boneCount;
	
	// Assignable
	r64 speedScale;
	r64 framerate;
	
	// Compute per frame
	matrix        transforms[SKELETAL_MAX_BONES];
	r64           currentTime;
};

#endif //SKELETAL_ANIMATION_H
