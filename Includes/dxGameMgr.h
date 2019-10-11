/****************************************************************************
*                                                                           *
* dxGameMgr.h -- MANAGER PROCESSING GAME                                    *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009-2011).             *
*                                                                           *
****************************************************************************/
#ifndef DXGAMEMGR_H
#define DXGAMEMGR_H

#include <string>
#include "..\Includes\winMain.h"
#include "..\Includes\dxTimer.h"
#include "..\Includes\dxObject.h"
#include "..\Includes\dxPlayer.h"
#include "..\Includes\dxTerrain.h"
#include "..\Includes\dxScene.h"
#include "..\Includes\dxD3DSettingsDlg.h"

class dxMgr;
class dxCamera; 

class dxText;
class dxPlane; 
class dxLight;
class dxTimer;
class dxInput;
class dxSound;
class dxMenu;
class dxModel;
class dxMeshAnimation;
class CSoundManager;
class dxScene;

class dxGameMgr
{
	public:
		dxGameMgr();
		~dxGameMgr();
		
		bool init(HWND wndHandle,HINSTANCE hInst);
		void update();
		enum { NOTHING = 0, MENU = 1, GAME = 2, OPTIONS = 3 };
		
		void MoveEntity_XYZ_Up(char c, int x);
		void MoveEntity_XYZ_Down(char c, int x);
		void RotationEntity_YawPitchRoll_Up(char c, int x);
		void RotationEntity_YawPitchRoll_Down(char c, int x);
		void NextAnimation( );
		void DetectCollision( );
		void PositionModel(float x, float y, float z) const;
		void AnimateObjects();

		//
		LRESULT     DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam );
		bool        InitInstance( HANDLE hInstance, LPCTSTR lpCmdLine, int iCmdShow );
		int         BeginGame( );
		bool        ShutDown( );

		const D3DTEXTUREOP   &GetColorOp          () const { return dx_ColorOp; }
		bool                 GetSinglePass        () const { return dx_bSinglePass; }
		float                GetNearPlaneDistance () const { return dx_fNearPlane; }
		float                GetFarPlaneDistance  () const { return dx_fFarPlane; }
		HWND                 GetWindow            () const { return dx_hWnd; }
		CPlayer             *GetPlayer             ()       { return &m_Player; }
		//CSoundManager      * GetSoundManager     ()       { return &m_SoundManager; }
		CSoundManager*		directSoundManager;
		void drawRect();
		void DrawTexture(LPDIRECT3DTEXTURE9 texture, RECT texcoords, float x, float y, int a);
		void LoadTexture(LPDIRECT3DTEXTURE9* texture, LPCTSTR filename);
		
	private:
		bool        BuildEntities     ( );
		void        ReleaseEntities   ( );
		void        FrameAdvance      ( );
		bool        CreateDisplay     ( );
		bool        CreateAudio       ( );
		void        ChangeDevice      ( );
		void        SetupGameState    ( );
		void        SetupRenderStates ( );
		void        ProcessInput      ( char* chObject);
		bool        TestDeviceCaps    ( );
		void        SelectMenuItems   ( );
		bool        OpenMeshFile      ( );
		void        BuildAnimations   ( );

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

		bool LoadXFile(const std::string &filename,int startAnimation ,int model =0);

		dxScene                    _objScene;             
		CPlayer					   m_Player;             
		CTerrain                   m_Terrain;            
		//dxTextureFormatEnum      dx_TextureFormats;    
		//dxSoundManager           dx_SoundManager;      

		D3DXMATRIX              dx_mtxIdentity;      
		ULONG                   dx_LastFrameRate;    
		dxTimer                 dx_Timer;            
		dxCamera			   *dx_pCamera;			 

		HWND                    dx_hWnd;             
		HICON                   dx_hIcon;            
		HMENU                   dx_hMenu;            

		bool                    dx_bLostDevice;      
		bool                    dx_bActive;          
		LPDIRECT3D9             dx_pD3D;             
		LPDIRECT3DDEVICE9       dx_pD3DDevice;       
		dxD3DSettings           dx_D3DSettings;      

		D3DFORMAT               dx_TextureFormat;    
		D3DFORMAT               dx_AlphaFormat;      
		
		D3DFILLMODE             dx_FillMode;         
		D3DTEXTUREOP            dx_ColorOp;          
		D3DTEXTUREFILTERTYPE    dx_MagFilter;        
		D3DTEXTUREFILTERTYPE    dx_MinFilter;        
		D3DTEXTUREFILTERTYPE    dx_MipFilter;        
		ULONG                   dx_Anisotropy;       

		bool                    dx_bSinglePass;      
		bool                    dx_MagFilterCaps[10];
		bool                    dx_MinFilterCaps[10];
		bool                    dx_MipFilterCaps[10];
		bool                    dx_ColorOpCaps[30];  
		ULONG                   dx_MaxTextures;      
		ULONG                   dx_MaxAnisotropy;    

		ULONG                   dx_nViewX;           
		ULONG                   dx_nViewY;           
		ULONG                   dx_nViewWidth;       
		ULONG                   dx_nViewHeight;      
	    
		float                   dx_fNearPlane;     
		float                   dx_fFarPlane;      

		POINT                   dx_OldCursorPos;   
		char                   *dx_strLastFile;    

		D3DMATERIAL9            m_BaseMaterial;    
		D3DLIGHT9               m_Light[6];        
		bool                    m_LightEnabled[6]; 
		LPD3DXFONT				m_font;
		static const int		FontSize=14;
 

		float w_entityYaw,w_entityPitch,w_entityRoll;
		float m_entityYaw1,m_entityPitch1,m_entityRoll1;
		float m_entityYaw2,m_entityPitch2,m_entityRoll2;
		D3DXVECTOR3 modelWorldPosition, modelEntityPos1, modelEntityPos2;

		CObjectC                 m_Object;           
		CMeshC                   m_PlayerMesh;       
		

		dxMgr				   *dxManager;
		dxText				   *textManager;
		dxPlane				   *testSurface;
		dxLight				   *lights;
		dxInput				   *input;
		dxSound				   *sounds;
		dxLight				   *objLight;
		dxModel				   *model;
		
		dxMenu				   *menu;
		int						active;
		bool					loadedRect;
		LPD3DXSPRITE d3dspt;
		LPDIRECT3DTEXTURE9 DisplayTexture;

		dxMeshAnimation *modelWorld, *modelEntity1, *modelEntity2;
};

class dxD3DInit : public dxD3DInitialize
{
	private:
		virtual bool        ValidateDisplayMode          ( const D3DDISPLAYMODE& Mode );
		virtual bool        ValidateDevice               ( const D3DDEVTYPE& Type, const D3DCAPS9& Caps );
		virtual bool        ValidateVertexProcessingType ( const VERTEXPROCESSING_TYPE& Type );
};

#endif