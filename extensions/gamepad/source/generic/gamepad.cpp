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

uint32* gamepadGetDeviceIds()
{
	return gamepadGetDeviceIds_platform();
}

uint32 gamepadGetNumDevices()
{
	return gamepadGetNumDevices_platform();
}

void gamepadUpdate()
{
	gamepadUpdate_platform();
}