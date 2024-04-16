/* date = March 2nd 2023 9:44 pm */

#ifndef COLLISION_H
#define COLLISION_H

#define collision_GetCenterWorldSpace(ent) (ent->position + ent->collision.center)

enum collision_bounding_type
{
	COLLISION_AABB,
	COLLISION_BOX,
	COLLISION_SPHERE,
	COLLISION_CYLINDER,

	COLLISION_TYPE_COUNT,
};

struct collision_node
{
	collision_bounding_type type;
	
	// NOTE(Ecy): center is an offset to the entity's current position
	v3 center;
	v3 stride;
	
	q4 rotation;
	
	r32 radius;
};

inline collision_node
collision_Make(collision_bounding_type type)
{
	collision_node node = {};
	node.type = type;
	
	return node;
}

inline b32
CollisionAABB(v3 pos1, v3 stride1, v3 pos2, v3 stride2)
{
    v3 aMin = pos1 - stride1;
    v3 aMax = pos1 + stride1;
    v3 bMin = pos2 - stride2;
    v3 bMax = pos2 + stride2;
    
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
    (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
    (aMin.z <= bMax.z && aMax.z >= bMin.z);
}

#endif //COLLISION_H
