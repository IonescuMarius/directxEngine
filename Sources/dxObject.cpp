#include "..\Includes\dxObject.h"

CObject::CObject()
{
    // Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set variables
    m_pMesh           = NULL;
    m_pAnimController = NULL;
    m_nObjectSetIndex = -1;
	//m_pActionStatus   = NULL;
	//m_pActor          = NULL;
}

CObject::CObject( CMesh * pMesh )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set Variables
    m_pMesh           = NULL;
    m_pAnimController = NULL;
    m_nObjectSetIndex = -1;
	//m_pActor          = NULL;
	//m_pActionStatus   = NULL;
}

CObject::CObject( CTriMesh * pMesh )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set Mesh
    m_pMesh = pMesh;
	//m_pActor  = NULL;
    ActorTime = 0.0;
}

//-----------------------------------------------------------------------------
// Name : CObject () (Alternate Constructor)
// Desc : CObject Class Constructor, sets the internal actor object
//-----------------------------------------------------------------------------
CObject::CObject( dxMeshAnimation * pActor )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set variables
    m_pMesh           = NULL;
    m_pAnimController = NULL;
    m_nObjectSetIndex = -1;
	m_pActor          = pActor;
	m_pActionStatus   = NULL;
}

CObject::~CObject( )
{
      // Release our reference to the animation controller if we have one
    if ( m_pAnimController ) m_pAnimController->Release();

    // Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set variables
    m_pMesh           = NULL;
    m_pAnimController = NULL;
	m_nObjectSetIndex = -1;
	//m_pActor          = NULL;
    //m_pActionStatus   = NULL;
}

float CObject::VectorLength3D( CVertex* A )
{
	return sqrtf( (A->x * A->x) + (A->y * A->y) + (A->z * A->z));
}

CVertex CObject::AddVectors3D( CVertex* A , CVertex* B)
{
	CVertex* C = new CVertex();
	C->x = A->x + B->x;
	C->y = A->y + B->y;
	C->z = A->z + B->z;

	return *C;
}

CVertex CObject::SubtractVector3D( CVertex* A , CVertex* B)
{
	CVertex* C = new CVertex();
	C->x = A->x - B->x;
	C->y = A->y - B->y;
	C->z = A->z - B->z;

	return *C;
}

CVertex CObject::VectorMultiply3D (CVertex* A , float scalar)
{
	CVertex* C = new CVertex();
	C->x = A->x * scalar;
	C->y = A->y * scalar;
	C->z = A->z * scalar;

	return *C;
}

CVertex CObject::VectorNormalize3D ( CVertex* A )
{
	float length = VectorLength3D ( A );
	A->x = A->x / length;
	A->y = A->y / length;
	A->z = A->z / length;

	return *A;
}

CVertex CObject::VectorCrossProduct ( CVertex* A , CVertex* B )
{
	CVertex* C = new CVertex();
	C->x = (A->y * B->z)-(A->z * B->y);
	C->y = (A->z * B->x)-(A->x * B->z);
	C->z = (A->x * B->y)-(A->y * B->x);

	return *C;
}

CVertex CObject::GeneratePolygonNormal( CPolygon* P )
{
	CVertex* Edge1 = new CVertex();
	CVertex* Edge2 = new CVertex();
	CVertex* Normal = new CVertex();
	*Edge1 = SubtractVector3D ( P->m_pVertex, P->m_pVertex );
	*Edge2 = SubtractVector3D ( P->m_pVertex, P->m_pVertex );
	*Normal = VectorCrossProduct ( Edge1, Edge2);
	*Normal = VectorNormalize3D ( Normal );
	return *Normal;
}

float CObject::VectorDotProduct3D (CVertex* A , CVertex* B)
{
	return (A->x * B->x + A->y * B->y + A->z * B->z);
}

float CObject::FindVectorAngles3D (CVertex* A, CVertex* B)
{
	float LengthOfA = VectorLength3D ( A );
	float LengthOfB = VectorLength3D ( B );
	return acos ( (A->x*B->x + A->y*B->y + A->z+B->z) / (LengthOfA * LengthOfB) );
}

CMesh::CMesh()
{
	// Reset / Clear all required values
    m_nPolygonCount = 0;
    m_pPolygon      = NULL;
	m_pNext         = NULL;

    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

}

CMesh::CMesh( ULONG Count )
{
	// Reset / Clear all required values
    m_nPolygonCount = 0;
    m_pPolygon      = NULL;
	m_pNext         = NULL;

    // Add Polygons
    AddPolygon( Count );
}

//-----------------------------------------------------------------------------
// Name : CMesh () (Alternate Constructor)
// Desc : CMesh Class Constructor, adds specified number of vertices / indices
//-----------------------------------------------------------------------------
CMesh::CMesh( ULONG VertexCount, ULONG IndexCount )
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Add Vertices & indices if required
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

CMesh::~CMesh()
{
	Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release any allocated memory for this device.
// Note : Added to allow for clean up prior to rebuilding without delete call.
//-----------------------------------------------------------------------------
void CMesh::Release()
{
	// Release our mesh components
    if ( m_pPolygon ) 
    {
        // Delete all individual polygons in the array.
        for ( ULONG i = 0; i < m_nPolygonCount; i++ )
        {
            if ( m_pPolygon[i] ) 
				delete m_pPolygon[i];
        
        } // Next Polygon

        // Free up the array itself
        delete []m_pPolygon;
    
    } // End if

    // Clear variables
    m_pPolygon      = NULL;
    m_nPolygonCount = 0;

	// Release our mesh components
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();

    // Clear variables
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetVertexFormat ()
// Desc : Inform the mesh class about which format the vertices are assumed to
//        be, and also the size of each individual vertex.
//-----------------------------------------------------------------------------
void CMesh::SetVertexFormat( ULONG FVFCode, UCHAR Stride )
{
    // Store the values
    m_nFVFCode = FVFCode;
    m_nStride  = Stride;
}

long CMesh::AddPolygon( ULONG Count )
{

    CPolygon ** pPolyBuffer = NULL;
    
    // Allocate new resized array
    if (!( pPolyBuffer = new CPolygon*[ m_nPolygonCount + Count ] )) 
		return -1;

    // Clear out slack pointers
    ZeroMemory( &pPolyBuffer[ m_nPolygonCount ], Count * sizeof( CPolygon* ) );

    // Existing Data?
    if ( m_pPolygon )
    {
        // Copy old data into new buffer
        memcpy( pPolyBuffer, m_pPolygon, m_nPolygonCount * sizeof( CPolygon* ) );

        // Release old buffer
        delete []m_pPolygon;

    } // End if
    
    // Store pointer for new buffer
    m_pPolygon = pPolyBuffer;

    // Allocate new polygon pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new poly
        if (!( m_pPolygon[ m_nPolygonCount ] = new CPolygon() )) 
			return -1;

        // Increase overall poly count
        m_nPolygonCount++;

    } // Next Polygon
    
    // Return first polygon
    return m_nPolygonCount - Count;
}

CPolygon::CPolygon()
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;
}

CPolygon::CPolygon( USHORT Count )
{
	// Reset / Clear all required values
    m_nVertexCount  = 0;
    m_pVertex       = NULL;

    // Add vertices
    AddVertex( Count );
}

//-----------------------------------------------------------------------------
// Name : CPolygon () (Alternate Constructor)
// Desc : CPolygon Class Constructor, adds specified number of vertices
//-----------------------------------------------------------------------------
CPolygon::CPolygon( USHORT VertexCount, USHORT IndexCount )
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;

    // Add vertices & Indices
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

CPolygon::~CPolygon()
{
	// Release our vertices & Indices
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    // Clear variables
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;
}

long CPolygon::AddVertex( USHORT Count )
{
    CVertex * pVertexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pVertexBuffer = new CVertex[ m_nVertexCount + Count ] )) 
		return -1;

    // Existing Data?
    if ( m_pVertex )
    {
        // Copy old data into new buffer
        memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(CVertex) );

        // Release old buffer
        delete []m_pVertex;

    } // End if

    // Store pointer for new buffer
    m_pVertex = pVertexBuffer;
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this polygon.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CPolygon::AddIndex( USHORT Count )
{
    USHORT * pIndexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pIndexBuffer = new USHORT[ m_nIndexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pIndex )
    {
        // Copy old data into new buffer
        memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

        // Release old buffer
        delete []m_pIndex;

    } // End if

    // Store pointer for new buffer
    m_pIndex = pIndexBuffer;
    m_nIndexCount += Count;

    // Return first Index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : SetVertexBuffer ()
// Desc : Allows us to set the vertex buffer referenced by this object
//-----------------------------------------------------------------------------
void CObject::SetVertexBuffer( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer )
{
    // Release previous vertex buffer
    if ( m_pVertexBuffer ) 
		m_pVertexBuffer->Release();

    // Set the nwq vertex Buffer
    m_pVertexBuffer = pVertexBuffer;

    // Add a reference to it if we didn't pass in NULL
    if ( m_pVertexBuffer ) 
		m_pVertexBuffer->AddRef();
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this mesh.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddVertex( ULONG Count )
{
    UCHAR * pVertexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pVertexBuffer = new UCHAR[ (m_nVertexCount + Count) * m_nStride ] )) return -1;

    // Existing Data?
    if ( m_pVertex )
    {
        // Copy old data into new buffer
        memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * m_nStride );

        // Release old buffer
        delete []m_pVertex;

    } // End if

    // Store pointer for new buffer
    m_pVertex = pVertexBuffer;
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this mesh.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddIndex( ULONG Count )
{
    USHORT * pIndexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pIndexBuffer = new USHORT[ m_nIndexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pIndex )
    {
        // Copy old data into new buffer
        memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

        // Release old buffer
        delete []m_pIndex;

    } // End if

    // Store pointer for new buffer
    m_pIndex = pIndexBuffer;
    m_nIndexCount += Count;

    // Return first index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the mesh to build a set of index / vertex buffers from the
//        data currently stored within the mesh object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed (including vertex / index counts being
//        reset) so make sure you duplicate any data you may require.
//-----------------------------------------------------------------------------
HRESULT CMesh::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals )
{
	HRESULT     hRet    = S_OK;
    UCHAR      *pVertex = NULL;
    USHORT     *pIndex  = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();
    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Create our vertex buffer
    hRet = pD3DDevice->CreateVertexBuffer( m_nStride * m_nVertexCount, ulUsage, m_nFVFCode,
                                             D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, m_nStride * m_nVertexCount, (void**)&pVertex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the vertex data
    memcpy( pVertex, m_pVertex, m_nStride * m_nVertexCount );

    // We are finished with the vertex buffer
    m_pVertexBuffer->Unlock();

    
    // Create our index buffer
    hRet = pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
                                            D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the index buffer ready to fill data
    hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the index data
    memcpy( pIndex, m_pIndex, sizeof(USHORT) * m_nIndexCount );

    // We are finished with the indexbuffer
    m_pIndexBuffer->Unlock();

    // Calculate the mesh bounding box extents if FVF includes a position
    ULONG PosCode = (m_nFVFCode & D3DFVF_POSITION_MASK);
    if ( PosCode == D3DFVF_XYZ || PosCode == D3DFVF_XYZRHW )
    {
        m_BoundsMin = D3DXVECTOR3( 999999.0f, 999999.0f, 999999.0f );
        m_BoundsMax = D3DXVECTOR3( -999999.0f, -999999.0f, -999999.0f );
        for ( ULONG i = 0; i < m_nVertexCount; ++i )
        {
            D3DXVECTOR3 * Pos = (D3DXVECTOR3*)&m_pVertex[i * m_nStride];
            if ( Pos->x < m_BoundsMin.x ) m_BoundsMin.x = Pos->x;
            if ( Pos->y < m_BoundsMin.y ) m_BoundsMin.y = Pos->y;
            if ( Pos->z < m_BoundsMin.z ) m_BoundsMin.z = Pos->z;
            if ( Pos->x > m_BoundsMax.x ) m_BoundsMax.x = Pos->x;
            if ( Pos->y > m_BoundsMax.y ) m_BoundsMax.y = Pos->y;
            if ( Pos->z > m_BoundsMax.z ) m_BoundsMax.z = Pos->z;
    
        } // Next Vertex
    
    } // End if contains position
    
    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our mesh components
        if ( m_pVertex ) delete []m_pVertex;
        if ( m_pIndex  ) delete []m_pIndex;

        // Clear variables
        m_pVertex       = NULL;
        m_pIndex        = NULL;

    } // End if ReleaseOriginals

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : CTriMesh () (Constructor)
// Desc : CTriMesh Class Constructor
//-----------------------------------------------------------------------------
CTriMesh::CTriMesh()
{
    // Reset Variables
	m_pAdjacency      = NULL;
    m_pMesh           = NULL;
    m_pPMesh          = NULL;
    m_pAttribData     = NULL;
    m_nAttribCount    = 0;
    
    m_nVertexCount    = 0;
    m_nFaceCount      = 0;
    m_pAttribute      = NULL;
    m_pIndex          = NULL;
    m_pVertex         = NULL;
    m_nVertexStride   = 0;
    m_nIndexStride    = 0;
    m_nVertexFVF      = 0;
    m_nVertexCapacity = 0;
    m_nFaceCapacity   = 0;

    ZeroMemory( m_strMeshName, MAX_PATH * sizeof(TCHAR) );
    
    // Clear structures
    for ( ULONG i = 0; i < CALLBACK_COUNT; ++i ) ZeroMemory( &m_CallBack[i], sizeof(CALLBACK_FUNC) );
}

//-----------------------------------------------------------------------------
// Name : ~CTriMesh () (Destructor)
// Desc : CTriMesh Class Destructor
//-----------------------------------------------------------------------------
CTriMesh::~CTriMesh()
{
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release any and all objects, data etc.
//-----------------------------------------------------------------------------
void CTriMesh::Release( )
{
    // Release objects
    if ( m_pAdjacency ) m_pAdjacency->Release();
    if ( m_pMesh      ) m_pMesh->Release();
    if ( m_pPMesh     ) m_pPMesh->Release();

    // Release attribute data.
    if ( m_pAttribData )
    {
        for ( ULONG i = 0; i < m_nAttribCount; i++ )
        {
            // First release the texture object (addref was called earlier)
            if ( m_pAttribData[i].Texture ) m_pAttribData[i].Texture->Release();

            // And also the effect object
            if ( m_pAttribData[i].Effect  ) m_pAttribData[i].Effect->Release();
        
        } // Next Subset

        delete []m_pAttribData;
    
    } // End if subset data exists

    // Release flat arrays
    if ( m_pVertex    ) delete []m_pVertex; 
    if ( m_pIndex     ) delete []m_pIndex;
    if ( m_pAttribute ) delete []m_pAttribute;
    
    // Clear out variables
	m_pAdjacency      = NULL;
    m_pMesh           = NULL;
    m_pPMesh          = NULL;
    m_pAttribData     = NULL;
    m_nAttribCount    = 0;

    m_nVertexCount    = 0;
    m_nFaceCount      = 0;
    m_pAttribute      = NULL;
    m_pIndex          = NULL;
    m_pVertex         = NULL;
    m_nVertexStride   = 0;
    m_nIndexStride    = 0;
    m_nVertexFVF      = 0;
    m_nVertexCapacity = 0;
    m_nFaceCapacity   = 0;

    ZeroMemory( m_strMeshName, MAX_PATH * sizeof(TCHAR) );
    
    // Callbacks NOT cleared here!!!
}

//-----------------------------------------------------------------------------
// Name : RegisterCallback () (Destructor)
// Desc : Registers a callback function for one of the callback types.
//-----------------------------------------------------------------------------
bool CTriMesh::RegisterCallback( CALLBACK_TYPE Type, LPVOID pFunction, LPVOID pContext )
{
    // Validate Parameters
    if ( Type > CALLBACK_COUNT ) return false;

    // You cannot set the functions to anything other than NULL
    // if mesh data already exists (i.e. it's too late to change your mind :)
    if ( pFunction != NULL && m_pAttribData ) return false;

    // Store function pointer and context
    m_CallBack[ Type ].pFunction = pFunction;
    m_CallBack[ Type ].pContext  = pContext;

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : LoadMeshFromX ()
// Desc : Function, used to create the mesh object.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::LoadMeshFromX( LPCSTR pFileName, DWORD Options, LPDIRECT3DDEVICE9 pD3D )
{
    HRESULT      hRet;
    LPD3DXBUFFER pMatBuffer;            // Stores D3DXMATERIAL data (including texture filename)
    LPD3DXBUFFER pEffectBuffer;         // Stores effect file instances.
    ULONG        AttribID, i;
    ULONG        AttribCount;
    ULONG       *pAttribRemap  = NULL;
    bool         ManageAttribs = false;
    bool         RemapAttribs  = false;

    // Validate parameters
    if ( !pFileName || !pD3D ) return D3DERR_INVALIDCALL;

    // Release any old data
    Release();
    
    // Attempt to load the mesh
    hRet = D3DXLoadMeshFromX( pFileName, Options, pD3D, &m_pAdjacency, &pMatBuffer, &pEffectBuffer, &AttribCount, &m_pMesh );
    if ( FAILED(hRet) ) return hRet;

    // Are we managing our own attributes ?
    ManageAttribs = (m_CallBack[ CALLBACK_ATTRIBUTEID ].pFunction == NULL);
    m_nAttribCount = AttribCount;

    // Allocate the attribute data if this is a manager mesh
    if ( ManageAttribs == true && AttribCount > 0 )
    {
        m_pAttribData  = new MESH_ATTRIB_DATA[ m_nAttribCount ];
        if ( !m_pAttribData ) return E_OUTOFMEMORY;
        ZeroMemory( m_pAttribData, m_nAttribCount * sizeof(MESH_ATTRIB_DATA));
    
    } // End if managing attributes
    else
    {
        // Allocate attribute remap array
        pAttribRemap = new ULONG[ AttribCount ];
        if ( !pAttribRemap ) return E_OUTOFMEMORY;

        // Default remap to their initial values.
        for ( i = 0; i < AttribCount; ++i ) pAttribRemap[ i ] = i;
    
    } // End if not managing attributes

    // Retrieve data pointers
    D3DXMATERIAL       * pMaterials = (D3DXMATERIAL*)pMatBuffer->GetBufferPointer();
    D3DXEFFECTINSTANCE * pEffects   = (D3DXEFFECTINSTANCE*)pEffectBuffer->GetBufferPointer();

    // Loop through and process the attribute data
    for ( i = 0; i < AttribCount; ++i )
    {
        if ( ManageAttribs == true )
        {
            // Store material
            m_pAttribData[i].Material = pMaterials[i].MatD3D;

            // Note : The X File specification contains no ambient material property.
            //        We should ideally set this to full intensity to allow us to 
            //        control ambient brightness via the D3DRS_AMBIENT renderstate.
            m_pAttribData[i].Material.Ambient = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

            // Request texture pointer via callback
            if ( m_CallBack[ CALLBACK_TEXTURE ].pFunction )
            {
                COLLECTTEXTURE CollectTexture = (COLLECTTEXTURE)m_CallBack[ CALLBACK_TEXTURE ].pFunction;
                m_pAttribData[i].Texture = CollectTexture( m_CallBack[ CALLBACK_TEXTURE ].pContext, pMaterials[i].pTextureFilename );

                // Add reference. We are now using this
                if ( m_pAttribData[i].Texture ) m_pAttribData[i].Texture->AddRef();
        
            } // End if callback available

            // Request effect pointer via callback
            if ( m_CallBack[ CALLBACK_EFFECT ].pFunction )
            {
                COLLECTEFFECT CollectEffect = (COLLECTEFFECT)m_CallBack[ CALLBACK_EFFECT ].pFunction;
                m_pAttribData[i].Effect = CollectEffect( m_CallBack[ CALLBACK_EFFECT ].pContext, pEffects[i] );

                // Add reference. We are now using this
                if ( m_pAttribData[i].Effect ) m_pAttribData[i].Effect->AddRef();
        
            } // End if callback available

        } // End if attributes are managed
        else
        {
            // Request attribute ID via callback
            if ( m_CallBack[ CALLBACK_ATTRIBUTEID ].pFunction )
            {
                COLLECTATTRIBUTEID CollectAttributeID = (COLLECTATTRIBUTEID)m_CallBack[ CALLBACK_ATTRIBUTEID ].pFunction;
                AttribID = CollectAttributeID( m_CallBack[ CALLBACK_ATTRIBUTEID ].pContext, pMaterials[i].pTextureFilename,
                                               &pMaterials[i].MatD3D, &pEffects[i] );

                // Store this in our attribute remap table
                pAttribRemap[i] = AttribID;

                // Determine if any changes are required so far
                if ( AttribID != i ) RemapAttribs = true;
        
            } // End if callback available

        } // End if we don't manage attributes

    } // Next Material

    // Clean up buffers
    if ( pMatBuffer    ) pMatBuffer->Release();
    if ( pEffectBuffer ) pEffectBuffer->Release();

    // Remap attributes if required
    if ( pAttribRemap != NULL && RemapAttribs == true )
    {
        ULONG * pAttributes = NULL;

        // Lock the attribute buffer
        hRet = m_pMesh->LockAttributeBuffer( 0, &pAttributes );
        if ( FAILED(hRet) ) { delete []pAttribRemap; return hRet; }

        // Loop through all faces
        for ( i = 0; i < m_pMesh->GetNumFaces(); ++i )
        {
            // Retrieve the current attribute ID for this face
            AttribID = pAttributes[i];
        
            // Replace it with the remap value
            pAttributes[i] = pAttribRemap[AttribID];
        
        } // Next Face

        // Finish up
        m_pMesh->UnlockAttributeBuffer( );

    } // End if remap attributes

    // Release remap data
    if ( pAttribRemap ) delete []pAttribRemap;

    // Copy the filename over
    strcpy( m_strMeshName, pFileName );

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : CloneMeshFVF ()
// Desc : Clone this mesh OUT to another mesh, duplicating it's details.
// Note : This is only valid POST 'Load' or POST 'Build'. Works slightly 
//        differently than D3DX's CloneMeshFVF in that the mesh must already
//        be instantiated.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::CloneMeshFVF( ULONG Options, ULONG FVF, CTriMesh * pMeshOut, MESH_TYPE MeshType /* = MESH_DEFAULT */, LPDIRECT3DDEVICE9 pD3DDevice /* = NULL */ )
{
    HRESULT         hRet;
    LPD3DXBASEMESH  pCloneMesh  = NULL;
          

    // Validate requirements
    if ( (!m_pMesh && !m_pPMesh) || !pMeshOut ) return D3DERR_INVALIDCALL;

    // Generate adjacency if not yet available
    if (!m_pAdjacency)
    {
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no adjacency

    // If no new device was passed... 
    if ( !pD3DDevice )
    {
        // we'll use the same device as this mesh
        // This automatically calls 'AddRef' for the device
        pD3DDevice = GetDevice();

    } // End if no new device
    else
    {
        // Otherwise we'll add a reference here so that we can
        // release later for both cases without doing damage :)
        pD3DDevice->AddRef();
    
    } // End if new device

    switch ( MeshType )
    {
        case MESH_DEFAULT:  // Continue to use the same type as 'this'
            if ( m_pPMesh )
            {
                // Attempt to clone the mesh
                hRet = m_pPMesh->ClonePMeshFVF( Options, FVF, pD3DDevice, (LPD3DXPMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }

            } // End if Progressive
            else
            {
                // Attempt to clone the mesh
                hRet = m_pMesh->CloneMeshFVF( Options, FVF, pD3DDevice, (LPD3DXMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }
            
            } // End if standard
            break;

        case MESH_STANDARD: // Convert to, or continue to use standard mesh type
            if ( m_pPMesh )
            {
                // Attempt to clone the mesh
                hRet = m_pPMesh->CloneMeshFVF( Options, FVF, pD3DDevice, (LPD3DXMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }

            } // End if Progressive
            else
            {
                // Attempt to clone the mesh
                hRet = m_pMesh->CloneMeshFVF( Options, FVF, pD3DDevice, (LPD3DXMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }
            
            } // End if standard
            break;

        case MESH_PROGRESSIVE: // Convert to, or continue to use progressive mesh type
            if ( m_pPMesh )
            {
                // Attempt to clone the mesh
                hRet = m_pPMesh->ClonePMeshFVF( Options, FVF, pD3DDevice, (LPD3DXPMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }

            } // End if Progressive
            else
            {
                // Attempt to clone the mesh
                hRet = D3DXGeneratePMesh( m_pMesh, (DWORD*)m_pAdjacency->GetBufferPointer(), NULL, NULL, 
                                          1, D3DXMESHSIMP_FACE, (LPD3DXPMESH*)&pCloneMesh );
                if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }
            
            } // End if standard
            break;
    
    } // End type switch
    

    // Attach this D3DX mesh to the output mesh
    // This automatically adds a reference to the mesh passed in.
    pMeshOut->Attach( pCloneMesh );

    // We can now release our copy of the cloned mesh
    pCloneMesh->Release();

    // Copy over attributes if there is anything here
    if ( m_pAttribData )
    {
        // Add the correct number of attributes
        if ( pMeshOut->AddAttributeData( m_nAttribCount ) < 0 ) return E_OUTOFMEMORY;

        // Copy over attribute data
        MESH_ATTRIB_DATA * pAttributes = pMeshOut->GetAttributeData();
        for ( ULONG i = 0; i < m_nAttribCount; ++i )
        {
            MESH_ATTRIB_DATA * pAttrib = &pAttributes[i];
            // Store details
            pAttrib->Material = m_pAttribData[i].Material;
            pAttrib->Texture  = m_pAttribData[i].Texture;
            pAttrib->Effect   = m_pAttribData[i].Effect;

            // Add references so that objects aren't released when either of these
            // meshes are released, or vice versa.
            if ( pAttrib->Texture ) pAttrib->Texture->AddRef();
            if ( pAttrib->Effect  ) pAttrib->Effect->AddRef();
            
        } // Next Attribute

    } // End if managed

    // Release our referenced D3D Device
    if (pD3DDevice) pD3DDevice->Release();

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : Attach ()
// Desc : Attach an already created mesh type to this CTriMesh.
// Note : Because no attribute data is transferred, this will only really be
//        of any use for incidental processes, non managed rendering or in
//        cases where attribute data is manually transferred.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::Attach( LPD3DXBASEMESH pMesh, LPD3DXBUFFER pAdjacency /* = NULL */, bool bReplaceMeshOnly /* = false */ )
{
    HRESULT         hRet;
    MESH_TYPE       MeshType    = MESH_DEFAULT;
    LPD3DXBASEMESH  pAttachMesh = NULL;

    // Validate Requirements
    if ( !pMesh ) return D3DERR_INVALIDCALL;

    // Clear our current data unless otherwise specified
    if ( !bReplaceMeshOnly )
    {
        Release();
    
    } // End if replace all
    else
    {
        // Merely release meshes
        if ( m_pMesh  ) m_pMesh->Release();
        if ( m_pPMesh ) m_pPMesh->Release();
        m_pMesh  = NULL;
        m_pPMesh = NULL;

    } // End if replace mesh only

    // Determine the type of mesh
    // This will automatically add a reference of the type required
    hRet = pMesh->QueryInterface( IID_ID3DXMesh, (void**)&pAttachMesh );
    if ( FAILED(hRet) )
    {
        // Attempt to query for progressive mesh.
        hRet = pMesh->QueryInterface( IID_ID3DXPMesh, (void**)&pAttachMesh );
        if ( FAILED(hRet) ) return hRet;
        MeshType = MESH_PROGRESSIVE;
    
    } // End if not a standard mesh
    else
    {
        MeshType = MESH_STANDARD;
    
    } // End if standard mesh

    // Calculate strides etc
    m_nVertexFVF    = pAttachMesh->GetFVF();
    m_nVertexStride = pAttachMesh->GetNumBytesPerVertex();
    m_nIndexStride  = (GetOptions() & D3DXMESH_32BIT) ? 4 : 2;

    // If adjacency information was passed, reference it
    // if none was passed, it will be generated later, if required.
    if ( pAdjacency )
    {
        m_pAdjacency = pAdjacency;
        m_pAdjacency->AddRef();
    
    } // End if adjacency was passed

    // Store this in the correct mesh 'slot'
    if ( MeshType == MESH_STANDARD )
        m_pMesh = (LPD3DXMESH)pAttachMesh;
    else
        m_pPMesh = (LPD3DXPMESH)pAttachMesh;

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : Optimize ()
// Desc : Allows us to optimize the mesh in several ways.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::Optimize( ULONG Flags, CTriMesh * pMeshOut, MESH_TYPE MeshType /* = MESH_DEFAULT */, LPD3DXBUFFER * ppFaceRemap /* = NULL */, LPD3DXBUFFER * ppVertexRemap /* = NULL */, LPDIRECT3DDEVICE9 pD3DDevice /* = NULL */ )
{
    HRESULT             hRet;
    LPD3DXMESH          pOptimizeMesh = NULL;
    LPD3DXBUFFER        pFaceRemapBuffer = NULL;
    LPD3DXBUFFER        pAdjacency = NULL;
    ULONG              *pData      = NULL;

    // Validate requirements
    if ( (!m_pMesh && !m_pPMesh) || !pMeshOut ) return D3DERR_INVALIDCALL;

    // Generate adjacency if not yet available
    if (!m_pAdjacency)
    {
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no adjacency

    // If no new device was passed... 
    if ( !pD3DDevice )
    {
        // we'll use the same device as this mesh
        // This automatically calls 'AddRef' for the device
        pD3DDevice = GetDevice( );

    } // End if no new device
    else
    {
        // Otherwise we'll add a reference here so that we can
        // release later for both cases without doing damage :)
        pD3DDevice->AddRef();
    
    } // End if new device

    // Allocate new adjacency output buffer
    hRet = D3DXCreateBuffer( (3 * GetNumFaces()) * sizeof(ULONG), &pAdjacency );
    if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }

    // Allocate the output face remap if requested
    if ( ppFaceRemap )
    {
        // Allocate new face remap output buffer
        hRet = D3DXCreateBuffer( GetNumFaces() * sizeof(ULONG), ppFaceRemap );
        if ( FAILED(hRet) ) { pD3DDevice->Release(); pAdjacency->Release(); return hRet; }
        pData = (ULONG*)(*ppFaceRemap)->GetBufferPointer();
    
    } // End if allocate face remap data

    // Optimize out to a standard mesh
    if ( m_pPMesh )
    {
        // Attempt to optimize the mesh
        hRet = m_pPMesh->Optimize( Flags, (ULONG*)pAdjacency->GetBufferPointer(), pData, ppVertexRemap, &pOptimizeMesh );
        if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }


    } // End if Progressive
    else
    {
        // Attempt to optimize the mesh
        hRet = m_pMesh->Optimize( Flags, (ULONG*)m_pAdjacency->GetBufferPointer(), (ULONG*)pAdjacency->GetBufferPointer(),
                                  pData, ppVertexRemap, &pOptimizeMesh );
        if ( FAILED(hRet) ) { pD3DDevice->Release(); return hRet; }
    
    } // End if standard

    // Attach this D3DX mesh to the output mesh
    // This automatically adds a reference to the mesh passed in.
    pMeshOut->Attach( pOptimizeMesh, pAdjacency );

    switch ( MeshType )
    {
        case MESH_DEFAULT:  // Continue to use the same type as 'this'
            
            // Already a standard mesh, does it need conerting ?
            if ( m_pPMesh ) pMeshOut->GeneratePMesh( NULL, NULL, 0, D3DXMESHSIMP_FACE, true );
            break;

        case MESH_PROGRESSIVE: // Convert to, or continue to use progressive mesh type

            // Already a standard mesh, convert it
            pMeshOut->GeneratePMesh( NULL, NULL, 0, D3DXMESHSIMP_FACE, true );
            break;
    
    } // End type switch

    // We can now release our copy of the optimized mesh and the adjacency buffer
    pOptimizeMesh->Release();
    pAdjacency->Release();

    // Copy over attributes if there is anything here
    if ( m_pAttribData )
    {
        // Add the correct number of attributes
        if ( pMeshOut->AddAttributeData( m_nAttribCount ) < 0 ) return E_OUTOFMEMORY;

        // Copy over attribute data
        MESH_ATTRIB_DATA * pAttributes = pMeshOut->GetAttributeData();
        for ( ULONG i = 0; i < m_nAttribCount; ++i )
        {
            MESH_ATTRIB_DATA * pAttrib = &pAttributes[i];
            // Store details
            pAttrib->Material = m_pAttribData[i].Material;
            pAttrib->Texture  = m_pAttribData[i].Texture;
            pAttrib->Effect   = m_pAttribData[i].Effect;

            // Add references so that objects aren't released when either of these
            // meshes are released, or vice versa.
            if ( pAttrib->Texture ) pAttrib->Texture->AddRef();
            if ( pAttrib->Effect  ) pAttrib->Effect->AddRef();
            
        } // Next Attribute

    } // End if managed

    // Release our referenced D3D Device
    if (pD3DDevice) pD3DDevice->Release();

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : OptimizeInPlace ()
// Desc : Allows us to optimize the mesh in several ways.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::OptimizeInPlace( DWORD Flags, LPD3DXBUFFER * ppFaceRemap /* = NULL */, LPD3DXBUFFER * ppVertexRemap /* = NULL */ )
{
    HRESULT      hRet;
    LPD3DXBUFFER pFaceRemapBuffer = NULL;
    ULONG       *pData = NULL;

    // Validate Requirements
    if ( (!m_pMesh && !m_pPMesh) ) return D3DERR_INVALIDCALL;

    // Generate adjacency if none yet provided
    if (!m_pAdjacency)
    {
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no adjacency

    // Allocate the output face remap if requested
    if ( ppFaceRemap )
    {
        // Allocate new face remap output buffer
        hRet = D3DXCreateBuffer( GetNumFaces() * sizeof(ULONG), ppFaceRemap );
        if ( FAILED(hRet) ) { return hRet; }
        pData = (ULONG*)(*ppFaceRemap)->GetBufferPointer();
    
    } // End if allocate face remap data

    // Optimize the data
    if ( m_pPMesh )
        hRet = m_pPMesh->OptimizeBaseLOD( Flags, pData );
    else
        hRet = m_pMesh->OptimizeInplace( Flags, (DWORD*)m_pAdjacency->GetBufferPointer(), (DWORD*)m_pAdjacency->GetBufferPointer(), pData, ppVertexRemap );
    if ( FAILED( hRet ) ) return hRet;

    // Adjacency information altered, regenerate it.
    hRet = GenerateAdjacency();
    if ( FAILED(hRet) ) return hRet;

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : GenerateAdjacency ()
// Desc : Generate the adjacency information for this mesh.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::GenerateAdjacency( float Epsilon /* = 1e-3f */ )
{
    HRESULT hRet;

    // Validate Requirements
    if ( (!m_pMesh && !m_pPMesh) || GetNumFaces() == 0 ) return D3DERR_INVALIDCALL;

    // Clear out any old adjacency information
    if (m_pAdjacency) m_pAdjacency->Release();
    
    // Create the new adjacency buffer
    hRet = D3DXCreateBuffer( GetNumFaces() * (3 * sizeof(DWORD)), &m_pAdjacency );
    if ( FAILED(hRet) ) return hRet;

    // Retrieve the buffer pointer
    ULONG * Buffer = (ULONG*)m_pAdjacency->GetBufferPointer();
    
    // Generate the new adjacency information
    if ( m_pPMesh )
        hRet = m_pPMesh->GenerateAdjacency( Epsilon, Buffer );
    else
        hRet = m_pMesh->GenerateAdjacency( Epsilon, Buffer );
    if ( FAILED(hRet) ) return hRet;

    // Success !!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : WeldVertices ()
// Desc : Weld the vertices of this mesh. If no epsilons are provided, we'll
//        give them a helping hand.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::WeldVertices( ULONG Flags, const D3DXWELDEPSILONS * pEpsilon /* = NULL */ )
{
    HRESULT          hRet;
    D3DXWELDEPSILONS WeldEpsilons;

    // Validate Requirements (only valid for standard mesh)
    if ( !m_pMesh ) return D3DERR_INVALIDCALL;

    // Generate adjacency if none yet provided
    if (!m_pAdjacency)
    {
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no adjacency

    // Fill out an epsilon structure if none provided
    if ( !pEpsilon )
    {
        // Set all epsilons to 0.001; 
        float * pFloats = (float*)&WeldEpsilons;
        for ( ULONG i = 0; i < sizeof(D3DXWELDEPSILONS) / sizeof(float); i++ ) *pFloats++ = 1e-3f;

        // Store a pointer (this doesn't get passed back or anything, we're just reusing the empty var)
        pEpsilon = &WeldEpsilons;
    
    } // End if

    // Weld the vertices
    hRet = D3DXWeldVertices( m_pMesh, Flags, pEpsilon, (DWORD*)m_pAdjacency->GetBufferPointer(), (DWORD*)m_pAdjacency->GetBufferPointer(), NULL, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Adjacency information altered, regenerate it.
    hRet = GenerateAdjacency();
    if ( FAILED(hRet) ) return hRet;
    
    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : DrawSubset ()
// Desc : Draws only the specified subset of the mesh. Also optionally sets
//        the attribute data if that information is being managed.
// Note : The material data can be overriden so that it is set separately from
//        the attribute ID itself.
//-----------------------------------------------------------------------------
void CTriMesh::DrawSubset( ULONG AttributeID, long MaterialOverride /* = -1 */ )
{
    UINT              PassCount  = 0;
    LPDIRECT3DDEVICE9 pD3DDevice = NULL;
    LPD3DXEFFECT      pEffect    = NULL;
    LPD3DXBASEMESH    pMesh      = m_pMesh;
    ULONG             MaterialID = (MaterialOverride < 0) ? AttributeID : (ULONG)MaterialOverride;

    // Retrieve mesh pointer
    if ( !pMesh ) pMesh = m_pPMesh;
    if ( !pMesh ) return;

    // Set the attribute data
    if ( m_pAttribData && AttributeID < m_nAttribCount )
    {
        // Retrieve the Direct3D device
        pD3DDevice = GetDevice( );

        pD3DDevice->SetMaterial( &m_pAttribData[MaterialID].Material );
        pD3DDevice->SetTexture( 0, m_pAttribData[MaterialID].Texture );
        pEffect = m_pAttribData[MaterialID].Effect;

        // Release the device
        pD3DDevice->Release();
    
    } // End if attribute data is managed
        
    // Determine method of rendering
    if ( pEffect != NULL )
    {
        // Note : The valid technique we wish to use must already 
        //        as active, usually by the CollectEffect callback.
        
        // Begin. Records device and shader states, and retrieves the passcount
        if ( FAILED( pEffect->Begin( &PassCount, 0 ) ) ) return;

        // Loop through each effect file pass
        for ( ULONG j = 0; j < PassCount; ++j )
        {
            // Set the states
            if ( FAILED( pEffect->BeginPass( j ) ) ) continue;

            // Draw this subset.
            pMesh->DrawSubset( AttributeID );

            // End the effect pass
            pEffect->EndPass( );

        } // Next Pass

        // End. Restores device and shader states.
        pEffect->End( );

    } // End if effect provided
    else
    {
        // Otherwise simply render the subset(s)
        pMesh->DrawSubset( AttributeID );
    
    } // End if no effect provided
}

//-----------------------------------------------------------------------------
// Name : Draw ()
// Desc : Draws all subsets of this mesh (includes setting of data)
//-----------------------------------------------------------------------------
void CTriMesh::Draw(  )
{
    UINT              PassCount  = 0;
    LPDIRECT3DDEVICE9 pD3DDevice = NULL;
    LPD3DXEFFECT      pEffect    = NULL;
    LPD3DXBASEMESH    pMesh      = m_pMesh;

    // Retrieve mesh pointer
    if ( !pMesh ) pMesh = m_pPMesh;
    if ( !pMesh ) return;

    // This function is invalid if there is no managed data
    if ( !m_pAttribData ) return;

    // Retrieve the Direct3D device
    pD3DDevice = GetDevice( );

    // Render the subsets
    for ( ULONG i = 0; i < m_nAttribCount; ++i )
    {
        // Here is an ideal place to set the device's FVF flags. However,
        // since ID3DXMesh::DrawSubset does this for us, we need only do this
        // if we are implementing the mesh classes / systems for ourselves.
        //pD3DDevice->SetFVF( GetFVF() );

        // Set the attribute data
        pD3DDevice->SetMaterial( &m_pAttribData[i].Material );
        pD3DDevice->SetTexture( 0, m_pAttribData[i].Texture );
        pEffect = m_pAttribData[i].Effect;
        
        // Determine method of rendering
        if ( pEffect != NULL )
        {
            // Note : The valid technique we wish to use must already 
            //        as active, usually by the CollectEffect callback.
            
            // Begin. Records device and shader states, and retrieves the passcount
            if ( FAILED( pEffect->Begin( &PassCount, 0 ) ) ) continue;

            // Loop through each effect file pass
            for ( ULONG j = 0; j < PassCount; ++j )
            {
                // Set the states
                if ( FAILED( pEffect->BeginPass( j ) ) ) continue;

                // Draw this subset.
                pMesh->DrawSubset( i );

                // End shader pass
                pEffect->EndPass( );

            } // Next Pass

            // End. Restores device and shader states.
            pEffect->End( );

        } // End if effect provided
        else
        {
            // Otherwise simply render the subset(s)
            pMesh->DrawSubset( i );
        
        } // End if no effect provided
        
    } // Next attribute

    // Release the device
    pD3DDevice->Release();
}

//-----------------------------------------------------------------------------
// Name : SetDataFormat ()
// Desc : Inform the mesh class about the size of the vertices, and indices
//        about to be fed in.
//-----------------------------------------------------------------------------
void CTriMesh::SetDataFormat( ULONG VertexFVF, ULONG IndexStride )
{
    // Store the values
    m_nVertexFVF    = VertexFVF;
    m_nVertexStride = D3DXGetFVFVertexSize( VertexFVF );
    m_nIndexStride  = IndexStride;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this mesh.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTriMesh::AddVertex( ULONG Count /* = 1 */, LPVOID pVertices /* = NULL */  )
{
    UCHAR * pVertexBuffer = NULL;
    
    if ( m_nVertexCount + Count > m_nVertexCapacity )
    {
        // Adjust our vertex capacity (resize 100 at a time)
        for ( ; m_nVertexCapacity < (m_nVertexCount + Count) ; ) m_nVertexCapacity += 100;
        
        // Allocate new resized array
        if (!( pVertexBuffer = new UCHAR[ (m_nVertexCapacity) * m_nVertexStride ] )) return -1;

        // Existing Data?
        if ( m_pVertex )
        {
            // Copy old data into new buffer
            memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * m_nVertexStride );

            // Release old buffer
            delete []m_pVertex;

        } // End if

        // Store pointer for new buffer
        m_pVertex = pVertexBuffer;

    } // End if a resize is required
    
    // Copy over vertex data if provided
    if ( pVertices ) memcpy( &m_pVertex[m_nVertexCount * m_nVertexStride], pVertices, Count * m_nVertexStride );

    // Increase Vertex Count
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddFace()
// Desc : Adds a face, or multiple faces, to this mesh.
// Note : Returns the index for the first face added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTriMesh::AddFace( ULONG Count /* = 1 */, LPVOID pIndices /* = NULL */, ULONG AttribID /* = 0 */ )
{
    UCHAR * pIndexBuffer = NULL;
    ULONG * pAttributeBuffer = NULL;
    
    if ( m_nFaceCount + Count > m_nFaceCapacity )
    {
        // Adjust our face capacity (resize 100 at a time)
        for ( ; m_nFaceCapacity < (m_nFaceCount + Count) ; ) m_nFaceCapacity += 100;
        
        // Allocate new resized array
        if (!( pIndexBuffer = new UCHAR[ (m_nFaceCapacity * 3) * m_nIndexStride ] )) return -1;

        // Existing Data?
        if ( m_pIndex )
        {
            // Copy old data into new buffer
            memcpy( pIndexBuffer, m_pIndex, (m_nFaceCount * 3) * m_nIndexStride );

            // Release old buffer
            delete []m_pIndex;

        } // End if

        // Store pointer for new buffer
        m_pIndex = pIndexBuffer;

        // Allocate new resized attribute array
        if (!( pAttributeBuffer = new ULONG[ m_nFaceCapacity ] )) return -1;

        // Existing Data?
        if ( m_pAttribute )
        {
            // Copy old data into new buffer
            memcpy( pAttributeBuffer, m_pAttribute, m_nFaceCount * sizeof(ULONG) );

            // Release old buffer
            delete []m_pAttribute;

        } // End if

        // Store pointer for new buffer
        m_pAttribute = pAttributeBuffer;

    } // End if a resize is required
    
    // Copy over index and attribute data if provided
    if ( pIndices ) memcpy( &m_pIndex[(m_nFaceCount * 3) * m_nIndexStride], pIndices, (Count * 3) * m_nIndexStride );
    for ( ULONG i = m_nFaceCount; i < m_nFaceCount + Count; ++i) m_pAttribute[i] = AttribID;

    // Increase Face Count
    m_nFaceCount += Count;

    // Return first face
    return m_nFaceCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddAttributeData()
// Desc : Adds an attribute, or multiple attributes, to this mesh.
// Note : Returns the index for the attribute added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTriMesh::AddAttributeData( ULONG Count /* = 1 */ )
{
    MESH_ATTRIB_DATA * pAttribBuffer = NULL;
        
    // Allocate new resized array
    if (!( pAttribBuffer = new MESH_ATTRIB_DATA[ m_nAttribCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pAttribData )
    {
        // Copy old data into new buffer
        memcpy( pAttribBuffer, m_pAttribData, m_nAttribCount * sizeof(MESH_ATTRIB_DATA) );

        // Release old buffer
        delete []m_pAttribData;

    } // End if

    // Store pointer for new buffer
    m_pAttribData = pAttribBuffer;

    // Clear the new items
    ZeroMemory( &m_pAttribData[m_nAttribCount], Count * sizeof(MESH_ATTRIB_DATA) );

    // Increase Attrib Count
    m_nAttribCount += Count;

    // Return first Attrib
    return m_nAttribCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildMesh()
// Desc : Assuming mesh data has been entered, this function will build the
//        internal mesh, and release that original data.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::BuildMesh( ULONG Options, LPDIRECT3DDEVICE9 pDevice, bool ReleaseOriginals /* = true */ )
{
    HRESULT hRet;
    LPVOID  pVertices   = NULL;
    LPVOID  pIndices    = NULL;
    ULONG  *pAttributes = NULL;

    // Validate Requirements
    if ( !m_pVertex || !m_pIndex || !m_pAttribute || !pDevice ) return D3DERR_INVALIDCALL;

    // First release the original meshes if they exist
    if ( m_pMesh ) { m_pMesh->Release(); m_pMesh = NULL; }
    if ( m_pPMesh ) { m_pPMesh->Release(); m_pPMesh = NULL; }
    
    // Force 32 bit mesh if required
    if ( m_nIndexStride == 4 ) Options |= D3DXMESH_32BIT;

    // Create the blank empty mesh
    hRet = D3DXCreateMeshFVF( m_nFaceCount, m_nVertexCount, Options, m_nVertexFVF, pDevice, &m_pMesh );
    if ( FAILED(hRet) ) return hRet;

    // Lock the vertex buffer and copy the data
    hRet = m_pMesh->LockVertexBuffer( 0, &pVertices );
    if ( FAILED(hRet) ) return hRet;
    memcpy( pVertices, m_pVertex, m_nVertexCount * m_nVertexStride );
    m_pMesh->UnlockVertexBuffer();

    // Lock the index buffer and copy the data
    hRet = m_pMesh->LockIndexBuffer( 0, &pIndices );
    if ( FAILED(hRet) ) return hRet;
    memcpy( pIndices, m_pIndex, (m_nFaceCount * 3) * m_nIndexStride );
    m_pMesh->UnlockIndexBuffer();

    // Lock the attribute buffer and copy the data
    hRet = m_pMesh->LockAttributeBuffer( 0, &pAttributes );
    if ( FAILED(hRet) ) return hRet;
    memcpy( pAttributes, m_pAttribute, m_nFaceCount * sizeof(ULONG) );
    m_pMesh->UnlockAttributeBuffer();

    // Release the original data if requested
    if ( ReleaseOriginals )
    {
        if ( m_pVertex    ) delete []m_pVertex; 
        if ( m_pIndex     ) delete []m_pIndex;
        if ( m_pAttribute ) delete []m_pAttribute;
    
        m_nVertexCount    = 0;
        m_nFaceCount      = 0;
        m_pAttribute      = NULL;
        m_pIndex          = NULL;
        m_pVertex         = NULL;
        m_nVertexCapacity = 0;
        m_nFaceCapacity   = 0;
    
    } // End if release

    // We're done :)
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : GeneratePMesh ()
// Desc : Take the already loaded mesh data and generate the progressive mesh.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::GeneratePMesh( const LPD3DXATTRIBUTEWEIGHTS pAttributeWeights, CONST FLOAT *pVertexWeights, ULONG MinValue, ULONG Options, bool ReleaseOriginal /* = true */ )
{
    HRESULT         hRet;
    LPD3DXMESH      pTempMesh      = NULL;
    LPD3DXBUFFER    pTempAdjacency = NULL;

    // Validate Parameters
    if (!m_pMesh) return D3DERR_INVALIDCALL;

    // Release previous Progressive mesh if one already exists
    if ( m_pPMesh ) { m_pPMesh->Release(); m_pPMesh = NULL; }

    // Generate adjacency if not yet available
    if (!m_pAdjacency)
    {
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no adjacency

    // Validate the base mesh
    hRet = D3DXValidMesh( m_pMesh, (DWORD*)m_pAdjacency->GetBufferPointer(), NULL );
    if ( FAILED(hRet) )
    {
        // Allocate the temporary adjacency buffer
        hRet = D3DXCreateBuffer( m_pAdjacency->GetBufferSize(), &pTempAdjacency );
        if ( FAILED(hRet) ) return hRet;

        // Repair the mesh
        hRet = D3DXCleanMesh( D3DXCLEAN_SIMPLIFICATION, m_pMesh, (DWORD*)m_pAdjacency->GetBufferPointer(), &pTempMesh, (DWORD*)pTempAdjacency->GetBufferPointer(), NULL );
        if ( FAILED(hRet) ) { pTempAdjacency->Release(); return hRet; }

    } // End if invalid
    else
    {
        // Simply store common pointers
        pTempAdjacency = m_pAdjacency;
        pTempMesh      = m_pMesh;

        // Add references so that the originals are not released
        m_pAdjacency->AddRef();
        m_pMesh->AddRef();
    
    } // End if valid mesh

    // Generate the progressive mesh
    hRet = D3DXGeneratePMesh( pTempMesh, (DWORD*)pTempAdjacency->GetBufferPointer(), pAttributeWeights, pVertexWeights, MinValue, Options, &m_pPMesh );

    // Release all used objects
    pTempMesh->Release();
    pTempAdjacency->Release();

    // Release the original mesh if requested, and PMesh generation was a success
    if ( ReleaseOriginal && SUCCEEDED(hRet) ) { m_pMesh->Release(); m_pMesh = NULL; }
    
    // Success??
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SnapshotToMesh ()
// Desc : If a progressive mesh exists, clone it back into a standard mesh.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::SnapshotToMesh( bool ReleaseProgressive /* = true */ )
{
    HRESULT     hRet;
    LPD3DXMESH  pCloneMesh = NULL;

    // Validate parameters
    if ( !m_pPMesh ) return D3DERR_INVALIDCALL;

    // Retrieve the device for cloning
    LPDIRECT3DDEVICE9 pDevice = NULL;
    m_pPMesh->GetDevice( &pDevice );

    // Clone the mesh
    hRet = m_pPMesh->CloneMeshFVF( m_pPMesh->GetOptions(), m_pPMesh->GetFVF(), pDevice, &pCloneMesh );

    // Release the retrieved device
    pDevice->Release();

    // Bail out if the clone failed
    if ( FAILED( hRet ) ) return hRet;

    // Release any original mesh if it already existed
    if ( m_pMesh ) { m_pMesh->Release(); m_pMesh = NULL; }

    // Store the cloned mesh
    m_pMesh = pCloneMesh;

    // Release the original if requested
    if ( ReleaseProgressive ) { m_pPMesh->Release(); m_pPMesh = NULL; }
    
    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : SetNumFaces ()
// Desc : If a progressive mesh exists, set the number of faces to draw.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::SetNumFaces( ULONG FaceCount )
{
    HRESULT hRet = D3DERR_INVALIDCALL;
    
    // Set number of faces (this is a no-op if there is no pmesh)
    if ( m_pPMesh ) hRet = m_pPMesh->SetNumFaces( FaceCount );

    // Success??
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SetNumVertices ()
// Desc : If a progressive mesh exists, set the number of vertices to draw.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::SetNumVertices( ULONG VertexCount )
{
    HRESULT hRet = D3DERR_INVALIDCALL;
    
    // Set number of vertices (this is a no-op if there is no pmesh)
    if ( m_pPMesh ) hRet = m_pPMesh->SetNumVertices( VertexCount );

    // Success??
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : TrimByFaces ()
// Desc : If a progressive mesh exists, trim the faces.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::TrimByFaces( ULONG NewFacesMin, ULONG NewFacesMax )
{
    HRESULT hRet = D3DERR_INVALIDCALL;
    
    // Trim by faces
    if ( m_pPMesh )
    {
        // Drop through to P mesh trimming
        hRet = m_pPMesh->TrimByFaces( NewFacesMin, NewFacesMax, NULL, NULL );

    } // End if PMesh exists
    else if ( m_pMesh )
    {
        LPD3DXMESH pMeshOut = NULL;

        // Generate adjacency if not yet available
        if (!m_pAdjacency)
        {
            hRet = GenerateAdjacency();
            if ( FAILED(hRet) ) return hRet;
    
        } // End if no adjacency

        // Since there is no PMesh, we can only comply by simplifying
        hRet = D3DXSimplifyMesh( m_pMesh, (DWORD*)m_pAdjacency->GetBufferPointer(), NULL, NULL,
                                 NewFacesMax, D3DXMESHSIMP_FACE, &pMeshOut );
        if ( FAILED(hRet) ) return hRet;

        // Store this new mesh
        m_pMesh->Release();
        m_pMesh = pMeshOut;

        // Adjacency will be out of date, update it
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;

    } // End if no PMesh

    // Success??
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : TrimByVertices ()
// Desc : If a progressive mesh exists, trim the vertices.
//-----------------------------------------------------------------------------
HRESULT CTriMesh::TrimByVertices( ULONG NewVerticesMin, ULONG NewVerticesMax )
{
    HRESULT hRet = D3DERR_INVALIDCALL;
    
    // Trim by vertices
    if ( m_pPMesh )
    {
        // Drop through to P mesh trimming
        hRet = m_pPMesh->TrimByVertices( NewVerticesMin, NewVerticesMax, NULL, NULL );

    } // End if PMesh exists
    else if ( m_pMesh )
    {
        LPD3DXMESH pMeshOut = NULL;

        // Generate adjacency if not yet available
        if (!m_pAdjacency)
        {
            hRet = GenerateAdjacency();
            if ( FAILED(hRet) ) return hRet;
    
        } // End if no adjacency

        // Since there is no PMesh, we can only comply by simplifying
        hRet = D3DXSimplifyMesh( m_pMesh, (DWORD*)m_pAdjacency->GetBufferPointer(), NULL, NULL,
                                 NewVerticesMax, D3DXMESHSIMP_VERTEX, &pMeshOut );
        if ( FAILED(hRet) ) return hRet;

        // Store this new mesh
        m_pMesh->Release();
        m_pMesh = pMeshOut;

        // Adjacency will be out of date, update it
        hRet = GenerateAdjacency();
        if ( FAILED(hRet) ) return hRet;

    } // End if no PMesh

    // Success??
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : GetVertices()
// Desc : Retrieves the vertex array used during mesh creation.
//-----------------------------------------------------------------------------
void * CTriMesh::GetVertices( ) const
{
    return m_pVertex;
}

//-----------------------------------------------------------------------------
// Name : GetFaces()
// Desc : Retrieves the face (or index) array used during mesh creation.
//-----------------------------------------------------------------------------
void * CTriMesh::GetFaces( ) const
{
    return m_pIndex;
}

//-----------------------------------------------------------------------------
// Name : GetAttributes()
// Desc : Retrieves the attribute ID array used during mesh creation.
//-----------------------------------------------------------------------------
ULONG * CTriMesh::GetAttributes( ) const
{
    return m_pAttribute;
}

//-----------------------------------------------------------------------------
// Name : GetAttributeData()
// Desc : Retrieves the attribute data array.
//-----------------------------------------------------------------------------
MESH_ATTRIB_DATA * CTriMesh::GetAttributeData( ) const
{
    return m_pAttribData;
}

//-----------------------------------------------------------------------------
// Name : GetMesh()
// Desc : Retrieves the underlying mesh object
//-----------------------------------------------------------------------------
LPD3DXMESH CTriMesh::GetMesh( ) const
{
    if ( !m_pMesh) return NULL;

    // Add a reference to the mesh so don't forget to release it!!!
    m_pMesh->AddRef();
    return m_pMesh;
}

//-----------------------------------------------------------------------------
// Name : GetPMesh()
// Desc : Retrieves the underlying mesh object
//-----------------------------------------------------------------------------
LPD3DXPMESH CTriMesh::GetPMesh( ) const
{
    if ( !m_pPMesh) return NULL;

    // Add a reference to the mesh so don't forget to release it!!!
    m_pPMesh->AddRef();
    return m_pPMesh;
}

//-----------------------------------------------------------------------------
// Name : GetAdjacencyBuffer()
// Desc : Retrieves the underlying adjacency buffer object
//-----------------------------------------------------------------------------
LPD3DXBUFFER CTriMesh::GetAdjacencyBuffer( ) const
{
    if ( !m_pAdjacency ) return NULL;

    // Add a reference to the buffer so don't forget to release it!!!
    m_pAdjacency->AddRef();
    return m_pAdjacency;
}

//-----------------------------------------------------------------------------
// Name : GetDevice()
// Desc : Retrieves the direct 3d device being used.
//-----------------------------------------------------------------------------
LPDIRECT3DDEVICE9 CTriMesh::GetDevice( ) const
{
    LPDIRECT3DDEVICE9 pDevice = NULL;

    // Automatically adds are ference so don't forget to release it !!
    if ( m_pPMesh )
        m_pPMesh->GetDevice( &pDevice );
    else if ( m_pMesh ) 
        m_pMesh->GetDevice( &pDevice );
    
    return pDevice;
}

//-----------------------------------------------------------------------------
// Name : GetOptions()
// Desc : Retrieves the option set used by the mesh.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetOptions( ) const
{
    // Validation!!
    if ( m_pPMesh )
        return m_pPMesh->GetOptions();
    else if ( m_pMesh )
        return m_pMesh->GetOptions();
    else
        return 0;
}

//-----------------------------------------------------------------------------
// Name : GetNumVertices()
// Desc : Retrieves the number of vertices in the mesh, or optionally the
//        stride of the vertices stored ready for building.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetNumVertices( ) const
{
    // Validation!!
    if ( m_pPMesh )
        return m_pPMesh->GetNumVertices();
    else if ( m_pMesh )
        return m_pMesh->GetNumVertices();
    else
        return m_nVertexCount;
}

//-----------------------------------------------------------------------------
// Name : GetVertexStride()
// Desc : Retrieves the vertex stride used by the mesh, or optionally the
//        stride of the vertices stored ready for building.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetVertexStride( ) const
{
    // Validation!!
    if ( m_pPMesh )
        return m_pPMesh->GetNumBytesPerVertex();
    else if ( m_pMesh )
        return m_pMesh->GetNumBytesPerVertex();
    else
        return m_nVertexStride;
}

//-----------------------------------------------------------------------------
// Name : GetFVF()
// Desc : Retrieves the FVF code used by the mesh, or optionally the
//        FVF of the vertices stored ready for building.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetFVF( ) const
{
    // Validation!!
    if ( m_pPMesh )
        return m_pPMesh->GetFVF();
    else if ( m_pMesh )
        return m_pMesh->GetFVF();
    else
        return m_nVertexFVF;
}

//-----------------------------------------------------------------------------
// Name : GetNumFaces()
// Desc : Retrieves the number of faces in the mesh, or optionally the
//        number of faces stored ready for building.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetNumFaces( ) const
{
    // Validation!!
    if ( m_pPMesh )
        return m_pPMesh->GetNumFaces();
    else if ( m_pMesh )
        return m_pMesh->GetNumFaces();
    else
        return m_nFaceCount;
}

//-----------------------------------------------------------------------------
// Name : GetIndexStride()
// Desc : Retrieves the Index stride used by the mesh, or optionally the
//        stride of the vertices stored ready for building.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetIndexStride( ) const
{
    // Validation!!
    if ( !m_pMesh && !m_pPMesh )
        return m_nIndexStride; 
    else 
        return (GetOptions() & D3DXMESH_32BIT) ? 4 : 2;
}

//-----------------------------------------------------------------------------
// Name : GetAttributeCount ()
// Desc : Retrieves the number of attributes being managed by this mesh.
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetAttributeCount( ) const
{
    return m_nAttribCount;
}

//-----------------------------------------------------------------------------
// Name : GetMaxFaces ()
// Desc : Retrieves the maximum number of faces for the mesh.
// Note : If you are planning to use the standard and progressive mesh types
//        interchangably, and you always want the actual number of faces,
//        then you should use this function over GetNumFaces()
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetMaxFaces( ) const
{
    if ( m_pPMesh )
        return m_pPMesh->GetMaxFaces();
    else if ( m_pMesh )
        return m_pMesh->GetNumFaces();
    else
        return m_nFaceCount;
}

//-----------------------------------------------------------------------------
// Name : GetMinFaces ()
// Desc : Retrieves the minimum number of faces for the mesh.
// Note : If you are planning to use the standard and progressive mesh types
//        interchangably, and you always want the actual number of faces,
//        then you should use this function over GetNumFaces()
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetMinFaces( ) const
{
    if ( m_pPMesh )
        return m_pPMesh->GetMinFaces();
    else if ( m_pMesh )
        return m_pMesh->GetNumFaces();
    else
        return m_nFaceCount;
}

//-----------------------------------------------------------------------------
// Name : GetMaxVertices ()
// Desc : Retrieves the maximum number of Vertices for the mesh.
// Note : If you are planning to use the standard and progressive mesh types
//        interchangably, and you always want the actual number of Vertices,
//        then you should use this function over GetNumVertices()
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetMaxVertices( ) const
{
    if ( m_pPMesh )
        return m_pPMesh->GetMaxVertices();
    else if ( m_pMesh )
        return m_pMesh->GetNumVertices();
    else
        return m_nVertexCount;
}

//-----------------------------------------------------------------------------
// Name : GetMinVertices ()
// Desc : Retrieves the minimum number of Vertices for the mesh.
// Note : If you are planning to use the standard and progressive mesh types
//        interchangably, and you always want the actual number of Vertices,
//        then you should use this function over GetNumVertices()
//-----------------------------------------------------------------------------
ULONG CTriMesh::GetMinVertices( ) const
{
    if ( m_pPMesh )
        return m_pPMesh->GetMinVertices();
    else if ( m_pMesh )
        return m_pMesh->GetNumVertices();
    else
        return m_nVertexCount;
}

//-----------------------------------------------------------------------------
// Name : GetMeshName ()
// Desc : Retrieve the filename used to load / reference the mesh.
//-----------------------------------------------------------------------------
LPCSTR CTriMesh::GetMeshName( ) const
{
    return m_strMeshName;
}

//////////////////////////////////////////////////////////////////////////////////////////
//CUSTOM OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
CObjectC::CObjectC()
{
	// Reset / Clear all required values
    m_pMesh = NULL;
    D3DXMatrixIdentity( &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name : CObject () (Alternate Constructor)
// Desc : CObject Class Constructor, sets the internal mesh object
//-----------------------------------------------------------------------------
CObjectC::CObjectC( CMeshC * pMesh )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set Mesh
    m_pMesh = pMesh;
}

//-----------------------------------------------------------------------------
// Name : CMesh () (Constructor)
// Desc : CMesh Class Constructor
//-----------------------------------------------------------------------------
CMeshC::CMeshC()
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

}

//-----------------------------------------------------------------------------
// Name : CMesh () (Alternate Constructor)
// Desc : CMesh Class Constructor, adds specified number of vertices / indices
//-----------------------------------------------------------------------------
CMeshC::CMeshC( ULONG VertexCount, ULONG IndexCount )
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Add Vertices & indices if required
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

//-----------------------------------------------------------------------------
// Name : ~CMesh () (Destructor)
// Desc : CMesh Class Destructor
//-----------------------------------------------------------------------------
CMeshC::~CMeshC()
{
	// Release our mesh components
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();

    // Clear variables
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this mesh.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMeshC::AddVertex( ULONG Count )
{
    CVertexC * pVertexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pVertexBuffer = new CVertexC[ m_nVertexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pVertex )
    {
        // Copy old data into new buffer
        memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(CVertexC) );

        // Release old buffer
        delete []m_pVertex;

    } // End if

    // Store pointer for new buffer
    m_pVertex = pVertexBuffer;
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this mesh.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMeshC::AddIndex( ULONG Count )
{
    USHORT * pIndexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pIndexBuffer = new USHORT[ m_nIndexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pIndex )
    {
        // Copy old data into new buffer
        memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

        // Release old buffer
        delete []m_pIndex;

    } // End if

    // Store pointer for new buffer
    m_pIndex = pIndexBuffer;
    m_nIndexCount += Count;

    // Return first index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the mesh to build a set of index / vertex buffers from the
//        data currently stored within the mesh object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed (including vertex / index counts being
//        reset) so make sure you duplicate any data you may require.
//-----------------------------------------------------------------------------
HRESULT CMeshC::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals )
{
    HRESULT     hRet    = S_OK;
    CVertexC    *pVertex = NULL;
    USHORT     *pIndex  = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();
    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Create our vertex buffer
    hRet = pD3DDevice->CreateVertexBuffer( sizeof(CVertexC) * m_nVertexCount, ulUsage, D3DFVF_XYZ | D3DFVF_DIFFUSE,
                                             D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, sizeof(CVertexC) * m_nVertexCount, (void**)&pVertex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the vertex data
    memcpy( pVertex, m_pVertex, sizeof(CVertexC) * m_nVertexCount );

    // We are finished with the vertex buffer
    m_pVertexBuffer->Unlock();

    
    // Create our index buffer
    hRet = pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
                                            D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the index buffer ready to fill data
    hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the index data
    memcpy( pIndex, m_pIndex, sizeof(USHORT) * m_nIndexCount );

    // We are finished with the indexbuffer
    m_pIndexBuffer->Unlock();

    // Calculate the mesh bounding box extents
    m_BoundsMin = D3DXVECTOR3( 999999.0f, 999999.0f, 999999.0f );
    m_BoundsMax = D3DXVECTOR3( -999999.0f, -999999.0f, -999999.0f );
    for ( ULONG i = 0; i < m_nVertexCount; ++i )
    {
        D3DXVECTOR3 * Pos = (D3DXVECTOR3*)&m_pVertex[i];
        if ( Pos->x < m_BoundsMin.x ) m_BoundsMin.x = Pos->x;
        if ( Pos->y < m_BoundsMin.y ) m_BoundsMin.y = Pos->y;
        if ( Pos->z < m_BoundsMin.z ) m_BoundsMin.z = Pos->z;
        if ( Pos->x > m_BoundsMax.x ) m_BoundsMax.x = Pos->x;
        if ( Pos->y > m_BoundsMax.y ) m_BoundsMax.y = Pos->y;
        if ( Pos->z > m_BoundsMax.z ) m_BoundsMax.z = Pos->z;
    
    } // Next Vertex
    
    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our mesh components
        if ( m_pVertex ) delete []m_pVertex;
        if ( m_pIndex  ) delete []m_pIndex;

        // Clear variables
        m_pVertex       = NULL;
        m_pIndex        = NULL;
        m_nVertexCount  = 0;
        m_nIndexCount   = 0;

    } // End if ReleaseOriginals

    return S_OK;
}