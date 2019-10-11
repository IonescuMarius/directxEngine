/****************************************************************************
*                                                                           *
* dxInput.h -- MANAGER INPUT                                                *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class dxInput
{
	public:
		dxInput(void);
		~dxInput(void);

		bool init(HINSTANCE hInst, HWND wndHandle);

		void getInput();
		//mouse functions
		bool isButtonDown(int button);
		int getMouseMovingX();
		int getMouseMovingY();
		//keyboard functions
		bool keyDown(DWORD key);
		bool keyUp(DWORD key);
		bool keyPress(DWORD key);

	private:

		LPDIRECTINPUT8        dInput;			// the direct input object
		LPDIRECTINPUTDEVICE8  mouseDevice;		// the direct input device for the mouse
		LPDIRECTINPUTDEVICE8 keyboardDevice;	// the direct input device for the keyboard

		DIMOUSESTATE mouseState;			// The current state of the mouse device
		
		UCHAR keyState[256];				//state of the keys
		UCHAR keyPressState[256];			//used for the keyPressed function

};
