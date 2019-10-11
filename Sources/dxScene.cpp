#include "..\Includes\dxScene.h"
#include "..\Includes\dxCamera.h"
#include "..\Includes\dxObject.h"
#include "..\Includes\dxMeshAnimation.h"
#include "..\Includes\dxtimer.h"

namespace
{
    const UCHAR Wheel_FL          = 0;
    const UCHAR Wheel_FR          = 1;
    const UCHAR Wheel_BL          = 2;
    const UCHAR Wheel_BR          = 3;
    const UCHAR Wheel_Pivot_FL    = 4;
    const UCHAR Wheel_Pivot_FR    = 5;
    float       fWheelYaw         = 0.0f;
    float       fWheelYawVelocity = 1000.0f;

};
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
dxScene::dxScene()
{
    m_pCamera            = NULL;
    m_p3rdPersonObject   = NULL;
    m_CameraMode         = 0;
    m_nUpdatePlayerCount = 0;
    m_nUpdateCameraCount = 0;

    m_vecPos             = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecRight           = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp              = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook            = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

    m_vecCamOffset       = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_fCameraLag         = 0.0f;

    m_vecVelocity        = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecGravity         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_fMaxVelocityXZ     = 125.0f;
    m_fMaxVelocityY      = 125.0f;
    m_fFriction          = 250.0f;

    m_Volume.Min         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_Volume.Max         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	modelEntity = 0;
	m_entityYaw = 0;
	m_entityPitch = 0;
	m_entityRoll = 0;

	m_nObjectCount     = 0;
    m_pObject          = NULL;
	
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
dxScene::~dxScene()
{
    if ( m_pCamera ) 
		delete m_pCamera;
    
    m_pCamera           = NULL;
    m_p3rdPersonObject  = NULL;

}

//-----------------------------------------------------------------------------
// Move
//-----------------------------------------------------------------------------
void dxScene::Move( ULONG Direction, float Distance, bool Velocity )
{
    D3DXVECTOR3 vecShift = D3DXVECTOR3( 0, 0, 0 );

    if ( Direction & DIR_FORWARD  ) vecShift += m_vecLook  * Distance;
    if ( Direction & DIR_BACKWARD ) vecShift -= m_vecLook  * Distance;
    if ( Direction & DIR_RIGHT    ) vecShift += m_vecRight * Distance;
    if ( Direction & DIR_LEFT     ) vecShift -= m_vecRight * Distance;
    if ( Direction & DIR_UP       ) vecShift += m_vecUp    * Distance;
    if ( Direction & DIR_DOWN     ) vecShift -= m_vecUp    * Distance;

    if ( Direction ) 
		Move( vecShift, Velocity );
}

//-----------------------------------------------------------------------------
// Move ()
//-----------------------------------------------------------------------------
void dxScene::Move( const D3DXVECTOR3& vecShift, bool Velocity )
{
    if ( Velocity )
    {
        m_vecVelocity += vecShift;
    }
    else
    {
        m_vecPos += vecShift;
		modelEntityPos = m_vecPos;
        m_pCamera->Move( vecShift );
    }
}

//-----------------------------------------------------------------------------
// Name : SetCamOffset ()
//-----------------------------------------------------------------------------
void dxScene::SetCamOffset( const D3DXVECTOR3& Offset )
{
    m_vecCamOffset = Offset;

    if (!m_pCamera) return;
    m_pCamera->SetPosition( m_vecPos + Offset );
}

//-----------------------------------------------------------------------------
// Update ()
//-----------------------------------------------------------------------------
void dxScene::Update( float TimeScale )
{
    ULONG i;

    m_vecVelocity += m_vecGravity * TimeScale;

    float Length = sqrtf(m_vecVelocity.x * m_vecVelocity.x + m_vecVelocity.z * m_vecVelocity.z);
    if ( Length > m_fMaxVelocityXZ )
    {
        m_vecVelocity.x *= ( m_fMaxVelocityXZ / Length );
        m_vecVelocity.z *= ( m_fMaxVelocityXZ / Length );
    
    }

    Length = sqrtf(m_vecVelocity.y * m_vecVelocity.y);
    if ( Length > m_fMaxVelocityY )
    {
        m_vecVelocity.y *= ( m_fMaxVelocityY / Length );
    }
        
    Move( m_vecVelocity * TimeScale, false );

    for ( i =0; i < m_nUpdatePlayerCount; i++ )
    {
        UPDATESCENE UpdatePlayer = (UPDATESCENE)m_pUpdatePlayer[i].pFunction;
        UpdatePlayer( m_pUpdatePlayer[i].pContext, this, TimeScale );

    }

    m_pCamera->Update( TimeScale, m_fCameraLag );

    for ( i =0; i < m_nUpdateCameraCount; i++ )
    {
        UPDATECAMERA UpdateCamera = (UPDATECAMERA)m_pUpdateCamera[i].pFunction;
        UpdateCamera( m_pUpdateCamera[i].pContext, m_pCamera, TimeScale );

    }

    D3DXVECTOR3 vecDec = -m_vecVelocity;
    D3DXVec3Normalize( &vecDec, &vecDec );

    Length = D3DXVec3Length( &m_vecVelocity );

    float Dec = (m_fFriction * TimeScale);
    if ( Dec > Length ) Dec = Length;

    m_vecVelocity += vecDec * Dec;

}

//-----------------------------------------------------------------------------
// SetCameraMode ()
//-----------------------------------------------------------------------------
bool dxScene::SetCameraMode( ULONG Mode )
{
    dxCamera * pNewCamera = NULL;

    if ( m_pCamera && m_CameraMode == Mode ) return true;

    switch ( Mode )
    {
        case dxCamera::MODE_FPS:
            
            if ( !(pNewCamera = new dxCam1stPerson( m_pCamera ))) return false;
            break;

        case dxCamera::MODE_THIRDPERSON_SC:
        case dxCamera::MODE_THIRDPERSON:

            if ( !(pNewCamera = new dxCam3rdPerson( m_pCamera ))) return false;
            break;

        case dxCamera::MODE_SPACECRAFT:
            
            if ( !(pNewCamera = new dxCamSpaceCraft( m_pCamera ))) return false;
            break;
    
    }

    if (!pNewCamera) return false;

    if ( m_CameraMode == dxCamera::MODE_SPACECRAFT || m_CameraMode == dxCamera::MODE_THIRDPERSON_SC )
    {
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );

        m_fPitch = 0.0f;
        m_fRoll  = 0.0f;
        m_fYaw   = D3DXToDegree( acosf( D3DXVec3Dot( &D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), &m_vecLook ) ) );
        if ( m_vecLook.x < 0.0f ) m_fYaw = -m_fYaw;

    }
    else if ( m_pCamera && Mode == dxCamera::MODE_SPACECRAFT || Mode == dxCamera::MODE_THIRDPERSON_SC )
    {
        m_vecRight = m_pCamera->GetRight();
        m_vecLook  = m_pCamera->GetLook();
        m_vecUp    = m_pCamera->GetUp();
    }
    m_CameraMode = Mode;

    pNewCamera->AttachToScene( this );

    if ( m_pCamera ) delete m_pCamera;
    m_pCamera = pNewCamera;

    return true;
}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the player around the players local axis.
//-----------------------------------------------------------------------------
void dxScene::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    if (!m_pCamera) return;

    if ( m_CameraMode == dxCamera::MODE_FPS || m_CameraMode == dxCamera::MODE_THIRDPERSON )
    {
        if ( x )
        {
            m_fPitch += x;
            if ( m_fPitch >  89.0f ) { x -= (m_fPitch - 89.0f); m_fPitch = 89.0f; }
            if ( m_fPitch < -89.0f ) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
        } 

        if ( y )
        {
            m_fYaw += y;
            if ( m_fYaw >  360.0f ) m_fYaw -= 360.0f;
            if ( m_fYaw <  0.0f   ) m_fYaw += 360.0f;
        } 

        if ( z ) 
        {
            m_fRoll += z;
            if ( m_fRoll >  20.0f ) { z -= (m_fRoll - 20.0f); m_fRoll = 20.0f; }
            if ( m_fRoll < -20.0f ) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
        }

        m_pCamera->Rotate( x, y, z );

        if ( y )
        {
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecUp, D3DXToRadian( y ) );
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
        }
    }
    else if ( m_CameraMode == dxCamera::MODE_SPACECRAFT || m_CameraMode == dxCamera::MODE_THIRDPERSON_SC )
    {
        m_pCamera->Rotate( x, y, z );
        if ( x != 0 ) 
        {
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecRight, D3DXToRadian( x ) );
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        }

        if ( y != 0 ) 
        {
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecUp, D3DXToRadian( y ) );
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
        }

        if ( z != 0 ) 
        {
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecLook, D3DXToRadian( z ) );
            D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
        }
    } 

    D3DXVec3Normalize( &m_vecLook, &m_vecLook );
    D3DXVec3Cross( &m_vecRight, &m_vecUp, &m_vecLook );
    D3DXVec3Normalize( &m_vecRight, &m_vecRight );
    D3DXVec3Cross( &m_vecUp, &m_vecLook, &m_vecRight );
    D3DXVec3Normalize( &m_vecUp, &m_vecUp );
}

void dxScene::SetVolumeInfo( const VOLUME_INFO2& Volume )
{
    m_Volume = Volume;
}

const VOLUME_INFO2& dxScene::GetVolumeInfo( ) const
{
    return m_Volume;
}

void dxScene::AddSceneCallback( UPDATESCENE pFunc, LPVOID pContext )
{
    m_pUpdatePlayer[m_nUpdatePlayerCount].pFunction = (LPVOID)pFunc;
    m_pUpdatePlayer[m_nUpdatePlayerCount].pContext  = pContext;
    m_nUpdatePlayerCount++;
}

void dxScene::AddCameraCallback( UPDATECAMERA pFunc, LPVOID pContext )
{
    // Store callback details
    m_pUpdateCamera[m_nUpdateCameraCount].pFunction = (LPVOID)pFunc;
    m_pUpdateCamera[m_nUpdateCameraCount].pContext  = pContext;
    m_nUpdateCameraCount++;
}

void dxScene::RemoveSceneCallback( UPDATESCENE pFunc, LPVOID pContext )
{
    int i;

    for ( i = 0; i < m_nUpdatePlayerCount; i++ )
    {
        if ( m_pUpdatePlayer[i].pFunction == pFunc && m_pUpdatePlayer[i].pContext == pContext ) break;

    }

    if ( i == m_nUpdatePlayerCount ) return;

    if ( i < (m_nUpdatePlayerCount - 1))
    {
        memmove( &m_pUpdatePlayer[i], &m_pUpdatePlayer[i + 1], (m_nUpdatePlayerCount - 2) * sizeof(CALLBACK_FUNC) );
    
    } 

	m_nUpdatePlayerCount--;
}

void dxScene::RemoveCameraCallback( UPDATECAMERA pFunc, LPVOID pContext )
{
    int i;

    for ( i = 0; i < m_nUpdateCameraCount; i++ )
    {
        if ( m_pUpdateCamera[i].pFunction == pFunc && m_pUpdateCamera[i].pContext == pContext ) break;
    }

    if ( i == m_nUpdateCameraCount ) return;

    if ( i < (m_nUpdateCameraCount - 1))
    {
        memmove( &m_pUpdateCamera[i], &m_pUpdateCamera[i + 1], (m_nUpdateCameraCount - 2) * sizeof(CALLBACK_FUNC) );
    }

    m_nUpdateCameraCount--;
}

bool dxScene::LoadXFile(const std::string &filename,int startAnimation, int model, LPDIRECT3DDEVICE9 pDevice, TCHAR * strFileName)
{
	HRESULT hRet;

	switch(model)
	{
		case 1: 
			{
				if (modelEntity)
				{
					delete modelEntity;
					modelEntity=0;
				}

				modelEntity = new dxMeshAnimation(pDevice);
				/*if (!modelEntity->Load(filename))
				{
					delete modelEntity;
					modelEntity=0;
					return false;
				}*/

				modelEntity->SetAnimationSet(startAnimation);

				//ACTOR
				dxMeshAnimation *pNewActor = new dxMeshAnimation(pDevice);
				if (!pNewActor) 
					return false;

				//pNewActor->RegisterCallback( dxMeshAnimation::CALLBACK_ATTRIBUTEID, CollectAttributeID, this );
				hRet = modelEntity->LoadActorFromX( strFileName, D3DXMESH_MANAGED, pDevice );
				if ( FAILED(hRet) ) 
				{ 
					delete pNewActor; 
					return false; 
				}

				if ( AddActor( ) < 0 ) 
				{ 
					delete pNewActor; 
					return false; 
				}

				m_pActor[ m_nActorCount - 1 ] = pNewActor;

				//OBJECT
				CObject *pNewObject = new CObject( pNewActor );
				if ( !pNewObject ) 
					return false;

				if ( AddObject() < 0 ) 
				{ 
					delete pNewObject; 
					return false; 
				}

				m_pObject[ m_nObjectCount - 1 ] = pNewObject;

				//BuildMeshAnimation();
			}
		break;

		case 2: 
			{
				if (!strFileName) 
					return false;
			    
				if ( m_strDataPath ) 
					free( m_strDataPath );
				m_strDataPath = _tcsdup( strFileName );
			    
				TCHAR * LastSlash = _tcsrchr( m_strDataPath, _T('\\') );
				if (!LastSlash) 
					LastSlash = _tcsrchr( m_strDataPath, _T('/') );
				if (LastSlash) 
					LastSlash[1] = _T('\0'); else m_strDataPath[0] = _T('\0');

				dxMeshAnimation *pNewActor = new dxMeshAnimation(pDevice);
				if (!pNewActor) 
					return false;

				hRet = pNewActor->LoadActorFromX( strFileName, D3DXMESH_MANAGED, pDevice );
				if ( FAILED(hRet) ) 
				{ 
					delete pNewActor; 
					return false; 
				}

				if ( AddActor( ) < 0 ) 
				{ 
					delete pNewActor; 
					return false; 
				}
				m_pActor[ m_nActorCount - 1 ] = pNewActor;

				CObject * pNewObject = new CObject( pNewActor );
				if ( !pNewObject ) 
					return false;

				if ( AddObject() < 0 ) 
				{ 
					delete pNewObject; 
					return false; 
				}
				m_pObject[ m_nObjectCount - 1 ] = pNewObject;
			}
		break;
		
		default:
			break;
	}
	
	modelEntityPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	
	return true;
}

void dxScene::BuildMeshAnimation ( )
{
	ULONG   i, j;
    LPCTSTR FrameNames[] = { _T("Wheel_FL"), _T("Wheel_FR"), 
                             _T("Wheel_BL"), _T("Wheel_BR"),
                             _T("Wheel_Pivot_FL"), _T("Wheel_Pivot_FR") };

    for ( i = 0; i < 6; ++i )
    {
        for ( j = 0; j < m_nObjectCount; ++j )
        {
            CObject * pObject = m_pObject[j];
            if ( !pObject ) 
				continue;
			modelEntity = pObject->m_pActor;
            if ( !modelEntity) 
				continue;
            LPD3DXFRAME pFrame = modelEntity->GetFrameByName( FrameNames[i] );

			if ( pFrame )
            {
                m_pAnimList[i].Attach( &pFrame->TransformationMatrix, FrameNames[i] );
                break;
            }
            else
            {
                m_pAnimList[i].Attach( NULL );
            }
        }
    }

	m_pAnimList[ Wheel_Pivot_FL ].RotationY( D3DXToRadian( fWheelYaw ) );
    m_pAnimList[ Wheel_Pivot_FR ].RotationY( D3DXToRadian( fWheelYaw ) );
}

void dxScene::AnimateMesh ( dxTimer & Timer )
{
    float fRevsPerSecond = 2.0f, fRadAngle;

    fRadAngle = D3DXToRadian( (360.0f * fRevsPerSecond) * Timer.GetTimeElapsed() );

    m_pAnimList[ Wheel_FL ].RotationX( fRadAngle );
    m_pAnimList[ Wheel_FR ].RotationX( fRadAngle );
    m_pAnimList[ Wheel_BL ].RotationX( fRadAngle );
    m_pAnimList[ Wheel_BR ].RotationX( fRadAngle );
}

//-----------------------------------------------------------------------------
// Name : Render ()
//-----------------------------------------------------------------------------
void dxScene::Render( LPDIRECT3DDEVICE9 pDevice )
{
    CObjectC * pObject = NULL;

    // Validate Parameters
    if (!pDevice) return;

    // Select which object to render
    if ( m_pCamera )
    {
        if ( m_CameraMode == dxCamera::MODE_THIRDPERSON || m_CameraMode == dxCamera::MODE_THIRDPERSON_SC ) 
			pObject = m_p3rdPersonObject;
    
    }
    else
    {
        pObject = m_p3rdPersonObject;
    }
    
    // Validate
    if (!pObject) return;
    
    // Update our object's world matrix
    D3DXMATRIX * pMatrix = &pObject->m_mtxWorld;
    pMatrix->_11 = m_vecRight.x; pMatrix->_21 = m_vecUp.x; pMatrix->_31 = m_vecLook.x;
	pMatrix->_12 = m_vecRight.y; pMatrix->_22 = m_vecUp.y; pMatrix->_32 = m_vecLook.y;
	pMatrix->_13 = m_vecRight.z; pMatrix->_23 = m_vecUp.z; pMatrix->_33 = m_vecLook.z;

	pMatrix->_41 = m_vecPos.x;
	pMatrix->_42 = m_vecPos.y;
	pMatrix->_43 = m_vecPos.z;

    // Render our player mesh object
    CMeshC * pMesh = pObject->m_pMesh;
    pDevice->SetTransform( D3DTS_WORLD, &pObject->m_mtxWorld );
    pDevice->SetStreamSource( 0, pMesh->m_pVertexBuffer, 0, sizeof(CVertexC) );
    pDevice->SetIndices( pMesh->m_pIndexBuffer );
    pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8, 0, 14 );
    
}

void dxScene::RenderMesh( LPDIRECT3DDEVICE9 pDevice,float timeElapsed )
{
	if (!pDevice) 
		return;

	//pDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

    D3DXMATRIX pMatrix;
    pMatrix._11 = m_vecRight.x; pMatrix._21 = m_vecUp.x; pMatrix._31 = m_vecLook.x;
	pMatrix._12 = m_vecRight.y; pMatrix._22 = m_vecUp.y; pMatrix._32 = m_vecLook.y;
	pMatrix._13 = m_vecRight.z; pMatrix._23 = m_vecUp.z; pMatrix._33 = m_vecLook.z;

    pMatrix._41 = m_vecPos.x;
    pMatrix._42 = m_vecPos.y;
    pMatrix._43 = m_vecPos.z;

	if (modelEntity)
	{
		D3DXMATRIX matRotX,matRotY,matRotZ,matTrans;
		D3DXMatrixRotationX( &matRotX, m_fPitch );
		D3DXMatrixRotationY( &matRotY, m_fYaw/100 );  
		D3DXMatrixRotationZ( &matRotZ, m_fRoll ); 
		D3DXMatrixTranslation(&matTrans,modelEntityPos.x, modelEntityPos.y - 3.5f, modelEntityPos.z);
		pMatrix=(matRotY)*matTrans;
		
		pDevice->SetTransform( D3DTS_WORLD, &pMatrix );
		modelEntity->FrameMove(timeElapsed,&pMatrix);
		modelEntity->Render();
	}
}

void dxScene::SetAnimation( int numAnim )
{
	modelEntity->SetAnimationSet( numAnim );
}

long dxScene::AddObject( ULONG Count)
{
    CObject ** pObjectBuffer = NULL;
        
    if (!( pObjectBuffer = new CObject*[ m_nObjectCount + Count ] )) 
		return -1;

    if ( m_pObject )
    {
        memcpy( pObjectBuffer, m_pObject, m_nObjectCount * sizeof(CObject*) );
        delete []m_pObject;
    }

    m_pObject = pObjectBuffer;
    ZeroMemory( &m_pObject[m_nObjectCount], Count * sizeof(CObject*) );

    m_nObjectCount += Count;

    return m_nObjectCount - Count;
}

long dxScene::AddActor( ULONG Count /* = 1 */ )
{
	dxMeshAnimation ** pActorBuffer = NULL;
        
    if (!( pActorBuffer = new dxMeshAnimation*[ m_nActorCount + Count ] )) return -1;

    if ( m_pActor )
    {
        memcpy( pActorBuffer, m_pActor, m_nActorCount * sizeof(dxMeshAnimation*) );

        delete []m_pActor;
    }

    m_pActor = pActorBuffer;
    ZeroMemory( &m_pActor[m_nActorCount], Count * sizeof(dxMeshAnimation*) );

    m_nActorCount += Count;

    return m_nActorCount - Count;
}

long dxScene::AddMesh( ULONG Count /* = 1 */ )
{
    CTriMesh ** pMeshBuffer = NULL;
        
    if (!( pMeshBuffer = new CTriMesh*[ m_nMeshCount + Count ] )) return -1;

    if ( m_pMesh )
    {
        memcpy( pMeshBuffer, m_pMesh, m_nMeshCount * sizeof(CTriMesh*) );
        delete []m_pMesh;
    }

    m_pMesh = pMeshBuffer;
    ZeroMemory( &m_pMesh[m_nMeshCount], Count * sizeof(CTriMesh*) );
    m_nMeshCount += Count;

    return m_nMeshCount - Count;
}

void dxScene::RenderMeshObject( LPDIRECT3DDEVICE9 pDevice,float timeElapsed )
{
	ULONG i,j;

	D3DXMATRIX pMatrix;
	pMatrix._11 = m_vecRight.x; pMatrix._21 = m_vecUp.x; pMatrix._31 = m_vecLook.x;
	pMatrix._12 = m_vecRight.y; pMatrix._22 = m_vecUp.y; pMatrix._32 = m_vecLook.y;
	pMatrix._13 = m_vecRight.z; pMatrix._23 = m_vecUp.z; pMatrix._33 = m_vecLook.z;

	pMatrix._41 = m_vecPos.x;
	pMatrix._42 = m_vecPos.y;
	pMatrix._43 = m_vecPos.z;

    if ( !pDevice ) return;
		pDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

	for ( i = 0; i < m_nObjectCount; ++i )
    {
        dxMeshAnimation    * pActor = m_pObject[i]->m_pActor;
        CTriMesh * pMesh  = m_pObject[i]->m_pMesh;
        if ( !pMesh && !pActor ) continue;

        if ( pActor )
        {
            pDevice->SetTransform( D3DTS_WORLD, &pMatrix );
            //pDevice->SetFVF( pMesh->GetFVF() );
		}

		for ( j = 0; j < m_nAttribCount; j++ )
        {
            if ( pMesh )
                pMesh->DrawSubset( j );
            else
                pActor->DrawActorSubset( j );
        }
    }
}