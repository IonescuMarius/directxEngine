#include "..\Includes\dxSprite.h"

dxSprite::dxSprite(HINSTANCE hAppInst, int imageID, int maskID, const dxCircle &bc, const dxVec2D &p0, const dxVec2D &v0)
{
	mhAppInst = hAppInst;

	mhImage = LoadBitmap(hAppInst, MAKEINTRESOURCE(imageID));
	mhMask	= LoadBitmap(hAppInst, MAKEINTRESOURCE(maskID));

	GetObject(mhImage,	sizeof(BITMAP), &mImageBM);
	GetObject(mhMask,	sizeof(BITMAP), &mMaskBM);

	assert(mImageBM.bmWidth  == mMaskBM.bmWidth);
	assert(mImageBM.bmHeight == mMaskBM.bmHeight);

	mBoundingCircle	= bc;
	mPosition		= p0;
	mVelocity		= v0;
}

dxSprite::~dxSprite()
{
	DeleteObject(mhImage);
	DeleteObject(mhMask);
}

int dxSprite::GetWidth()
{
	return mImageBM.bmWidth;
}

int dxSprite::GetHeight()
{
	return mImageBM.bmHeight;
}

void dxSprite::Update(float dt)
{
	mPosition += mVelocity * dt;
	mBoundingCircle.c = mPosition;
}

void dxSprite::Draw(HDC hBackBufferDC, HDC hSpriteDC)
{
	int w = GetWidth();
	int h = GetHeight();
	int x = (int)mPosition.x - (w/2);
	int y = (int)mPosition.y - (h/2);
	
	HGDIOBJ oldObj = SelectObject(hSpriteDC, mhMask);
	BitBlt(hBackBufferDC,x,y,w,h,hSpriteDC,0,0,SRCAND);
	SelectObject(hSpriteDC,mhImage);
	BitBlt(hBackBufferDC,x,y,w,h,hSpriteDC,0,0,SRCPAINT);
	SelectObject(hSpriteDC,oldObj);
}

RECT dxSprite::GetSpriteSize()
{
	RECT SpriteRect = { (LONG)mPosition.x - GetWidth()/2,
						(LONG)mPosition.y - GetHeight()/2,
						(LONG)mPosition.x + GetWidth()/2,
						(LONG)mPosition.y + GetHeight()/2 };
	return SpriteRect; 
}
