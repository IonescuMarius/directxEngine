#include "..\Includes\dxInput.h"
#include "..\Includes\dxWorldTransformation.h"

dxWorldTransformation::dxWorldTransformation(void)
{
	
}

dxWorldTransformation::~dxWorldTransformation(void)
{
	
}

bool dxWorldTransformation::createRotationX(LPDIRECT3DDEVICE9 device, float index, bool LR)
{
	D3DXMATRIX matRotateX;
	static float indX = 0.0f; 
	if(LR)
		indX += index; 
	else
		indX -= index; 
	D3DXMatrixRotationX(&matRotateX, indX);
    device->SetTransform(D3DTS_WORLD, &matRotateX);

	return true;
}

bool dxWorldTransformation::createRotationY(LPDIRECT3DDEVICE9 device, float index, bool LR)
{
	D3DXMATRIX matRotateY;
	static float indY = 0.0f; 
	if(LR)
		indY += index; 
	else
		indY -= index; 
	D3DXMatrixRotationY(&matRotateY, indY);
    device->SetTransform(D3DTS_WORLD, &matRotateY);

	return true;
}

bool dxWorldTransformation::createRotationZ(LPDIRECT3DDEVICE9 device, float index, bool LR)
{
	D3DXMATRIX matRotateZ;
	static float indZ = 0.0f; 
	if(LR)
		indZ += index; 
	else
		indZ -= index; 
	D3DXMatrixRotationZ(&matRotateZ, indZ);
    device->SetTransform(D3DTS_WORLD, &matRotateZ);

	return true;
}

bool dxWorldTransformation::createRotationAxis(LPDIRECT3DDEVICE9 device, float index, bool LR)
{
	static float angle = 0.0f; 
	if(LR)
		angle += index;
	else
		angle -= index;
	D3DXMatrixRotationAxis(&matRotate, &pOutRotation, angle);
    device->SetTransform(D3DTS_WORLD, &matRotate);

	return true;
}

bool dxWorldTransformation::createRotationQuaternion(LPDIRECT3DDEVICE9 device)
{
	D3DXMatrixRotationQuaternion(&matRotate, &pQuaternion);
    device->SetTransform(D3DTS_WORLD, &matRotate);

	return true;
}

bool dxWorldTransformation::createRotationYawPitchRoll(LPDIRECT3DDEVICE9 device, float yaw, float pitch, float roll)
{
	D3DXMatrixRotationYawPitchRoll(&matRotate, yaw, pitch, roll);
    device->SetTransform(D3DTS_WORLD, &matRotate);

	return true;
}

bool dxWorldTransformation::createTranslation(LPDIRECT3DDEVICE9 device, float index, DWORD direction)
{
	static float indexX = index;
	static float indexY = 0.0f;
	D3DXMATRIX matTranslate;
	D3DXMATRIX matRotateX;
	switch(direction)
	{
		case DIK_LEFT:
			indexX+=0.03f;
			D3DXMatrixTranslation(&matTranslate, indexX, indexY, 0.0f);
			device->SetTransform(D3DTS_WORLD, &matTranslate);
		break;
		case DIK_RIGHT:
			indexX-=0.03f;
			D3DXMatrixTranslation(&matTranslate, indexX, indexY, 0.0f);
			device->SetTransform(D3DTS_WORLD, &matTranslate);
		break;
		case DIK_UP:
			indexY+=0.03f;
			D3DXMatrixTranslation(&matTranslate, indexX, indexY, 0.0f);
			device->SetTransform(D3DTS_WORLD, &matTranslate);
		break;
		case DIK_DOWN:
			indexY-=0.03f;
			D3DXMatrixTranslation(&matTranslate, indexX, indexY, 0.0f);
			device->SetTransform(D3DTS_WORLD, &matTranslate);
		break;
	}

	return true;
}

bool dxWorldTransformation::createScaling(LPDIRECT3DDEVICE9 device, float index, DWORD direction)
{
	static float scalX = 0.0f;
	static float scalY = 0.0f;
	static float scalZ = 0.0f;
	D3DXMATRIX matScale;
	switch(direction)
	{
		case DIK_Q:
			scalX+=index;
			scalY+=index;
			scalZ+=index;
			D3DXMatrixScaling(&matScale, scalX, scalY, scalZ);
			device->SetTransform(D3DTS_WORLD, &matScale);
		break;
		case DIK_W:
			scalX-=index;
			scalY-=index;
			scalZ-=index;
			D3DXMatrixScaling(&matScale, scalX, scalY, scalZ);
			device->SetTransform(D3DTS_WORLD, &matScale);
		break;
	}
	return true;
}