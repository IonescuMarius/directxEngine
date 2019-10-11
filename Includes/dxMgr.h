/****************************************************************************
*                                                                           *
* dxMgr.h -- PROCESSING LPDIRECT3D9 AND LPDIRECT3DDEVICE9                   *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/
#ifndef dxMgr_H
#define dxMgr_H

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <stdlib.h>
#include <vector>
using namespace std;

class dxCustomVertex;
class dxMeshAnimation;
class dxCamera;
class dxInput;
class dxLight;
class dxMenu;
class dxAnimation;

class dxMgr
{
	public:
		dxMgr(void);
		~dxMgr(void);
		
		void beginRender(void);
		void endRender(void);
		bool Initialise(HWND hWndX, HINSTANCE hInstX, int width, int height, BOOL fullscreen, BOOL showCursor);
		bool LoadXFile(const std::string &filename,int startAnimation ,int model =0);
		bool setupD3DPRESENT(HWND hWnd, int width, int height);
		bool modelRenderState(LPDIRECT3DDEVICE9 d3dd3v, BOOL showCursor);
		void displayModel();
		void DisplayPositionModel(const std::string nameModel, float x, float y, float z, LONG pos);
		void DisplayText(float x) const;
		LPDIRECT3DDEVICE9 getD3DDevice();
		void resetDevice();
		void releaseModel();
		
		float w_entityYaw,w_entityPitch,w_entityRoll;
		float m_entityYaw1,m_entityPitch1,m_entityRoll1;
		float m_entityYaw2,m_entityPitch2,m_entityRoll2;
		static const int FontSize=14;
		dxCamera *GetCamera() const;
		bool windowed;
		bool bOptions;
		
		//WORLD TRANSFORMATION
		void MoveEntity_XYZ_Up(char c, int x);
		void MoveEntity_XYZ_Down(char c, int x);
		void YawWorldEntity(float value){w_entityYaw+=value;}
		void PitchWorldEntity(float value){w_entityPitch+=value;}
		void RollWorldEntity(float value){w_entityRoll+=value;}
		void YawEntity1(float value){m_entityYaw1+=value;}
		void PitchEntity1(float value){m_entityPitch1+=value;}
		void RollEntity1(float value){m_entityRoll1+=value;}
		void YawEntity2(float value){m_entityYaw2+=value;}
		void PitchEntity2(float value){m_entityPitch2+=value;}
		void RollEntity2(float value){m_entityRoll2+=value;}
		
		void RotationEntity_YawPitchRoll_Up(char c, int x);
		void RotationEntity_YawPitchRoll_Down(char c, int x);
		
		void NextAnimation();
		void DetectCollision();

	private:
		LPDIRECT3D9 direct3d;
		LPDIRECT3DDEVICE9 d3dd3v;
		D3DPRESENT_PARAMETERS d3dpp;
		LPD3DXFONT dxFont;
		D3DXVECTOR3 modelWorldPosition, modelEntityPos1, modelEntityPos2;
		LPD3DXSPRITE d3dspt;

		dxMeshAnimation *modelWorld, * modelEntity1, * modelEntity2;	
		dxCamera *m_camera;
		dxLight  *objLight;
};		

#endif