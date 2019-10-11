/****************************************************************************
*                                                                           *
* dxAnimation.h -- Animation of model                                       *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#ifndef _CANIMATION_H_
#define _CANIMATION_H_

#include <d3d9.h>
#include <d3dx9tex.h>
#include "..\Includes\winMain.h"

class dxModel;
class dxInput;

class dxAnimation
{
	public:
		dxAnimation();
		~dxAnimation();

		bool init(LPDIRECT3DDEVICE9 device);
		bool initSpaceship(LPDIRECT3DDEVICE9 device);
		void updateFrames(int numberOfFrames);
		void moveLeft(float index);
		void moveRight(float index);
		void moveUp(float index);
		void moveDown(float index);
		void scaling(D3DXVECTOR3 scal);
		void setPosition(D3DXVECTOR3 pos);
		void rotationModel(D3DXVECTOR3 rot);
		void render(LPDIRECT3DDEVICE9 device);
		D3DXVECTOR3 position;
		D3DXVECTOR3 rotation;
		D3DXVECTOR3 scalingModel;

	private:
		dxModel* model;
		int direction;
		dxInput* inputX;
	
		D3DXMATRIX matRotate;
};

class CAnimation
{
public:
             CAnimation();
    virtual ~CAnimation();
    void            Attach      ( LPD3DXMATRIX pMatrix, LPCTSTR strName = NULL );
    LPCTSTR         GetName     ( ) const { return m_strName; }
    LPD3DXMATRIX    GetMatrix   ( ) const { return m_pMatrix; }

    void            RotationX   ( float fRadAngle, bool bLocalAxis = true );
    void            RotationY   ( float fRadAngle, bool bLocalAxis = true );
    void            RotationZ   ( float fRadAngle, bool bLocalAxis = true );

private:
    LPD3DXMATRIX    m_pMatrix;
    LPTSTR          m_strName;
};

#endif