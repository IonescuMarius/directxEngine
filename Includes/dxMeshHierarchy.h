/****************************************************************************
*                                                                           *
* dxMeshHierarchy.h -- MeshHierarchy                                        *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "..\Includes\dxMeshStructures.h"

class dxMeshAnimation;

class dxMeshHierarchy : public ID3DXAllocateHierarchy
{
public:	
    STDMETHOD( CreateFrame )(LPCSTR Name, LPD3DXFRAME *retNewFrame );

    STDMETHOD( CreateMeshContainer )(LPCSTR Name, CONST D3DXMESHDATA * meshData, 
                            CONST D3DXMATERIAL * materials, CONST D3DXEFFECTINSTANCE * effectInstances,
                            DWORD numMaterials, CONST DWORD * adjacency, LPD3DXSKININFO skinInfo, 
                            LPD3DXMESHCONTAINER * retNewMeshContainer );

    STDMETHOD( DestroyFrame )(LPD3DXFRAME frameToFree );

    STDMETHOD( DestroyMeshContainer )(LPD3DXMESHCONTAINER meshContainerToFree );

	dxMeshAnimation  * m_pActor;
};