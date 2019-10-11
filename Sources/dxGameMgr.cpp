/****************************************************************************
*                                                                           *
* dxGameMgr.cpp -- MANAGER PROCESSING GAME                                  *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009-2011).             *
*                                                                           *
****************************************************************************/

#include "..\Includes\dxMgr.h"
#include "..\Includes\dxGameMgr.h"
#include "..\Includes\dxUtility.h"
#include "..\Includes\dxText.h"
#include "..\Includes\dxPlane.h"
#include "..\Includes\dxLight.h"
#include "..\Includes\dxCamera.h"
#include "..\Includes\dxTimer.h"
#include "..\Includes\dxInput.h"
#include "..\Includes\dxSound.h"
#include "..\Includes\dxLight.h"
#include "..\Includes\dxMenu.h"
#include "..\Includes\dxModel.h"
#include "..\Common\dxDirectives.h"
#include "..\Includes\dxObject.h"
#include "..\Includes\dxMeshAnimation.h"
#include "..\Includes\dxScene.h"

dxGameMgr::dxGameMgr()
{
	dx_hWnd          = NULL;
    dx_pD3D          = NULL;
    dx_pD3DDevice    = NULL;
    dx_hIcon         = NULL;
    dx_hMenu         = NULL;
    dx_strLastFile   = NULL;
    dx_bLostDevice   = false;
    dx_LastFrameRate = 0;
    
    dx_FillMode      = D3DFILL_SOLID;
    dx_MinFilter     = D3DTEXF_LINEAR;
    dx_MagFilter     = D3DTEXF_LINEAR;
    dx_MipFilter     = D3DTEXF_LINEAR;
    dx_Anisotropy    = 1;
	m_font			 = NULL;
	loadedRect		 = false;
}

dxGameMgr::~dxGameMgr()
{
	ShutDown();
}

bool dxGameMgr::InitInstance( HANDLE hInstance, LPCTSTR lpCmdLine, int iCmdShow )
{    
    if (!CreateDisplay()) 
	{ 
		ShutDown(); 
		return false; 
	}

    if (!TestDeviceCaps( )) 
	{ 
		ShutDown(); 
		return false; 
	}

    if (!BuildEntities()) 
	{ 
		ShutDown(); 
		return false; 
	}

    SetupGameState();
    SetupRenderStates();

    return true;
}

bool dxGameMgr::CreateDisplay()
{
    D3DDISPLAYMODE  MatchMode;
    dxD3DSettingsDlg SettingsDlg;
    dxD3DInit       Initialize;
    LPTSTR          WindowTitle  = _T("Engine DX");
    USHORT          Width        = 640;
    USHORT          Height       = 480;
    RECT            rc;

    // D3D Object
    dx_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if (!dx_pD3D) 
    {
        MessageBox( dx_hWnd, _T("No compatible Direct3D object could be created."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    }

    // system graphics adapters    
    if ( FAILED(Initialize.Enumerate( dx_pD3D ) ))
    {
        MessageBox( dx_hWnd, _T("Device enumeration failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    }

    MatchMode.Width       = 640;
    MatchMode.Height      = 480;
    MatchMode.Format      = D3DFMT_UNKNOWN;
    MatchMode.RefreshRate = 0;
    Initialize.FindBestFullscreenMode( dx_D3DSettings, &MatchMode );
    
    Initialize.FindBestWindowedMode( dx_D3DSettings );

    // direct 3d device
    if ( FAILED( Initialize.CreateDisplay( dx_D3DSettings, 0, NULL, StaticWndProc, WindowTitle, Width, Height, this ) ))
    {
        MessageBox( dx_hWnd, _T("Device creation failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    }
		
    dx_pD3DDevice = Initialize.GetDirect3DDevice( );
    dx_hWnd       = Initialize.GetHWND( );

	HRESULT hrSprite = D3DXCreateSprite(dx_pD3DDevice, &d3dspt);
	std::string texturePath = CUtility::GetTheCurrentDirectory()+"/data/ASHSEN_2.bmp";
	LoadTexture(&DisplayTexture, texturePath.c_str()); 

    dx_hIcon = LoadIcon( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_ICON ) );
    dx_hMenu = LoadMenu( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDR_MENU ) );

    SetClassLong( dx_hWnd, GCL_HICON, (long)dx_hIcon );

    // windowed mode
    if ( dx_D3DSettings.Windowed )
    {
        SetMenu( dx_hWnd, dx_hMenu );
    
    }

    // menu states
    ::CheckMenuRadioItem( dx_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_SOLID, MF_BYCOMMAND );
	::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC, ID_CAMERAMODE_THIRDPERSON, MF_BYCOMMAND );

    ::GetClientRect( dx_hWnd, &rc );
    dx_nViewX      = rc.left;
    dx_nViewY      = rc.top;
    dx_nViewWidth  = rc.right - rc.left;
    dx_nViewHeight = rc.bottom - rc.top;

	input = new dxInput();
	input->init((HINSTANCE)GetModuleHandle(NULL), dx_hWnd);

    // Show the window
	ShowWindow(dx_hWnd, SW_SHOW);

    return true;
}

bool dxGameMgr::ShutDown()
{
	ReleaseEntities ( );

    if ( dx_pD3DDevice ) 
		dx_pD3DDevice->Release();
    if ( dx_pD3D       ) 
		dx_pD3D->Release();
    dx_pD3D          = NULL;
    dx_pD3DDevice    = NULL;
    
    if ( dx_hMenu ) 
		DestroyMenu( dx_hMenu );
    dx_hMenu         = NULL;

    SetMenu( dx_hWnd, NULL );
    if ( dx_hWnd ) DestroyWindow( dx_hWnd );
		dx_hWnd = NULL;

    if ( dx_strLastFile ) 
		free( dx_strLastFile );
    dx_strLastFile = NULL;

	//if ( m_font ) 
		//m_font->Release();

	return true;
}

bool dxGameMgr::TestDeviceCaps()
{
    HRESULT  hRet;
    D3DCAPS9 Caps;
    ULONG    Enable, Value;

    // Retrieve device caps, the following states are not required (as in ValidateDevice)
    dxD3DSettings::Settings *pSettings = dx_D3DSettings.GetSettings();
    hRet = dx_pD3D->GetDeviceCaps( pSettings->AdapterOrdinal, pSettings->DeviceType, &Caps );
    if ( FAILED(hRet) ) 
		return false;
    
    ZeroMemory( dx_MinFilterCaps, 10 * sizeof(bool) );
    ZeroMemory( dx_MagFilterCaps, 10 * sizeof(bool) );
    ZeroMemory( dx_MipFilterCaps, 10 * sizeof(bool) );
    
    // Set up those states always supported
    dx_MinFilterCaps[D3DTEXF_NONE] = true;
    dx_MagFilterCaps[D3DTEXF_NONE] = true;
    dx_MipFilterCaps[D3DTEXF_NONE] = true;
    dx_MaxAnisotropy = 0;
    
    // Test Texture Filter Caps
    Value = Caps.TextureFilterCaps;

    // Determine if anisotropic minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFANISOTROPIC) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MINFILTER_ANISOTROPIC, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MinFilterCaps[D3DTEXF_ANISOTROPIC] = true;

    // Determine if linear minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFLINEAR) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MINFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MinFilterCaps[D3DTEXF_LINEAR] = true;
    
    // Determine if point minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFPOINT) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MINFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MinFilterCaps[D3DTEXF_POINT] = true;

    // Determine if anisotropic magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFANISOTROPIC) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MAGFILTER_ANISOTROPIC, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MagFilterCaps[D3DTEXF_ANISOTROPIC] = true;

    // Determine if linear magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFLINEAR) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MAGFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MagFilterCaps[D3DTEXF_LINEAR] = true;
    
    // Determine if point magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFPOINT) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MAGFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MagFilterCaps[D3DTEXF_POINT] = true;
    
    // Determine if linear mip filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MIPFLINEAR) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MIPFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MipFilterCaps[D3DTEXF_LINEAR] = true;

    // Determine if point mip filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MIPFPOINT) ) 
		Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( dx_hMenu, ID_MIPFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MipFilterCaps[D3DTEXF_POINT] = true;

    // Test anisotropy levels
    Value = Caps.MaxAnisotropy;

    // Determine which anisotropy levels are supported
    if ( Value < 1 ) Enable = MF_DISABLED | MF_GRAYED; 
	else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_1, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 1;

    if ( Value < 2 ) 
		Enable = MF_DISABLED | MF_GRAYED; 
	else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_2, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 2;

    if ( Value < 4 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_4, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 4;

    if ( Value < 8 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_8, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 8;

    if ( Value < 16 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_16, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 16;
    
    if ( Value < 32 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_32, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 32;

    if ( Value < 64 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( dx_hMenu, ID_MAXANISOTROPY_64, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) 
		dx_MaxAnisotropy = 64;

    // Now determine if our currently selected states are supported, swap otherwise
    if ( dx_MinFilterCaps[ dx_MinFilter ] == false )
    {
        if ( dx_MinFilterCaps[ D3DTEXF_ANISOTROPIC ] ) dx_MinFilter = D3DTEXF_ANISOTROPIC;
        else if ( dx_MinFilterCaps[ D3DTEXF_LINEAR      ] ) dx_MinFilter = D3DTEXF_LINEAR;
        else if ( dx_MinFilterCaps[ D3DTEXF_POINT       ] ) dx_MinFilter = D3DTEXF_POINT;
        else if ( dx_MinFilterCaps[ D3DTEXF_NONE        ] ) dx_MinFilter = D3DTEXF_NONE;
        else return false;
    }

    if ( dx_MagFilterCaps[ dx_MagFilter ] == false )
    {
        if ( dx_MagFilterCaps[ D3DTEXF_ANISOTROPIC ] ) dx_MagFilter = D3DTEXF_ANISOTROPIC;
        else if ( dx_MagFilterCaps[ D3DTEXF_LINEAR      ] ) dx_MagFilter = D3DTEXF_LINEAR;
        else if ( dx_MagFilterCaps[ D3DTEXF_POINT       ] ) dx_MagFilter = D3DTEXF_POINT;
        else if ( dx_MagFilterCaps[ D3DTEXF_NONE        ] ) dx_MagFilter = D3DTEXF_NONE;
        else return false;
    }

    if ( dx_MipFilterCaps[ dx_MipFilter ] == false )
    {
        if ( dx_MipFilterCaps[ D3DTEXF_ANISOTROPIC ] ) dx_MipFilter = D3DTEXF_ANISOTROPIC;
        else if ( dx_MipFilterCaps[ D3DTEXF_LINEAR      ] ) dx_MipFilter = D3DTEXF_LINEAR;
        else if ( dx_MipFilterCaps[ D3DTEXF_POINT       ] ) dx_MipFilter = D3DTEXF_POINT;
        else if ( dx_MipFilterCaps[ D3DTEXF_NONE        ] ) dx_MipFilter = D3DTEXF_NONE;
        else return false;
    } 

    // Test max anisotropy
    if ( dx_Anisotropy > dx_MaxAnisotropy ) 
		dx_Anisotropy = dx_MaxAnisotropy;

    ULONG      Ordinal = pSettings->AdapterOrdinal;
    D3DDEVTYPE Type    = pSettings->DeviceType;
    D3DFORMAT  AFormat = pSettings->DisplayMode.Format;
    
    dx_TextureFormat    = D3DFMT_UNKNOWN;
    dx_AlphaFormat      = D3DFMT_UNKNOWN;
    
    if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3 )) )
        dx_TextureFormat = D3DFMT_DXT3;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8 )) )
        dx_TextureFormat = D3DFMT_X8R8G8B8;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_R5G6B5 )) )
        dx_TextureFormat = D3DFMT_R5G6B5;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5 )) )
        dx_TextureFormat = D3DFMT_X1R5G5B5;
    else
    {
        MessageBox( 0, "No valid texture formats could be found.", "Error.", MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    }

    // alpha texture formats 
    if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3 )) ) 
        dx_AlphaFormat = D3DFMT_DXT3;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8 )) )
        dx_AlphaFormat = D3DFMT_A8R8G8B8;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5 )) )
        dx_AlphaFormat = D3DFMT_A1R5G5B5;
    else if ( SUCCEEDED( dx_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4 )) )
        dx_AlphaFormat = D3DFMT_A4R4G4B4;
    else
    {
        MessageBox( 0, "No valid alpha texture formats could be found.", "Error.", MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    } 

    return true;
}

void dxGameMgr::SetupGameState()
{
	D3DXMatrixIdentity( &dx_mtxIdentity );
    dx_bActive = true;

	modelEntityPos1 = D3DXVECTOR3(10, 0, -0);

	m_Player.SetCameraMode( dxCamera::MODE_THIRDPERSON );
    dx_pCamera = m_Player.GetCamera();
	m_Player.SetCamOffset ( D3DXVECTOR3( 0.0f, 20.0f, -50.0f ) );
    
    //m_Player.SetFriction( 10.0f );
    //m_Player.SetGravity( D3DXVECTOR3( 0, 0.0f, 0 ) );
    //m_Player.SetMaxVelocityXZ( 10.0f );
    //m_Player.SetMaxVelocityY ( 100.0f );
    //m_Player.SetCamLag( 0.08f );
    //m_Player.SetPosition( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );

	_objScene.SetCameraMode( dxCamera::MODE_THIRDPERSON );
	dx_pCamera = _objScene.GetCamera();
	_objScene.SetCamOffset ( D3DXVECTOR3( 0.0f, 0, -50.0f ) );
	//_objScene.SetPosition( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );

    dx_pCamera->SetFOV( 60.0f );
    dx_pCamera->SetViewport( dx_nViewX, dx_nViewY, dx_nViewWidth, dx_nViewHeight, 1.01f, 5000.0f );

    ZeroMemory( &m_Light, 6 * sizeof(D3DLIGHT9) );
    
    // Main static directional light
    m_Light[0].Type      = D3DLIGHT_DIRECTIONAL;
    m_Light[0].Direction = D3DXVECTOR3( 0.650945f, -10.390567f, -100.650945f );
    m_Light[0].Diffuse.a = 1.0f;
    m_Light[0].Diffuse.r = 1.0f;
    m_Light[0].Diffuse.g = 1.0f;
    m_Light[0].Diffuse.b = 1.0f;
    m_LightEnabled[0]    = true;

    // Players following light
    m_Light[1].Type         = D3DLIGHT_SPOT;
	m_Light[1].Position     = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z); //m_Player.GetPosition();
    m_Light[1].Direction	=  D3DXVECTOR3( 20.00f, 30.30f, 0.00f );//m_Player.GetPosition();
	m_Light[1].Range        = 1.0f;
    m_Light[1].Attenuation0 = 1.0f;
    //m_Light[1].Attenuation2 = 0.02f;
    m_Light[1].Diffuse.a    = 1.0f;
    m_Light[1].Diffuse.r    = 0.0f; 
    m_Light[1].Diffuse.g    = 1.0f;
    m_Light[1].Diffuse.b    = 0.0f;
	m_Light[1].Phi			= 50.0f;//D3DXToRadian(40.0f);    // set the outer cone to 30 degrees
    m_Light[1].Theta		= 10.5f;//D3DXToRadian(20.0f);    // set the inner cone to 10 degrees
    m_Light[1].Falloff		= 1.0f;    // use the typical falloff
    m_LightEnabled[1]       = true;

    // Dynamic floating light 1
    m_Light[2].Type         = D3DLIGHT_POINT;
    m_Light[2].Position     = D3DXVECTOR3( 20.00f, 30.30f, 0.00f );
    m_Light[2].Range        = 500.0f;
    m_Light[2].Attenuation1 = 0.0002f;
    m_Light[2].Attenuation2 = 0.0002f;
    m_Light[2].Diffuse.a    = 1.0f;
    m_Light[2].Diffuse.r    = 1.0f; 
    m_Light[2].Diffuse.g    = 1.0f;
    m_Light[2].Diffuse.b    = 1.0f;
    m_LightEnabled[2]       = false;

    // Dynamic floating light 2
    m_Light[3].Type         = D3DLIGHT_POINT;
    m_Light[3].Position     = D3DXVECTOR3( 1000, 0, 1000 );
    m_Light[3].Range        = 500.0f;
    m_Light[3].Attenuation1 = 0.000002f;
    m_Light[3].Attenuation2 = 0.00002f;
    m_Light[3].Diffuse.a    = 1.0f;
    m_Light[3].Diffuse.r    = 0.0f; 
    m_Light[3].Diffuse.g    = 0.0f;
    m_Light[3].Diffuse.b    = 1.0f;
    m_LightEnabled[3]       = true;

    // Dynamic floating light 3
    /*m_Light[4].Type         = D3DLIGHT_POINT;
    m_Light[4].Position     = D3DXVECTOR3( 1500, 0, 1500 );
    m_Light[4].Position.y   = m_Terrain.GetHeight( m_Light[4].Position.x, m_Light[4].Position.z ) + 30.0f;
    m_Light[4].Range        = 500.0f;
    m_Light[4].Attenuation1 = 0.00002f;
    m_Light[4].Attenuation2 = 0.00002f;
    m_Light[4].Diffuse.a    = 1.0f;
    m_Light[4].Diffuse.r    = 1.0f; 
    m_Light[4].Diffuse.g    = 1.0f;
    m_Light[4].Diffuse.b    = 0.5f;
    m_LightEnabled[4]       = true;*/

	m_Light[4].Type      = D3DLIGHT_DIRECTIONAL;
    m_Light[4].Direction = D3DXVECTOR3( 0.650945f, -0.390567f, -0.650945f );
    m_Light[4].Diffuse.a = 1.0f;
    m_Light[4].Diffuse.r = 1.0f;
    m_Light[4].Diffuse.g = 1.0f;
    m_Light[4].Diffuse.b = 1.0f;
    m_LightEnabled[4]    = true;

    m_Light[5].Type      = D3DLIGHT_DIRECTIONAL;
    m_Light[5].Direction = D3DXVECTOR3( 0.650945f, -10.390567f, 100.650945f );
	m_Light[5].Attenuation1 = 5.02f;
    m_Light[5].Attenuation2 = 5.02f;
    m_Light[5].Diffuse.a = 1.0f;
    m_Light[5].Diffuse.r = 1.0f;
    m_Light[5].Diffuse.g = 1.0f;
    m_Light[5].Diffuse.b = 1.0f;
    m_LightEnabled[5]    = true;

}

void dxGameMgr::SetupRenderStates ()
{
    if (!dx_pD3DDevice || !dx_pCamera ) 
		return;

	if (!TestDeviceCaps( )) 
	{ 
		PostQuitMessage(0); 
		return; 
	}

    dx_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    dx_pD3DDevice->SetRenderState( D3DRS_DITHERENABLE,  TRUE );
    dx_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    dx_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    dx_pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    dx_pD3DDevice->SetRenderState( D3DRS_AMBIENT, 0x0D0D0D );
    dx_pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	dx_pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE); 

	dx_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER    , dx_MinFilter );
    dx_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER    , dx_MagFilter );
    dx_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER    , dx_MipFilter );
    dx_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, dx_Anisotropy );
    dx_pD3DDevice->SetRenderState( D3DRS_FILLMODE, dx_FillMode );                
   
	//dx_pD3DDevice->SetFVF( D3DFVF_XYZ | D3DFVF_NORMAL | VERTEX_FVF);
    //dx_pD3DDevice->SetMaterial( &m_BaseMaterial );
	//dx_Scene.SetTextureFormat( dx_TextureFormat );

	//objLight = new dxLight(dx_pD3DDevice);
	//objLight->initLightCustom(dx_pD3DDevice);

	D3DXCreateFont( dx_pD3DDevice, FontSize, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font );


    for ( ULONG i = 0; i < 6; i++ )
    {
        dx_pD3DDevice->SetLight( i, &m_Light[i] );
        dx_pD3DDevice->LightEnable( i, m_LightEnabled[i] );
    }

    dx_pCamera->UpdateRenderView( dx_pD3DDevice );
    dx_pCamera->UpdateRenderProj( dx_pD3DDevice );

	SelectMenuItems();
}

void dxGameMgr::SelectMenuItems()
{
	// Select which minfication filter menu item is selected
    switch ( dx_MinFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_LINEAR, MF_BYCOMMAND );
            break;
        case D3DTEXF_ANISOTROPIC: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_ANISOTROPIC, MF_BYCOMMAND );
            break;
    
    } // End Switch

    // Select which magnification filter menu item is selected
    switch ( dx_MagFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_LINEAR, MF_BYCOMMAND );
            break;
        case D3DTEXF_ANISOTROPIC: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_ANISOTROPIC, MF_BYCOMMAND );
            break;
    
    } // End Switch
    
    // Select which mip-filter menu item is selected
    switch ( dx_MipFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( dx_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_LINEAR, MF_BYCOMMAND );
            break;

    } // End Switch

    // Select which fill mode menu item is selected
    switch ( dx_FillMode )
    {
        case D3DFILL_WIREFRAME:
            ::CheckMenuRadioItem( dx_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_WIREFRAME, MF_BYCOMMAND );
            break;

        case D3DFILL_SOLID:
            ::CheckMenuRadioItem( dx_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_SOLID, MF_BYCOMMAND );
            break;

    } // End Switch

    // Select which anisotropy level menu item is selected
    switch ( dx_Anisotropy )
    {
        case 1:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_1, MF_BYCOMMAND );
            break;
        case 2:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_2, MF_BYCOMMAND );
            break;
        case 4:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_4, MF_BYCOMMAND );
            break;
        case 8:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_8, MF_BYCOMMAND );
            break;
        case 16:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_16, MF_BYCOMMAND );
            break;
        case 32:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_32, MF_BYCOMMAND );
            break;
        case 64:
            ::CheckMenuRadioItem( dx_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_64, MF_BYCOMMAND );
            break;

    } // End Switch
}

int dxGameMgr::BeginGame()
{
    MSG  dxMsg;

    while (1) 
    {
        if ( PeekMessage(&dxMsg, NULL, 0, 0, PM_REMOVE) ) 
        {
            if (dxMsg.message == WM_QUIT) break;
            TranslateMessage( &dxMsg );
            DispatchMessage ( &dxMsg );
        } 
        else 
        {
            FrameAdvance();
        }
    }

    return 0;
}

LRESULT CALLBACK dxGameMgr::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    if ( Message == WM_CREATE ) 
		SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

    dxGameMgr *Destination = (dxGameMgr*)GetWindowLong( hWnd, GWL_USERDATA );
    
    if (Destination) 
		return Destination->DisplayWndProc( hWnd, Message, wParam, lParam );
    
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

LRESULT dxGameMgr::DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
    dxD3DInit  Initialize;
	
    // Message type
	switch (Message)
    {
		case WM_CREATE:
            break;
		
        case WM_CLOSE:
			PostQuitMessage(0);
			break;
		
        case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
        case WM_SIZE:

            if ( wParam == SIZE_MINIMIZED )
            {
                dx_bActive = false;
            }
            else
            {
                dx_bActive = true;

                RECT rc;
                ::GetClientRect( dx_hWnd, &rc );
                dx_nViewWidth  = rc.right - rc.left;
                dx_nViewHeight = rc.bottom - rc.top;
        
                if (dx_pD3DDevice) 
                {
                    // Reset the device
                    if ( dx_pCamera ) dx_pCamera->SetViewport( dx_nViewX, dx_nViewY, dx_nViewWidth, dx_nViewHeight, 1.01f, 5000.0f );
                    Initialize.ResetDisplay( dx_pD3DDevice, dx_D3DSettings );
                    SetupRenderStates( );
                }
            }

			break;

        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            // Capture the mouse
            SetCapture( dx_hWnd );
            GetCursorPos( &dx_OldCursorPos );
            break;

        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
            // Release the mouse
            ReleaseCapture( );
            break;

        case WM_KEYDOWN:

            // Which key was pressed?
			switch (wParam) 
            {
				case VK_ESCAPE:
					PostQuitMessage(0);
					return 0;

                case VK_RETURN:
                    if ( GetKeyState( VK_SHIFT ) & 0xFF00 )
                    {
                        // Toggle fullscreen / windowed
                        dx_D3DSettings.Windowed = !dx_D3DSettings.Windowed;
                        Initialize.ResetDisplay( dx_pD3DDevice, dx_D3DSettings, dx_hWnd );

                        // Store new viewport sizes
                        if ( !dx_D3DSettings.Windowed )
                        {
                            dx_nViewWidth  = dx_D3DSettings.Fullscreen_Settings.DisplayMode.Width;
                            dx_nViewHeight = dx_D3DSettings.Fullscreen_Settings.DisplayMode.Height;
                        
                        } // End if full screen
                        else
                        {
                            RECT rc;
                            ::GetClientRect( dx_hWnd, &rc );
                            dx_nViewWidth  = rc.right - rc.left;
                            dx_nViewHeight = rc.bottom - rc.top;

                        } // End if windowed
                        if ( dx_pCamera ) dx_pCamera->SetViewport( dx_nViewX, dx_nViewY, dx_nViewWidth, dx_nViewHeight, 1.01f, 5000.0f );

                        // Setup the render states
                        SetupRenderStates( );

                        // Set menu only in windowed mode
                        // (Removed by ResetDisplay automatically in fullscreen)
                        if ( dx_D3DSettings.Windowed )
                        {
                            SetMenu( dx_hWnd, dx_hMenu );
                        } // End if Windowed

                    } // End if
                    break;
                    
			} // End Switch

			break;

        case WM_COMMAND:

            // Process Menu Items
            switch( LOWORD(wParam) )
            {
                case ID_FILE_OPENMESH:
                    // Open a new mesh file
                    OpenMeshFile( );
                    break;

                case ID_FILE_CHANGEDEVICE:
                    // Signal that we want to change devices
                    ChangeDevice();
                    break;
                
                case ID_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( dx_hWnd, WM_CLOSE, 0, 0 );
                    return 0;

                case ID_CAMERAMODE_FPS:
                    // Set camera mode to FPS style
                    ::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_FPS, MF_BYCOMMAND );

                    //Switch camera mode
                    m_Player.SetCameraMode    ( dxCamera::MODE_FPS_FLY );
                    dx_pCamera = m_Player.GetCamera();
                    
                    //Setup player details
                    m_Player.SetFriction      ( 10.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0.0f, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 10.0f );
                    m_Player.SetMaxVelocityY  ( 10.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
                    m_Player.SetCamLag        ( 0.0f ); // No camera lag

                    break;

                case ID_CAMERAMODE_ORBITCAM:
                    // Set camera mode to orbiting camera style
                    ::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_ORBITCAM, MF_BYCOMMAND );

                    // Switch camera mode
                    m_Player.SetCameraMode( dxCamera::MODE_THIRDPERSON );
                    dx_pCamera = m_Player.GetCamera();

                    //Setup player details
                    m_Player.SetFriction      ( 0.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 0.0f );
                    m_Player.SetMaxVelocityY  ( 0.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 0.0f, -40.0f ) );
                    m_Player.SetCamLag        ( 0.0f ); // No camera lag

                    break;

				case ID_CAMERAMODE_SPACECRAFT:
                    // Set camera mode to SPACECRAFT style
                    ::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_SPACECRAFT, MF_BYCOMMAND );

                    // Setup player details
                    m_Player.SetFriction      ( 10.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 10.0f );
                    m_Player.SetMaxVelocityY  ( 10.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
                    m_Player.SetCamLag        ( 0.0f ); // No camera lag

                    // Switch camera mode
                    m_Player.SetCameraMode( dxCamera::MODE_SPACECRAFT );
                    dx_pCamera = m_Player.GetCamera();

                    break;

                case ID_CAMERAMODE_THIRDPERSON:
                    // Set camera mode to third person style
                    ::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_THIRDPERSON, MF_BYCOMMAND );

                    // Setup Player details
                    m_Player.SetFriction      ( 250.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, -10.0f, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 10.0f );
                    m_Player.SetMaxVelocityY  ( 10.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 40.0f, -60.0f ) );
                    m_Player.SetCamLag        ( 0.08f ); // 1/4 second camera lag
                    
                    // Switch camera mode
                    m_Player.SetCameraMode    ( dxCamera::MODE_THIRDPERSON );
                    dx_pCamera = m_Player.GetCamera();

                    break;

                case ID_CAMERAMODE_THIRDPERSON_SC:
                    // Set camera mode to third person space craft style
                    ::CheckMenuRadioItem( dx_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_THIRDPERSON_SC, MF_BYCOMMAND );

                    // Setup Player details
                    m_Player.SetFriction      ( 125.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 400.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 40.0f, -60.0f ) );
                    m_Player.SetCamLag        ( 0.08f ); // 1/12 second camera lag
                    
                    // Switch camera mode
                    m_Player.SetCameraMode    ( dxCamera::MODE_THIRDPERSON_SC );
                    dx_pCamera = m_Player.GetCamera();

                    break;

                case ID_FILLMODE_SOLID:
                    // Set fill mode to solid
                    dx_FillMode = D3DFILL_SOLID;
                    dx_pD3DDevice->SetRenderState( D3DRS_FILLMODE, dx_FillMode );
                    SelectMenuItems();
                    break;

                case ID_FILLMODE_WIREFRAME:
                    // Set fill mode to wireframe
                    dx_FillMode = D3DFILL_WIREFRAME;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_NONE:
                    // Set filter modes to none
                    dx_MinFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_POINT:
                    // Set filter modes to point
                    dx_MinFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_LINEAR:
                    // Set filter modes to linear
                    dx_MinFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_ANISOTROPIC:
                    // Set filter modes to anisotropic
                    dx_MinFilter = D3DTEXF_ANISOTROPIC;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_NONE:
                    // Set filter modes to none
                    dx_MagFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_POINT:
                    // Set filter modes to point
                    dx_MagFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_LINEAR:
                    // Set filter modes to linear
                    dx_MagFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_ANISOTROPIC:
                    // Set filter modes to anisotropic
                    dx_MagFilter = D3DTEXF_ANISOTROPIC;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;


                case ID_MIPFILTER_NONE:
                    // Set mip filter modes to none
                    dx_MipFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MIPFILTER_POINT:
                    // Set mip filter modes to point
                    dx_MipFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MIPFILTER_LINEAR:
                    // Set mip filter modes to linear
                    dx_MipFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_1:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 1;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_2:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 2;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_4:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 4;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_8:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 8;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_16:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 16;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_32:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 32;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAXANISOTROPY_64:
                    // Set alternate max anisotropy 
                    dx_Anisotropy = 64;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;
            
            } // End Switch

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    
    return 0;
}

void dxGameMgr::ChangeDevice()
{
    dxD3DInit      Initialize;
    dxD3DSettingsDlg SettingsDlg;

    ReleaseEntities();

    if ( FAILED(Initialize.Enumerate( dx_pD3D ) ))
    {
        MessageBox( dx_hWnd, _T("Device enumeration failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        PostQuitMessage( 0 );
        return;
    }

    // Display the settings dialog
    int RetCode = SettingsDlg.ShowDialog( &Initialize, &dx_D3DSettings );
    if ( RetCode != IDOK ) return;
    dx_D3DSettings = SettingsDlg.GetD3DSettings();

    if ( dx_pD3DDevice ) dx_pD3DDevice->Release();
    dx_pD3DDevice = NULL;

    if ( FAILED (Initialize.CreateDisplay( dx_D3DSettings, 0, dx_hWnd )) )
    {
        MessageBox( dx_hWnd, _T("Device creation failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        PostQuitMessage( 0 );
        return;
    }

    dx_pD3DDevice = Initialize.GetDirect3DDevice( );

    if ( !dx_D3DSettings.Windowed )
    {
        dx_nViewWidth  = dx_D3DSettings.Fullscreen_Settings.DisplayMode.Width;
        dx_nViewHeight = dx_D3DSettings.Fullscreen_Settings.DisplayMode.Height;
    
    } 
    else
    {
        RECT rc;
        ::GetClientRect( dx_hWnd, &rc );
        dx_nViewWidth  = rc.right - rc.left;
        dx_nViewHeight = rc.bottom - rc.top;

    } 
    if ( dx_pCamera ) 
		dx_pCamera->SetViewport( dx_nViewX, dx_nViewY, dx_nViewWidth, dx_nViewHeight, 1.01f, 5000.0f );

    if ( dx_D3DSettings.Windowed )
    {
        SetMenu( dx_hWnd, dx_hMenu );
    } 

    BuildEntities();

    SetupRenderStates( );
}

bool dxGameMgr::OpenMeshFile()
{
	static ULONG             LastFilter = 1;
    OPENFILENAME             File;
    TCHAR                    FileName[MAX_PATH];
    dxD3DSettings::Settings *pSettings = dx_D3DSettings.GetSettings();
    bool                     HardwareTnL = true;
    D3DCAPS9                 Caps;
    
    ZeroMemory( &File, sizeof(OPENFILENAME) );
    ZeroMemory( FileName, MAX_PATH * sizeof(TCHAR));
    File.lStructSize  = sizeof(OPENFILENAME);
    File.hwndOwner    = dx_hWnd;
	File.lpstrFilter = _T("All Supported Formats\0")
		_T("*.IWF;*.BWF;*.X\0")
		_T("Interchangable World Format (*.iwf,*.bwf)\0")
		_T("*.IWF;*.BWF\0")
		_T("DirectX Mesh Files (*.x)\0")
		_T("*.X\0");
    
    File.nFilterIndex = LastFilter;
    File.Flags        = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
    File.lpstrFile    = FileName;
    File.nMaxFile     = MAX_PATH - 1;

    if ( !GetOpenFileName( &File ) ) 
		return false;

    LastFilter = File.nFilterIndex;

    if ( pSettings->VertexProcessingType == SOFTWARE_VP ) 
		HardwareTnL = false;

    dx_pD3D->GetDeviceCaps( pSettings->AdapterOrdinal, pSettings->DeviceType, &Caps );
    ULONG LightLimit = Caps.MaxActiveLights;

    return true;
}

bool dxGameMgr::BuildEntities()
{
	//bones_all//world_02//scene//biped
	char *m_strFile = _tcsdup( _T("Data\\car.x") );
	char *m_strFileBiped = _tcsdup( _T("Data\\biped.x") );
	LPCTSTR m_strFileNmae = _tcsdup( _T("Data\\car.x") );
	LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/scene.x",0, 1);
	LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/spherex.x",0, 2);
	//LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/biped.x",0, 2);
	//LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/car.x",0, 4);
	LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/audi_a6.x",0, 5);
	m_Player.LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/car.x",1, 1, dx_pD3DDevice, m_strFileBiped);
	_objScene.LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/car.x",1, 1, dx_pD3DDevice, m_strFile);

	VERTEXPROCESSING_TYPE vp = dx_D3DSettings.GetSettings()->VertexProcessingType;
	bool HardwareTnL = true;
    
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) 
		HardwareTnL = false;

	//_objScene.SetD3DDevice(dx_pD3DDevice,HardwareTnL);
	//_objScene.LoadSceneFromX(m_strFileNmae, 1, 1);

    ReleaseEntities();

    m_Player.Set3rdPersonObject( &m_Object );
	_objScene.Set3rdPersonObject( &m_Object );

    return true;
}

void dxGameMgr::ReleaseEntities( )
{
    if ( dx_pD3DDevice ) 
    {
        dx_pD3DDevice->SetStreamSource( 0, NULL, 0, 0 );
        dx_pD3DDevice->SetIndices( NULL );
        dx_pD3DDevice->SetTexture( 0, NULL );
    }
}

//
//Frame Advance
//
void dxGameMgr::FrameAdvance()
{
	static TCHAR FrameRate[ 50 ];
    static TCHAR TitleBuffer[ 255 ];

    dx_Timer.Tick( );

    if ( !dx_bActive ) 
		return;
    
    if ( dx_LastFrameRate != dx_Timer.GetFrameRate() )
    {
        dx_LastFrameRate = dx_Timer.GetFrameRate( FrameRate );
        _stprintf( TitleBuffer, _T("Engine DX : %s"), FrameRate );
        SetWindowText( dx_hWnd, TitleBuffer );
    }

    if ( dx_bLostDevice )
    {
        HRESULT hRet = dx_pD3DDevice->TestCooperativeLevel();
        if ( hRet == D3DERR_DEVICENOTRESET )
        {
            dxD3DInit Initialize;
            Initialize.ResetDisplay( dx_pD3DDevice, dx_D3DSettings, dx_hWnd );
            SetupRenderStates( );
            dx_bLostDevice = false;
        } 
        else
        {
            return;
        } 
    }

	ProcessInput( "player" );

	//_objScene.AnimateObjects( dx_Timer );
	//m_Player.AnimateMesh( dx_Timer );
	
	if(loadedRect == true)
	{
		drawRect();
		d3dspt->Begin(D3DXSPRITE_ALPHABLEND);	
		drawRect();
		d3dspt->End();
	}
	else
	{
		dx_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x3113E1, 1.0f, 0 );
		dx_pD3DDevice->BeginScene();// Begin Scene Rendering

		static DWORD lastTime=timeGetTime();
		float timeElapsed=0.001f*(timeGetTime()-lastTime);
		lastTime=timeGetTime();

		if (modelWorld)
		{
			D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
			D3DXMatrixRotationX( &matRotX, w_entityPitch );//x
			D3DXMatrixRotationY( &matRotY, w_entityYaw );//y  
			D3DXMatrixRotationZ( &matRotZ, w_entityRoll );//z 
			D3DXMatrixTranslation(&matTrans,modelWorldPosition.x,modelWorldPosition.y,modelWorldPosition.z);
			D3DXMATRIX matWorld=(matRotX*matRotY*matRotZ)*matTrans;
		
			modelWorld->FrameMove(timeElapsed,&matWorld);
			modelWorld->Render();
		}

		if (modelEntity1)
		{
			D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
			D3DXMatrixRotationX( &matRotX, m_entityPitch1 );//x
			D3DXMatrixRotationY( &matRotY, m_entityYaw1 );//y  
			D3DXMatrixRotationZ( &matRotZ, m_entityRoll1 );//z 
			D3DXMatrixTranslation(&matTrans,modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z);
			D3DXMATRIX matWorld=(matRotX*matRotY*matRotZ)*matTrans;
		
			modelEntity1->FrameMove(timeElapsed,&matWorld);
			modelEntity1->Render();
		}

		m_Player.RenderMesh( dx_pD3DDevice, timeElapsed );
		_objScene.RenderMesh( dx_pD3DDevice, timeElapsed );

		// End Scene Rendering
		dx_pD3DDevice->EndScene();
    
		if ( FAILED(dx_pD3DDevice->Present( NULL, NULL, NULL, NULL )) ) 
			dx_bLostDevice = true;
	}
}

void dxGameMgr::drawRect()
{
	RECT rt1;
	SetRect(&rt1, 1, 1, 505, 13);
	DrawTexture(DisplayTexture, rt1, 250, 656, 255);
    return;
}

void dxGameMgr::DrawTexture(LPDIRECT3DTEXTURE9 texture, RECT texcoords, float x, float y, int a)
{
    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f), position(x, y, 0.0f);
    d3dspt->Draw(texture, &texcoords, &center, &position, D3DCOLOR_ARGB(a,255, 255, 255));

    return;
}

void dxGameMgr::LoadTexture(LPDIRECT3DTEXTURE9* texture, LPCTSTR filename)
{
    D3DXCreateTextureFromFileEx(dx_pD3DDevice, filename, D3DX_DEFAULT, D3DX_DEFAULT,
        D3DX_DEFAULT, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, 
        D3DX_DEFAULT, D3DCOLOR_XRGB(255, 0, 255), NULL, NULL, texture);

    return;
}

void dxGameMgr::ProcessInput( char* chObject)
{
	static UCHAR pKeyBuffer[ 256 ];
    ULONG        Direction = 0;
    POINT        CursorPos;
    float        X = 0.0f, Y = 0.0f;

	input->getInput();

	static DWORD lastTime=timeGetTime();
	float timeElapsed=0.001f*(timeGetTime()-lastTime);
	lastTime=timeGetTime();

    if ( !GetKeyboardState( pKeyBuffer ) ) 
		return;

    if ( pKeyBuffer[ VK_UP    ] & 0xF0 || input->keyDown(DIK_W)) Direction |= CPlayer::DIR_FORWARD;
    if ( pKeyBuffer[ VK_DOWN  ] & 0xF0 || input->keyDown(DIK_S)) Direction |= CPlayer::DIR_BACKWARD;
    if ( pKeyBuffer[ VK_LEFT  ] & 0xF0 || input->keyDown(DIK_A)) Direction |= CPlayer::DIR_LEFT;
    if ( pKeyBuffer[ VK_RIGHT ] & 0xF0 || input->keyDown(DIK_D)) Direction |= CPlayer::DIR_RIGHT;
    if ( pKeyBuffer[ VK_PRIOR ] & 0xF0 ) Direction |= CPlayer::DIR_UP;
    if ( pKeyBuffer[ VK_NEXT  ] & 0xF0 ) Direction |= CPlayer::DIR_DOWN;

	if (input->keyDown(DIK_SPACE))
		m_Player.SetAnimation(2);
		//modelEntity1->SetAnimationSet(2);
	if (input->keyDown(DIK_Z))
		m_Player.SetAnimation(1);
		//modelEntity1->SetAnimationSet(1);
    
    if ( GetCapture() == dx_hWnd )
    {
        SetCursor( NULL );
        GetCursorPos( &CursorPos );
        X = (float)(CursorPos.x - dx_OldCursorPos.x) / 3.0f;
        Y = (float)(CursorPos.y - dx_OldCursorPos.y) / 3.0f;
        SetCursorPos( dx_OldCursorPos.x, dx_OldCursorPos.y );
    }

    if ( Direction > 0 || X != 0.0f || Y != 0.0f )
    {
        if ( X || Y ) 
        {
            if ( pKeyBuffer[ VK_RBUTTON ] & 0xF0 )
				if(chObject == "player")
					m_Player.Rotate( Y, 0.0f, -X );
				else
					_objScene.Rotate( Y, 0.0f, -X );
            else
				if(chObject == "player")
					m_Player.Rotate( Y, X, 0.0f );
				else
					_objScene.Rotate( Y, X, 0.0f );
        } 

        if ( Direction ) 
        {
			if(chObject == "player")
				m_Player.Move( Direction, 700.0f * timeElapsed, true );
			else
				_objScene.Move( Direction, 700.0f * timeElapsed, true );
			
			m_Light[1].Type       = D3DLIGHT_SPOT;
			m_Light[1].Diffuse.r  = 1.0f;
			m_Light[1].Diffuse.g  = 1.0f;
			m_Light[1].Diffuse.b  = 1.0f;
			m_Light[1].Diffuse.a  = 1.0f;
			m_Light[1].Position  = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z);
			m_Light[1].Direction = m_Player.GetPosition();
			m_Light[1].Range=300.0f;
			m_Light[1].Theta=1.0f;
			m_Light[1].Phi=2.0f;
			m_Light[1].Falloff=1.0f;
			m_Light[1].Attenuation0= 0.1f;
			dx_pD3DDevice->SetLight( 1, &m_Light[1] );
        }
    }

	/*if(chObject == "player")
		
	else*/
	m_Player.Update(timeElapsed );
	_objScene.Update(timeElapsed );
    dx_pCamera->UpdateRenderView( dx_pD3DDevice );
}

void dxGameMgr::AnimateObjects()
{
	if (input->keyDown(DIK_B))
	{
		m_Light[1].Type         = D3DLIGHT_SPOT;
		m_Light[1].Position     = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z); //m_Player.GetPosition();
		m_Light[1].Direction	= m_Player.GetPosition();
		m_Light[1].Range        = 1.0f;
		m_Light[1].Attenuation0 = 1.0f;
		m_Light[1].Diffuse.a    = 1.0f;
		m_Light[1].Diffuse.r    = 0.0f; 
		m_Light[1].Diffuse.g    = 1.0f;
		m_Light[1].Diffuse.b    = 0.0f;
		m_Light[1].Phi			= 50.0f;//D3DXToRadian(40.0f);    // set the outer cone to 30 degrees
		m_Light[1].Theta		= 10.5f;//D3DXToRadian(20.0f);    // set the inner cone to 10 degrees
		m_Light[1].Falloff		= 1.0f;    // use the typical falloff
		dx_pD3DDevice->SetLight( 1, &m_Light[1] );
	}

	if (input->keyDown(DIK_V))
	{
		m_Light[1].Position     = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z); //m_Player.GetPosition();
		m_Light[1].Type       = D3DLIGHT_SPOT;
		m_Light[1].Diffuse.r  = 0.0f;
		m_Light[1].Diffuse.g  = 0.0f;
		m_Light[1].Diffuse.b  = 1.0f;
		m_Light[1].Diffuse.a  = 1.0f;
		m_Light[1].Position  = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z);
		m_Light[1].Direction = m_Player.GetPosition();
		m_Light[1].Range=100.0f;
		m_Light[1].Theta=1.0f;
		m_Light[1].Phi=2.0f;
		m_Light[1].Falloff=1.0f;
		m_Light[1].Attenuation0= 1.0f;
		dx_pD3DDevice->SetLight( 1, &m_Light[1] );
	}

	if (input->keyDown(DIK_F))
	{
		m_Light[1].Type       = D3DLIGHT_SPOT;
		m_Light[1].Diffuse.r  = 1.0f;
		m_Light[1].Diffuse.g  = 1.0f;
		m_Light[1].Diffuse.b  = 1.0f;
		m_Light[1].Diffuse.a  = 1.0f;
		m_Light[1].Position  = D3DXVECTOR3( modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z);
		m_Light[1].Direction = m_Player.GetPosition();
		m_Light[1].Range=100.0f;
		m_Light[1].Theta=1.0f;
		m_Light[1].Phi=2.0f;
		m_Light[1].Falloff=1.0f;
		m_Light[1].Attenuation0= 1.0f;
		dx_pD3DDevice->SetLight( 1, &m_Light[1] );
	}
}

void dxGameMgr::BuildAnimations()
{

}

void dxGameMgr::PositionModel(float x, float y, float z) const
{
	D3DCOLOR fontColor = D3DCOLOR_XRGB(0,255,255);    
	RECT rct;

    rct.left	= FontSize;
	rct.right	= dx_nViewWidth;
	rct.top		= FontSize;
	rct.bottom	= dx_nViewY + FontSize;
	
	rct.top += 10;
	rct.bottom = dx_nViewY + 50;
		std::string positionString="Model X:" + ToString(x) + "   Y:" + ToString(y) + "  Z:" + ToString(z);
	m_font->DrawText(NULL,positionString.c_str(), -1, &rct, 0, fontColor );
	
}

bool dxGameMgr::LoadXFile(const std::string &filename,int startAnimation, int model)
{
	switch(model)
	{
		// scene model
		case 1: 
			{
				if (modelWorld)
				{
					delete modelWorld;
					modelWorld=0;
				}

				modelWorld = new dxMeshAnimation(dx_pD3DDevice);
				if (!modelWorld->Load(filename))
				{
					delete modelWorld;
					modelWorld=0;
					return false;
				}

				modelWorld->SetAnimationSet(startAnimation);
			}
		break;
		
		case 2: 
			{
				//model entity 1
				if (modelEntity1)
				{
					delete modelEntity1;
					modelEntity1=0;
				}

				modelEntity1=new dxMeshAnimation(dx_pD3DDevice);
				if (!modelEntity1->Load(filename))
				{
					delete modelEntity1;
					modelEntity1=0;
					return false;
				}

				modelEntity1->SetAnimationSet(startAnimation);
			}
		break;
		
		case 3: 
		{
			//model enitity 2
			if (modelEntity2)
			{
				delete modelEntity2;
				modelEntity2=0;
			}

			modelEntity2=new dxMeshAnimation(dx_pD3DDevice);
			if (!modelEntity2->Load(filename))
			{
				delete modelEntity2;
				modelEntity2=0;
				return false;
			}

			modelEntity2->SetAnimationSet(startAnimation);
		}
		break;
		
		default:
			break;
	}
	
	return true;
}

bool dxD3DInit::ValidateDisplayMode( const D3DDISPLAYMODE &Mode )
{
    if ( Mode.Width < 640 || Mode.Height < 480 || Mode.RefreshRate < 60 ) 
		return false;
    return true;
}

bool dxD3DInit::ValidateDevice( const D3DDEVTYPE &Type, const D3DCAPS9 &Caps )
{
    if ( !(Caps.RasterCaps & D3DPRASTERCAPS_DITHER       ) ) return false;
    if ( !(Caps.ShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) ) return false;
    if ( !(Caps.PrimitiveMiscCaps & D3DPMISCCAPS_CULLCCW ) ) return false;
    if ( !(Caps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL         ) ) return false;

    return true;
}

bool dxD3DInit::ValidateVertexProcessingType( const VERTEXPROCESSING_TYPE &Type )
{
    if ( Type == MIXED_VP ) return false;

    return true;
}