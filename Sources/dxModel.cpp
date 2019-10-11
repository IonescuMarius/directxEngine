#include "..\Includes\dxModel.h"
#include "..\Includes\dxMeshAnimation.h"

dxModel::dxModel(void)
{
	mesh = NULL;
}

dxModel::~dxModel(void)
{
	if (mesh)
	{
		mesh->Release();
	}

	if (materialBuffer)
		materialBuffer->Release();

	if (materials)
	{
		delete[] materials;
	}

	if (textures){
		delete[] textures;
	}
}

bool dxModel::loadModel(LPDIRECT3DDEVICE9 device, std::string filename)
{
	HRESULT hr;

	scale = D3DXVECTOR3(1.0f, 1.0f , 1.0f);
	position = D3DXVECTOR3(0.0f, 0.0f , 0.0f);
	rotation = D3DXVECTOR3(1.0f, 1.0f , 1.0f); 
	
	hr = D3DXLoadMeshFromX(filename.c_str(), 
					  D3DXMESH_SYSTEMMEM,
					  device,
					  NULL,
					  &materialBuffer,
					  NULL,
					  &materialCount,
					  &mesh);

	if FAILED(hr){
		return false;
	}

	D3DXMATERIAL* modelMaterials = (D3DXMATERIAL*)materialBuffer->GetBufferPointer();
    
    materials = new D3DMATERIAL9[materialCount];
	textures  = new LPDIRECT3DTEXTURE9[materialCount];
    
    for(DWORD i = 0; i < materialCount; i++)
    {
        materials[i] = modelMaterials[i].MatD3D;
		materials[i].Ambient = materials[i].Diffuse;
		hr = D3DXCreateTextureFromFile( device, modelMaterials[i].pTextureFilename, &textures[i] );
		if FAILED(hr)
		{
			textures[i] = NULL;
		}
    }

	if (materialBuffer)
	{
		materialBuffer->Release();
	}

	return true;
}

void dxModel::render(LPDIRECT3DDEVICE9 device)
{
	DWORD i;

	D3DXMatrixScaling(&scaleMatrix, scale.x,scale.y,scale.z);
	D3DXMatrixTranslation(&transMatrix, position.x, position.y, position.z);
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix,rotation.x,rotation.y,rotation.z);
	D3DXMatrixMultiply(&transMatrix, &rotationMatrix, &transMatrix);
	D3DXMatrixMultiply(&transMatrix, &scaleMatrix, &transMatrix);
	device->SetTransform(D3DTS_WORLD, &transMatrix);

	for(i = 0; i < materialCount; i++ )
    {
		device->SetMaterial( &materials[i] );
		if (textures[i]!= NULL)
		{
	        device->SetTexture( 0, textures[i] );
		}
		mesh->DrawSubset( i );
	}
}

void dxModel::setPosition(D3DXVECTOR3 positionVector)
{
	position = positionVector;
}

void dxModel::changePosition(D3DXVECTOR3 changeVector)
{
	position = position + changeVector;
}

void dxModel::setRotation(D3DXVECTOR3 rotationVector)
{
	rotation = rotationVector;
}

void dxModel::setScale(D3DXVECTOR3 scaleVector)
{
	scale = scaleVector;
}

D3DXVECTOR3 dxModel::getPosition()
{
	return position;
}

bool dxModel::LoadXFile(const std::string &filename,int startAnimation, int model, LPDIRECT3DDEVICE9 pDevice)
{
	switch(model)
	{
		case 1: 
			{
				modelMesh = new dxMeshAnimation(pDevice);
				if (!modelMesh->Load(filename))
				{
					delete modelMesh;
					modelMesh=0;
					return false;
				}

				modelMesh->SetAnimationSet(startAnimation);
			}
		break;
		
		default:
			break;
	}

	modelEntityPos = D3DXVECTOR3(0, -1, -10);
	
	return true;
}

void dxModel::RenderMesh( LPDIRECT3DDEVICE9 pDevice,float timeElapsed )
{
	if (modelMesh)
	{
		D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
		D3DXMatrixRotationX( &matRotX, m_entityPitch );//x
		D3DXMatrixRotationY( &matRotY, m_entityYaw );//y  
		D3DXMatrixRotationZ( &matRotZ, m_entityRoll );//z 
		D3DXMatrixTranslation(&matTrans,modelEntityPos.x,modelEntityPos.y,modelEntityPos.z);
		D3DXMATRIX matWorld = (matRotX*matRotY*matRotZ)*matTrans;
		
		modelMesh->FrameMove(timeElapsed,&matWorld);
		modelMesh->Render();
	}

}