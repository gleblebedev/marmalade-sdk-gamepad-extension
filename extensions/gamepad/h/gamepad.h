/*
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This document is protected by copyright, and contains information
 * proprietary to Marmalade.
 *
 * This file consists of source code released by Marmalade under
 * the terms of the accompanying End User License Agreement (EULA).
 * Please do not use this program/source code before you have read the
 * EULA and have agreed to be bound by its terms.
 */
/*
 * WARNING: this is an autogenerated file and will be overwritten by
 * the extension interface script.
 */
#ifndef S3E_EXT_GAMEPAD_H
#define S3E_EXT_GAMEPAD_H

#include <s3eTypes.h>

typedef struct gamepadCallbackInfo {
	uint32 index;
	uint32 axesFlags;
	uint32 buttonsFlags;
} gamepadCallbackInfo;
// \cond HIDDEN_DEFINES
S3E_BEGIN_C_DECL
// \endcond

/**
 * Returns S3E_TRUE if the gamepad extension is available.
 */
s3eBool gamepadAvailable();

uint32 gamepadGetNumDevices();

uint32 gamepadGetDeviceId(uint32 index);

const char* gamepadGetDeviceName(uint32 index);

uint32 gamepadGetNumAxes(uint32 index);

uint32 gamepadGetNumButtons(uint32 index);

uint32 gamepadGetButtons(uint32 index);

int32 gamepadGetAxis(uint32 index, uint32 axisIndex);

uint32 gamepadIsPointOfViewAvailable(uint32 index);

int32 gamepadGetPointOfViewAngle(uint32 index);

void gamepadRegisterCallback(s3eCallback callback, void* userData);

void gamepadUnregisterCallback(s3eCallback callback);

void gamepadUpdate();

void gamepadReset();

void gamepadCalibrate();

S3E_END_C_DECL

#endif /* !S3E_EXT_GAMEPAD_H */
