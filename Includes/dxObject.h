/****************************************************************************
*                                                                           *
* dxObject.h -- OBJECT                                                      *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXOBJECT_H_
#define _DXOBJECT_H_

#include "..\Common\dxDirectives.h"
#include "..\Includes\winMain.h"
#include <D3DX9.h>

#define VERTEX_FVF      D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
#define TLITVERTEX_FVF  D3DFVF_XYZRHW | D3DFVF_DIFFUSE

class dxMeshAnimation;
class CActionStatus;

typedef LPDIRECT3DTEXTURE9 (*COLLECTTEXTURE     )( LPVOID pContext, LPCSTR FileName );
typedef LPD3DXEFFECT       (*COLLECTEFFECT      )( LPVOID pContext, const D3DXEFFECTINSTANCE & EffectInstance );
typedef ULONG              (*COLLECTATTRIBUTEID )( LPVOID pContext, LPCSTR strTextureFile, const D3DMATERIAL9 * pMaterial, const D3DXEFFECTINSTANCE * pEffectInstance );

typedef struct _MESH_ATTRIB_DATA    // Stores managed data for an individual mesh attribute
{
    D3DMATERIAL9        Material;   // Material to use for this attribute
    LPDIRECT3DTEXTURE9  Texture;    // Texture to use for this attrbute
    LPD3DXEFFECT        Effect;     // Effect to use for this attrbute

} MESH_ATTRIB_DATA;

//data element
typedef struct Data
{
	int index;
	int x;
	int y;
	int z;
}Dt;

//double linked list
typedef struct DoubleLinkedList
{
    Dt valoare;
    DoubleLinkedList *prev, *next;
}DblLinkedList;

//DoubleLinkedList* cap = NULL;

class CVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CVertex( float fX, float fY, float fZ, const D3DXVECTOR3 & vecNormal, float ftu = 0.0f, float ftv = 0.0f ) 
        { x = fX; y = fY; z = fZ; Normal = vecNormal; tu = ftu; tv = ftv; }
    
    CVertex( D3DXVECTOR3 & vecPos, const D3DXVECTOR3 & vecNormal, float ftu = 0.0f, float ftv = 0.0f ) 
        { x = vecPos.x; y = vecPos.y; z = vecPos.z; Normal = vecNormal; tu = ftu; tv = ftv; }
    
    CVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Normal = D3DXVECTOR3( 0, 0, 0 ); tu = 0.0f; tv = 0.0f; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    D3DXVECTOR3 Normal;     // Vertex normal.
    float       tu;         // Texture u coordinate
    float       tv;         // Texture v coordinate

};

//-----------------------------------------------------------------------------
// Name : CLitVertex (Class)
// Desc : Vertex class used to construct & store vertex components.
//-----------------------------------------------------------------------------
class CLitVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CLitVertex( float fX, float fY, float fZ, ULONG ulDiffuse = 0xFF000000, float ftu = 0.0f, float ftv = 0.0f ) 
        { x = fX; y = fY; z = fZ; Diffuse = ulDiffuse; tu = ftu; tv = ftv; }
    
    CLitVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Diffuse = 0xFF000000; tu = 0.0f; tv = 0.0f; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    ULONG       Diffuse;    // Diffuse vertex colour component
    float       tu;         // Texture u coordinate
    float       tv;         // Texture v coordinate
};

//-----------------------------------------------------------------------------
// Name : CTLitVertex (Class)
// Desc : Vertex class used to construct & store transformed vertex components.
//-----------------------------------------------------------------------------
class CTLitVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CTLitVertex( float fX, float fY, float fZ, float fW, ULONG ulDiffuse = 0xFF000000 ) 
        { x = fX; y = fY; z = fZ; w = fW; Diffuse = ulDiffuse; }
    
    CTLitVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; Diffuse = 0xFF000000; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    float       w;          // Vertex Position W Component
    ULONG       Diffuse;    // Diffuse vertex colour component
};

class CPolygon
{
	public:
		CPolygon( USHORT VertexCount );
		CPolygon( USHORT VertexCount, USHORT IndexCount = 0 );
		CPolygon();
		virtual ~CPolygon();

		long        AddVertex( USHORT Count = 1 );
		long        AddIndex( USHORT Count = 1 );

		short       m_nMaterial;            // Material index to use for this poly (-1 for none)
		USHORT      m_nVertexCount;         // Number of vertices stored.
		USHORT      m_nIndexCount;          // Number of indices stored
		USHORT      m_nVertexThreshold;     // Total allocated vertices
		USHORT      m_nIndexThreshold;      // Total allocated vertices
		USHORT     *m_pIndex;               // Simple index array
		CVertex    *m_pVertex;              // Simple vertex array
};

class CMesh
{
	public:
		CMesh();
		CMesh( ULONG Count );
		CMesh( ULONG VertexCount, ULONG IndexCount );
		virtual ~CMesh();

		void        SetVertexFormat ( ULONG FVFCode, UCHAR Stride );
		long        AddPolygon( ULONG Count = 1 );
		long        AddVertex    ( ULONG Count = 1 );
		long        AddIndex     ( ULONG Count = 1 );
		HRESULT     BuildBuffers ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = true );
		void        Release         ( );

		ULONG       m_nPolygonCount;        // Number of polygons stored
		CPolygon  **m_pPolygon;             // Simply polygon array.
		CMesh      *m_pNext;                // Linked list connectivity (if required)

		ULONG                   m_nVertexCount;     // Number of vertices stored
		UCHAR                  *m_pVertex;          // Simple temporary vertex array.
		ULONG                   m_nIndexCount;      // Number of indices stored
		USHORT                 *m_pIndex;           // Simple temporary index array
		LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer to be Rendered
		LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index Buffer to be Rendered
		UCHAR                   m_nStride;          // The stride of each individual vertex
		ULONG                   m_nFVFCode;         // The flexible vertex format code.

		D3DXVECTOR3             m_BoundsMin;        // Bounding box minimum extents
		D3DXVECTOR3             m_BoundsMax;        // Bounding box maximum extents
};

//-----------------------------------------------------------------------------
// Name : CTriMesh (Class)
// Desc : Basically extends ID3DXMesh, stores additional data internally.
//-----------------------------------------------------------------------------
class CTriMesh
{
public:
    //-------------------------------------------------------------------------
	// Public Enumerators for This Class.
	//-------------------------------------------------------------------------
    enum CALLBACK_TYPE { CALLBACK_TEXTURE = 0, CALLBACK_EFFECT = 1, CALLBACK_ATTRIBUTEID = 2, CALLBACK_COUNT = 3 };
    enum MESH_TYPE     { MESH_DEFAULT = 0, MESH_STANDARD = 1, MESH_PROGRESSIVE = 2 };

    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         CTriMesh();
	virtual ~CTriMesh();

    //-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    bool              RegisterCallback   ( CALLBACK_TYPE Type, LPVOID pFunction, LPVOID pContext );
    
    // Resource management functions
    HRESULT           LoadMeshFromX      ( LPCSTR pFileName, DWORD Options, LPDIRECT3DDEVICE9 pD3D );
    HRESULT           Attach             ( LPD3DXBASEMESH pMesh, LPD3DXBUFFER pAdjacency = NULL, bool bReplaceMeshOnly = false );
    void              Release            ( );
    
    // Rendering functions
    void              Draw               ( );
    void              DrawSubset         ( ULONG AttributeID, long MaterialOverride = -1 );
    
    // Utility functions
    HRESULT           CloneMeshFVF       ( ULONG Options, ULONG FVF, CTriMesh * pMeshOut, MESH_TYPE MeshType = MESH_DEFAULT, LPDIRECT3DDEVICE9 pD3DDevice = NULL );
    HRESULT           GenerateAdjacency  ( float Epsilon = 1e-3f );
    HRESULT           Optimize           ( ULONG Flags, CTriMesh * pMeshOut, MESH_TYPE MeshType = MESH_DEFAULT, LPD3DXBUFFER * ppFaceRemap = NULL, LPD3DXBUFFER * ppVertexRemap = NULL, LPDIRECT3DDEVICE9 pD3DDevice = NULL );
    HRESULT           OptimizeInPlace    ( ULONG Flags, LPD3DXBUFFER * ppFaceRemap = NULL, LPD3DXBUFFER * ppVertexRemap = NULL );
    HRESULT           WeldVertices       ( ULONG Flags, const D3DXWELDEPSILONS * pEpsilon = NULL );

    // PMesh utility functions
    HRESULT           GeneratePMesh      ( const LPD3DXATTRIBUTEWEIGHTS pAttributeWeights, CONST FLOAT *pVertexWeights, ULONG MinValue, ULONG Options, bool ReleaseOriginal = true );
    HRESULT           SnapshotToMesh     ( bool ReleaseProgressive = true );
    
    // Mesh creation functions
    void              SetDataFormat      ( ULONG VertexFVF, ULONG IndexStride );
    long              AddVertex          ( ULONG VertexCount = 1, LPVOID pVertices = NULL );
    long              AddFace            ( ULONG FaceCount = 1, LPVOID pIndices = NULL, ULONG AttribID = 0 );
    long              AddAttributeData   ( ULONG AttributeCount );
    void             *GetVertices        ( ) const;
    void             *GetFaces           ( ) const;
    ULONG            *GetAttributes      ( ) const;
    MESH_ATTRIB_DATA *GetAttributeData   ( ) const;
    HRESULT           BuildMesh          ( ULONG Options, LPDIRECT3DDEVICE9 pDevice, bool ReleaseOriginals = true );
    
    // Level of detail functions
    HRESULT           SetNumFaces        ( ULONG FaceCount );
    HRESULT           SetNumVertices     ( ULONG VertexCount );
    HRESULT           TrimByFaces        ( ULONG NewFacesMin, ULONG NewFacesMax );
    HRESULT           TrimByVertices     ( ULONG NewVerticesMin, ULONG NewVerticesMax );

    // Object access functions
    LPD3DXMESH        GetMesh            ( ) const;
    LPD3DXPMESH       GetPMesh           ( ) const;
    LPD3DXBUFFER      GetAdjacencyBuffer ( ) const;
    LPDIRECT3DDEVICE9 GetDevice          ( ) const;
    
    // Variable gathering functions
    ULONG             GetOptions         ( ) const;
    ULONG             GetNumVertices     ( ) const;
    ULONG             GetVertexStride    ( ) const;
    ULONG             GetFVF             ( ) const;
    ULONG             GetNumFaces        ( ) const;
    ULONG             GetIndexStride     ( ) const;
    ULONG             GetAttributeCount  ( ) const;
    ULONG             GetMaxFaces        ( ) const;
    ULONG             GetMaxVertices     ( ) const;
    ULONG             GetMinFaces        ( ) const;
    ULONG             GetMinVertices     ( ) const;
    LPCTSTR           GetMeshName        ( ) const;
    
    
private:
	//-------------------------------------------------------------------------
	// Private Variables for This Class
	//-------------------------------------------------------------------------
    LPD3DXBUFFER        m_pAdjacency;                   // Stores adjacency information
    CALLBACK_FUNC       m_CallBack[CALLBACK_COUNT];     // References the various callbacks
    LPD3DXMESH          m_pMesh;                        // Physical mesh object
    LPD3DXPMESH         m_pPMesh;                       // Physical PMesh object
    TCHAR               m_strMeshName[MAX_PATH];        // The filename used to load the mesh or later on for referencing

    // Managed Attribute Data
    MESH_ATTRIB_DATA   *m_pAttribData;                  // Individual mesh attribute data.
    ULONG               m_nAttribCount;                 // Number of items in the attribute data array.
    
    // Mesh creation data.
    ULONG               m_nVertexStride;                // Stride of the vertices
    ULONG               m_nVertexFVF;
    ULONG               m_nIndexStride;                 // Stride of the indices
    ULONG               m_nVertexCount;                 // Number of vertices to use during BuildMesh
    ULONG               m_nFaceCount;                   // Number of faces to use during BuildMesh
    ULONG               m_nVertexCapacity;              // We are currently capable of holding this many before a grow
    ULONG               m_nFaceCapacity;                // We are currently capable of holding this many before a grow
    ULONG              *m_pAttribute;                   // Attribute ID's for all faces
    UCHAR              *m_pIndex;                       // The face index data
    UCHAR              *m_pVertex;                      // The physical vertices.

    //-------------------------------------------------------------------------
	// Private Static Variables for This Class
	//-------------------------------------------------------------------------
    static ULONG        m_DefaultVertexStride;          // Default Vertex Stride
    static ULONG        m_DefaultIndexStride;           // Default Index Stride
};

class CObject
{
	public:
		CObject( CMesh * pMesh );
		CObject( CTriMesh * pMesh );
		CObject( dxMeshAnimation * pActor );
		CObject();
		virtual ~CObject( );
		CObject( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer );
		void    SetVertexBuffer ( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer );

		CMesh				   *s_pMesh;            // Mesh we are instancing
		CTriMesh			   *m_pMesh;            // Mesh we are instancing
		D3DXMATRIX              m_mtxWorld;         // Objects world matrix
		LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer we are instancing
		
		double      ActorTime;              // Stored time for the actor
		LPD3DXANIMATIONCONTROLLER   m_pAnimController;      // If we are referencing, we will maintain a cloned controller.
		long                        m_nObjectSetIndex;      // The index returned from the collision system, for a specific added actor.
	    dxMeshAnimation			   *m_pActor;               // Actor we are instancing
		CActionStatus              *m_pActionStatus;        // If we are referencing, maintain the current action state of the actor.	
		float		VectorLength3D(  CVertex* A);
		CVertex     SubtractVector3D( CVertex* A , CVertex* B );
		CVertex     AddVectors3D( CVertex* A , CVertex* B );
		CVertex     VectorMultiply3D (CVertex* A , float scalar);
		CVertex		VectorNormalize3D ( CVertex* A);
		CVertex		VectorCrossProduct (CVertex* A , CVertex* B);
		CVertex     GeneratePolygonNormal( CPolygon* P );
		float		VectorDotProduct3D (CVertex* A, CVertex* B);
		float		FindVectorAngles3D (CVertex* A, CVertex* B);
};

class CVertexC
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CVertexC( float fX, float fY, float fZ, const D3DXVECTOR3& vecNormal ) 
        { x = fX; y = fY; z = fZ; Normal = vecNormal; }
    
    CVertexC() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Normal = D3DXVECTOR3( 0, 0, 0 ); }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    D3DXVECTOR3 Normal;     // Vertex Normal
    
};

//-----------------------------------------------------------------------------
// Name : CMesh (Class)
// Desc : Basic mesh class used to store individual mesh data.
//-----------------------------------------------------------------------------
class CMeshC
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             CMeshC( ULONG VertexCount, ULONG IndexCount );
	         CMeshC();
	virtual ~CMeshC();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddVertex    ( ULONG Count = 1 );
    long        AddIndex     ( ULONG Count = 1 );
    HRESULT     BuildBuffers ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = true );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    ULONG                   m_nVertexCount;     // Number of vertices stored
    CVertexC               *m_pVertex;          // Simple temporary vertex array.
    ULONG                   m_nIndexCount;      // Number of indices stored
    USHORT                 *m_pIndex;           // Simple temporary index array
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer to be Rendered
    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index Buffer to be Rendered

    D3DXVECTOR3             m_BoundsMin;        // Bounding box minimum extents
    D3DXVECTOR3             m_BoundsMax;        // Bounding box maximum extents

};

//-----------------------------------------------------------------------------
// Name : CObject (Class)
// Desc : Mesh container class used to store instances of meshes.
//-----------------------------------------------------------------------------
class CObjectC
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
     CObjectC( CMeshC * pMesh );
	 CObjectC();

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    D3DXMATRIX  m_mtxWorld;             // Objects world matrix
    CMeshC      *m_pMesh;                // Mesh we are instancing

};

#endif