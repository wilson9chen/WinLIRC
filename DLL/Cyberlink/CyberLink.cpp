/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2010 Ian Curtis
 * Copyright (C) 2013 Jan Dubiec <jdx(at)onet(dot)pl>
 */

#include <Windows.h>
#include "../Common/LIRCDefines.h"
#include "../Common/Hardware.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/IRRemote.h"
#include "../Common/Win32Helpers.h"
#include "Globals.h"
#include "ReceiveData.h"

WL_API int init(HANDLE exitEvent)
{
	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);

	receiveData = new ReceiveData();

	if(!receiveData->init()) return 0;

	return 1;
}

WL_API void deinit()
{
	if ( receiveData ) {
		receiveData->deinit();
		delete receiveData;
		receiveData = NULL;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = NULL;
}

WL_API int hasGui()
{
	return FALSE;
}

WL_API void	loadSetupGui()
{
}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats)
{
	//
	// return false - we don't support this function
	//
	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out)
{
	if ( receiveData ) {
		if ( !receiveData->waitTillDataIsReady(0) ) {
			return 0;
		}

		receiveData->getData((lirc_t*)&irCode);

		if ( decodeCommand(remotes, out) ) {
			return 1;
		}
	}

	return 0;
}

WL_API struct hardware* getHardware()
{
	initHardwareStruct();
	return &hw;
}
