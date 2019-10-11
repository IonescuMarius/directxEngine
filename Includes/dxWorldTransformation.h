/****************************************************************************
*                                                                           *
* dxWorldTransformation.h -- Translation, Rotation, Scaling                 *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "..\Common\dxDirectives.h"

class dxWorldTransformation
{
	public:
		dxWorldTransformation();
		~dxWorldTransformation();
		//Translation
		bool createTranslation(LPDIRECT3DDEVICE9 device, float index, DWORD direction);
		//Scaling
		bool createScaling(LPDIRECT3DDEVICE9 device, float index, DWORD direction);
		//Rotation
		bool createRotationX(LPDIRECT3DDEVICE9 device, float index, bool LR);
		bool createRotationY(LPDIRECT3DDEVICE9 device, float index, bool LR);
		bool createRotationZ(LPDIRECT3DDEVICE9 device, float index, bool LR);
		bool createRotationAxis(LPDIRECT3DDEVICE9 device, float index, bool LR);
		bool createRotationQuaternion(LPDIRECT3DDEVICE9 device);
		bool createRotationYawPitchRoll(LPDIRECT3DDEVICE9 device, float yaw, float pitch, float roll);
		static int pozX;

	private:
		D3DXMATRIX matRotate;
		D3DXVECTOR3 pOutRotation;
		D3DXQUATERNION pQuaternion;
		D3DXVECTOR3 position;
};