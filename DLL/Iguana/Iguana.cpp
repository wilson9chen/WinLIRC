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
 */

#include <Windows.h>
#include "resource.h"
#include "Settings.h"
#include "Globals.h"
#include "../Common/LIRCDefines.h"
#include "../Common/IRRemote.h"
#include "../Common/Receive.h"
#include "../Common/Hardware.h"
#include "../Common/Send.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/Win32Helpers.h"
#include "iguanaIR.h"
#include "SendReceiveData.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

WL_API int init(HANDLE exitEvent) {

	init_rec_buffer();
	init_send_buffer();
	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;
	if(!sendReceiveData->setTransmitters(settings.getTransmitterChannels())) return 0;

	return 1;
}

WL_API void deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = NULL;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = NULL;
}

WL_API int hasGui() {

	return TRUE;
}

BOOL CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {
			
			//===============
			int transmitters;
			//===============

			transmitters = settings.getTransmitterChannels();

			if(transmitters&IGUANA_TRANSMITTER1) SendDlgItemMessage(hwnd,IDC_CHECK1,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER2) SendDlgItemMessage(hwnd,IDC_CHECK2,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER3) SendDlgItemMessage(hwnd,IDC_CHECK3,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER4) SendDlgItemMessage(hwnd,IDC_CHECK4,BM_SETCHECK,BST_CHECKED,0);

			SetDlgItemInt(hwnd,IDC_EDIT1,settings.getDeviceNumber(),FALSE);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					//===============
					int temp;
					int transmitters;
					//===============

					transmitters = 0;

					temp = GetDlgItemInt(hwnd,IDC_EDIT1,NULL,FALSE);

					if(temp<0) temp = 0;

					settings.setDeviceNumber(temp);
					
					if(SendDlgItemMessage(hwnd,IDC_CHECK1,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER1;
					if(SendDlgItemMessage(hwnd,IDC_CHECK2,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER2;
					if(SendDlgItemMessage(hwnd,IDC_CHECK3,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER3;
					if(SendDlgItemMessage(hwnd,IDC_CHECK4,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER4;

					settings.setTransmitterChannels(transmitters);

					settings.saveSettings();

					DestroyWindow (hwnd);
					return TRUE;
				}

				case IDCANCEL: {
					//
					//ignore changes
					//
					DestroyWindow (hwnd);
					return TRUE;
				}

			}

			return FALSE;

		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
		case WM_CLOSE:
			DestroyWindow (hwnd);
			return TRUE;
	}

    return FALSE;

}

WL_API void	loadSetupGui() {

	//==============
	HWND	hDialog;
	MSG		msg;
    INT		status;
	//==============

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),NULL,dialogProc);

    while ((status = GetMessage (& msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, & msg)) {

            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }

}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	if(sendReceiveData) {
		return sendReceiveData->send(remote,code,repeats);
	}

	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(sendReceiveData) {

		if(!sendReceiveData->waitTillDataIsReady(0)) {
			return 0;
		}

		clear_rec_buffer();

		if(decodeCommand(remotes,out)) {
			return 1;
		}
	}

	return 0;
}

WL_API int setTransmitters(unsigned int transmitterMask) {

	if(sendReceiveData) {
		return sendReceiveData->setTransmitters(transmitterMask);
	}

	return 0;
}

WL_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;

}