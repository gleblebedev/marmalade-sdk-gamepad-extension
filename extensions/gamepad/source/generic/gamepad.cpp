/*
Generic implementation of the gamepad extension.
This file should perform any platform-indepedentent functionality
(e.g. error checking) before calling platform-dependent implementations.
*/

/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */


#include "gamepad_internal.h"
s3eResult gamepadInit()
{
    //Add any generic initialisation code here
    return gamepadInit_platform();
}

void gamepadTerminate()
{
    //Add any generic termination code here
    gamepadTerminate_platform();
}

uint32 gamepadGetDeviceId(uint32 index)
{
	return gamepadGetDeviceId_platform(index);
}

uint32 gamepadGetNumDevices()
{
	return gamepadGetNumDevices_platform();
}

const char* gamepadGetDeviceName(uint32 index)
{
	return gamepadGetDeviceName_platform(index);
}

uint32 gamepadGetNumAxes(uint32 index)
{
	return gamepadGetNumAxes_platform(index);
}

uint32 gamepadGetNumButtons(uint32 index)
{
	return gamepadGetNumButtons_platform(index);
}

uint32 gamepadGetButtons(uint32 index)
{
	return gamepadGetButtons_platform(index);
}

int32 gamepadGetAxis(uint32 index, uint32 axisIndex)
{
	return gamepadGetAxis_platform(index, axisIndex);
}

void gamepadRegisterCallback(s3eCallback callback, void* userData)
{
	gamepadRegisterCallback_platform(callback,userData);
}

void gamepadUnregisterCallback(s3eCallback callback)
{
	gamepadUnregisterCallback_platform(callback);
}

void gamepadUpdate()
{
	gamepadUpdate_platform();
}

void gamepadReset()
{
	gamepadReset_platform();
}

