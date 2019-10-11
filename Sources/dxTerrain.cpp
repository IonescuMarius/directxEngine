#include "..\Includes\dxTerrain.h"
#include "..\Includes\dxObject.h"
#include "..\Includes\dxPlayer.h"
#include "..\Includes\dxCamera.h"

//-----------------------------------------------------------------------------
// Module Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const USHORT BlockWidth  = 17;                  // Number of vertices in a terrain block (X)
    const USHORT BlockHeight = 17;                  // Number of vertices in a terrain block (Z)
    const USHORT QuadsWide   = BlockWidth - 1;      // Number of quads in a terrain block (X)
    const USHORT QuadsHigh   = BlockHeight - 1;     // Number of quads in a terrain block (Z)
};

//-----------------------------------------------------------------------------
// Name : CTerrain () (Constructor)
// Desc : CTerrain Class Constructor
//-----------------------------------------------------------------------------
CTerrain::CTerrain()
{
    // Reset all required values
    m_pD3DDevice        = NULL;

    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;

    m_pMesh             = NULL;
    m_nMeshCount        = 0;

    m_vecScale          = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );

}

//-----------------------------------------------------------------------------
// Name : CTerrain () (Destructor)
// Desc : CTerrain Class Destructor
//-----------------------------------------------------------------------------
CTerrain::~CTerrain()
{
    // Release any allocated memory
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Allows us to release any allocated memory and reuse.
//-----------------------------------------------------------------------------
void CTerrain::Release()
{
    // Release Heightmap
    if ( m_pHeightMap ) delete[]m_pHeightMap;

    // Release Meshes
    if ( m_pMesh ) 
    {
        // Delete all individual meshes in the array.
        for ( ULONG i = 0; i < m_nMeshCount; i++ )
        {
            if ( m_pMesh[i] ) delete m_pMesh[i];
        
        } // Next Mesh

        // Free up the array itself
        delete []m_pMesh;
    
    } // End if

    // Release our D3D Object ownership
    if ( m_pD3DDevice ) m_pD3DDevice->Release();

    // Clear Variables
    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;
    m_pMesh             = NULL;
    m_nMeshCount        = 0;
    m_pD3DDevice        = NULL;
}

//-----------------------------------------------------------------------------
// Name : LoadHeightMap ()
// Desc : Load in the heightmap and build the meshes required to render it.
// Note : You are required to pass in the width and height of the 2d data
//        that you are loading from the raw heightmap file. These values must
//        be multiples of QuadsWide / QuadsHigh after subtracting one.
//-----------------------------------------------------------------------------
bool CTerrain::LoadHeightMap( LPCTSTR FileName, ULONG Width, ULONG Height )
{
    FILE * pFile = NULL;

    // Cannot load if already allocated (must be explicitly released for reuse)
    if ( m_pMesh ) return false;

    // Must have an already set D3D Device
    if ( !m_pD3DDevice ) return false;
    
    // First of all store the information passed
    m_nHeightMapWidth  = Width;
    m_nHeightMapHeight = Height;

    // A scale of 4 is roughly the best size for a 512 x 512 quad terrain.
    // Using the following forumla, lowering the size of the terrain 
    // simply lowers the vertex resolution but maintains the map size.
    m_vecScale.x = 4.0f * (512 / (m_nHeightMapWidth - 1));
    m_vecScale.y = 2.0f;
    m_vecScale.z = 4.0f * (512 / (m_nHeightMapHeight - 1));

    // Attempt to allocate space for this heightmap information
    m_pHeightMap = new UCHAR[Width * Height];
    if (!m_pHeightMap) return false;

    // Open up the heightmap file
    pFile = _tfopen( FileName, _T("rb") );
    if (!pFile) return false;

    // Read the heightmap data (grayscale)
    fread( m_pHeightMap, Width * Height, 1, pFile );
    
    // Finish up
    fclose( pFile );

    // Allocate enough meshes to store the separate blocks of this terrain
    if ( AddMesh( ((Width - 1) / QuadsWide) * ((Height - 1) / QuadsHigh) ) < 0 ) return false;

    // Build the mesh data itself
    return BuildMeshes( );
}

//-----------------------------------------------------------------------------
// Name : GetHeightMapNormal ()
// Desc : Retrieves the normal at this position in the heightmap
//-----------------------------------------------------------------------------
D3DXVECTOR3 CTerrain::GetHeightMapNormal( ULONG x, ULONG z )
{
	D3DXVECTOR3 Normal, Edge1, Edge2;
	ULONG       HMIndex, HMAddX, HMAddZ;
    float       y1, y2, y3;

	// Make sure we are not out of bounds
	if ( x < 0.0f || z < 0.0f || x >= m_nHeightMapWidth || z >= m_nHeightMapHeight ) return D3DXVECTOR3(0.0f, 1.0f, 0.0f);

    // Calculate the index in the heightmap array
    HMIndex = x + z * m_nHeightMapWidth;
    
    // Calculate the number of pixels to add in either direction to
    // obtain the best neighbouring heightmap pixel.
    if ( x < (m_nHeightMapWidth - 1))  HMAddX = 1; else HMAddX = -1;
	if ( z < (m_nHeightMapHeight - 1)) HMAddZ = m_nHeightMapWidth; else HMAddZ = -(signed)m_nHeightMapWidth;
	
    // Get the three height values
	y1 = (float)m_pHeightMap[HMIndex] * m_vecScale.y;
	y2 = (float)m_pHeightMap[HMIndex + HMAddX] * m_vecScale.y; 
	y3 = (float)m_pHeightMap[HMIndex + HMAddZ] * m_vecScale.y;
			
	// Calculate Edges
	Edge1 = D3DXVECTOR3( 0.0f, y3 - y1, m_vecScale.z );
	Edge2 = D3DXVECTOR3( m_vecScale.x, y2 - y1, 0.0f );
			
	// Calculate Resulting Normal
	D3DXVec3Cross( &Normal, &Edge1, &Edge2);
	D3DXVec3Normalize( &Normal, &Normal );
	
    // Return it.
	return Normal;
}

//-----------------------------------------------------------------------------
// Name : GetHeight ()
// Desc : Retrieves the height at the given world space location
// Note : Pass in true to the 'ReverseQuad' parameter to reverse the direction
//        in which the quads dividing edge is based (Normally Top Right to
//        bottom left assuming pixel space)
//-----------------------------------------------------------------------------
float CTerrain::GetHeight( float x, float z, bool ReverseQuad )
{
    float fTopLeft, fTopRight, fBottomLeft, fBottomRight;

    // Adjust Input Values
    x = x / m_vecScale.x;
    z = z / m_vecScale.z;

    // Make sure we are not OOB
    if ( x < 0.0f || z < 0.0f || x >= m_nHeightMapWidth || z >= m_nHeightMapHeight ) return 0.0f;	

    // First retrieve the Heightmap Points
    int ix = (int)x;
    int iz = (int)z;

    int AddX = ( x < (m_nHeightMapWidth-1) )  ? AddX = 1 : AddX = -1;
    int AddZ = ( z < (m_nHeightMapHeight-1) ) ? AddZ = 1 : AddZ = -1;
	
    // Calculate the remainder (percent across quad)
    float fPercentX = x - ((float)ix);
    float fPercentZ = z - ((float)iz);

    if ( ReverseQuad )
    {
        // First retrieve the height of each point in the dividing edge
        fTopLeft     = (float)m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomRight = (float)m_pHeightMap[(ix + AddX) + (iz + AddZ) * m_nHeightMapWidth] * m_vecScale.y;

        // Which triangle of the quad are we in ?
        if ( fPercentX < fPercentZ )
        {
            fBottomLeft = (float)m_pHeightMap[ix + (iz + AddZ) * m_nHeightMapWidth] * m_vecScale.y;
		    fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        
        } // End if Left Triangle
        else
        {
            fTopRight   = (float)m_pHeightMap[(ix + AddX) + iz * m_nHeightMapWidth] * m_vecScale.y;
		    fBottomLeft = fTopLeft + (fBottomRight - fTopRight);

        } // End if Right Triangle
    
    } // End if Quad is reversed
    else
    {
        // First retrieve the height of each point in the dividing edge
        fTopRight   = (float)m_pHeightMap[(ix + AddX) + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomLeft = (float)m_pHeightMap[ix + (iz + AddZ) * m_nHeightMapWidth] * m_vecScale.y;

        // Calculate which triangle of the quad are we in ?
        if ( fPercentX < (1.0f - fPercentZ)) 
        {
            fTopLeft = (float)m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
            fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
        
        } // End if Left Triangle
        else
        {
            fBottomRight = (float)m_pHeightMap[(ix + AddX) + (iz + AddZ) * m_nHeightMapWidth] * m_vecScale.y;
            fTopLeft = fTopRight + (fBottomLeft - fBottomRight);

        } // End if Right Triangle
    
    } // End if Quad is not reversed
    
    // Calculate the height interpolated across the top and bottom edges
    float fTopHeight    = fTopLeft    + ((fTopRight - fTopLeft) * fPercentX );
    float fBottomHeight = fBottomLeft + ((fBottomRight - fBottomLeft) * fPercentX );

    // Calculate the resulting height interpolated between the two heights
    return fTopHeight + ((fBottomHeight - fTopHeight) * fPercentZ );
}

//-----------------------------------------------------------------------------
// Name : BuildMeshes()
// Desc : Build all of the mesh data required to render the terrain.
//-----------------------------------------------------------------------------
bool CTerrain::BuildMeshes( )
{
    long x, z, vx, vz, Counter, StartX, StartZ;
    long BlocksWide = (m_nHeightMapWidth  - 1) / QuadsWide;
    long BlocksHigh = (m_nHeightMapHeight - 1) / QuadsHigh;

    D3DXVECTOR3 Normal;

    // Note : The following formulas are expanded for clarity. They could be reduced.
    // Calculate IndexCount.... ( Number required for quads ) + ( Extra Degenerates verts (one per quad row except last) )
    ULONG IndexCount  = ((BlockWidth * 2) * QuadsHigh) + ( QuadsHigh - 1 );
    
    // Calculate Primitive Count (( Number of quads ) * 2) + ( 3 degenerate tris per quad row except last )
    m_nPrimitiveCount = ((QuadsWide * QuadsHigh) * 2) + ((QuadsHigh - 1) * 3);

    // Loop through and generate the mesh data
    for ( z = 0; z < BlocksHigh; z++ )
    {
        for ( x = 0; x < BlocksWide; x++ )
        {
            CMeshC * pMesh = m_pMesh[ x + z * BlocksWide ];

            // Allocate all the vertices & indices required for this mesh
            if ( pMesh->AddVertex( BlockWidth * BlockHeight ) < 0 ) return false;
            if ( pMesh->AddIndex( IndexCount ) < 0 ) return false;

            // Calculate Vertex Positions
            Counter = 0;
            StartX  = x * (BlockWidth  - 1);
            StartZ  = z * (BlockHeight - 1);
            for ( vz = StartZ; vz < StartZ + BlockHeight; vz++ )
            {
                for ( vx = StartX; vx < StartX + BlockWidth; vx++ )
                {
                    // Calculate and Set The vertex data.
                    pMesh->m_pVertex[ Counter ].x = (float)vx * m_vecScale.x;
                    pMesh->m_pVertex[ Counter ].y = (float)m_pHeightMap[ vx + vz * m_nHeightMapWidth ] * m_vecScale.y;
                    pMesh->m_pVertex[ Counter ].z = (float)vz * m_vecScale.z;
                    Counter++;
                
                } // Next Vertex Column
            
            } // Next Vertex Row

            // Calculate vertex normals
            Counter = 0;
            for ( vz = StartZ; vz < StartZ + BlockHeight; vz++ )
            {
                for ( vx = StartX; vx < StartX + BlockWidth; vx++ )
                {
                    // Generate averaged normal
                    Normal  = GetHeightMapNormal( vx    , vz     );
                    Normal += GetHeightMapNormal( vx + 1, vz     );
                    Normal += GetHeightMapNormal( vx + 1, vz + 1 );
                    Normal += GetHeightMapNormal( vx    , vz + 1 );
                    Normal /= 4;

                    // Store Vertex Normal
                    pMesh->m_pVertex[ Counter ].Normal = Normal;
                    Counter++;
                
                } // Next Vertex Column
            
            } // Next Vertex Row

            Counter = 0;
            // Calculate the indices for the terrain block tri-strip 
            for ( vz = 0; vz < BlockHeight - 1; vz++ )
            {
                // Is this an odd or even row ?
                if ( (vz % 2) == 0 )
                {
                    for ( vx = 0; vx < BlockWidth; vx++ )
                    {
                        // Force insert winding order switch degenerate ?
                        if ( vx == 0 && vz > 0 ) pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);

                        // Insert next two indices
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

                    } // Next Index Column
                    
                } // End if even row
                else
                {
                    for ( vx = BlockWidth - 1; vx >= 0; vx--)
                    {
                        // Force insert winding order switch degenerate ?
                        if ( vx == (BlockWidth - 1) ) pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);

                        // Insert next two indices
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

                    } // Next Index Column

                } // End if odd row
            
            } // Next Index Row

            // Instruct mesh to build buffers
            if ( FAILED(pMesh->BuildBuffers( m_pD3DDevice, m_bHardwareTnL )) ) return false;

        } // Next Block Column
    
    } // Next Block Row

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : Render()
// Desc : Renders all of the meshes stored within this terrain object.
//-----------------------------------------------------------------------------
void CTerrain::Render( dxCamera * pCamera )
{
    // Validate parameters
    if( !m_pD3DDevice ) return;
    
    // Render Each Mesh
    for ( ULONG i = 0; i < m_nMeshCount; i++ )
    {
        // Skip if mesh is not within the viewing frustum
        if ( pCamera && (!pCamera->BoundsInFrustum( m_pMesh[i]->m_BoundsMin, m_pMesh[i]->m_BoundsMax )) ) continue;

        // Set the stream sources
        m_pD3DDevice->SetStreamSource( 0, m_pMesh[i]->m_pVertexBuffer, 0, sizeof(CVertexC) );
        m_pD3DDevice->SetIndices( m_pMesh[i]->m_pIndexBuffer );

        // Render the vertex buffer
        m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, BlockWidth * BlockHeight, 0, m_nPrimitiveCount );

    } // Next Mesh

}

//-----------------------------------------------------------------------------
// Name : AddMesh()
// Desc : Adds a mesh, or multiple meshes, to this object.
// Note : Returns the index for the first mesh added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTerrain::AddMesh( ULONG Count )
{

    CMeshC ** pMeshBuffer = NULL;
    
    // Allocate new resized array
    if (!( pMeshBuffer = new CMeshC*[ m_nMeshCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pMeshBuffer[ m_nMeshCount ], Count * sizeof( CMeshC* ) );

    // Existing Data?
    if ( m_pMesh )
    {
        // Copy old data into new buffer
        memcpy( pMeshBuffer, m_pMesh, m_nMeshCount * sizeof( CMeshC* ) );

        // Release old buffer
        delete []m_pMesh;

    } // End if
    
    // Store pointer for new buffer
    m_pMesh = pMeshBuffer;

    // Allocate new mesh pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new mesh
        if (!( m_pMesh[ m_nMeshCount ] = new CMeshC() )) return -1;

        // Increase overall mesh count
        m_nMeshCount++;

    } // Next Polygon
    
    // Return first mesh
    return m_nMeshCount - Count;
}

//-----------------------------------------------------------------------------
// Name : SetD3DDevice()
// Desc : Sets the D3D Device that will be used for buffer creation and renering
//-----------------------------------------------------------------------------
void CTerrain::SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL )
{
    // Validate Parameters
    if ( !pD3DDevice ) return;

    // Store D3D Device and add a reference
    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    // Store vertex processing type for buffer creation
    m_bHardwareTnL = HardwareTnL;
}

//-----------------------------------------------------------------------------
// Name : UpdatePlayer() (Static)
// Desc : Called to allow the terrain object to update the player details
//        based on the height from the terrain for example.
//-----------------------------------------------------------------------------
void CTerrain::UpdatePlayer( LPVOID pContext, CPlayer * pPlayer, float TimeScale )
{
    // Validate Parameters
    if ( !pContext || !pPlayer ) return;

    VOLUME_INFO Volume   = pPlayer->GetVolumeInfo();
    D3DXVECTOR3 Position = pPlayer->GetPosition();
    D3DXVECTOR3 Velocity = pPlayer->GetVelocity();
    bool        ReverseQuad = false;

    // Determine which row we are on
    int PosZ = (int)(Position.z / ((CTerrain*)pContext)->m_vecScale.z);
    if ( (PosZ % 2) != 0 ) ReverseQuad = true;

    // Retrieve the height of the terrain at this position
    float fHeight = ((CTerrain*)pContext)->GetHeight( Position.x, Position.z, ReverseQuad ) - Volume.Min.y;

    // Determine if the position is lower than the height at this position
    if ( Position.y < fHeight )
    {
        // Update camera details
        Velocity.y = 0;
        Position.y = fHeight;

        // Update the camera
        pPlayer->SetVelocity( Velocity );
        pPlayer->SetPosition( Position );

    } // End if colliding

}

//-----------------------------------------------------------------------------
// Name : UpdateCamera() (Static)
// Desc : Called to allow the terrain object to update the camera details
//        based on the height from the terrain for example.
//-----------------------------------------------------------------------------
void CTerrain::UpdateCamera( LPVOID pContext, dxCamera * pCamera, float TimeScale )
{
    // Validate Requirements
    if (!pContext || !pCamera ) return;
    if ( pCamera->GetCameraMode() != dxCamera::MODE_THIRDPERSON ) return;

    VOLUME_INFO Volume   = pCamera->GetVolumeInfo();
    D3DXVECTOR3 Position = pCamera->GetPosition();
    bool        ReverseQuad = false;

    // Determine which row we are on
    ULONG PosZ = (ULONG)(Position.z / ((CTerrain*)pContext)->m_vecScale.z);
    if ( (PosZ % 2) != 0 ) ReverseQuad = true; else ReverseQuad = false;

    float fHeight = ((CTerrain*)pContext)->GetHeight( Position.x, Position.z, ReverseQuad ) - Volume.Min.y;

    // Determine if the position is lower than the height at this position
    if ( Position.y < fHeight )
    {
        // Update camera details
        Position.y = fHeight;
        pCamera->SetPosition( Position );

    } // End if colliding

    // Retrieve the player at which the camera is looking
    CPlayer * pPlayer = pCamera->GetPlayer();
    if (!pPlayer) return;

    // We have updated the position of either our player or camera
    // We must now instruct the camera to look at the players position
    ((dxCam3rdPerson*)pCamera)->SetLookAt(  pPlayer->GetPosition() );

}