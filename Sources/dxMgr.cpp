#include "..\Includes\dxMgr.h"
#include "..\Includes\dxUtility.h"
#include "..\Includes\dxMeshAnimation.h"
#include "..\Includes\dxCamera.h"
#include "..\Common\dxTypes.h"
#include "..\Common\dxDirectives.h"
#include "..\Includes\dxInput.h"
#include "..\Includes\dxLight.h"
#include "..\Includes\dxMenu.h"
#include "..\Includes\dxSound.h"
#include "..\Includes\dxGameMgr.h"

dxMgr::dxMgr(void)
{
	direct3d = 0;
	d3dd3v = 0;
	d3dspt = NULL;
	modelWorld = 0;
	modelEntity1 = 0;
	modelEntity2 = 0;
	modelWorldPosition = D3DXVECTOR3(0, -3, 20);
	modelEntityPos1 = D3DXVECTOR3(-3, -3, 10);
	modelEntityPos2 = D3DXVECTOR3(3, -3, 10);
	w_entityYaw = 0;
	w_entityPitch = 0;
	w_entityRoll = 0;
	m_entityYaw1 = 0;
	m_entityPitch1 = 0;
	m_entityRoll1 = 0;
	m_entityYaw2 = 0;
	m_entityPitch2 = 0;
	m_entityRoll2 = 0;
	dxFont = 0;
	//m_camera = new dxCamera();
	windowed = NULL;
	bOptions = NULL;
}

dxMgr::~dxMgr(void)
{

}

void dxMgr::resetDevice()
{
	modelWorldPosition = D3DXVECTOR3(0, -3, 20);
	modelEntityPos1 = D3DXVECTOR3(-3, -3, 10);
	modelEntityPos2 = D3DXVECTOR3(3, -3, 10);
	w_entityYaw = 0;
	w_entityPitch = 0;
	w_entityRoll = 0;
	m_entityYaw1 = 0;
	m_entityPitch1 = 0;
	m_entityRoll1 = 0;
	m_entityYaw2 = 0;
	m_entityPitch2 = 0;
	m_entityRoll2 = 0;
}

void dxMgr::releaseModel()
{
	modelWorld->ReleaseMesh();
}

bool dxMgr::Initialise(HWND hWnd, HINSTANCE hInst, int width, int height, BOOL fullscreen, BOOL showCursor)
{	
	direct3d=Direct3DCreate9(D3D_SDK_VERSION);
	if (!direct3d)
	{
		CUtility::DebugString("Could not create Direct3D object\n");
		return false;
	}
	
	if (fullscreen == 0)
	{
        windowed = true;
    }

	setupD3DPRESENT(hWnd, width, height);

	HRESULT hr=direct3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, 
		&d3dpp, 
		&d3dd3v);
			
	if (CUtility::FailedHr(hr))
	{
		CUtility::DebugString("Could not create Direct3D device\n");
		return false;
	}
	
	modelRenderState(d3dd3v, showCursor);

	return true;
}

bool dxMgr::setupD3DPRESENT(HWND hWnd, int width, int height)
{
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = windowed; //Windowed or Fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //discards the previous frames
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; //The display format
	d3dpp.BackBufferCount  = 1;	//Number of back buffers
	d3dpp.BackBufferHeight = height; //height of the backbuffer
	d3dpp.BackBufferWidth  = width; //width of the backbuffer
	d3dpp.hDeviceWindow    = hWnd; //handle to our window
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16; //The stencil format
    d3dpp.EnableAutoDepthStencil = TRUE; //z-buffering
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;//D3DPRESENT_INTERVAL_IMMEDIATE

	return true;
}

bool dxMgr::modelRenderState(LPDIRECT3DDEVICE9 d3dd3v, BOOL showCursor)
{
	d3dd3v->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	d3dd3v->SetRenderState( D3DRS_LIGHTING, TRUE );
	d3dd3v->SetRenderState( D3DRS_DITHERENABLE, TRUE );
	d3dd3v->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	d3dd3v->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	d3dd3v->SetRenderState( D3DRS_AMBIENT, 0x99999999 );
	d3dd3v->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	d3dd3v->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	d3dd3v->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	d3dd3v->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	d3dd3v->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	d3dd3v->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	d3dd3v->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    d3dd3v->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 32);
    d3dd3v->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    d3dd3v->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    d3dd3v->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	d3dd3v->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(80,80,80));
	ShowCursor(showCursor);

	D3DXMATRIX matProj;	
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1024.0f/768.0f, 1.0f, 5000.0f );
	d3dd3v->SetTransform( D3DTS_PROJECTION, &matProj );

	objLight = new dxLight(d3dd3v);
	objLight->initLightCustom(d3dd3v);

	D3DXCreateFont( d3dd3v, FontSize, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &dxFont );
		
	//world//darlekNew//cube_00//cube//ball//test00	
	LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/world_02.x",0, 1);
	LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/darlekNew.x",0, 2);
	//LoadXFile(CUtility::GetTheCurrentDirectory()+"/data/darlekNew.x",0, 3);

	return true;
}

BOOL IDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return bShow;
}

LPDIRECT3DDEVICE9 dxMgr::getD3DDevice()
{ 
	return d3dd3v; 
}

dxCamera *dxMgr::GetCamera() const
{
	assert(m_camera);
	return m_camera;
}

void dxMgr::beginRender(void)
{
	D3DXMATRIX matView;
	m_camera->CalculateViewMatrix(&matView);
	d3dd3v->SetTransform( D3DTS_VIEW, &matView );
	
	HRESULT hr=d3dd3v->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(50,50,50), 1.0f, 0);
	
 	if (CUtility::FailedHr(hr))
		return;
	
	if(SUCCEEDED(d3dd3v->BeginScene()))
	{
		if(bOptions)
		{
			modelWorld->ReleaseMesh();
		}
		else
		{
			displayModel();
			DisplayText(0);
		}
	}
}

void dxMgr::endRender(void)
{
	d3dd3v->EndScene();
	d3dd3v->Present( NULL, NULL, NULL, NULL ); 
}

void dxMgr::displayModel(void)
{
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
		
		//modelWorld->ComputeBoundingBox(&matWorld);
		//DisplayPositionModel("World", modelWorldPosition.x, modelWorldPosition.y, modelWorldPosition.z, 30);
		
		modelWorld->FrameMove(timeElapsed,&matWorld);
		modelWorld->Render();
	}
	
	if (modelEntity1)
	{
		D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
		D3DXMatrixRotationX( &matRotX, m_entityPitch1 );//x
		D3DXMatrixRotationY( &matRotY, m_entityYaw1 );//y  
		D3DXMatrixRotationZ( &matRotZ, m_entityRoll1 );//z 
		D3DXMatrixTranslation(&matTrans,modelEntityPos1.x,modelEntityPos1.y,modelEntityPos1.z);
		D3DXMATRIX matWorld=(matRotX*matRotY*matRotZ)*matTrans;
		
		modelEntity1->ComputeBoundingBox(&matWorld);
		DisplayPositionModel("Model 1", modelEntityPos1.x, modelEntityPos1.y, modelEntityPos1.z, 30);
		
		modelEntity1->FrameMove(timeElapsed,&matWorld);
		modelEntity1->Render();
	}
	
	if (modelEntity2)
	{
		D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
		D3DXMatrixRotationX( &matRotX, m_entityPitch2 );//x
		D3DXMatrixRotationY( &matRotY, m_entityYaw2 );//y  
		D3DXMatrixRotationZ( &matRotZ, m_entityRoll2 );//z 
		D3DXMatrixTranslation(&matTrans,modelEntityPos2.x,modelEntityPos2.y,modelEntityPos2.z);
		D3DXMATRIX matWorld=(matRotX*matRotY*matRotZ)*matTrans;
		
		modelEntity2->ComputeBoundingBox(&matWorld);
		DisplayPositionModel("Model 2", modelEntityPos2.x, modelEntityPos2.y, modelEntityPos2.z, 60);
		
		modelEntity2->FrameMove(timeElapsed,&matWorld);
		modelEntity2->Render();
	}

	if(modelEntity1 && modelEntity2)
	{
		if(modelEntity1->maxBounds.x < modelEntity2->minBounds.x)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else if(modelEntity1->minBounds.x > modelEntity2->maxBounds.x)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else if(modelEntity1->maxBounds.y < modelEntity2->minBounds.y)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else if(modelEntity1->minBounds.y > modelEntity2->maxBounds.y)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else if(modelEntity1->maxBounds.z < modelEntity2->minBounds.z)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else if(modelEntity1->minBounds.z > modelEntity2->maxBounds.z)
			DisplayPositionModel("No Collision", NULL, NULL, NULL, 90);
		else
		{
			DisplayPositionModel("Collision", NULL, NULL, NULL, 90);
			//modelEntityPos1.x = modelEntity1->minBounds.x;
			//modelEntityPos1.y = modelEntity1->minBounds.y;
			//modelEntityPos1.z = modelEntity1->minBounds.z;
		}
	}
}

void dxMgr::DisplayPositionModel(const std::string nameModel, float x, float y, float z, LONG pos)
{
	D3DCOLOR fontColor = D3DCOLOR_XRGB(0,255,255);    
	RECT rct;
	rct.left=FontSize;
	rct.right=800;
	rct.top=FontSize;
	rct.bottom=rct.top + FontSize;
	
	rct.top += pos;
	rct.bottom = rct.top + pos;
		std::string positionString=nameModel + ": " + ToString(x) + "   Y:" + ToString(y) + "  Z:" + ToString(z);
	dxFont->DrawText(NULL,positionString.c_str(), -1, &rct, 0, fontColor );
	
}

void dxMgr::DisplayText(float x) const
{
	static DWORD lastTime=timeGetTime();
	static int numFrames=0;
	static float fps=0;

	numFrames++;
	DWORD timePassed=timeGetTime()-lastTime;
	if (timePassed>1000)
	{
		fps=0.001f*timePassed*numFrames;
		numFrames=0;
		lastTime=timeGetTime();
	}
	
	D3DCOLOR fontColor = D3DCOLOR_XRGB(0,255,255);    
	RECT rct;
	rct.left=FontSize;
	rct.right=800;
	rct.top=FontSize;
	rct.bottom=rct.top + FontSize;

	std::string fpsString="FPS: "+ToString(fps);
	dxFont->DrawText(NULL, fpsString.c_str(), -1, &rct, 0, fontColor );
	rct.top += FontSize;rct.bottom=rct.top + FontSize;
	
}

bool dxMgr::LoadXFile(const std::string &filename,int startAnimation, int model)
{
	switch(model)
	{
		case 1: 
			{
				if (modelWorld)
				{
					delete modelWorld;
					modelWorld=0;
				}

				modelWorld=new dxMeshAnimation(d3dd3v);
				if (!modelWorld->Load(filename))
				{
					delete modelWorld;
					modelWorld=0;
					return false;
				}

				modelWorld->SetAnimationSet(startAnimation);
				D3DXVECTOR3 pos = modelWorld->GetInitialCameraPosition();
				//m_camera->SetPosition(pos);
				//m_camera->SetYawPitchRoll(0,0,0);
			}
		break;
		
		case 2: 
			{
				if (modelEntity1)
				{
					delete modelEntity1;
					modelEntity1=0;
				}

				modelEntity1=new dxMeshAnimation(d3dd3v);
				if (!modelEntity1->Load(filename))
				{
					delete modelEntity1;
					modelEntity1=0;
					return false;
				}

				modelEntity1->SetAnimationSet(startAnimation);
				D3DXVECTOR3 pos = modelEntity1->GetInitialCameraPosition();
				//m_camera->SetPosition(pos);
				//m_camera->SetYawPitchRoll(0,0,0);
			}
		break;
		
		case 3: 
		{
			if (modelEntity2)
			{
				delete modelEntity2;
				modelEntity2=0;
			}

			modelEntity2=new dxMeshAnimation(d3dd3v);
			if (!modelEntity2->Load(filename))
			{
				delete modelEntity2;
				modelEntity2=0;
				return false;
			}

			modelEntity2->SetAnimationSet(startAnimation);
			D3DXVECTOR3 pos = modelEntity2->GetInitialCameraPosition();
			//m_camera->SetPosition(pos);
			//m_camera->SetYawPitchRoll(0,0,0);
		}
		break;
		
		default:
			break;
	}
	
	
	
	return true;
}