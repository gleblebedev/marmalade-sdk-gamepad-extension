/*
 * This file is part of the Marmalade SDK Code Samples.
 *
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This source code is intended only as a supplement to the Marmalade SDK.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "IwGx.h"
#include "gamepad.h"
#include <s3eTypes.h>

char g_callbackMsg [1024];

S3E_API int32 gamepadCallback(void* ptr, void* userData)
{
	gamepadCallbackInfo* a = (gamepadCallbackInfo*)ptr;
	sprintf(g_callbackMsg,"index: %d, 0x%08X 0x%08X",a->index,a->axesFlags,a->buttonsFlags);
	return 0;
}

// Standard C-style entry point. This can take args if required.
int main()
{
	sprintf(g_callbackMsg,"-");
	char buf [4096];
	uint32 i,j;
	uint32 num = gamepadGetNumDevices();

	// Initialise the IwGx drawing module
    IwGxInit();
	if (gamepadAvailable())
	{
		gamepadRegisterCallback(gamepadCallback,0);
		//gamepadInit();
	}

    // Set the background colour to (opaque) blue
    IwGxSetColClear(0, 0, 0xff, 0xff);
    // Loop forever, until the user or the OS performs some action to quit the app
    while (!s3eDeviceCheckQuitRequest())
    {
        // Clear the surface
        IwGxClear();

		if (!gamepadAvailable())
		{
	        IwGxPrintString(10, 10, "GamePad API is not available!");
		}
		else
		{
			gamepadUpdate();
			buf[0] = 0;
			for (i=0; i<num; ++i)
			{
				const char* name = gamepadGetDeviceName(i);
				if (!name) name = "";
				sprintf(buf+strlen(buf), "%d: id=0x%08X, name=%s\n", i, gamepadGetDeviceId(i), name);
				uint32 numAxes  = gamepadGetNumAxes(i);
				sprintf(buf+strlen(buf), "%d axes\n", numAxes);
				for (j=0; j<numAxes && j<6; ++j)
				{
					float v = gamepadGetAxis(i,j)/4096.0f;
					sprintf(buf+strlen(buf), "axis %d: %f\n", j, (double)v);
				}
				if (gamepadIsPointOfViewAvailable(i))
				{
					sprintf(buf+strlen(buf), "Point of view: %d\n", gamepadGetPointOfViewAngle(i));
				}
				uint32 numButtons = gamepadGetNumButtons(i);
				uint32 b = gamepadGetButtons(i);
				sprintf(buf+strlen(buf), "%d buttons\n", numButtons);
				for (j=0; j<numButtons && j<32; ++j)
				{
					sprintf(buf+strlen(buf), (b&1)?"X":"-");
					b = b >> 1;
				}
				sprintf(buf+strlen(buf), "\n");
				sprintf(buf+strlen(buf), g_callbackMsg);
			}
			
			IwGxPrintString(100, 10, buf);
		}

        // Standard EGL-style flush of drawing to the surface
        IwGxFlush();

        // Standard EGL-style flipping of double-buffers
        IwGxSwapBuffers();

        // Sleep for 0ms to allow the OS to process events etc.
        s3eDeviceYield(0);
    }
	if (gamepadAvailable())
	{
		//gamepadTerminate();
		gamepadUnregisterCallback(gamepadCallback);
	}
    // Shut down the IwGx drawing module
    IwGxTerminate();

    // Return
    return 0;
}
