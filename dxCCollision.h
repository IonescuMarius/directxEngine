/****************************************************************************
*                                                                           *
*  dxCCollision.h -- Our collision detection / scene library. Provides us   * 
*  with collision detection and response routines for use throughout our    *
*  application.                                                             *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#ifndef _CCOLLISION_H_
#define _CCOLLISION_H_

//-----------------------------------------------------------------------------
// CCollision Specific Includes
//-----------------------------------------------------------------------------
#include <d3dx9.h>
#include <vector>


class CActor;
class CTerrain;

class CCollision
{
public:

	//-------------------------------------------------------------------------
	// Public, class specific, Typedefs, structures & enumerators
	//-------------------------------------------------------------------------
	struct CollTriangle
	{
		ULONG       Indices[3];         // Indices referencing the triangle vertices from our vertex buffer
		D3DXVECTOR3 Normal;             // The cached triangle normal.
		USHORT      SurfaceMaterial;    // The surface material index assigned to this triangle.

	};

}

#endif // _CCOLLISION_H_