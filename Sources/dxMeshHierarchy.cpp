#include "..\Includes\dxMeshHierarchy.h"
#include "..\Includes\dxUtility.h"

HRESULT dxMeshHierarchy::CreateFrame(LPCSTR Name, LPD3DXFRAME *retNewFrame)
{
	*retNewFrame = 0;

    D3DXFRAME_EXTENDED *newFrame = new D3DXFRAME_EXTENDED;
	ZeroMemory(newFrame,sizeof(D3DXFRAME_EXTENDED));

    D3DXMatrixIdentity(&newFrame->TransformationMatrix);
    D3DXMatrixIdentity(&newFrame->exCombinedTransformationMatrix);

	newFrame->pMeshContainer = 0;
	newFrame->pFrameSibling = 0;
	newFrame->pFrameFirstChild = 0;

    *retNewFrame = newFrame;
	
	if (Name && strlen(Name))
	{
		newFrame->Name=CUtility::DuplicateCharString(Name);	
		CUtility::DebugString("Added frame: "+ToString(Name)+"\n");
	}
	else
	{
		CUtility::DebugString("Added frame: no name given\n");
	}
    return S_OK;
}

HRESULT dxMeshHierarchy::CreateMeshContainer(
    LPCSTR Name,
    CONST D3DXMESHDATA *meshData,
    CONST D3DXMATERIAL *materials,
    CONST D3DXEFFECTINSTANCE *effectInstances,
    DWORD numMaterials,
    CONST DWORD *adjacency,
    LPD3DXSKININFO pSkinInfo,
    LPD3DXMESHCONTAINER* retNewMeshContainer)
{    
	D3DXMESHCONTAINER_EXTENDED *newMeshContainer=new D3DXMESHCONTAINER_EXTENDED;
	ZeroMemory(newMeshContainer, sizeof(D3DXMESHCONTAINER_EXTENDED));

	*retNewMeshContainer = 0;

	if (Name && strlen(Name))
	{
		newMeshContainer->Name=CUtility::DuplicateCharString(Name);
		CUtility::DebugString("Added mesh: "+ToString(Name)+"\n");
	}
	else
	{
		CUtility::DebugString("Added Mesh: no name given\n");
	}

	if (meshData->Type!=D3DXMESHTYPE_MESH)
	{
		DestroyMeshContainer(newMeshContainer);
		return E_FAIL;
	}

	newMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
	
	DWORD dwFaces = meshData->pMesh->GetNumFaces();
	newMeshContainer->pAdjacency = new DWORD[dwFaces*3];
	memcpy(newMeshContainer->pAdjacency, adjacency, sizeof(DWORD) * dwFaces*3);
	
	LPDIRECT3DDEVICE9 pd3dDevice = 0;
	meshData->pMesh->GetDevice(&pd3dDevice);

	newMeshContainer->MeshData.pMesh=meshData->pMesh;
	newMeshContainer->MeshData.pMesh->AddRef();

	newMeshContainer->NumMaterials = max(numMaterials,1);
	newMeshContainer->exMaterials = new D3DMATERIAL9[newMeshContainer->NumMaterials];
	newMeshContainer->exTextures  = new LPDIRECT3DTEXTURE9[newMeshContainer->NumMaterials];

	ZeroMemory(newMeshContainer->exTextures, sizeof(LPDIRECT3DTEXTURE9) * newMeshContainer->NumMaterials);

	if (numMaterials>0)
	{
		for(DWORD i = 0; i < numMaterials; ++i)
		{
			newMeshContainer->exTextures[i] = 0;	
			newMeshContainer->exMaterials[i]=materials[i].MatD3D;

			if(materials[i].pTextureFilename)
			{
				std::string texturePath(materials[i].pTextureFilename);
				if (CUtility::FindFile(&texturePath))
				{
					if(FAILED(D3DXCreateTextureFromFile(pd3dDevice, texturePath.c_str(),
						&newMeshContainer->exTextures[i])))
					{
						CUtility::DebugString("Could not load texture: "+texturePath+"\n");					
					}
				}
				else
				{
					CUtility::DebugString("Could not find texture: "+ToString(materials[i].pTextureFilename)+"\n");					
				}
			}
		}
	}
	else    
    {
		ZeroMemory(&newMeshContainer->exMaterials[0], sizeof( D3DMATERIAL9 ) );
        newMeshContainer->exMaterials[0].Diffuse.r = 0.5f;
        newMeshContainer->exMaterials[0].Diffuse.g = 0.5f;
        newMeshContainer->exMaterials[0].Diffuse.b = 0.5f;
        newMeshContainer->exMaterials[0].Specular = newMeshContainer->exMaterials[0].Diffuse;
		newMeshContainer->exTextures[0]=0;
    }

	if (pSkinInfo)
	{
	    newMeshContainer->pSkinInfo = pSkinInfo;
	    pSkinInfo->AddRef();

	    UINT numBones = pSkinInfo->GetNumBones();
	    newMeshContainer->exBoneOffsets = new D3DXMATRIX[numBones];

		newMeshContainer->exFrameCombinedMatrixPointer = new D3DXMATRIX*[numBones];

	    for (UINT i = 0; i < numBones; i++)
	        newMeshContainer->exBoneOffsets[i] = *(newMeshContainer->pSkinInfo->GetBoneOffsetMatrix(i));

		CUtility::DebugString("Mesh has skinning info.\n Number of bones is: "+ToString(numBones)+"\n");
	}
	else	
	{
		newMeshContainer->pSkinInfo = 0;
		newMeshContainer->exBoneOffsets = 0;
		newMeshContainer->exSkinMesh = 0;
		newMeshContainer->exFrameCombinedMatrixPointer = 0;
	}

	pd3dDevice->Release();

	if (effectInstances)
	{
		if (effectInstances->pEffectFilename)
			CUtility::DebugString("This .x file references an effect file. Effect files are not handled by this demo\n");
	}
	
	*retNewMeshContainer = newMeshContainer;    

	return S_OK;
}

HRESULT dxMeshHierarchy::DestroyFrame(LPD3DXFRAME frameToFree) 
{
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameToFree;

	if (frame->Name)
		delete []frame->Name;
	delete frame;

    return S_OK; 
}

HRESULT dxMeshHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER meshContainerBase)
{
    D3DXMESHCONTAINER_EXTENDED* meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	if (!meshContainer)
		return S_OK;

	if (meshContainer->Name)
	{
		delete []meshContainer->Name;
		meshContainer->Name=0;
	}

	if (meshContainer->exMaterials)
	{
		delete []meshContainer->exMaterials;
		meshContainer->exMaterials=0;
	}

	if(meshContainer->exTextures)
	{
		for(UINT i = 0; i < meshContainer->NumMaterials; ++i)
		{
			if (meshContainer->exTextures[i])
				meshContainer->exTextures[i]->Release();
		}
	}

	if (meshContainer->exTextures)
		delete []meshContainer->exTextures;

	if (meshContainer->pAdjacency)
		delete []meshContainer->pAdjacency;
	
	if (meshContainer->exBoneOffsets)
	{
		delete []meshContainer->exBoneOffsets;
		meshContainer->exBoneOffsets=0;
	}
	
	if (meshContainer->exFrameCombinedMatrixPointer)
	{
		delete []meshContainer->exFrameCombinedMatrixPointer;
		meshContainer->exFrameCombinedMatrixPointer=0;
	}
	
	if (meshContainer->exSkinMesh)
	{
		meshContainer->exSkinMesh->Release();
		meshContainer->exSkinMesh=0;
	}
	
	if (meshContainer->MeshData.pMesh)
	{
		meshContainer->MeshData.pMesh->Release();
		meshContainer->MeshData.pMesh=0;
	}
		
	if (meshContainer->pSkinInfo)
	{
		meshContainer->pSkinInfo->Release();
		meshContainer->pSkinInfo=0;
	}
	
	delete meshContainer;
	meshContainer=0;

    return S_OK;
}
