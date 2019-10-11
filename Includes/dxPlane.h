/****************************************************************************
*                                                                           *
* dxPlane.h -- PLANE SURFACE                                                *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <d3d9.h>
#include <d3dx9tex.h>
#include <string>

class dxPlane
{
	public:
		dxPlane(void);
		virtual ~dxPlane(void);

		bool loadPlane(LPDIRECT3DDEVICE9 device, std::string filename, bool screen);
		void render(LPDIRECT3DDEVICE9 pDevice);
		void setPosition(int x, int y);
		void setSize(int percent);
		void setSrcRect(int left, int top, int height, int width);
		void setDestRect(int left, int top, int height, int width);

	private:
		IDirect3DSurface9* surface;
		RECT srcRect;
		RECT destRect;
		int width;
		int height;
		int imageScale;
};
