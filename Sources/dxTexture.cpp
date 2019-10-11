#include "..\Includes\dxTexture.h"
#include "..\Includes\dxUtility.h"

dxTexture::dxTexture(void)
{
	texture = NULL; 
}

dxTexture::~dxTexture(void)
{
	if( texture != NULL) 
	{
        texture->Release();
		texture = NULL;
	}
}

bool dxTexture::initTexture(LPDIRECT3DDEVICE9 d3ddev, LPDIRECT3DTEXTURE9* texture, LPCTSTR filename)
{
	 HRESULT hrTexure = D3DXCreateTextureFromFileEx(d3ddev, 
						filename, 
						D3DX_DEFAULT, 
						D3DX_DEFAULT,
						D3DX_DEFAULT, 
						NULL, 
						D3DFMT_A8R8G8B8, 
						D3DPOOL_MANAGED, 
						D3DX_DEFAULT, 
						D3DX_DEFAULT, 
						D3DCOLOR_XRGB(255, 0, 255), 
						NULL, 
						NULL, 
					texture);

	if (CUtility::FailedHr(hrTexure))
	{
		CUtility::DebugString("Could not create Texture\n");
		return false;
	}
        
	return true;
}

void dxTexture::DrawTexture(LPDIRECT3DTEXTURE9 texture, LPD3DXSPRITE d3dspt, RECT texcoords, float x, float y, int a)
{
    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f), position(x, y, 0.0f);
    d3dspt->Draw(texture, &texcoords, &center, &position, D3DCOLOR_ARGB(a,255, 255, 255));

    return;
}