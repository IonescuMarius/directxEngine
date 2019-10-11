#include "..\Includes\dxMeshAnimation.h"
#include "..\Includes\dxUtility.h"
#include "..\Includes\dxMeshHierarchy.h"
using namespace std;

const float kMoveTransitionTime=0.25f;

dxMeshAnimation::dxMeshAnimation()
{
}

dxMeshAnimation::dxMeshAnimation(LPDIRECT3DDEVICE9 d3dDevice)
{
	dx_d3dDevice = d3dDevice;
	m_speedAdjust = 1.0f;
	m_firstMesh = 0;
	m_currentTrack = 0;
	m_currentTime = 0;
	m_numAnimationSets = 0;
	m_currentAnimationSet = 0;
	m_maxBones = 0;
	m_sphereRadius = 0;
	m_sphereCentre = D3DXVECTOR3(0,0,0);
	m_boneMatrices = 0;

	/////////////////////////////////////////
	m_frameRoot        = NULL;
    m_animController   = NULL;
    m_nOptions          = 0;
    
    m_pSWMatrices       = NULL;
    m_nMaxSWMatrices    = 0;
    m_nSkinMethod       = SKINMETHOD_AUTODETECT;

    m_pActions          = NULL;
    m_nActionCount      = 0;
    m_pActionStatus     = NULL;

    m_nMaxAnimSets      = 1;
    m_nMaxAnimOutputs   = 1; 
    m_nMaxTracks        = 2;
    m_nMaxEvents        = 30;
    m_nMaxCallbackKeys  = 1024;
    ZeroMemory( m_strActorName, MAX_PATH * sizeof(TCHAR) );
    D3DXMatrixIdentity( &m_mtxWorld );
    for ( ULONG i = 0; i < CALLBACK_COUNT; ++i ) 
		ZeroMemory( &m_CallBack[i], sizeof(CALLBACK_FUNC) );
}

dxMeshAnimation::~dxMeshAnimation()
{
	if (m_animController)
	{
		m_animController->Release();
		m_animController=0;
	}

	if (m_frameRoot)
	{
		dxMeshHierarchy memoryAllocator;
		D3DXFrameDestroy(m_frameRoot, &memoryAllocator);
		m_frameRoot=0;
	}

	if (m_boneMatrices)
	{
		delete []m_boneMatrices;
		m_boneMatrices=0;
	}

	Release();
}

void dxMeshAnimation::Release()
{
    CAllocateHierarchy Allocator( this );
    ReleaseCallbackData( );

    //if ( m_pActionStatus   ) m_pActionStatus->Release();
    if ( m_pActions        ) delete []m_pActions;
    if ( m_pSWMatrices     ) delete []m_pSWMatrices;
    if ( m_frameRoot      ) D3DXFrameDestroy( m_frameRoot, &Allocator );
    if ( m_animController ) m_animController->Release();
    m_frameRoot      = NULL;
    m_animController = NULL;
    dx_d3dDevice      = NULL;
    m_nOptions        = 0;
    m_pSWMatrices     = NULL;
    m_nMaxSWMatrices  = 0;
    m_pActions        = NULL;
    m_nActionCount    = 0;
    m_pActionStatus   = NULL;

    ZeroMemory( m_strActorName, MAX_PATH * sizeof(TCHAR) );
}

void dxMeshAnimation::ReleaseCallbackData( )
{
    HRESULT             hRet;
    ULONG               i, SetCount;
    ID3DXAnimationSet * pSet = NULL;

    if ( !m_animController ) return;

    for ( i = 0, SetCount = m_animController->GetNumAnimationSets(); i < SetCount; ++i )
    {
        hRet = m_animController->GetAnimationSet( i, &pSet );
        if ( FAILED(hRet) ) continue;

        ReleaseCallbackData( pSet );

        pSet->Release();
    }
}

void dxMeshAnimation::ReleaseCallbackData( ID3DXAnimationSet * pSet )
{
    double     Position  = 0.0;
    ULONG      Flags     = 0;
    IUnknown * pCallback = NULL;

    for ( ; SUCCEEDED(pSet->GetCallback( Position, Flags, &Position, (void**)&pCallback )) ; )
    {
        if ( Position >= pSet->GetPeriod() ) break;
        if ( pCallback ) pCallback->Release();
        Flags = D3DXCALLBACK_SEARCH_EXCLUDING_INITIAL_POSITION; 
    }
}

bool dxMeshAnimation::RegisterCallback( CALLBACK_TYPE Type, LPVOID pFunction, LPVOID pContext )
{
    if ( Type > CALLBACK_COUNT ) return false;
    if ( pFunction != NULL && IsLoaded() ) return false;

    m_CallBack[ Type ].pFunction = pFunction;
    m_CallBack[ Type ].pContext  = pContext;

    return true;
}

void dxMeshAnimation::SetSkinningMethod( ULONG SkinMethod )
{
    m_nSkinMethod = SkinMethod;
}

ULONG dxMeshAnimation::GetSkinningMethod( ) const
{
    return m_nSkinMethod;
}

bool dxMeshAnimation::Load(const std::string &filename)
{
	dxMeshHierarchy *memoryAllocator=new dxMeshHierarchy;

	std::string currentDirectory=CUtility::GetTheCurrentDirectory();

	std::string xfilePath;
	CUtility::SplitPath(filename,&xfilePath,&m_filename);

	SetCurrentDirectory(xfilePath.c_str());

	HRESULT hr = D3DXLoadMeshHierarchyFromX(filename.c_str(), 
							D3DXMESH_MANAGED, 
							dx_d3dDevice, 
							memoryAllocator, 
							NULL, 
							&m_frameRoot, 
							&m_animController);

	delete memoryAllocator;
	memoryAllocator=0;

	SetCurrentDirectory(currentDirectory.c_str());
	
	if (CUtility::FailedHr(hr))
		return false; 

	// if the x file contains any animation remember how many sets there are
	if(m_animController)
		m_numAnimationSets = m_animController->GetMaxNumAnimationSets();

	// Bones for skining
	if(m_frameRoot)
	{
		// Set the bones up
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)m_frameRoot, NULL);

		// Create the bone matrices array for use during FrameMove to hold the final transform
		m_boneMatrices  = new D3DXMATRIX[m_maxBones];
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		// Calculate the Bounding Sphere for this model (used in CalculateInitialViewMatrix to position camera correctly)
		D3DXFrameCalculateBoundingSphere(m_frameRoot, &m_sphereCentre, &m_sphereRadius);
	}
	
	return true;
}

D3DXVECTOR3 dxMeshAnimation::GetInitialCameraPosition() const
{
	D3DXVECTOR3 cameraPos(0.0f,m_sphereCentre.y,-(m_sphereRadius*3));
	return cameraPos;
}

void dxMeshAnimation::SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix)
{
	// Cast to our extended structure first
	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)pFrame->pMeshContainer;

	// If this frame has a mesh
	if(pMesh)
	{
		// We need to remember which is the first mesh in the hierarchy for later when we 
		// update (FrameMove)
		if(!m_firstMesh)
			m_firstMesh = pMesh;
		
		// if there is skin info, then setup the bone matrices
		if(pMesh->pSkinInfo && pMesh->MeshData.pMesh)
		{
			// Create a copy of the mesh to skin into later
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
			if (FAILED(pMesh->MeshData.pMesh->GetDeclaration(Declaration)))
				return;

			pMesh->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, 
				Declaration, dx_d3dDevice, 
				&pMesh->exSkinMesh);

			// Max bones is calculated for later use (to know how big to make the bone matrices array)
			m_maxBones=max(m_maxBones,(int)pMesh->pSkinInfo->GetNumBones());

			// For each bone work out its matrix
			for (unsigned int i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
			{   
				// Find the frame containing the bone
				D3DXFRAME_EXTENDED* pTempFrame = (D3DXFRAME_EXTENDED*)D3DXFrameFind(m_frameRoot, 
						pMesh->pSkinInfo->GetBoneName(i));

				// set the bone part - point it at the transformation matrix
				pMesh->exFrameCombinedMatrixPointer[i] = &pTempFrame->exCombinedTransformationMatrix;
			}

		}
	}

	// Pass on to sibblings
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameSibling, pParentMatrix);

	// Pass on to children
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameFirstChild, &pFrame->exCombinedTransformationMatrix);
}

void dxMeshAnimation::FrameMove(float elapsedTime,const D3DXMATRIX *matWorld)
{
	// Adjust animation speed
	elapsedTime/=m_speedAdjust;

	// Advance the time and set in the controller
    if (m_animController != NULL)
        m_animController->AdvanceTime(elapsedTime, NULL);
        
    CUtility::DebugString("Debug frame: "+ToString(elapsedTime)+"\n");

	m_currentTime+=elapsedTime;

	// Now update the model matrices in the hierarchy
    UpdateFrameMatrices(m_frameRoot, matWorld);

	// If the model contains a skinned mesh update the vertices
	D3DXMESHCONTAINER_EXTENDED* pMesh = m_firstMesh;
	if(pMesh && pMesh->pSkinInfo)
	{
		unsigned int Bones = pMesh->pSkinInfo->GetNumBones();

		for (unsigned int i = 0; i < Bones; ++i)
			D3DXMatrixMultiply(&m_boneMatrices[i],&pMesh->exBoneOffsets[i], pMesh->exFrameCombinedMatrixPointer[i]);

		void *srcPtr=0;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr=0;
		pMesh->exSkinMesh->LockVertexBuffer(0, (void**)&destPtr);

		// Update the skinned mesh 
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		// Unlock the meshes vertex buffers
		pMesh->exSkinMesh->UnlockVertexBuffer();
		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

void dxMeshAnimation::UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix)
{
    D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	// If parent matrix exists multiply our frame matrix by it
    if (parentMatrix != NULL)
        D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, &currentFrame->TransformationMatrix, parentMatrix);
    else
        currentFrame->exCombinedTransformationMatrix = currentFrame->TransformationMatrix;

	// If we have a sibling recurse 
    if (currentFrame->pFrameSibling != NULL)
        UpdateFrameMatrices(currentFrame->pFrameSibling, parentMatrix);

	// If we have a child recurse 
    if (currentFrame->pFrameFirstChild != NULL)
        UpdateFrameMatrices(currentFrame->pFrameFirstChild, &currentFrame->exCombinedTransformationMatrix);

	D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)currentFrame->pMeshContainer;
}

void dxMeshAnimation::UpdateFrameMatrices( LPD3DXFRAME pFrame, const D3DXMATRIX * pParentMatrix )
{
    D3DXFRAME_MATRIX * pMtxFrame = (D3DXFRAME_MATRIX*)pFrame;

    if ( pParentMatrix != NULL)
        D3DXMatrixMultiply( &pMtxFrame->mtxCombined, &pMtxFrame->TransformationMatrix, pParentMatrix);
    else
        pMtxFrame->mtxCombined = pMtxFrame->TransformationMatrix;

    // Move onto sibling frame
    if ( pMtxFrame->pFrameSibling ) 
		UpdateFrameMatrices( pMtxFrame->pFrameSibling, pParentMatrix );
    
    // Move onto first child frame
    if ( pMtxFrame->pFrameFirstChild ) 
		UpdateFrameMatrices( pMtxFrame->pFrameFirstChild, &pMtxFrame->mtxCombined );
}

void dxMeshAnimation::Render() const
{
	if (m_frameRoot)
		DrawFrame(m_frameRoot);
}

void dxMeshAnimation::DrawFrame(LPD3DXFRAME frame) const
{
	// Draw all mesh containers in this frame
    LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
    while (meshContainer)
    {
        DrawMeshContainer(meshContainer, frame);
        meshContainer = meshContainer->pNextMeshContainer;
    }

	// Recurse for sibblings
    if (frame->pFrameSibling != NULL)
        DrawFrame(frame->pFrameSibling);

    // Recurse for children
	if (frame->pFrameFirstChild != NULL)
        DrawFrame(frame->pFrameFirstChild);
}

void dxMeshAnimation::DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase) const
{
	// Cast to our extended frame type
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameBase;		

	// Cast to our extended mesh container
	D3DXMESHCONTAINER_EXTENDED *meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	
	// Set the world transform
    dx_d3dDevice->SetTransform(D3DTS_WORLD, &frame->exCombinedTransformationMatrix);

	// Loop through all the materials in the mesh rendering each subset
    for (unsigned int iMaterial = 0; iMaterial < meshContainer->NumMaterials; iMaterial++)
    {
		// use the material in our extended data rather than the one in meshContainer->pMaterials[iMaterial].MatD3D
		dx_d3dDevice->SetMaterial( &meshContainer->exMaterials[iMaterial] );
		dx_d3dDevice->SetTexture( 0, meshContainer->exTextures[iMaterial] );

		// Select the mesh to draw, if there is skin then use the skinned mesh else the normal one
		LPD3DXMESH pDrawMesh = (meshContainer->pSkinInfo) ? meshContainer->exSkinMesh: meshContainer->MeshData.pMesh;

		// Finally Call the mesh draw function
        pDrawMesh->DrawSubset(iMaterial);
    }
}

void dxMeshAnimation::SetAnimationSet(unsigned int index)
{
	if(m_animController)
	{
		//m_numAnimationSets = m_animController->GetMaxNumAnimationSets();
		if (index==m_currentAnimationSet)
			return;

		if (index>=m_numAnimationSets)
			index=0;

		// Remember current animation
		m_currentAnimationSet=index;

		// Get the animation set from the controller
		LPD3DXANIMATIONSET set;
		m_animController->GetAnimationSet(m_currentAnimationSet, &set );	

		// Alternate tracks
		DWORD newTrack = ( m_currentTrack == 0 ? 1 : 0 );

		// Assign to our track
		m_animController->SetTrackAnimationSet( newTrack, set );
		
		//if(m_numAnimationSets == 0)
			set->Release();	

		// Clear any track events currently assigned to our two tracks
		m_animController->UnkeyAllTrackEvents( m_currentTrack );
		m_animController->UnkeyAllTrackEvents( newTrack );

		// Add an event key to disable the currently playing track kMoveTransitionTime seconds in the future
		m_animController->KeyTrackEnable( m_currentTrack, FALSE, m_currentTime + kMoveTransitionTime );
		// Add an event key to change the speed right away so the animation completes in kMoveTransitionTime seconds
		m_animController->KeyTrackSpeed( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
		// Add an event to change the weighting of the current track (the effect it has blended with the secon track)
		m_animController->KeyTrackWeight( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

		// Enable the new track
		m_animController->SetTrackEnable( newTrack, TRUE );
		// Add an event key to set the speed of the track
		m_animController->KeyTrackSpeed( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
		// Add an event to change the weighting of the current track (the effect it has blended with the first track)
		// As you can see this will go from 0 effect to total effect(1.0f) in kMoveTransitionTime seconds and the first track goes from 
		// total to 0.0f in the same time.
		m_animController->KeyTrackWeight( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

		// Remember current track
		m_currentTrack = newTrack;
    }
}

void dxMeshAnimation::NextAnimation()
{	
	unsigned int newAnimationSet=m_currentAnimationSet+1;
	if (newAnimationSet>=m_numAnimationSets)
		newAnimationSet=0;

	SetAnimationSet(newAnimationSet);
}

std::string dxMeshAnimation::GetAnimationSetName(unsigned int index)
{
	if (index>=m_numAnimationSets)
		return "Error: No set exists";

	// Get the animation set
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	std::string nameString(set->GetName());

	set->Release();

	return nameString;
}

void dxMeshAnimation::AnimateSlower()
{
	m_speedAdjust+=0.1f;
}

void dxMeshAnimation::AnimateFaster()
{
	if (m_speedAdjust>0.1f)
		m_speedAdjust-=0.1f;
}

void dxMeshAnimation::ReleaseMesh()
{
	SAFE_RELEASE(m_animController);
	SAFE_DELETE_ARRAY(m_boneMatrices);
}

bool dxMeshAnimation::ComputeBoundingBox(D3DXMATRIX *matWorld)
{
	BYTE* pVertices=NULL;
	
	if(m_frameRoot->pMeshContainer)
	{
		LPD3DXMESH mesh = m_frameRoot->pMeshContainer->MeshData.pMesh;
		HRESULT hr = mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pVertices);
		
		if (FAILED(hr))
			return FALSE;

		D3DXComputeBoundingBox((D3DXVECTOR3*)pVertices, mesh->GetNumVertices(), D3DXGetFVFVertexSize(mesh->GetFVF()), &minBounds, &maxBounds);

		mesh->UnlockVertexBuffer();
		
		worldBounds[0] = D3DXVECTOR3( minBounds.x, minBounds.y, minBounds.z ); // xyz
		worldBounds[1] = D3DXVECTOR3( maxBounds.x, minBounds.y, minBounds.z ); // Xyz
		worldBounds[2] = D3DXVECTOR3( minBounds.x, maxBounds.y, minBounds.z ); // xYz
		worldBounds[3] = D3DXVECTOR3( maxBounds.x, maxBounds.y, minBounds.z ); // XYz
		worldBounds[4] = D3DXVECTOR3( minBounds.x, minBounds.y, maxBounds.z ); // xyZ
		worldBounds[5] = D3DXVECTOR3( maxBounds.x, minBounds.y, maxBounds.z ); // XyZ
		worldBounds[6] = D3DXVECTOR3( minBounds.x, maxBounds.y, maxBounds.z ); // xYZ
		worldBounds[7] = D3DXVECTOR3( maxBounds.x, maxBounds.y, maxBounds.z ); // XYZ
		
		for( int i = 0; i < 8; i++ )
			D3DXVec3TransformCoord( &worldBounds[i], &worldBounds[i], matWorld );
	     
		CalcAABBFromOBB(worldBounds, &minBounds, &maxBounds);  
		
		return true;
	}
	
	else
		return false;
}

void dxMeshAnimation::CalcAABBFromOBB(const D3DXVECTOR3 *obb, D3DXVECTOR3 *minB, D3DXVECTOR3 *maxB)
{
	assert(minB);
	assert(maxB);
	assert(obb);

	minB->x=maxB->x=obb[0].x;
	minB->y=maxB->y=obb[0].y;
	minB->z=maxB->z=obb[0].z;

	for (int i=1;i<8;i++)
	{
		if (obb[i].x < minB->x) 
			minB->x=obb[i].x;
		if (obb[i].x > maxB->x)
			maxB->x=obb[i].x;
		if (obb[i].y < minB->y) 
			minB->y=obb[i].y;
		if (obb[i].y > maxB->y) 
			maxB->y=obb[i].y;
		if (obb[i].z < minB->z) 
			minB->z=obb[i].z;
		if (obb[i].z > maxB->z) 
			maxB->z=obb[i].z;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT dxMeshAnimation::LoadActorFromX( LPCTSTR FileName, ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice, bool bApplyCustomSets /* = true */, void * pCallbackData /* = NULL */ )
{
	//Load actor from X file for the scene rendering with callback posibility
	HRESULT hRet;
    //CAllocateHierarchy Allocator( this );
	dxMeshHierarchy *memoryAllocator=new dxMeshHierarchy;

    // Validate parameters
    if ( !FileName || !pD3DDevice ) 
		return D3DERR_INVALIDCALL;

    //Release();

    // Store the D3D Device
    dx_d3dDevice = pD3DDevice;
    dx_d3dDevice->AddRef();

    m_nOptions = Options;

    // Load the mesh heirarchy
    hRet = D3DXLoadMeshHierarchyFromX( FileName, Options, dx_d3dDevice, memoryAllocator, NULL, &m_frameRoot, NULL );
    if ( FAILED(hRet) ) return hRet;

    // Copy the filename over
    _tcscpy( m_strActorName, FileName );

    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : SaveActorToX ()
// Desc : Save this actor to file.
// Note : IMPORTANT - This should only be used when using 'manager' meshes.
//        This is to say, if you have remapped the attribute data, and your
//        scene is managing data itself, the material information stored may
//        be corrupted.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SaveActorToX( LPCTSTR FileName, ULONG Format )
{
    HRESULT hRet;

    // Validate parameters
    if ( !FileName ) return D3DERR_INVALIDCALL;

    // If we are NOT managing our own attributes, fail
    if ( GetCallback( dxMeshAnimation::CALLBACK_ATTRIBUTEID ).pFunction != NULL ) return D3DERR_INVALIDCALL;

    // Save the hierarchy back out to file
    hRet = D3DXSaveMeshHierarchyToFile( FileName, Format, m_frameRoot, m_animController, NULL );
    if ( FAILED(hRet) ) return hRet;

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : DrawActor ()
// Desc : Render the currently loaded actor.
//-----------------------------------------------------------------------------
void dxMeshAnimation::DrawActor( )
{
    if ( !IsLoaded() ) return;

    // Draw the frame heirarchy
    DrawFrame( m_frameRoot );
}

//-----------------------------------------------------------------------------
// Name : DrawActorSubset ()
// Desc : Render the specified subset of the currently loaded actor.
//-----------------------------------------------------------------------------
void dxMeshAnimation::DrawActorSubset( ULONG AttributeID )
{
    if ( !IsLoaded() ) return;

    // Draw the frame heirarchy
    DrawFrame( m_frameRoot);
}

//-----------------------------------------------------------------------------
// Name : DrawFrame () (Private)
// Desc : Iteratively called to render a frame in the hierarchy
//-----------------------------------------------------------------------------
void dxMeshAnimation::DrawFrame( LPD3DXFRAME pFrame, long AttributeID /* = -1 */ )
{
    LPD3DXMESHCONTAINER pMeshContainer;
    D3DXFRAME_MATRIX  * pMtxFrame  = (D3DXFRAME_MATRIX*)pFrame;

	// Set the frames combined matrix
    dx_d3dDevice->SetTransform( D3DTS_WORLD, &pMtxFrame->mtxCombined );

    // Loop through the frame's mesh container linked list
    for ( pMeshContainer = pFrame->pMeshContainer; 
          pMeshContainer;
          pMeshContainer = pMeshContainer->pNextMeshContainer )
    {

        // Draw this container
        DrawMeshContainer( pMeshContainer, pFrame, AttributeID );

    } // Next Container

    // Move onto next sibling frame
    if ( pFrame->pFrameSibling ) DrawFrame( pFrame->pFrameSibling, AttributeID );

    // Move onto first child frame 
    if ( pFrame->pFrameFirstChild ) DrawFrame( pFrame->pFrameFirstChild, AttributeID );
}

//-----------------------------------------------------------------------------
// Name : DrawMeshContainer () (Private)
// Desc : Render the contents of the mesh container.
//-----------------------------------------------------------------------------
void dxMeshAnimation::DrawMeshContainer( LPD3DXMESHCONTAINER pMeshContainer, LPD3DXFRAME pFrame, long AttributeID /* = -1 */ )
{
    D3DXFRAME_MATRIX          * pMtxFrame  = (D3DXFRAME_MATRIX*)pFrame;
    D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer;
    CTriMesh                  * pMesh      = pContainer->pMesh;
    
    // Validate requirements
    if ( !pMesh ) return;

    // Render the mesh
    if ( AttributeID >= 0 )
    {
        // Set the FVF for the mesh
        dx_d3dDevice->SetFVF( pMesh->GetFVF() );
        pMesh->DrawSubset( (ULONG)AttributeID );
    
    } // End if attribute specified    
    else
    {
        pMesh->Draw( );
    
    } // End if no attribute specified
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSet ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackAnimationSet( ULONG TrackIndex, LPD3DXANIMATIONSET pAnimSet )
{
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set anim set
    return m_animController->SetTrackAnimationSet( TrackIndex, pAnimSet );
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSetByIndex ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackAnimationSetByIndex( ULONG TrackIndex, ULONG SetIndex )
{
    HRESULT hRet;
    LPD3DXANIMATIONSET pAnimSet = NULL;

    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Retrieve animation set by index
    hRet = m_animController->GetAnimationSet( SetIndex, &pAnimSet );
    if ( FAILED(hRet) ) return hRet;

    // Set anim set
    hRet = m_animController->SetTrackAnimationSet( TrackIndex, pAnimSet );

    // Release the one we retrieved
    pAnimSet->Release();

    // Return result
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSetByName ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackAnimationSetByName( ULONG TrackIndex, LPCTSTR SetName )
{
    HRESULT hRet;
    LPD3DXANIMATIONSET pAnimSet = NULL;

    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Retrieve animation set by name
    hRet = m_animController->GetAnimationSetByName( SetName, &pAnimSet );
    if ( FAILED(hRet) ) return hRet;

    // Set anim set
    hRet = m_animController->SetTrackAnimationSet( TrackIndex, pAnimSet );

    // Release the one we retrieved
    pAnimSet->Release();

    // Return result
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SetTrackPosition ()
// Desc : Set the position within the specified track
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackPosition( ULONG TrackIndex, DOUBLE Position )
{
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the position within the track.
    return m_animController->SetTrackPosition( TrackIndex, Position );
}


//-----------------------------------------------------------------------------
// Name: GetAnimationSetCount ()
// Desc: Retrieve the number of animation sets stored within the anim controller.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetAnimationSetCount( ) const
{
    // Validate
    if ( !m_animController ) return 0;

    // Return the count
    return m_animController->GetNumAnimationSets( );
}

//-----------------------------------------------------------------------------
// Name: GetAnimationSet ()
// Desc: Retrieve the animation set itself by index.
// Note: Automatically adds a reference which you must release!
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET dxMeshAnimation::GetAnimationSet( ULONG Index ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_animController ) return 0;

    // Get the animation set and return it
    if ( FAILED( m_animController->GetAnimationSet( Index, &pAnimSet ) )) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetAnimationSetByName ()
// Desc: Retrieve the animation set itself by name.
// Note: Automatically adds a reference which you must release!
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET dxMeshAnimation::GetAnimationSetByName( LPCTSTR strName ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_animController ) return 0;

    // Get the animation set and return it
    if ( FAILED( m_animController->GetAnimationSetByName( strName, &pAnimSet ) )) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumTracks ()
// Desc: Get the maximum number of mixer tracks currently supported by the
//       animation controller. This can be used to determine if there are
//       enough available for your needs, or whether you need to increase the
//       supported track count by making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetMaxNumTracks( ) const
{
    if ( !m_animController ) return 0;
    return m_animController->GetMaxNumTracks();
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumAnimationSets ()
// Desc: Get the maximum number of animation sets supported by the animation
//       controller. This can be used to determine if there are enough
//       available for your needs, or whether you need to increase the
//       supported set count by making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetMaxNumAnimationSets( ) const
{
    if ( !m_animController ) return 0;
    return m_animController->GetMaxNumAnimationSets( );
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumEvents ()
// Desc: Get the maximum number of events supported by the animation controller
//       This can be used to determine if there are enough available for your 
//       needs, or whether you need to increase the supported set count by 
//       making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetMaxNumEvents( ) const
{
    if ( !m_animController ) return 0;
    return m_animController->GetMaxNumEvents( );
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumAnimationOutputs ()
// Desc: Get the maximum number of outputs supported by the animation controller
//       This can be used to determine if there are enough available for your 
//       needs, or whether you need to increase the supported set count by 
//       making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetMaxNumAnimationOutputs( ) const
{
    if ( !m_animController ) return 0;
    return m_animController->GetMaxNumAnimationOutputs( );
}

//-----------------------------------------------------------------------------
// Name: SetTrackDesc ()
// Desc: Called to manually pass in all track setup details in one call, rather
//       than making many different calls to the SetTrack* functions.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackDesc( ULONG TrackIndex, D3DXTRACK_DESC * pDesc )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetTrackDesc( TrackIndex, pDesc );
}

//-----------------------------------------------------------------------------
// Name: SetTrackEnable ()
// Desc: Enable or disable the specified track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackEnable( ULONG TrackIndex, BOOL Enable )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetTrackEnable( TrackIndex, Enable );
}

//-----------------------------------------------------------------------------
// Name: SetTrackPriority ()
// Desc: Specify the current priority type of this track, i.e. low or high etc.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackPriority( ULONG TrackIndex, D3DXPRIORITY_TYPE Priority )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetTrackPriority( TrackIndex, Priority );
}

//-----------------------------------------------------------------------------
// Name: SetTrackSpeed ()
// Desc: Speed up or slow down the playing animation set in this track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackSpeed( ULONG TrackIndex, float Speed )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetTrackSpeed( TrackIndex, Speed );
}

//-----------------------------------------------------------------------------
// Name: SetTrackWeight ()
// Desc: When mixing is in use, this specifies how 'much' of this track's
//       resulting output data will be used in the mixing process.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetTrackWeight( ULONG TrackIndex, float Weight )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetTrackWeight( TrackIndex, Weight );

}

//-----------------------------------------------------------------------------
// Name: SetPriorityBlend ()
// Desc: Sets the global priority blending weight for the controller. This
//       determines how the High and Low priority tracks are blended together.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::SetPriorityBlend( float BlendWeight )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_animController->SetPriorityBlend( BlendWeight );

}

//-----------------------------------------------------------------------------
// Name: GetTrackAnimationSet ()
// Desc: Retrieve the current animation set currently selected into a track.
// Note: Automatically adds a reference to the set, so it must be released
//       by the caller.
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET dxMeshAnimation::GetTrackAnimationSet( ULONG TrackIndex ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_animController ) return NULL;

    // Get the track details (calls AddRef) and return it
    if ( FAILED(m_animController->GetTrackAnimationSet( TrackIndex, &pAnimSet )) ) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetTrackDesc ()
// Desc: Retrieve all of the properties associated with this track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::GetTrackDesc( ULONG TrackIndex, D3DXTRACK_DESC * pDesc ) const
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Get the details
    return m_animController->GetTrackDesc( TrackIndex, pDesc );
}

//-----------------------------------------------------------------------------
// Name: GetPriorityBlend ()
// Desc: Get the current priority blend weight value for the controller.
//-----------------------------------------------------------------------------
float dxMeshAnimation::GetPriorityBlend( ) const
{
    // Validate
    if ( !m_animController ) return 0.0f;

    // Get the details
    return m_animController->GetPriorityBlend( );
}

//-----------------------------------------------------------------------------
// Name: KeyPriorityBlend ()
// Desc: Insert / sequence an event key for transitioning the priority blending
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::KeyPriorityBlend( float NewBlendWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->KeyPriorityBlend( NewBlendWeight, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackEnable ()
// Desc: Insert / sequence an event key for enabling / disabling a track
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::KeyTrackEnable( ULONG TrackIndex, BOOL NewEnable, double StartTime )
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->KeyTrackEnable( TrackIndex, NewEnable, StartTime );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackPosition ()
// Desc: Insert / sequence an event key for switching to a new position within
//       the specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::KeyTrackPosition( ULONG TrackIndex, double NewPosition, double StartTime )
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->KeyTrackPosition( TrackIndex, NewPosition, StartTime );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackSpeed ()
// Desc: Insert / sequence an event key for altering the playing speed of the
//       specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::KeyTrackSpeed( ULONG TrackIndex, float NewSpeed, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->KeyTrackSpeed( TrackIndex, NewSpeed, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackWeight ()
// Desc: Insert / sequence an event key for transitioning the weight used for
//       mixing the animated output of the specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::KeyTrackWeight( ULONG TrackIndex, float NewWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->KeyTrackWeight( TrackIndex, NewWeight, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: UnkeyPriorityBlends ()
// Desc: Remove all sequenced priority blend keys.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::UnkeyAllPriorityBlends( )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_animController->UnkeyAllPriorityBlends( );
}

//-----------------------------------------------------------------------------
// Name: UnkeyAllTrackEvents ()
// Desc: Remove all sequenced track related event keys for the specified track.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::UnkeyAllTrackEvents( ULONG TrackIndex )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_animController->UnkeyAllTrackEvents( TrackIndex );
}

//-----------------------------------------------------------------------------
// Name: UnkeyEvent ()
// Desc: Remove the specified event only.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::UnkeyEvent( D3DXEVENTHANDLE hEvent )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_animController->UnkeyEvent( hEvent );
}

//-----------------------------------------------------------------------------
// Name: ValidateEvent ()
// Desc: Ensures that the specified event handle is valid, and that it has not
//       yet been triggered and removed. (This will validate running events
//       successfully, but all those which have finished will fail).
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::ValidateEvent( D3DXEVENTHANDLE hEvent )
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_animController->ValidateEvent( hEvent );
}

//-----------------------------------------------------------------------------
// Name: GetCurrentTrackEvent ()
// Desc: Retrieve the handle of any event currently running on the specified
//       track, of the specified type.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::GetCurrentTrackEvent( ULONG TrackIndex, D3DXEVENT_TYPE EventType ) const
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->GetCurrentTrackEvent( TrackIndex, EventType );
}

//-----------------------------------------------------------------------------
// Name: GetCurrentPriorityBlend ()
// Desc: Get the event handle of any priority blend currently running on the
//       global timeline.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE dxMeshAnimation::GetCurrentPriorityBlend( ) const
{
    // Validate
    if ( !m_animController ) return NULL;

    // Send the details
    return m_animController->GetCurrentPriorityBlend( );
}

//-----------------------------------------------------------------------------
// Name: GetEventDesc ()
// Desc: Retrieve the descriptor for the event.
//-----------------------------------------------------------------------------
HRESULT dxMeshAnimation::GetEventDesc( D3DXEVENTHANDLE hEvent, LPD3DXEVENT_DESC pDesc ) const
{
    // Validate
    if ( !m_animController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_animController->GetEventDesc( hEvent, pDesc );
}

//-----------------------------------------------------------------------------
// Name : GetCallback ()
// Desc : Retrieve a currently registered callback.
//-----------------------------------------------------------------------------
CALLBACK_FUNC dxMeshAnimation::GetCallback( CALLBACK_TYPE Type ) const
{
    return m_CallBack[Type];
}

//-----------------------------------------------------------------------------
// Name : GetOptions ()
// Desc : Retrieve the options requested via Load*.
//-----------------------------------------------------------------------------
ULONG dxMeshAnimation::GetOptions( ) const
{
    return m_nOptions;
}

//-----------------------------------------------------------------------------
// Name : SetWorldMatrix ()
// Desc : Updates the actor using the specified matrix.
// Note : You can pass NULL here. Doing so will assume identity.
//-----------------------------------------------------------------------------
void dxMeshAnimation::SetWorldMatrix( const D3DXMATRIX * mtxWorld /* = NULL */, bool UpdateFrames /* = false */ )
{
    // Store the currently set world matrix
    if ( mtxWorld )
        m_mtxWorld = *mtxWorld;
    else
        D3DXMatrixIdentity( &m_mtxWorld );

    // Update the frame matrices
    if ( IsLoaded() && UpdateFrames ) UpdateFrameMatrices( m_frameRoot, mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: AdvanceTime ()
// Desc: Progresses the time forward by the specified amount
//-----------------------------------------------------------------------------
void dxMeshAnimation::AdvanceTime( double fTimeElapsed, bool UpdateFrames /* = true */, LPD3DXANIMATIONCALLBACKHANDLER pCallbackHandler /* = NULL */ )
{
    if ( !IsLoaded() ) return;

    // Set the current time if applicable
    if ( m_animController ) m_animController->AdvanceTime( fTimeElapsed, pCallbackHandler );

    // Update the frame matrices
    if ( UpdateFrames ) UpdateFrameMatrices( m_frameRoot, &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: ResetTime ()
// Desc: Reset the global time to 0.0, whilst retaining the tracks periodic
//       position.
//-----------------------------------------------------------------------------
void dxMeshAnimation::ResetTime( bool UpdateFrames /* = false */ )
{
    if ( !IsLoaded() ) return;

    // Set the current time if applicable
    if ( m_animController ) m_animController->ResetTime( );

    // Update the frame matrices
    if ( UpdateFrames ) UpdateFrameMatrices( m_frameRoot, &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: GetTime ()
// Desc: Retrieve the current time of the animation.
//-----------------------------------------------------------------------------
double dxMeshAnimation::GetTime( ) const
{
    if ( !m_animController) return 0.0f;
    return m_animController->GetTime();
}

//-----------------------------------------------------------------------------
// Name: GetActorName ()
// Desc: Retrieve the filename used to load this actor.
//-----------------------------------------------------------------------------
LPCTSTR dxMeshAnimation::GetActorName( ) const
{
    return m_strActorName;
}

//-----------------------------------------------------------------------------
// Name: GetDevice ()
// Desc: Retrieve the D3D device being used by the actor
// Note: Adds a reference when it returns, so you must release when you're done
//-----------------------------------------------------------------------------
LPDIRECT3DDEVICE9 dxMeshAnimation::GetDevice( ) const
{
    dx_d3dDevice->AddRef();
    return dx_d3dDevice;
}

//-----------------------------------------------------------------------------
// Name: GetRootFrame ()
// Desc: Return the root frame.
//-----------------------------------------------------------------------------
LPD3DXFRAME dxMeshAnimation::GetRootFrame( ) const
{
    return m_frameRoot;
}

//-----------------------------------------------------------------------------
// Name: GetFrameByName ()
// Desc: Retrieve the filename used to load this actor.
//-----------------------------------------------------------------------------
LPD3DXFRAME dxMeshAnimation::GetFrameByName( LPCTSTR strName, LPD3DXFRAME pFrame /* = NULL */ ) const
{
    LPD3DXFRAME pRetFrame = NULL;

    // Any start frame passed in?
    if ( !pFrame ) pFrame = m_frameRoot;

    // Does this match ?
    if ( pFrame->Name && _tcsicmp( strName, pFrame->Name ) == 0 ) return pFrame;

    // Check sibling
    if ( pFrame->pFrameSibling ) 
    {
        pRetFrame = GetFrameByName( strName, pFrame->pFrameSibling );
        if ( pRetFrame ) return pRetFrame;

    } // End if has sibling

    // Check child
    if ( pFrame->pFrameFirstChild ) 
    {
        pRetFrame = GetFrameByName( strName, pFrame->pFrameFirstChild );
        if ( pRetFrame ) return pRetFrame;

    } // End if has sibling

    // Nothing found
    return NULL;
}

//-----------------------------------------------------------------------------
// Name : CreateFrame ()
// Desc : Called by D3DX to inform us that we are required to allocate a new
//        frame in any way we wish to do so, and pass that pointer back.
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateFrame(LPCTSTR Name, LPD3DXFRAME *ppNewFrame)
{
    D3DXFRAME_MATRIX * pNewFrame = NULL;

    // Clear out the passed frame (it may not be NULL)
    *ppNewFrame = NULL;
    
    // Allocate a new frame
    pNewFrame = new D3DXFRAME_MATRIX;
    if ( !pNewFrame ) return E_OUTOFMEMORY;

    // Clear out the frame
    ZeroMemory( pNewFrame, sizeof(D3DXFRAME_MATRIX) );

    // Copy over, and default other values.
    if ( Name ) pNewFrame->Name = _tcsdup( Name );
    D3DXMatrixIdentity( &pNewFrame->TransformationMatrix );

    // Pass this new pointer back out
    *ppNewFrame = (D3DXFRAME*)pNewFrame;

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : CreateMeshContainer ()
// Desc : Called by D3DX to inform us that we are required to allocate a new
//        mesh container, and initialize it as we wish.
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateMeshContainer(LPCTSTR Name, CONST D3DXMESHDATA * pMeshData, CONST D3DXMATERIAL * pMaterials,
                                                CONST D3DXEFFECTINSTANCE *pEffectInstances, DWORD NumMaterials, CONST DWORD *pAdjacency,
                                                LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER *ppNewMeshContainer) 
{
    ULONG                      AttribID, i;
    HRESULT                    hRet;
    LPD3DXMESH                 pMesh          = NULL;
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = NULL;
    LPDIRECT3DDEVICE9          pDevice        = NULL;
    CTriMesh                 *pNewMesh       = NULL;
    MESH_ATTRIB_DATA          *pAttribData    = NULL;
    ULONG                     *pAttribRemap   = NULL;
    bool                       ManageAttribs  = false;
    bool                       RemapAttribs   = false;
    CALLBACK_FUNC              Callback;

    // We only support standard meshes (i.e. no patch or progressive meshes in this demo)
    if ( pMeshData->Type != D3DXMESHTYPE_MESH ) return E_FAIL;

    // Extract the standard mesh from the structure
    pMesh = pMeshData->pMesh;

    // We require FVF compatible meshes only
    if ( pMesh->GetFVF() == 0 ) return E_FAIL;

    // Allocate a mesh container structure
    pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
    if ( !pMeshContainer ) return E_OUTOFMEMORY;

    // Clear out the structure to begin with
    ZeroMemory( pMeshContainer, sizeof(D3DXMESHCONTAINER_DERIVED) );

    // Copy over the name. We can't simply copy the pointer here because the memory
    // for the string belongs to the caller (D3DX)
    if ( Name ) pMeshContainer->Name = _tcsdup( Name );

    // Allocate a new CTriMesh
    pNewMesh = new CTriMesh;
    if ( !pNewMesh ) { hRet = E_OUTOFMEMORY; goto ErrorOut; }

    // If there are no normals, this demo requires them, so add them to the mesh's FVF
    if ( !(pMesh->GetFVF() & D3DFVF_NORMAL) || (pMesh->GetOptions() != m_pActor->GetOptions()) )
    {
        LPD3DXMESH pCloneMesh = NULL;

        // Retrieve the mesh's device (this adds a reference)
        pMesh->GetDevice( &pDevice );

        // Clone the mesh
        hRet = pMesh->CloneMeshFVF( m_pActor->GetOptions(), pMesh->GetFVF() | D3DFVF_NORMAL, pDevice, &pCloneMesh );
        if ( FAILED( hRet ) ) goto ErrorOut;

        // Note: we don't release the old mesh here, because we don't own it
        pMesh = pCloneMesh;

        // Compute the normals for the new mesh if there was no normal to begin with
        if ( !(pMesh->GetFVF() & D3DFVF_NORMAL) ) D3DXComputeNormals( pMesh, pAdjacency );

        // Release the device, we're done with it
        pDevice->Release();
        pDevice = NULL;

        // Attach our specified mesh to the new mesh (this addref's the chosen mesh)
        pNewMesh->Attach( pCloneMesh );

        // We can release the cloned mesh here, it's owned by our CTriMesh
        pCloneMesh->Release();
    
    } // End if no vertex normal, or options are hosed.
    else
    {
        // Simply attach our specified mesh to the new mesh (this addref's the chosen mesh)
        pNewMesh->Attach( pMesh );
    
    } // End if vertex normals

    // Are we managing our own attributes ?
    ManageAttribs = (m_pActor->GetCallback( dxMeshAnimation::CALLBACK_ATTRIBUTEID ).pFunction == NULL);
    
    // Allocate the attribute data if this is a manager mesh
    if ( ManageAttribs == true && NumMaterials > 0 )
    {
        if ( pNewMesh->AddAttributeData( NumMaterials ) < -1 ) { hRet = E_OUTOFMEMORY; goto ErrorOut; }
        pAttribData = pNewMesh->GetAttributeData();
    
    } // End if managing attributes
    else
    {
        // Allocate attribute remap array
        pAttribRemap = new ULONG[ NumMaterials ];
        if ( !pAttribRemap ) { hRet = E_OUTOFMEMORY; goto ErrorOut; }

        // Default remap to their initial values.
        for ( i = 0; i < NumMaterials; ++i ) pAttribRemap[ i ] = i;
    
    } // End if not managing attributes

    // Loop through and process the attribute data
    for ( i = 0; i < NumMaterials; ++i )
    {
        if ( ManageAttribs == true )
        {
            // Store material
            pAttribData[i].Material = pMaterials[i].MatD3D;

            // Note : The X File specification contains no ambient material property.
            //        We should ideally set this to full intensity to allow us to 
            //        control ambient brightness via the D3DRS_AMBIENT renderstate.
            pAttribData[i].Material.Ambient = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

            // Request texture pointer via callback
			Callback = m_pActor->GetCallback( dxMeshAnimation::CALLBACK_TEXTURE );
            if ( Callback.pFunction )
            {
                COLLECTTEXTURE CollectTexture = (COLLECTTEXTURE)Callback.pFunction;
                pAttribData[i].Texture = CollectTexture( Callback.pContext, pMaterials[i].pTextureFilename );

                // Add reference. We are now using this
                if ( pAttribData[i].Texture ) pAttribData[i].Texture->AddRef();
        
            } // End if callback available

            // Request effect pointer via callback
            Callback = m_pActor->GetCallback( dxMeshAnimation::CALLBACK_EFFECT );
            if ( Callback.pFunction )
            {
                COLLECTEFFECT CollectEffect = (COLLECTEFFECT)Callback.pFunction;
                pAttribData[i].Effect = CollectEffect( Callback.pContext, pEffectInstances[i] );

                // Add reference. We are now using this
                if ( pAttribData[i].Effect ) pAttribData[i].Effect->AddRef();
        
            } // End if callback available

        } // End if attributes are managed
        else
        {
            // Request attribute ID via callback
            Callback = m_pActor->GetCallback( dxMeshAnimation::CALLBACK_ATTRIBUTEID );
            if ( Callback.pFunction )
            {
                COLLECTATTRIBUTEID CollectAttributeID = (COLLECTATTRIBUTEID)Callback.pFunction;
                AttribID = CollectAttributeID( Callback.pContext, pMaterials[i].pTextureFilename,
                                               &pMaterials[i].MatD3D, &pEffectInstances[i] );

                // Store this in our attribute remap table
                pAttribRemap[i] = AttribID;

                // Determine if any changes are required so far
                if ( AttribID != i ) RemapAttribs = true;
        
            } // End if callback available

        } // End if we don't manage attributes

    } // Next Material

    // Remap attributes if required
    if ( pAttribRemap != NULL && RemapAttribs == true )
    {
        ULONG * pAttributes = NULL;

        // Lock the attribute buffer
        hRet = pMesh->LockAttributeBuffer( 0, &pAttributes );
        if ( FAILED(hRet) ) goto ErrorOut;

        // Loop through all faces
        for ( i = 0; i < pMesh->GetNumFaces(); ++i )
        {
            // Retrieve the current attribute ID for this face
            AttribID = pAttributes[i];
        
            // Replace it with the remap value
            pAttributes[i] = pAttribRemap[AttribID];
        
        } // Next Face

        // Finish up
        pMesh->UnlockAttributeBuffer( );

    } // End if remap attributes

    // Release remap data
    if ( pAttribRemap ) delete []pAttribRemap;

    // Attempt to optimize the new mesh
    pNewMesh->WeldVertices( 0 );
    pNewMesh->OptimizeInPlace( D3DXMESHOPT_VERTEXCACHE );
    
    // Store our mesh in the container
    pMeshContainer->pMesh = pNewMesh;

    // Store the details so that the save functions have access to them.
    pMeshContainer->MeshData.pMesh = pNewMesh->GetMesh();
    pMeshContainer->MeshData.Type  = D3DXMESHTYPE_MESH;
    pMeshContainer->NumMaterials   = NumMaterials;

    // Copy over material data only if in managed mode (i.e. we can save)
    if ( NumMaterials > 0 && ManageAttribs == true )
    {
        // Allocate material array
        pMeshContainer->pMaterials = new D3DXMATERIAL[ NumMaterials ];

        // Loop through and copy
        for ( i = 0; i < NumMaterials; ++i )
        {
            pMeshContainer->pMaterials[i].MatD3D = pMaterials[i].MatD3D;
            pMeshContainer->pMaterials[i].pTextureFilename = _tcsdup( pMaterials[i].pTextureFilename );

        } // Next Material
        
    } // End if any materials to copy

    // Store this new mesh container pointer
    *ppNewMeshContainer = (D3DXMESHCONTAINER*)pMeshContainer;

    // Success!!
    return D3D_OK;

ErrorOut:
    // If we drop here, something failed
    DestroyMeshContainer( pMeshContainer );
    
    if ( pDevice      ) pDevice->Release();
    if ( pAttribRemap ) delete []pAttribRemap;
    if ( pNewMesh     ) delete pNewMesh;

    // Failed....
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : DestroyFrame ()
// Desc : Called simply to destroy the specified frame.
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyFrame( LPD3DXFRAME pFrameToFree ) 
{
    D3DXFRAME_MATRIX * pMtxFrame = (D3DXFRAME_MATRIX*)pFrameToFree;

    // Validate Parameters
    if ( !pMtxFrame ) return D3D_OK;

    // Release data
    if ( pMtxFrame->Name ) free( pMtxFrame->Name ); // '_tcsdup' allocated.
    delete pMtxFrame;                               // 'new' allocated.

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : DestroyMeshContainer ()
// Desc : Called simply to destroy the specified mesh container.
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyMeshContainer( LPD3DXMESHCONTAINER pContainerToFree ) 
{
    ULONG i;
    D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pContainerToFree;

    // Validate Parameters
    if ( !pContainer ) return D3D_OK;

    // Release material data
    if ( pContainer->pMaterials )
    {
        for ( i = 0; i < pContainer->NumMaterials; ++i )
        {
            // Release the string data
            if ( pContainer->pMaterials[i].pTextureFilename ) free( pContainer->pMaterials[i].pTextureFilename );
        
        } // Next Material

        // Destroy the array
        delete []pContainer->pMaterials;
        pContainer->pMaterials = NULL;

    } // End if any material data

    // Release other data
    if ( pContainer->MeshData.pMesh ) pContainer->MeshData.pMesh->Release();
    if ( pContainer->Name ) free( pContainer->Name );   // '_tcsdup' allocated.
    if ( pContainer->pMesh ) delete pContainer->pMesh;
    delete pContainer;

    // Success!!
    return D3D_OK;
}
