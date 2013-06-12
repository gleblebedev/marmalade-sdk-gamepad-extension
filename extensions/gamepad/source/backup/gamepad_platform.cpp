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

static UINT gamepad_devices;
static RAWINPUTDEVICE* gamepad_device_list = 0;
static HHOOK hook = 0;

LRESULT CALLBACK gamepadGetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(hook, code, wParam, lParam); 
}

s3eResult gamepadInit_platform()
{
	GetRawInputDeviceList(0, &gamepad_devices, sizeof(RAWINPUTDEVICELIST));

	if (gamepad_devices > 0)
	{
		gamepad_device_list = (RAWINPUTDEVICE*)malloc(sizeof(RAWINPUTDEVICE)*gamepad_devices);
		if(!RegisterRawInputDevices(gamepad_device_list, gamepad_devices, sizeof(RAWINPUTDEVICE)))
		{
			free(gamepad_device_list);
			gamepad_device_list = 0;
			return S3E_RESULT_ERROR;
		}
	}

	hook = SetWindowsHookEx( WH_GETMESSAGE, (HOOKPROC)gamepadGetMsgProc, s3eEdkGetHinstance(), GetCurrentThreadId() );

    // Add any platform-specific initialisation code here
    return S3E_RESULT_SUCCESS;
}

void gamepadTerminate_platform()
{
    // Add any platform-specific termination code here
	if (gamepad_device_list != 0)
	{
		free(gamepad_device_list);
		gamepad_device_list = 0;
	}

	if (hook != 0)
	{
		UnhookWindowsHookEx(hook);
		hook = 0;
	}
}

uint32* gamepadGetDeviceIds_platform()
{
	//HWND win = s3eEdkGetHwnd();

    return 0	;
}
