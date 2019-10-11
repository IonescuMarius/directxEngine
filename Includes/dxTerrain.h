/****************************************************************************
*                                                                           *
* dxTerrain.h -- TERRAIN                                                    *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXTERRAIN_H_
#define _DXTERRAIN_H_

//-----------------------------------------------------------------------------
// CTerrain Specific Includes
//-----------------------------------------------------------------------------
#include "..\Includes\winMain.h"

class CMeshC;
class CPlayer;
class dxCamera;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CTerrain (Class)
// Desc : Game Timer class, queries performance hardware if available, and 
//        calculates all the various values required for frame rate based
//        vector / value scaling.
//-----------------------------------------------------------------------------
class CTerrain
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
	         CTerrain();
	virtual ~CTerrain();

	//-------------------------------------------------------------------------
	// Public Functions For This Class
	//-------------------------------------------------------------------------
    void            SetD3DDevice  ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL );
    bool            LoadHeightMap ( LPCTSTR FileName, ULONG Width, ULONG Height );
    float           GetHeight     ( float x, float z, bool ReverseQuad = false );
    void            Render        ( dxCamera * pCamera );
    void            Release       ( );

    //-------------------------------------------------------------------------
	// Public Static Functions For This Class
	//-------------------------------------------------------------------------
    static void     UpdatePlayer  ( LPVOID pContext, CPlayer * pPlayer, float TimeScale );
    static void     UpdateCamera  ( LPVOID pContext, dxCamera * pCamera, float TimeScale );

private:
	//-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
    D3DXVECTOR3         m_vecScale;         // Amount to scale the terrain meshes
    UCHAR              *m_pHeightMap;       // The physical heightmap data loaded
    ULONG               m_nHeightMapWidth;  // Width of the 2D heightmap data
    ULONG               m_nHeightMapHeight; // Height of the 2D heightmap data
    
    CMeshC            **m_pMesh;            // Simple array of mesh pointers
    ULONG               m_nMeshCount;       // Number of meshes stored here
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // D3D Device to use for creation / rendering.
    bool                m_bHardwareTnL;     // Used hardware vertex processing ?

    ULONG               m_nPrimitiveCount;  // Pre-Calculated. Number of primitives for D3D Render
    
	//-------------------------------------------------------------------------
	// Private Functions For This Class
	//-------------------------------------------------------------------------
    long            AddMesh             ( ULONG Count = 1 );
    bool            BuildMeshes         ( );
    D3DXVECTOR3     GetHeightMapNormal  ( ULONG x, ULONG z );
    
    
};

#endif