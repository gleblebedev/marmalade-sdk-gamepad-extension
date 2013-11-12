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
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>

struct s3eCallbackInfo
{
	s3eCallback fn;
	void* userData;
};

typedef char device_name[256];

struct GamePadInfo
{
	UINT handler;
	JOYINFOEX current;
	JOYINFOEX previous;
	JOYCAPS caps;
	device_name name;

	GamePadInfo(){
		memset(this,0,sizeof(GamePadInfo));
	}
};

static UINT gamepad_device_count = 0;
static GamePadInfo gamepad_device_info[16];
static GamePadInfo gamepad_device_info_nullobject;

static s3eCallbackInfo gamepad_callbacks[16];
static UINT gamepad_num_callbacks = 0;
static HHOOK hook = 0;

static GamePadInfo* gamepadGetInfo ( int index )
{
	if (index < 0 || index >= gamepad_device_count) return &gamepad_device_info_nullobject;
	return gamepad_device_info+index;
}

static int gamepadGetOEMProductName ( int index )
{
    char buffer [ 256 ];

    char OEMKey [ 256 ];

	GamePadInfo* info = gamepad_device_info+index;

	sprintf(info->name,"Microsoft PC joystick driver");

    HKEY  hKey;
    DWORD dwcb;
    LONG  lr;

    /* Open .. MediaResources\CurrentJoystickSettings */
	sprintf ( buffer, "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, &info->caps.szRegKey, REGSTR_KEY_JOYCURR );

	//HKEY_LOCAL_MACHINE
	auto hkey = HKEY_CURRENT_USER;
	lr = RegOpenKeyEx ( hkey, buffer, 0, KEY_QUERY_VALUE, &hKey);

    if ( lr != ERROR_SUCCESS ) {
		IwTrace(GAMEPAD_VERBOSE, ("Can't open registry %s for JOYSTICKID%d", buffer, index));
		return 0;
	}

    /* Get OEM Key name */
    dwcb = sizeof(OEMKey);

    /* JOYSTICKID1-16 is zero-based; registry entries for VJOYD are 1-based. */
    sprintf ( buffer, "Joystick%d%s", index + 1, REGSTR_VAL_JOYOEMNAME );

    lr = RegQueryValueEx ( hKey, buffer, 0, 0, (LPBYTE) OEMKey, &dwcb);
    RegCloseKey ( hKey );

    if ( lr != ERROR_SUCCESS ) return 0;

    /* Open OEM Key from ...MediaProperties */
    sprintf ( buffer, "%s\\%s", REGSTR_PATH_JOYOEM, OEMKey );

    lr = RegOpenKeyEx ( hkey, buffer, 0, KEY_QUERY_VALUE, &hKey );

    if ( lr != ERROR_SUCCESS ) return 0;

    /* Get OEM Name */
    dwcb = 255;

	lr = RegQueryValueEx ( hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE) &info->name, &dwcb );
    RegCloseKey ( hKey );

    if ( lr != ERROR_SUCCESS ) return 0;

    return 1;
}

bool gamepadCompare(JOYINFOEX* a, JOYINFOEX* b)
{
	for (uint32 i=0; i<sizeof(JOYINFOEX); ++i)
	{
		if (((const unsigned char*)a)[i] != ((const unsigned char*)b)[i])
			return false;
	}
	return true;
}

void gamepadReleaseDevices()
{
		for (uint32 i=0; i<gamepad_device_count && i<15; ++i)
	{
		MMRESULT mmResult = joyReleaseCapture(gamepad_device_info[i].handler);
		if (JOYERR_NOERROR == mmResult)
		{
			IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d released", gamepad_device_info[i].handler-JOYSTICKID1+1));
		}
		else
		{
			switch (mmResult)
			{
			case MMSYSERR_NODRIVER:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be released: MMSYSERR_NODRIVER", i+1));
				break;
			default:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be released: 0x%08X", i+1, mmResult));
				break;
			}
		}
	}
	gamepad_device_count = 0;

}
void gamepadCaptureDevices()
{
		gamepad_device_count = 0;
	for (uint32 i=0; i<joyGetNumDevs() && i<15; ++i)
	{
		MMRESULT mmResult = joySetCapture(s3eEdkGetHwnd(), JOYSTICKID1+i, 10, FALSE);
		if (JOYERR_NOERROR == mmResult)
		{
			gamepad_device_info[gamepad_device_count].handler = JOYSTICKID1+i;
			IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d captured", i+1));
			joyGetDevCaps(JOYSTICKID1+i, &gamepad_device_info[gamepad_device_count].caps, sizeof(JOYCAPS));
			IwTrace(GAMEPAD_VERBOSE, ("JOYSTICKID%d szPname=%s", JOYSTICKID1+i, gamepad_device_info[gamepad_device_count].caps.szPname));
			IwTrace(GAMEPAD_VERBOSE, ("JOYSTICKID%d szRegKey=%s", JOYSTICKID1+i, gamepad_device_info[gamepad_device_count].caps.szRegKey));
			IwTrace(GAMEPAD_VERBOSE, ("JOYSTICKID%d szOEMVxD=%s", JOYSTICKID1+i, gamepad_device_info[gamepad_device_count].caps.szOEMVxD));
			gamepadGetOEMProductName(gamepad_device_count);
			++gamepad_device_count;
		}
		else
		{
			switch (mmResult)
			{
			case MMSYSERR_NODRIVER:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: MMSYSERR_NODRIVER", i+1));
				break;
			case MMSYSERR_INVALPARAM:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: MMSYSERR_INVALPARAM", i+1));
				break;
			case JOYERR_NOCANDO:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: JOYERR_NOCANDO", i+1));
				break;
			case JOYERR_UNPLUGGED:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: JOYERR_UNPLUGGED", i+1));
				break;
			case JOYERR_PARMS:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: JOYERR_PARMS", i+1));
				break;
			default:
				IwTrace(GAMEPAD_VERBOSE, ("joystick JOYSTICKID%d failed to be captured: 0x%08X", i+1, mmResult));
				break;
			}
		}
	}

}

void gamepadUpdate_platform()
{
	for (uint32 i=0; i<gamepad_device_count;++i)
	{
		uint32 j = gamepad_device_info[i].handler;
		LPJOYINFOEX pInfo = &gamepad_device_info[i].current;
		LPJOYINFOEX pOldInfo = &gamepad_device_info[i].previous;
		pInfo->dwSize = sizeof(JOYINFOEX);
		pInfo->dwFlags = JOY_RETURNALL;
		joyGetPosEx(j, pInfo);

		if (!gamepadCompare(pOldInfo, pInfo))
		{
			gamepadCallbackInfo info;
			info.axesFlags = 0;
			if (pOldInfo->dwXpos != pInfo->dwXpos)
				info.axesFlags |= 1<<0;
			if (pOldInfo->dwYpos != pInfo->dwYpos)
				info.axesFlags |= 1<<1;
			if (pOldInfo->dwZpos != pInfo->dwZpos)
				info.axesFlags |= 1<<2;
			if (pOldInfo->dwRpos != pInfo->dwRpos)
				info.axesFlags |= 1<<3;
			if (pOldInfo->dwUpos != pInfo->dwUpos)
				info.axesFlags |= 1<<4;
			if (pOldInfo->dwVpos != pInfo->dwVpos)
				info.axesFlags |= 1<<5;
			info.buttonsFlags = (pOldInfo->dwButtons)^(pInfo->dwButtons);
			info.index = i;
			*pOldInfo = *pInfo;
			for (uint32 x=0; x<gamepad_num_callbacks; ++x)
			{
				(*gamepad_callbacks[x].fn)(&info, gamepad_callbacks[x].userData);
			}
			//IwTrace(GAMEPAD_VERBOSE, ("Gamepad %d: (x:%d, y:%d)", j, pInfo->dwXpos, pInfo->dwYpos));
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

void gamepadCalibrate_platform()
{
  IwTrace(GAMEPAD, ("Launching calibration"));
  LPSTR lpCommandLine = "control.exe joy.cpl";

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  CreateProcess(
    NULL,
    lpCommandLine,
    NULL, NULL,
    FALSE, 0, NULL, NULL,
    &si,
    &pi);
}

void gamepadReset_platform()
{
	gamepadReleaseDevices();
	gamepadCaptureDevices();
	//if (S3E_RESULT_SUCCESS != gamepadInit_platform())
	//	return S3E_RESULT_ERROR;
	//return S3E_RESULT_SUCCESS;
}
s3eResult gamepadInit_platform()
{
	IwTrace(GAMEPAD_VERBOSE, ("gamepadInit"));

	gamepadCaptureDevices();
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

	gamepadReleaseDevices();
		
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

uint32 gamepadGetNumAxes_platform(uint32 index)
{
	return gamepadGetInfo(index)->caps.wMaxAxes;
	//return max(gamepad_device_caps[index].wMaxAxes,32);
}

uint32 gamepadGetNumButtons_platform(uint32 index)
{
	return gamepadGetInfo(index)->caps.wMaxButtons;
	//return max(gamepad_device_caps[index].wMaxButtons,32);
}

uint32 gamepadIsPointOfViewAvailable(uint32 index)
{
	return (gamepadGetInfo(index)->caps.wCaps & JOYCAPS_HASPOV);
}
int32 gamepadGetPointOfViewAngle(uint32 index)
{
	int32 pov = gamepadGetInfo(index)->current.dwPOV;
	if (pov == 65535)
		return -1;
	return (pov * 4096 / 36000);
}

uint32 gamepadGetButtons_platform(uint32 index)
{
	return (gamepadGetInfo(index)->current.dwButtons);
}

const char* gamepadGetDeviceName_platform(uint32 index)
{
	return (gamepadGetInfo(index)->name);
	//return (gamepad_device_caps[index].szPname);
}

int32 gamepadGetAxis_platform(uint32 index, uint32 axisIndex)
{
	GamePadInfo* gamepadinfo = gamepadGetInfo(index);
	JOYINFOEX* info = &gamepadinfo->current;
	JOYCAPS* caps = &gamepadinfo->caps;
	switch (axisIndex)
	{
	case 0:
		return (info->dwXpos-caps->wXmin)*8192/(caps->wXmax - caps->wXmin)-4096;
	case 1:
		return (info->dwYpos-caps->wYmin)*8192/(caps->wYmax - caps->wYmin)-4096;
	case 2:
		return (info->dwZpos-caps->wZmin)*8192/(caps->wZmax - caps->wZmin)-4096;
	case 3:
		return (info->dwRpos-caps->wRmin)*8192/(caps->wRmax - caps->wRmin)-4096;
	case 4:
		return (info->dwUpos-caps->wUmin)*8192/(caps->wUmax - caps->wUmin)-4096;
	case 5:
		return (info->dwVpos-caps->wVmin)*8192/(caps->wVmax - caps->wVmin)-4096;
	default:
		return 0;
	}
}

void gamepadRegisterCallback_platform(s3eCallback callback, void* userData)
{
	gamepad_callbacks[gamepad_num_callbacks].fn = callback;
	gamepad_callbacks[gamepad_num_callbacks].userData = userData;
	++gamepad_num_callbacks;
}

void gamepadUnregisterCallback_platform(s3eCallback callback)
{
	for (UINT i=0; i<gamepad_num_callbacks; ++i)
	{
		if (gamepad_callbacks[i].fn == callback)
		{
			for (; i<gamepad_num_callbacks; ++i)
			{
				gamepad_callbacks[i] = gamepad_callbacks[i+1];
			}
			--gamepad_num_callbacks;
			return;
		}
	}
}

uint32 gamepadGetDeviceId_platform(uint32 index)
{
	return gamepadGetInfo(index)->handler;
}
