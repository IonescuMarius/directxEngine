#include "..\Includes\dxCamera.h"
#include "..\Includes\dxPlayer.h"
#include "..\Includes\dxObject.h"

dxCamera::dxCamera()
{
	// Reset / Clear all required values
	m_position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_roll = 0.0f;
    m_pPlayer         = NULL;
    m_vecRight        = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp           = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook         = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    m_vecPos          = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

    m_fFOV            = 60.0f;
    m_fNearClip       = 1.0f;
    m_fFarClip        = 100.0f;
    m_Viewport.X      = 0;
    m_Viewport.Y      = 0;
    m_Viewport.Width  = 640;
    m_Viewport.Height = 480;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;
    D3DXMatrixIdentity( &m_mtxView );
    D3DXMatrixIdentity( &m_mtxProj );
}

dxCamera::dxCamera(D3DXVECTOR3 startPos) : m_position(startPos),m_yaw(0),m_pitch(0),m_roll(0)
{
	// Start with an orthagonal camera axis looking down z
	m_up=D3DXVECTOR3(0.0f,1.0f,0.0f);//y
	m_look=D3DXVECTOR3(0.0f,0.0f,1.0f);//z
	m_right=D3DXVECTOR3(1.0f,0.0f,0.0f);//x
}

dxCamera::dxCamera( const dxCamera * pCamera )
{
    // Reset / Clear all required values
    m_pPlayer        = NULL;
	m_pScene         = NULL;
    m_vecRight       = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp          = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook        = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    m_vecPos         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

    m_fFOV            = 60.0f;
    m_fNearClip       = 1.0f;
    m_fFarClip        = 100.0f;
    m_Viewport.X      = 0;
    m_Viewport.Y      = 0;
    m_Viewport.Width  = 640;
    m_Viewport.Height = 480;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;

    // Set matrices to identity
    D3DXMatrixIdentity( &m_mtxView );
    D3DXMatrixIdentity( &m_mtxProj );
}

dxCamera::~dxCamera(void)
{

}

bool dxCamera::createCamera(LPDIRECT3DDEVICE9 device, float nearView, float farView)
{
	d3dDevice = device;
	nearClip = nearView;//nearest point at which the objects stop rendering 
	farClip = farView;//farthest point at which the objects stop rendering

	position.x = 0.0f;
	position.y = 0.0f;
	position.z = 0.0f;

	target.x = 0.0f;
	target.y = 0.0f;
	target.z = 0.0f;

	float aspect = 1.333f; // the aspect ratio of the screen

	//Setup the Projection Matrix
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DX_PI / 4.0f, aspect, nearClip, farClip);
    //Set the Projection
	d3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	return true;
}

void dxCamera::setPosition(D3DXVECTOR3 newPosition)
{
	position = newPosition;
	resetView();
}

void dxCamera::setLookAt(D3DXVECTOR3 newTarget)
{
	target = newTarget;	
	resetView();
} 

void dxCamera::resetView()
{
	D3DXMatrixLookAtLH(&viewMatrix, 
						&position,		//camera's position
						&target,		//camera's target
						&D3DXVECTOR3(0.0f, 1.0f, 0.0f));// the up direction
	d3dDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
}

// Sets yaw pitch and roll
void dxCamera::SetYawPitchRoll(float yaw,float pitch, float roll)
{
	m_yaw=yaw;
	m_pitch=pitch;
	m_roll=roll;
}

/*
	Calculate a view matrix by rotating our axis from the default orientation (looking down z)
	via the yaw, pitch and roll member variables. Also add in any translation.
*/
void dxCamera::CalculateViewMatrix(D3DXMATRIX *viewMatrix)
{
	// Start with our camera axis pointing down z
	m_up=D3DXVECTOR3(0.0f,1.0f,0.0f);
	m_look=D3DXVECTOR3(0.0f,0.0f,1.0f);
	m_right=D3DXVECTOR3(1.0f,0.0f,0.0f);

	// Yaw is rotation around the y axis (m_up)
	// Create a matrix that can carry out this rotation
	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &m_up, m_yaw);
	// To apply yaw we rotate the m_look & m_right vectors about the m_up vector (using our yaw matrix)
	D3DXVec3TransformCoord(&m_look, &m_look, &yawMatrix); 
	D3DXVec3TransformCoord(&m_right, &m_right, &yawMatrix); 

	// Pitch is rotation around the x axis (m_right)
	// Create a matrix that can carry out this rotation
	D3DXMATRIX pitchMatrix;
	D3DXMatrixRotationAxis(&pitchMatrix, &m_right, m_pitch);
	// To apply pitch we rotate the m_look and m_up vectors about the m_right vector (using our pitch matrix)
	D3DXVec3TransformCoord(&m_look, &m_look, &pitchMatrix); 
	D3DXVec3TransformCoord(&m_up, &m_up, &pitchMatrix); 
		
	// Roll is rotation around the z axis (m_look)
	// Create a matrix that can carry out this rotation
	D3DXMATRIX rollMatrix;
	D3DXMatrixRotationAxis(&rollMatrix, &m_look, m_roll);
	// To apply roll we rotate up and right about the look vector (using our roll matrix)
	// Note: roll only really applies for things like aircraft unless you are implementing lean
	D3DXVec3TransformCoord(&m_right, &m_right, &rollMatrix); 
	D3DXVec3TransformCoord(&m_up, &m_up, &rollMatrix); 
	
	// Build the view matrix from the transformed camera axis
	D3DXMatrixIdentity(viewMatrix);

	viewMatrix->_11 = m_right.x; viewMatrix->_12 = m_up.x; viewMatrix->_13 = m_look.x;
	viewMatrix->_21 = m_right.y; viewMatrix->_22 = m_up.y; viewMatrix->_23 = m_look.y;
	viewMatrix->_31 = m_right.z; viewMatrix->_32 = m_up.z; viewMatrix->_33 = m_look.z;
	
	viewMatrix->_41 = - D3DXVec3Dot( &m_position,&m_right); 
	viewMatrix->_42 = - D3DXVec3Dot( &m_position,&m_up);
	viewMatrix->_43 = - D3DXVec3Dot( &m_position,&m_look);
}

// Yaw - rotation around y axis
void dxCamera::Yaw(float amount) 
{
	m_yaw+=amount;
	m_yaw=RestrictAngleTo360Range(m_yaw);
}	

// Pitch - rotation around x axis
void dxCamera::Pitch(float amount)
{
	m_pitch+=amount;
	m_pitch=RestrictAngleTo360Range(m_pitch);
}

// Roll - rotation around z axis
// Note: normally only used for aircraft type cameras rather than land based ones
void dxCamera::Roll(float amount) 
{
	m_roll+=amount;
	m_roll=RestrictAngleTo360Range(m_roll);
}

// Keep the angle in the range 0 to 360 (2*PI radians)
float dxCamera::RestrictAngleTo360Range(float angle) const
{
	while(angle>2*D3DX_PI)
		angle-=2*D3DX_PI;

	while(angle<0)
		angle+=2*D3DX_PI;

	return angle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name : AttachToPlayer ()
// Desc : Attach the camera to the specified player object
//-----------------------------------------------------------------------------
void dxCamera::AttachToPlayer( CPlayer * pPlayer )
{
    // Attach ourslves to the player
    m_pPlayer = pPlayer;

}

//-----------------------------------------------------------------------------
// Name : AttachToScene ()
// Desc : Attach the camera to the specified scene object
//-----------------------------------------------------------------------------
void dxCamera::AttachToScene( dxScene * pScene )
{
    // Attach ourslves to the player
    m_pScene = pScene;

}

//-----------------------------------------------------------------------------
// Name : DetachFromPlayer ()
// Desc : Detach the camera from the current player object
//-----------------------------------------------------------------------------
void dxCamera::DetachFromPlayer()
{
    m_pPlayer = NULL;
}

//-----------------------------------------------------------------------------
// Name : DetachFromScene ()
// Desc : Detach the camera from the current scene object
//-----------------------------------------------------------------------------
void dxCamera::DetachFromScene()
{
    m_pScene = NULL;
}

void dxCamera::SetViewport( long Left, long Top, long Width, long Height, float NearClip, float FarClip, LPDIRECT3DDEVICE9 pDevice )
{
    // Set viewport sizes
    m_Viewport.X      = Left;
    m_Viewport.Y      = Top;
    m_Viewport.Width  = Width;
    m_Viewport.Height = Height;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;
    m_fNearClip       = NearClip;
    m_fFarClip        = FarClip;
    m_bProjDirty      = true;

    // Update device if requested
    if ( pDevice ) pDevice->SetViewport( &m_Viewport );
}

const D3DXMATRIX& dxCamera::GetProjMatrix()
{
    // Only update matrix if something has changed
    if ( m_bProjDirty ) 
    {     
        float fAspect = (float)m_Viewport.Width / (float)m_Viewport.Height;

        // Set the perspective projection matrix
        D3DXMatrixPerspectiveFovLH( &m_mtxProj, D3DXToRadian( m_fFOV ), fAspect, m_fNearClip, m_fFarClip );
            
        // Proj Matrix has been updated
        m_bProjDirty = false; 

    } // End If Projection matrix needs updating

    // Return the projection matrix.
    return m_mtxProj;
}

const D3DXMATRIX& dxCamera::GetViewMatrix()
{
    // Only update matrix if something has changed
    if ( m_bViewDirty ) 
    {
        // Because many rotations will cause floating point errors, the axis will eventually become
        // non-perpendicular to one other causing all hell to break loose. Therefore, we must
        // perform base vector regeneration to ensure that all vectors remain unit length and
        // perpendicular to one another. This need not be done on EVERY call to rotate (i.e. you
        // could do this once every 50 calls for instance).
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );
        D3DXVec3Cross( &m_vecRight, &m_vecUp, &m_vecLook );
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Cross( &m_vecUp, &m_vecLook, &m_vecRight );
        D3DXVec3Normalize( &m_vecUp, &m_vecUp );

        // Set view matrix values
        m_mtxView._11 = m_vecRight.x; m_mtxView._12 = m_vecUp.x; m_mtxView._13 = m_vecLook.x;
	    m_mtxView._21 = m_vecRight.y; m_mtxView._22 = m_vecUp.y; m_mtxView._23 = m_vecLook.y;
	    m_mtxView._31 = m_vecRight.z; m_mtxView._32 = m_vecUp.z; m_mtxView._33 = m_vecLook.z;
	    m_mtxView._41 =- D3DXVec3Dot( &m_vecPos, &m_vecRight );
	    m_mtxView._42 =- D3DXVec3Dot( &m_vecPos, &m_vecUp    );
	    m_mtxView._43 =- D3DXVec3Dot( &m_vecPos, &m_vecLook  );

        // View Matrix has been updated
        m_bViewDirty = false;

    } // End If View Dirty

    // Return the view matrix.
    return m_mtxView;
}

//-----------------------------------------------------------------------------
// Name : UpdateRenderView ()
// Desc : Update the render device with the updated view matrix etc.
//-----------------------------------------------------------------------------
void dxCamera::UpdateRenderView( LPDIRECT3DDEVICE9 pD3DDevice )
{  
    // Validate Parameters
    if (!pD3DDevice) return;

    // Update the device with this matrix.
    pD3DDevice->SetTransform( D3DTS_VIEW, &GetViewMatrix() );
}

//-----------------------------------------------------------------------------
// Name : UpdateRenderProj ()
// Desc : Update the render device with the updated projection matrix etc.
//-----------------------------------------------------------------------------
void dxCamera::UpdateRenderProj( LPDIRECT3DDEVICE9 pD3DDevice )
{  
    // Validate Parameters
    if (!pD3DDevice) return;

    // Update the device with this matrix.
    pD3DDevice->SetTransform( D3DTS_PROJECTION, &GetProjMatrix() );
}

//-----------------------------------------------------------------------------
// Name : BoundsInFrustum ()
// Desc : Determine whether or not the box passed is within the frustum.
//-----------------------------------------------------------------------------
bool dxCamera::BoundsInFrustum( const D3DXVECTOR3 & vecMin, const D3DXVECTOR3 & vecMax, const D3DXMATRIX * mtxWorld /*= NULL*/ )
{
    // First calculate the frustum planes
    CalcFrustumPlanes();

    ULONG i;
    D3DXVECTOR3 NearPoint, FarPoint, Normal, Min = vecMin, Max = vecMax;

    // Transform bounds if matrix provided
    if ( mtxWorld )
    {
        D3DXVec3TransformCoord( &Min, &Min, mtxWorld );
        D3DXVec3TransformCoord( &Max, &Max, mtxWorld );
    
    } // End if matrix provided

    // Loop through all the planes
    for ( i = 0; i < 6; i++ )
    {
        // Store the plane normal
        Normal = D3DXVECTOR3( m_Frustum[i].a, m_Frustum[i].b, m_Frustum[i].c );

        if ( Normal.x > 0.0f )
        {
            if ( Normal.y > 0.0f )
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Min.x; NearPoint.y = Min.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Min.x; NearPoint.y = Min.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0
            
            } // End if Normal.y > 0
            else
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Min.x; NearPoint.y = Max.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Min.x; NearPoint.y = Max.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0

            } // End if Normal.y <= 0
        
        } // End if Normal.x > 0
        else
        {
            if ( Normal.y > 0.0f )
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Max.x; NearPoint.y = Min.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Max.x; NearPoint.y = Min.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0
            
            } // End if Normal.y > 0
            else
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Max.x; NearPoint.y = Max.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Max.x; NearPoint.y = Max.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0

            } // End if Normal.y <= 0

        } // End if Normal.x <= 0
                
        // Near extreme point is outside, and thus the
        // AABB is totally outside the frustum ?
        if ( D3DXVec3Dot( &Normal, &NearPoint ) + m_Frustum[i].d > 0.0f ) return false;

    } // Next Plane

    // Is within the frustum
    return true;
}

//-----------------------------------------------------------------------------
// Name : CalcFrustumPlanes () (Private)
// Desc : Calculate the 6 frustum planes based on the current values.
//-----------------------------------------------------------------------------
void dxCamera::CalcFrustumPlanes()
{
    // Only update planes if something has changed
    if ( !m_bFrustumDirty ) return;

    // Build a combined view & projection matrix
    D3DXMATRIX m = GetViewMatrix() * GetProjMatrix();

    // Left clipping plane
    m_Frustum[0].a = -(m._14 + m._11);
    m_Frustum[0].b = -(m._24 + m._21);
    m_Frustum[0].c = -(m._34 + m._31);
    m_Frustum[0].d = -(m._44 + m._41);

    // Right clipping plane
    m_Frustum[1].a = -(m._14 - m._11);
    m_Frustum[1].b = -(m._24 - m._21);
    m_Frustum[1].c = -(m._34 - m._31);
    m_Frustum[1].d = -(m._44 - m._41);

    // Top clipping plane
    m_Frustum[2].a = -(m._14 - m._12);
    m_Frustum[2].b = -(m._24 - m._22);
    m_Frustum[2].c = -(m._34 - m._32);
    m_Frustum[2].d = -(m._44 - m._42);

    // Bottom clipping plane
    m_Frustum[3].a = -(m._14 + m._12);
    m_Frustum[3].b = -(m._24 + m._22);
    m_Frustum[3].c = -(m._34 + m._32);
    m_Frustum[3].d = -(m._44 + m._42);

    // Near clipping plane
    m_Frustum[4].a = -(m._13);
    m_Frustum[4].b = -(m._23);
    m_Frustum[4].c = -(m._33);
    m_Frustum[4].d = -(m._43);

    // Far clipping plane
    m_Frustum[5].a = -(m._14 - m._13);
    m_Frustum[5].b = -(m._24 - m._23);
    m_Frustum[5].c = -(m._34 - m._33);
    m_Frustum[5].d = -(m._44 - m._43);

    // Normalize the m_Frustum
    for ( ULONG i = 0; i < 6; i++ ) D3DXPlaneNormalize( &m_Frustum[i], &m_Frustum[i] );

    // Frustum is no longer dirty
    m_bFrustumDirty = false;
}

//-----------------------------------------------------------------------------
// Name : SetVolumeInfo ()
// Desc : Set the players collision volume information
//-----------------------------------------------------------------------------
void dxCamera::SetVolumeInfo( const VOLUME_INFO& Volume )
{
    m_Volume = Volume;
}

//-----------------------------------------------------------------------------
// Name : GetVolumeInfo ()
// Desc : Retrieve the players collision volume information
//-----------------------------------------------------------------------------
const VOLUME_INFO& dxCamera::GetVolumeInfo( ) const
{
    return m_Volume;
}

//-----------------------------------------------------------------------------
// CCam1stPerson Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCam1stPerson () (Alternate Constructor)
// Desc : CCam1stPerson Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
dxCam1stPerson::dxCam1stPerson( const dxCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void dxCam1stPerson::SetCameraDetails( const dxCamera * pCamera )
{
    // Validate Parameters
    if (!pCamera) return;

    // Reset / Clear all required values
    m_vecPos    = pCamera->GetPosition();
    m_vecRight  = pCamera->GetRight();
    m_vecLook   = pCamera->GetLook();
    m_vecUp     = pCamera->GetUp();
    m_fFOV      = pCamera->GetFOV();
    m_fNearClip = pCamera->GetNearClip();
    m_fFarClip  = pCamera->GetFarClip();
    m_Viewport  = pCamera->GetViewport();
    m_Volume    = pCamera->GetVolumeInfo();

    if ( pCamera->GetCameraMode() == MODE_SPACECRAFT )
    {
        // Flatten out the vectors
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        // Finally, normalize them
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );
    }

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;
}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the camera about the players local axis.
//-----------------------------------------------------------------------------
void dxCam1stPerson::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    if (!m_pPlayer || !m_pScene) 
		return;

    if ( x != 0 ) 
    {
        D3DXMatrixRotationAxis( &mtxRotate, &m_vecRight, D3DXToRadian( x ) );
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
    }

    if ( y != 0 ) 
    {
		if(m_pPlayer)
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian( y ) );
		else
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian( y ) );
        
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
    
    }

    if ( z != 0 ) 
    {
		if(m_pPlayer)
		{
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetLook(), D3DXToRadian( z ) );
			m_vecPos -= m_pPlayer->GetPosition();
			D3DXVec3TransformCoord ( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pPlayer->GetPosition();
		}
		else
		{
			D3DXMatrixRotationAxis( &mtxRotate, &m_pScene->GetLook(), D3DXToRadian( z ) );
			m_vecPos -= m_pScene->GetPosition();
			D3DXVec3TransformCoord ( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pScene->GetPosition();
		}
        
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
    }
        
    m_bViewDirty = true;
    m_bFrustumDirty = true;
}

//-----------------------------------------------------------------------------
// CCam3rdPerson Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCam3rdPerson () (Alternate Constructor)
// Desc : CCam3rdPerson Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
dxCam3rdPerson::dxCam3rdPerson( const dxCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void dxCam3rdPerson::SetCameraDetails( const dxCamera * pCamera )
{
    // Validate Parameters
    if (!pCamera) return;

    // Reset / Clear all required values
    m_vecPos    = pCamera->GetPosition();
    m_vecRight  = pCamera->GetRight();
    m_vecLook   = pCamera->GetLook();
    m_vecUp     = pCamera->GetUp();
    m_fFOV      = pCamera->GetFOV();
    m_fNearClip = pCamera->GetNearClip();
    m_fFarClip  = pCamera->GetFarClip();
    m_Viewport  = pCamera->GetViewport();
    m_Volume    = pCamera->GetVolumeInfo();

    if ( pCamera->GetCameraMode() == MODE_SPACECRAFT )
    {
        // Flatten out the vectors
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        // Finally, normalize them
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );

    } // End if MODE_SPACECRAFT

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;

}

//-----------------------------------------------------------------------------
// Camera Update ()
//-----------------------------------------------------------------------------
void dxCam3rdPerson::Update( float TimeScale, float Lag )
{
    D3DXMATRIX  mtxRotate;
    D3DXVECTOR3 vecOffset, vecPosition, vecDir;

    // Validate requirements
	if (!m_pPlayer || !m_pScene) return;

    float fTimeScale = 1.0f, Length = 0.0f;
    if ( Lag != 0.0f ) fTimeScale = TimeScale * (1.0f / Lag);

    // Build a rotation matrix so that we can rotate our offset vector to it's position behind the player
    D3DXMatrixIdentity( &mtxRotate );
	
	if (m_pPlayer)
	{
		D3DXVECTOR3 vecRight = m_pPlayer->GetRight(), vecUp = m_pPlayer->GetUp(), vecLook = m_pPlayer->GetLook();
		mtxRotate._11 = vecRight.x; mtxRotate._21 = vecUp.x; mtxRotate._31 = vecLook.x;
		mtxRotate._12 = vecRight.y; mtxRotate._22 = vecUp.y; mtxRotate._32 = vecLook.y;
		mtxRotate._13 = vecRight.z; mtxRotate._23 = vecUp.z; mtxRotate._33 = vecLook.z;
	}
	else
	{
		D3DXVECTOR3 vecRightS = m_pScene->GetRight(), vecUpS = m_pScene->GetUp(), vecLookS = m_pScene->GetLook();
		mtxRotate._11 = vecRightS.x; mtxRotate._21 = vecUpS.x; mtxRotate._31 = vecLookS.x;
		mtxRotate._12 = vecRightS.y; mtxRotate._22 = vecUpS.y; mtxRotate._32 = vecLookS.y;
		mtxRotate._13 = vecRightS.z; mtxRotate._23 = vecUpS.z; mtxRotate._33 = vecLookS.z;
	}

    // Calculate our rotated offset vector
	if (m_pPlayer)
		D3DXVec3TransformCoord( &vecOffset, &m_pPlayer->GetCamOffset(), &mtxRotate );
	else
		D3DXVec3TransformCoord( &vecOffset, &m_pScene->GetCamOffset(), &mtxRotate );

    // vecOffset now contains information to calculate where our camera position SHOULD be.
	if (m_pPlayer)
		vecPosition = m_pPlayer->GetPosition() + vecOffset;
	else
		vecPosition = m_pScene->GetPosition() + vecOffset;
    vecDir = vecPosition - m_vecPos;
    Length = D3DXVec3Length( &vecDir );
    D3DXVec3Normalize( &vecDir, &vecDir );

    // Move based on camera lag
    float Distance = Length * fTimeScale;
    if ( Distance > Length ) Distance = Length;
    
    // If we only have a short way to travel, move all the way
    if ( Length < 0.01f ) Distance = Length;
    
    // Update our camera
    if ( Distance > 0 )
    {
        m_vecPos += vecDir * Distance;
		if (m_pPlayer)
			SetLookAt( m_pPlayer->GetPosition() );
		else
			SetLookAt( m_pScene->GetPosition() );
        m_bViewDirty    = true;
        m_bFrustumDirty = true;
    }

}

//-----------------------------------------------------------------------------
// Name : SetLookAt () (Private)
// Desc : Set the cameras look at vector, this function will also update the
//        various direction vectors so that they are correct.
//-----------------------------------------------------------------------------
void dxCam3rdPerson::SetLookAt( const D3DXVECTOR3& vecLookAt )
{
    D3DXMATRIX Matrix;

    // Generate a look at matrix
	if (m_pPlayer)
		D3DXMatrixLookAtLH( &Matrix, &m_vecPos, &vecLookAt, &m_pPlayer->GetUp() );
	else
		D3DXMatrixLookAtLH( &Matrix, &m_vecPos, &vecLookAt, &m_pScene->GetUp() );
    
    // Extract the vectors
    m_vecRight = D3DXVECTOR3( Matrix._11, Matrix._21, Matrix._31 );
    m_vecUp    = D3DXVECTOR3( Matrix._12, Matrix._22, Matrix._32 );
    m_vecLook  = D3DXVECTOR3( Matrix._13, Matrix._23, Matrix._33 );

    // Set view matrix as dirty
    m_bViewDirty = true;

    // Frustum is now dirty by default
    m_bFrustumDirty   = true;
}

//-----------------------------------------------------------------------------
// CCamSpaceCraft Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCamSpaceCraft () (Alternate Constructor)
// Desc : CCamSpaceCraft Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
dxCamSpaceCraft::dxCamSpaceCraft( const dxCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void dxCamSpaceCraft::SetCameraDetails( const dxCamera * pCamera )
{
    // Validate Parameters
    if (!pCamera) return;

    // Reset / Clear all required values
    m_vecPos    = pCamera->GetPosition();
    m_vecRight  = pCamera->GetRight();
    m_vecLook   = pCamera->GetLook();
    m_vecUp     = pCamera->GetUp();
    m_fFOV      = pCamera->GetFOV();
    m_fNearClip = pCamera->GetNearClip();
    m_fFarClip  = pCamera->GetFarClip();
    m_Viewport  = pCamera->GetViewport();
    m_Volume    = pCamera->GetVolumeInfo();

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;

}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the camera about the cameras local axis.
// Note : Just performs a standard three vector rotation. This is essentially 
//        how we perform 'anti-gimble-lock' space craft rotations.
//-----------------------------------------------------------------------------
void dxCamSpaceCraft::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    // Validate Requirements
    if (!m_pPlayer || !m_pScene) return;

    if ( x != 0 ) 
    {

        // Build rotation matrix
		if (m_pPlayer)
		{
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetRight(), D3DXToRadian( x ) );
			D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
			 // Adjust position
			m_vecPos -= m_pPlayer->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pPlayer->GetPosition();
		}
		else
		{
			D3DXMatrixRotationAxis( &mtxRotate, &m_pScene->GetRight(), D3DXToRadian( x ) );
			D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
			 // Adjust position
			m_vecPos -= m_pScene->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pScene->GetPosition();
		}
			
    } // End if Pitch

    if ( y != 0 ) 
    {
		if (m_pPlayer)
		{
			// Build rotation matrix
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian( y ) );
			D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

			// Adjust position
			m_vecPos -= m_pPlayer->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pPlayer->GetPosition();
		}
		else
		{
			// Build rotation matrix
			D3DXMatrixRotationAxis( &mtxRotate, &m_pScene->GetUp(), D3DXToRadian( y ) );
			D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

			// Adjust position
			m_vecPos -= m_pScene->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pScene->GetPosition();
		}
    
    } // End if Yaw

    if ( z != 0 ) 
    {
		if (m_pPlayer)
		{
			// Build rotation matrix
			D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetLook(), D3DXToRadian( z ) );
			D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

			// Adjust position
			m_vecPos -= m_pPlayer->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pPlayer->GetPosition();
		}
		else
		{
			// Build rotation matrix
			D3DXMatrixRotationAxis( &mtxRotate, &m_pScene->GetLook(), D3DXToRadian( z ) );
			D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
			D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

			// Adjust position
			m_vecPos -= m_pScene->GetPosition();
			D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
			m_vecPos += m_pScene->GetPosition();
		}
    
    } // End if Roll
        
    // Set view matrix as dirty
    m_bViewDirty = true;

    // Frustum is now dirty by default
    m_bFrustumDirty = true;
}