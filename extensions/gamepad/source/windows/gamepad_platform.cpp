/*
 * windows-specific implementation of the gamepad extension.
 * Add any platform-specific functionality here.
 */
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
#include "gamepad_internal.h"
#include <s3eEdk_windows.h>
#include "IwDebug.h"


static UINT gamepad_device_count = 0;
static UINT gamepad_device_handlers[16];
static JOYINFOEX gamepad_device_info[16];
static JOYINFOEX gamepad_device_info_old[16];

static HHOOK hook = 0;

bool gamepadCompare(JOYINFOEX* a, JOYINFOEX* b)
{
	for (uint32 i=0; i<sizeof(JOYINFOEX); ++i)
	{
		if (((const unsigned char*)a)[i] != ((const unsigned char*)b)[i])
			return false;
	}
	return true;
}
void gamepadUpdate_platform()
{
	for (uint32 i=0; i<gamepad_device_count;++i)
	{
		uint32 j = gamepad_device_handlers[i];
		LPJOYINFOEX pInfo = &gamepad_device_info[j];
		LPJOYINFOEX pOldInfo = &gamepad_device_info_old[j];
		pInfo->dwSize = sizeof(JOYINFOEX);
		pInfo->dwFlags = JOY_RETURNALL;
		joyGetPosEx(j, pInfo);

		if (!gamepadCompare(pOldInfo, pInfo))
		{
			*pOldInfo = *pInfo;
			IwTrace(GAMEPAD_VERBOSE, ("Gamepad %d: (x:%d, y:%d)", j, pInfo->dwXpos, pInfo->dwYpos));
		}
	}
}

LRESULT CALLBACK gamepadGetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	MSG* msg = (MSG*)lParam;

	//IwTrace(GAMEPAD_VERBOSE, ("GetMsgProc (%08X, %08X, lParam=%08X)", code, wParam, lParam));
	switch (msg->message)
	{
		case MM_JOY1MOVE :                     // changed position 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY1MOVE (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
		case MM_JOY2MOVE :                     // changed position 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY2MOVE (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
		case MM_JOY1BUTTONDOWN :               // button is down 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY1BUTTONDOWN (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
		case MM_JOY1BUTTONUP :                 // button is up 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY1BUTTONUP (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
		case MM_JOY2BUTTONDOWN :               // button is down 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY2BUTTONDOWN (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
		case MM_JOY2BUTTONUP :                 // button is up 
			{
				auto fwButtons = wParam;
				auto xPos = LOWORD(lParam);
				auto yPos = HIWORD(lParam);
				IwTrace(GAMEPAD_VERBOSE, ("MM_JOY2BUTTONUP (%08X, %d, %d)", fwButtons, xPos, yPos));
			}
			break; 
	}

	return CallNextHookEx(hook, code, wParam, lParam); 
}

s3eResult gamepadInit_platform()
{
	IwTrace(GAMEPAD_VERBOSE, ("gamepadInit"));

	gamepad_device_count = 0;
	for (uint32 i=0; i<joyGetNumDevs() && i<15; ++i)
	{
		if (JOYERR_NOERROR == joySetCapture(s3eEdkGetHwnd(), JOYSTICKID1+i, 1, FALSE))
		{
			gamepad_device_handlers[gamepad_device_count] = JOYSTICKID1+i;
			IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d captured", i+1));
			++gamepad_device_count;
		}
	}
	gamepadUpdate_platform();

	//if (GetRawInputDeviceList(0, &gamepad_total_device_count, sizeof(RAWINPUTDEVICELIST)))
	//{
	//	IwTrace(GAMEPAD_VERBOSE, ("GetRawInputDeviceList() returns error"));
	//	gamepadTerminate_platform();
	//	return S3E_RESULT_ERROR;
	//}
	////const unsigned short usage_array[] = {
	////		HID_DEVICE_SYSTEM_MOUSE, //RIM_TYPEMOUSE
	////		HID_DEVICE_SYSTEM_KEYBOARD, //RIM_TYPEKEYBOARD
	////		HID_DEVICE_SYSTEM_GAME, //RIM_TYPEHID
	////		//HID_DEVICE_SYSTEM_CONTROL
	////	};

	//IwTrace(GAMEPAD_VERBOSE, ("GetRawInputDeviceList()==%d", gamepad_total_device_count));
	//if (gamepad_total_device_count > 0)
	//{
	//	gamepad_device_list = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST)*gamepad_total_device_count);
	//	gamepad_device_info = (RID_DEVICE_INFO*)malloc(sizeof(RID_DEVICE_INFO)*gamepad_total_device_count);

	//	if (gamepad_device_list == 0)
	//	{
	//		IwTrace(GAMEPAD_VERBOSE, ("Unable to allocate memory for raw input device list."));
	//		gamepadTerminate_platform();
	//		return S3E_RESULT_ERROR;

	//	}

	//	if (GetRawInputDeviceList(gamepad_device_list, &gamepad_total_device_count, sizeof(RAWINPUTDEVICELIST))== -1)
	//	{
	//		IwTrace(GAMEPAD_VERBOSE, ("GetRawInputDeviceList() returns error"));
	//		gamepadTerminate_platform();
	//		return S3E_RESULT_ERROR;

	//	}
	//	gamepad_device_count = 0;
	//	for (int i = 0; i < gamepad_total_device_count; i++) {
	//		if (gamepad_device_list[i].dwType == RIM_TYPEHID)
	//		{
	//			++gamepad_device_count;
	//		}
	//	}
	//	if (gamepad_device_count > 0)
	//	{
	//		gamepad_device_handlers = (uint32*)malloc(sizeof(uint32)*gamepad_device_count);
	//		gamepad_device_count = 0;
	//		for (int i = 0; i < gamepad_total_device_count; i++) {
	//			if (gamepad_device_list[i].dwType == RIM_TYPEHID)
	//			{
	//				gamepad_device_handlers[gamepad_device_count] = (uint32)gamepad_device_list[i].hDevice;
	//				++gamepad_device_count;
	//			}
	//		}

	//		gamepad_devices = (RAWINPUTDEVICE*)malloc(sizeof(RAWINPUTDEVICE)*2);

	//		gamepad_devices[0].dwFlags = 0;
	//		gamepad_devices[0].hwndTarget = 0;//s3eEdkGetHwnd();
	//		gamepad_devices[0].usUsage		= 0x04;
	//		gamepad_devices[0].usUsagePage	= 0x01;

	//		gamepad_devices[1].dwFlags = 0;
	//		gamepad_devices[1].hwndTarget = 0;//s3eEdkGetHwnd();
	//		gamepad_devices[1].usUsage		= 0x05;
	//		gamepad_devices[1].usUsagePage	= 0x01;

	//		if(!RegisterRawInputDevices(gamepad_devices, 2, sizeof(RAWINPUTDEVICE)))
	//		{
	//			IwTrace(GAMEPAD_VERBOSE, ("Can't register devices"));
	//			gamepadTerminate_platform();
	//			return S3E_RESULT_ERROR;
	//		}
	//		IwTrace(GAMEPAD_VERBOSE, ("Initialization complete"));
	//	}
	//}
	//IwTrace(GAMEPAD_VERBOSE, ("s3eEdkGetHinstance = %08X", s3eEdkGetHinstance()));
	//IwTrace(GAMEPAD_VERBOSE, ("s3eEdkGetHwnd = %08X", s3eEdkGetHwnd()));
	DWORD ThreadId = GetWindowThreadProcessId(s3eEdkGetHwnd(), NULL);
	////GetCurrentThreadId()
	
	hook = SetWindowsHookEx( WH_GETMESSAGE, (HOOKPROC)gamepadGetMsgProc, GetModuleHandle(NULL), ThreadId );
	if (!hook)
	{
		IwTrace(GAMEPAD_VERBOSE, ("Hook is not set"));
		gamepadTerminate_platform();
		return S3E_RESULT_ERROR;
	}
	IwTrace(GAMEPAD_VERBOSE, ("Hook is set"));
    // Add any platform-specific initialisation code here
    return S3E_RESULT_SUCCESS;
}

void gamepadTerminate_platform()
{
	IwTrace(GAMEPAD_VERBOSE, ("gamepadTerminate"));

	for (uint32 i=0; i<gamepad_device_count && i<15; ++i)
	{
		joyReleaseCapture(gamepad_device_handlers[i]);
		IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d released", gamepad_device_handlers[i]));
	}
	gamepad_device_count = 0;
		
    // Add any platform-specific termination code here
	//if (gamepad_device_list != 0)
	//{
	//	free(gamepad_device_list);
	//	gamepad_device_list = 0;
	//}
	//if (gamepad_devices != 0)
	//{
	//	free(gamepad_devices);
	//	gamepad_devices = 0;
	//}
	//if (gamepad_device_handlers != 0)
	//{
	//	free(gamepad_device_handlers);
	//	gamepad_device_handlers = 0;
	//}
	if (hook != 0)
	{
		UnhookWindowsHookEx(hook);
		IwTrace(GAMEPAD_VERBOSE, ("Hook is unset"));
		hook = 0;
	}
}

uint32 gamepadGetNumDevices_platform()
{
	return gamepad_device_count;
}

uint32* gamepadGetDeviceIds_platform()
{
	return gamepad_device_handlers;
}