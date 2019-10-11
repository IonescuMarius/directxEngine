/****************************************************************************
*                                                                           *
* dxModel.h -- Model drawing                                                *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <d3d9.h>
#include <d3dx9tex.h>
#include <string>

class dxMeshAnimation;

class dxModel
{
	public:
		dxModel(void);
		virtual ~dxModel(void);

		bool loadModel(LPDIRECT3DDEVICE9 device, std::string filename);
		void render(LPDIRECT3DDEVICE9 device);
		void setPosition(D3DXVECTOR3 positionVector);
		void changePosition(D3DXVECTOR3 positionVector);
		void setRotation(D3DXVECTOR3 rotationVector);
		void setScale(D3DXVECTOR3 scaleVector);
		D3DXVECTOR3 getPosition();
		bool LoadXFile(const std::string &filename, int startAnimation , int model =0, LPDIRECT3DDEVICE9 pDevice = NULL);
		void RenderMesh( LPDIRECT3DDEVICE9 pDevice, float timeElapsed );

	private:
		LPD3DXMESH mesh;
		DWORD materialCount;

		LPDIRECT3DTEXTURE9* textures;
		D3DMATERIAL9* materials;
		LPD3DXBUFFER materialBuffer;

		D3DXMATRIX transMatrix;
		D3DXMATRIX rotationMatrix;
		D3DXMATRIX scaleMatrix;

		D3DXVECTOR3 scale;
		D3DXVECTOR3 position;
		D3DXVECTOR3 rotation;

		dxMeshAnimation *modelMesh;
		float m_entityYaw,m_entityPitch,m_entityRoll;
		D3DXVECTOR3 modelEntityPos;

};
