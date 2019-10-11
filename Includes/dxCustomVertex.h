/****************************************************************************
*                                                                           *
* dxCustomVertex.h -- CUSTOM VERTEX                                         *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "..\Includes\dxWorldTransformation.h"

class dxMgr;
class dxCustomVertex;
class dxTexture;

class dxCustomVertex
{
     public:
		dxCustomVertex(void);
		~dxCustomVertex(void);
		void initVertex(LPDIRECT3DDEVICE9 direct3dDevice);
		void init_Texture(LPDIRECT3DDEVICE9 direct3dDevice);
		void initGraphicsCube(LPDIRECT3DDEVICE9 direct3dDevice);
		void initGraphicsPyramid(LPDIRECT3DDEVICE9 direct3dDevice);
		void initGraphicsStarCraft(LPDIRECT3DDEVICE9 direct3dDevice);
		void initVertexBlending(LPDIRECT3DDEVICE9 direct3dDevice);
		void drawGraph(LPDIRECT3DDEVICE9 direct3dDevice);
		void drawGraphIndices(LPDIRECT3DDEVICE9 direct3dDevice);
		void drawGraphBlending(LPDIRECT3DDEVICE9 direct3dDevice);
		void initLight(LPDIRECT3DDEVICE9 direct3dDevice);
		void initMesh(LPDIRECT3DDEVICE9 direct3dDevice);
		void drawMesh(LPDIRECT3DDEVICE9 direct3dDevice);

     private:
		
		LPDIRECT3DVERTEXBUFFER9 v_buffer;
		LPDIRECT3DINDEXBUFFER9 i_buffer;
		dxWorldTransformation* dxWorldTransf;
		LPDIRECT3DTEXTURE9 texture; 

		LPD3DXMESH meshX;
		LPD3DXMESH meshSpaceship;    // define the mesh pointer
		D3DMATERIAL9* material;    // define the material object
		DWORD numMaterials;    // stores the number of materials in the mesh
};
