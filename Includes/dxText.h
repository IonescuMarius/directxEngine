/****************************************************************************
*                                                                           *
* dxText.h -- SPRITE TEXT                                                   *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#include <d3d9.h>
#include <d3dx9tex.h>
#include <string>

class dxText
{
     public:
		 dxText(void);
		 ~dxText(void);

		 bool init(DWORD size, LPDIRECT3DDEVICE9 device);
		 void drawText(std::string text, int x, int y, int width, int height);

     private:
		ID3DXFont *g_font;
};
