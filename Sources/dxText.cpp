#include "..\Includes\dxText.h"

dxText::dxText(void)
{
	g_font = NULL; //The font object
}

dxText::~dxText(void)
{
	if( g_font != NULL) 
	{
        g_font->Release();
		g_font = NULL;
	}
}

bool dxText::init(DWORD size, LPDIRECT3DDEVICE9 device)
{	

	// Create a font
	D3DXCreateFont(device,     //D3D Device
                    size,               //Font height
                    0,                //Font width
                    FW_NORMAL,        //Font Weight
                    1,                //MipLevels
                    false,            //Italic
                    DEFAULT_CHARSET,  //CharSet
                    OUT_DEFAULT_PRECIS, //OutputPrecision
                    ANTIALIASED_QUALITY, //Quality
                    DEFAULT_PITCH|FF_DONTCARE,//PitchAndFamily
                    "Arial",          //pFacename,
                    &g_font);         //ppFont

	return true;
}

void dxText::drawText(std::string text, int x, int y, int width, int height)
{

	RECT font_rect = {0,0,width,height}; //sets the size of our font surface rect

	SetRect(&font_rect, //our font rect
			x, //Left
			y, //Top
			width, //width
			height //height
			);

	g_font->DrawText(NULL,        //pSprite
					text.c_str(),  //pString
					-1,          //Count
					&font_rect,  //pRect
					DT_LEFT|DT_NOCLIP,//Format,
					0xFFFFFFFF); //Color (white)

}