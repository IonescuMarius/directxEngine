/****************************************************************************
*                                                                           *
* dxBackBuffer.h -- BackBuffer                                              *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef dxBackBuffer_H
#define dxBackBuffer_H

#include <windows.h>

class dxBackBuffer
{
	public:
		dxBackBuffer(HWND hWnd, int width, int height);
		~dxBackBuffer();
		HDC getDC();
		int width();
		int height();
		void present();
	private:
		dxBackBuffer(const dxBackBuffer& rhs);
		dxBackBuffer& operator=(const dxBackBuffer& rhs);
	private:
		HWND mhWnd;
		HDC mhDC;
		HBITMAP mhSurface;
		HBITMAP mhOldObject;
		int mWidth;
		int mHeight;
};
#endif 