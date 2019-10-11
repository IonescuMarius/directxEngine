/****************************************************************************
*                                                                           *
* dxSprite.h -- SPRITE                                                      *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef SPRITE_H
#define SPRITE_H

#include <windows.h>
#include <cassert>
#include "dxCircle.h"
#include "dxVec2D.h" 

class dxSprite
{
	public:
		dxSprite(HINSTANCE hAppInst, int imageID, int maskID, const dxCircle& bc, const dxVec2D& p0, const dxVec2D& v0);

		~dxSprite();

		int GetWidth();
		int GetHeight();
		
		RECT GetSpriteSize();

		void Update(float dt);
		void Draw(HDC hBackBufferDC, HDC hSpriteDC);

	public:
		dxCircle	mBoundingCircle;	// area of the sprite
		dxVec2D	mPosition;
		dxVec2D	mVelocity;

	private:
		dxSprite(const dxSprite &rhs);
		dxSprite& operator=(const dxSprite &rhs);

	protected:
		HINSTANCE	mhAppInst;
		HBITMAP		mhImage;
		HBITMAP		mhMask;
		BITMAP		mImageBM;
		BITMAP		mMaskBM;
};

#endif
