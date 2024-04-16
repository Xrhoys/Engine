/* date = December 28th 2022 9:22 pm */

#ifndef INPUT_H
#define INPUT_H

#define GAME_STATE_BUTTON_COUNT 15

struct input_button_state
{
	u32 halfTransitionCount;
	b32 endedDown;
	
	r32 startHoldTime; // Event not duration
};

struct input_controller
{
	b32 isConnected;
	b32 isAnalog;
	r32 stickAverageX;
	r32 stickAverageY;
	
	union {
		input_button_state buttons[GAME_STATE_BUTTON_COUNT];
		struct
		{
			input_button_state w;
			input_button_state s;
			input_button_state a;
			input_button_state d;
			
			input_button_state space;
			
			input_button_state alt;
			input_button_state shift;
			input_button_state ctrl;
			
			input_button_state f1;
			input_button_state f2;
			input_button_state f3;
			input_button_state f4;
			input_button_state f5;
			input_button_state f6;
			// input_button_state interact;
			// input_button_state alternative;
			
			input_button_state esc;
		};
	};
};

struct input_context
{
	union
	{
		input_button_state buttons[5];
		struct
		{
			input_button_state left;
			input_button_state right;
			input_button_state middle;
			input_button_state forward;
			input_button_state backward;
		};
	} mouseButtons;
	
	i32 mouseX, mouseY;
	r32 sensX, sensY, sensZ;
	r32 dragX, dragY, dragZ;
	
	input_controller controllers[5];
};


#endif //INPUT_H
