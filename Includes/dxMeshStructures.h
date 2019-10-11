/****************************************************************************
*                                                                           *
* dxMeshStructures.h -- Mesh Structures                                     *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once

#define SAFE_RELEASE(p){if(p) {(p)->Release(); (p)=NULL;}}
#define SAFE_DELETE(p) {if(p) {delete(p); (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[](p); (p)=NULL;}}

struct D3DXMESHCONTAINER_EXTENDED: public D3DXMESHCONTAINER
{
    LPDIRECT3DTEXTURE9*  exTextures;
	D3DMATERIAL9*		 exMaterials;
                                
	LPD3DXMESH           exSkinMesh;
	LPD3DXMESH           exMesh;
	D3DXMATRIX*			 exBoneOffsets;
	D3DXMATRIX**		 exFrameCombinedMatrixPointer;
};

struct D3DXFRAME_EXTENDED: public D3DXFRAME
{
    D3DXMATRIX exCombinedTransformationMatrix;
};

