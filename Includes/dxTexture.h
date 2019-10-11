/****************************************************************************
*                                                                           *
* dxTexture.h -- TEXTURE                                                    *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#include <d3d9.h>
#include <d3dx9tex.h>
#include <string>

class dxTexture
{
     public:
		 dxTexture(void);
		 ~dxTexture(void);
		 bool initTexture(LPDIRECT3DDEVICE9 d3ddev, LPDIRECT3DTEXTURE9* texture, LPCTSTR filename);
		 void DrawTexture(LPDIRECT3DTEXTURE9 texture, LPD3DXSPRITE d3dspt, RECT texcoords, float x, float y, int a);	

     private:
		LPDIRECT3DTEXTURE9 texture;
};
