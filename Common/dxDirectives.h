/****************************************************************************
*                                                                           *
* dxDirectives.h -- Preprocessor directives                                 *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define getmax(a,b) a>b?a:b
#define getmin(a,b) a<b?a:b

#define DegToRad( x ) ( x *( pi/180 ) )

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"d3dxof.lib")
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"XInput.lib")

#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define CUSTOMFVF2 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define CUSTOMFVF3 (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define FULLSCREEN 0
#define SHOWCURSOR 1
