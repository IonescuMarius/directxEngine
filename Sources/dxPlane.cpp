#include "..\Includes\dxPlane.h"
#include "..\Common\dxDirectives.h"

dxPlane::dxPlane(void)
{
    surface = NULL;
}

dxPlane::~dxPlane(void)
{
    if( surface != NULL) 
	{
        surface->Release();
		surface = NULL;
	}
}

bool dxPlane::loadPlane(LPDIRECT3DDEVICE9 device, std::string filename, bool screen)
{
	imageScale = 100;
	HRESULT hResult;
	D3DXIMAGE_INFO imageInfo;

	hResult = D3DXGetImageInfoFromFile(filename.c_str(), &imageInfo);
	if FAILED (hResult)
	{
		return false;
	}

	if(screen == true)
	{
		height = SCREEN_HEIGHT;
		width = SCREEN_WIDTH;
	}
	else
	{
		height = imageInfo.Height;
		width = imageInfo.Width;
	}
	hResult = device->CreateOffscreenPlainSurface(width,
		height,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&surface,
		NULL
		);

	if (FAILED(hResult))
		return false;

	hResult = D3DXLoadSurfaceFromFile(surface,
			NULL,
			NULL,
			filename.c_str(),
			NULL,
			D3DX_DEFAULT,
			0,
			NULL
		);

	if (FAILED(hResult))
		return false;

	destRect.left = 0;
	destRect.top = 0;
	destRect.bottom = destRect.top + height;
	destRect.right = destRect.left + width;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.bottom = destRect.top + height;
	srcRect.right = destRect.left + width;

	return true;
}

void dxPlane::render(LPDIRECT3DDEVICE9 pDevice)
{
	destRect.bottom = destRect.top + (int)(height * (imageScale / 100));
	destRect.right = destRect.left + (int)(width * (imageScale / 100));

	IDirect3DSurface9* backbuffer = NULL;
	pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	
	pDevice->StretchRect(surface, &srcRect, backbuffer, &destRect, D3DTEXF_NONE);
}

void dxPlane::setPosition(int x, int y){
	destRect.left = x;
	destRect.top = y;
	destRect.bottom = destRect.top + height;
	destRect.right = destRect.left + width;
}

void dxPlane::setSize(int percent)
{
	imageScale = percent;
}


void dxPlane::setSrcRect(int left, int top, int width, int height)
{
	srcRect.left = left;
	srcRect.top = top;
	srcRect.bottom = srcRect.top + height;
	srcRect.right = srcRect.left + width;
}

void dxPlane::setDestRect(int left, int top, int width, int height){
	destRect.left = left;
	destRect.top = top;
	destRect.bottom = destRect.top + height;
	destRect.right = destRect.left + width;
}