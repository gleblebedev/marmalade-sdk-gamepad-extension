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

/**
 * @page ExampleIwGxHelloWorld IwGx Hello World Example
 *
 * The following example, in typical Hello World style, displays the phrase
 * "Hello, World!" on screen.
 *
 * The functions required to achieve this are:
 * Printing the text to screen:
 *  - IwGxPrintString()
 *
 * Standard IwGx API:
 *  - IwGxInit()
 *  - IwGxTerminate()
 *  - IwGxSetColClear()
 *  - IwGxFlush()
 *  - IwGxSwapBuffers()
 *
 * Device interoperability through the s3e API:
 *  - s3eDeviceCheckQuitRequest()
 *  - s3eDeviceYield()
 *
 * All examples will follow this basic pattern; a brief description of what
 * the example does will be given followed by a list of all the important
 * functions and, perhaps, classes.
 *
 * Should the example be more complex, a more detailed explanation of what the
 * example does and how it does it will be added. Note that most examples
 * use an example framework to remove boilerplate code and allow the projects
 * to be made up of a single source file for easy viewing. This framework can
 * be found in the examples/s3e/ExamplesMain directory.
 *
 * @include IwGxHelloWorld.cpp
 */

// Include the single header file for the IwGx module
#include "IwGx.h"
#include "gamepad.h"

// Standard C-style entry point. This can take args if required.
int main()
{
	char buf [4096];
	uint32 i;
	uint32 num = gamepadGetNumDevices();
	uint32* ids = gamepadGetDeviceIds();

	// Initialise the IwGx drawing module
    IwGxInit();
	if (gamepadAvailable())
	{
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
	        IwGxPrintString(120, 150, "GamePad API is not available!");
		}
		else
		{
			gamepadUpdate();
			buf[0] = 0;
			for (i=0; i<num; ++i)
			{
				sprintf(buf+strlen(buf), "%d: id=0x%08X\n", i, ids[i]);
			}
			
			IwGxPrintString(120, 150, buf);
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
	}
    // Shut down the IwGx drawing module
    IwGxTerminate();

    // Return
    return 0;
}
