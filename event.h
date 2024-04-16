/* date = March 25th 2023 11:30 pm */

#ifndef EVENT_H
#define EVENT_H

/* 
Event processing

////////////////////////////////
Multithreaded: how would write and read work without lock in that case?
Network compatible
////////////////////////////////
Event for a player separates input processing and game world states mutation
 Event for the game itself: for instance, boss has 50% HP remaining, the room starts shrinking until final size. The triggering of the event is done from boss IA, which interfaces with the event system from an extra layer. The event gets information from the layer to mutate the properties of the room. The actual event is being processed later.
////////////////////////////////

Event:
- Spawn of an entity by another entity
 - End of life of an entity
- Game events?
 
*/

enum event_type
{
	EVENT_SPAWN_PROJECTILE,
	EVENT_LOADING_ASSET, // asynchronous asset loading
	
	EVENT_COUNT,
};

struct event
{
	event_type type;
	
	u32 originId; // player or IA
	
	union
	{
		v3  position;
		v3  target;
		
	} skill;
	
	union
	{
		v3  linearAcc;
	r32 maxVel;
		
	} projectile;
	
	
};

struct event_queue
{
	event *queue;
	u32   count;
};

#endif //EVENT_H
