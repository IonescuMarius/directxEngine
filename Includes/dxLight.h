/****************************************************************************
*                                                                           *
* dxLight.h -- Light Settings                                               *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <d3d9.h>
#include <d3dx9tex.h>
#include <vector>


class dxLight
{
	public:
		//some review
		dxLight(LPDIRECT3DDEVICE9 device);
		~dxLight();
		void enableLight(int lightNumber);
		void disableLight(int lightNumber);

		int createLight();
		int createLightCustom();
		int createLightCustom2();
		bool initLightCustom(LPDIRECT3DDEVICE9 d3dd3v);

		//set diffuse
		void setDiffuse(int lightNumber, float r, float g, float b);
		
		void setSpecular(int lightNumber, float r, float g, float b);
		void setAmbient(int lightNumber, float r, float g, float b);
		void setPosition(int lightNumber, D3DXVECTOR3 newPosition);
		void setRange(int lightNumber, float newRange);

	private:
		std::vector <D3DLIGHT9 *> lights;
		LPDIRECT3DDEVICE9 d3dDevice;
		D3DLIGHT9 light2;

};