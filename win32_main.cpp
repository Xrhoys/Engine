#include <windows.h>
#include <xaudio2.h>
// NOTE(Ecy): Portable file included from previous version, this file could be deprecated
#include <hidusage.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "platform.h"
#include "memory.h"
#include "game.h"

#include "win32_main.h"

// IMPORTANT: DEBUG ONLY TEMPORARY
#include "game.cpp"
#include "renderer.cpp"
#include "asset.cpp"
//

global u64                         g_perfCount;
global LARGE_INTEGER               g_bootCounter;
global LARGE_INTEGER               g_lastCounter;

global RECT                        g_rcClip;
global RECT                        g_rcOldClip;

inline r32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
	// NOTE: should g_perfCount be refetched here?
	r64 endTime   = (r64)end.QuadPart;
    r64 startTime = (r64)start.QuadPart;
	r64 perfCount = (r64)g_perfCount;
	r32 result    = (r32)((endTime - startTime) / perfCount);
    
    return result;
}

inline LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline r32
Win32GetLastElapsed()
{
    LARGE_INTEGER currentCounter = Win32GetWallClock();
	r32 elapsed = Win32GetSecondsElapsed(g_lastCounter, currentCounter);
    
    return elapsed;
}

internal
DEBUG_PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
	debug_read_file_result File = {};
    
    void *Result = 0;
    DWORD BytesRead;
    u32 FileSize32;
    HANDLE FileHandle = CreateFileA(filename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            // NOTE: Will cause problem for 64bits
            FileSize32 = (u32)FileSize.QuadPart;
            Result = VirtualAlloc(0, FileSize.QuadPart, 
                                  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            if(Result)
            {
                if(ReadFile(FileHandle, Result, FileSize32, &BytesRead, 0))
                {
                    File.contents = Result;
                    File.contentSize = FileSize32;
                }
                else
                {
                    VirtualFree(Result, 0, MEM_RELEASE);
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }
    
    return File;
}

internal
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(Win32WriteEntireFile)
{
	b32 result = false;
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if(WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
        {
            result = (bytesWritten == memorySize);
        }
        else
        {
            // TODO: Logging
        }
		
        CloseHandle(fileHandle);
    }
    else
    {
        // TODO: Logging
    }
	
    return(result);
}

internal void
Win32ToogleCursor()
{
	// NOTE(Ecy): Doesn't work correctly :/
	// HCURSOR hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
	ShowCursor(false);
}

internal
DEBUG_PLATFORM_FREE_FILE_MEMORY(Win32FreeFileMemory)
{
	VirtualFree(memory, 0, MEM_RELEASE);
}

internal void
ProcessKeyboardMessage(input_button_state *state, b32 isDown)
{
    if(state->endedDown != isDown)
    {
        state->endedDown = isDown;
        ++state->halfTransitionCount;
		
		if(isDown)
		{
			state->startHoldTime = Win32GetSecondsElapsed(g_bootCounter, g_lastCounter);
		}
		else
		{
			state->startHoldTime = INFINITY;
		}
    }
}

inline void
ProcessInput(input_context *winInput, LPARAM *lParam)
{
	u32 dwSize;
	
	HRAWINPUT input = (HRAWINPUT)*lParam;
	
	GetRawInputData(input, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	
	if(dwSize != 0)
	{
		// TODO: Experiment, moving input on a secondary thread
		// Could have latency concerns
		char buffer[256]; // 1KB, may be enough?
		LPBYTE *lpb = (LPBYTE *)buffer;
		
		GetRawInputData(input, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
		
		RAWINPUT* raw = (RAWINPUT *)lpb;
		
		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			RAWKEYBOARD keyboardData = raw->data.keyboard;
			input_controller* playerController = &winInput->controllers[0];
			
#if 0			
			// NOTE: doesn't seem like this is necessary
			b32 wasDown = ((keyboardData.Message & (1 << 30)) != 0);
			b32 isDown  = ((keyboardData.Message & (1 << 31)) == 0);
#endif
			b32 isDown = keyboardData.Flags == RI_KEY_MAKE;
			u8  key     = keyboardData.VKey;
#if 0
			char buffer[256];
			_snprintf_s(buffer, sizeof(buffer), "%d, %d\n", rawInputIsDown, rawInputIsUp);
			OutputDebugStringA(buffer);
#endif
			if (key == 'W')
			{
				ProcessKeyboardMessage(&playerController->w, isDown);
			}
			else if (key == 'A')
			{
				ProcessKeyboardMessage(&playerController->a, isDown);
			}
			else if (key == 'S')
			{
				ProcessKeyboardMessage(&playerController->s, isDown);
			}
			else if (key == 'D')
			{
				ProcessKeyboardMessage(&playerController->d, isDown);
			}
			else if (key == VK_ESCAPE)
			{
				ProcessKeyboardMessage(&playerController->esc, isDown);
			}
			else if (key == VK_MENU)
			{
				ProcessKeyboardMessage(&playerController->alt, isDown);
			}
			else if (key == VK_CONTROL)
			{
				ProcessKeyboardMessage(&playerController->ctrl, isDown);
			}
			else if (key == VK_SPACE)
			{
				ProcessKeyboardMessage(&playerController->space, isDown);
			}
			else if (key == VK_F1)
			{
				ProcessKeyboardMessage(&playerController->f1, isDown);
			}
			else if (key == VK_F2)
			{
				ProcessKeyboardMessage(&playerController->f2, isDown);
			}
			else if (key == VK_F3)
			{
				ProcessKeyboardMessage(&playerController->f3, isDown);
			}
			else if (key == VK_F4)
			{
				ProcessKeyboardMessage(&playerController->f4, isDown);
			}
			else if (key == VK_F5)
			{
				ProcessKeyboardMessage(&playerController->f5, isDown);
			}
			else if (key == VK_F6)
			{
				ProcessKeyboardMessage(&playerController->f6, isDown);
			}
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			RAWMOUSE mouseData = raw->data.mouse;
			
			winInput->dragY += winInput->sensX * mouseData.lLastX;
			winInput->dragX += winInput->sensY * mouseData.lLastY;
			
			// TODO(Ecy): do not leave it here, this is game code, not platform code
			winInput->dragX = Clamp(winInput->dragX, - Pi / 3, Pi / 3);
			
			u16 leftDown   = mouseData.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
			u16 rightDown  = mouseData.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
			u16 middleDown = mouseData.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN;
			
			// mouse button flag mapping
			// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
			
			ProcessKeyboardMessage(&winInput->mouseButtons.left, leftDown == RI_MOUSE_LEFT_BUTTON_DOWN);
			ProcessKeyboardMessage(&winInput->mouseButtons.right,  rightDown == RI_MOUSE_RIGHT_BUTTON_DOWN);
			ProcessKeyboardMessage(&winInput->mouseButtons.middle,  middleDown == RI_MOUSE_MIDDLE_BUTTON_DOWN);
			
			// NOTE(Ecy): handle vertical wheel scroll
			if((mouseData.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL)
			{
				r32 wheelDelta = (r32)(i16)mouseData.usButtonData;
				r32 numTicks   = wheelDelta / WHEEL_DELTA;
				
				winInput->dragZ += numTicks;
			}
		}
	}
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_SIZE:
        {
			GetWindowRect(hWnd, &g_rcClip);
            return 0;
        } break;
        
        case WM_MOVE:
        {
            
        } break;
        
        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // NOTE: Disable ALT application menu
                return 0;
        } break;
        
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        } break;
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
	WNDCLASSEX wc = { 
        sizeof(WNDCLASSEX), 
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, 
        WndProc, 
        0L, 0L, 
        GetModuleHandle(NULL), 
        NULL, NULL, NULL, NULL, 
        "Engine", 
        NULL 
    };
	
	RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(wc.lpszClassName, 
                             "Engine", 
                             WS_OVERLAPPEDWINDOW, 
                             100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, 
                             NULL, NULL, wc.hInstance, NULL);
	
	ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);
	
	platform_engine engine = {};
	input_context *input = &engine.input;
	input_context inputs[2];
	input_context *newInput = &inputs[0];
	input_context *oldInput = &inputs[1];
	u64 lastCycleCount;
	r64 cyclesPerFrame;
	{
		engine.running = true;
		
		// Render
		render_Initialize(hWnd);
		
		// Memory
		{
			engine.memory.permanentStorageSize = Megabytes(256);
			engine.memory.transientStorageSize = Gigabytes(1);
			engine.memory.permanentStorage = VirtualAlloc(0, engine.memory.permanentStorageSize, 
														  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(!engine.memory.permanentStorage)
			{
				// TODO(Ecy): Log errors
				Assert(false);
			}
			
			engine.memory.transientStorage = VirtualAlloc(0, engine.memory.transientStorageSize, 
														  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(!engine.memory.transientStorage)
			{
				// TODO(Ecy): Log errors
				Assert(false);
			}
			
			engine.memory.DEBUGPlatformReadEntireFile = Win32ReadEntireFile;
			engine.memory.DEBUGPlatformWriteEntireFile = Win32WriteEntireFile;
			engine.memory.DEBUGPlatformFreeFileMemory = Win32FreeFileMemory;
			
			engine.memory.permanentArena = AllocArena(engine.memory.permanentStorage, engine.memory.permanentStorageSize);
			engine.memory.transientArena = AllocArena(engine.memory.transientStorage, engine.memory.transientStorageSize);
		}
		
		// Input
		{
			RAWINPUTDEVICE devices[2];
			
			devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[0].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
			devices[0].dwFlags     = RIDEV_NOLEGACY;
			devices[0].hwndTarget  = 0;
			
			devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[1].usUsage     = HID_USAGE_GENERIC_MOUSE;
			devices[1].dwFlags     = RIDEV_NOLEGACY;
			devices[1].hwndTarget  = 0;
			
			if (RegisterRawInputDevices(devices, 2, sizeof(devices[0])) == FALSE)
			{
				// TODO: logging
			}
			
			// Default player controller
			input->controllers[0].isConnected = true;
			input->controllers[0].isAnalog    = false;
			
			for (u32 index = 0;
				 index < GAME_STATE_BUTTON_COUNT;
				 index++)
			{
				input->controllers[0].buttons[index].startHoldTime = INFINITY;
			}
			
		}
		
		// Clock
		engine.clock.gameTime = 0.0f;
		engine.clock.dt = 0.0f;
		lastCycleCount = __rdtsc();
		
		LARGE_INTEGER counter;
		LARGE_INTEGER perfCountFrequencyResult;
		QueryPerformanceCounter(&counter);
		QueryPerformanceFrequency(&perfCountFrequencyResult);
		
		g_bootCounter = counter; 
		g_lastCounter = counter;
		g_perfCount = perfCountFrequencyResult.QuadPart;
	}
	
	// NOTE(Ecy): Window cliping
	{
		GetClipCursor(&g_rcOldClip);
		GetWindowRect(hWnd, &g_rcClip);
		// NOTE(Ecy): Confines it by default, but the show/hide cursor doesn't seem to
		// work correctly. Attempts to create a cursor instead
		ClipCursor(&g_rcClip);
		ShowCursor(false);
	}
	
	asset_Init(&engine);
	GameInit(&engine);
	
	while(engine.running)
	{
		// game_input updates  
		{
			input_controller *oldKeyboardController = &oldInput->controllers[0];
			input_controller *newKeyboardController = &newInput->controllers[0];
			
			*newKeyboardController = {};
			
			newKeyboardController->isConnected = true;
			
			for (u32 index = 0;
				 index < GAME_STATE_BUTTON_COUNT;
				 ++index)
			{
				newKeyboardController->buttons[index].endedDown = oldKeyboardController->buttons[index].endedDown;
				newKeyboardController->buttons[index].halfTransitionCount 
					= oldKeyboardController->buttons[index].halfTransitionCount;
			}
			
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			input->mouseX = pt.x;
			input->mouseY = pt.y;
			
			if(oldKeyboardController->esc.endedDown)
			{
				engine.running = false;
				continue;
			}
		}
		
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
				case WM_QUIT:
				{
					engine.running = false;
				} break;
				
				case WM_INPUT:
				{
					ProcessInput(input, &msg.lParam);
				} break;
			}
			
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		
		GameUpdate(&engine);
		
		{
			input_controller *controller = &newInput->controllers[0];
			
			input_context *temp = newInput;
			newInput = oldInput;
			oldInput = temp;
		}
		
		{
			// Update cycle counter
			// frameTime       = Win32GetLastElapsed();
			// fps             = 1.0 / frameTime;
			
			u64 endCycleCounter = __rdtsc();
			u64 cyclesElapsed   = endCycleCounter - lastCycleCount;
			cyclesPerFrame      = cyclesElapsed / (1000.0f * 1000.0f);
			
			engine.clock.gameTime = Win32GetLastElapsed();
			engine.clock.dt       = Win32GetSecondsElapsed(g_bootCounter, g_lastCounter);
			
			lastCycleCount      = endCycleCounter;
			g_lastCounter       = Win32GetWallClock();
		}
		
	}
	
	render_CleanUp();
	DestroyWindow(hWnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	
	return 0;
}
