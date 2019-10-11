/****************************************************************************
*                                                                           *
* dxCamera.h -- Camera Settings                                             *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXCAMERA_H_
#define _DXCAMERA_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

#include "..\Includes\winMain.h"
#include "..\Includes\dxPlayer.h"
#include "..\Includes\dxScene.h"

class dxCamera
{
	public:
		enum CAMERA_MODE {
			MODE_FPS			= 1,
			MODE_FPS_FLY		= 2,
			MODE_THIRDPERSON	= 3,
			MODE_SPACECRAFT     = 4,
			MODE_THIRDPERSON_SC = 5,
			MODE_FORCE_32BIT	= 0x7FFFFFFF,
			MODE_TERRAIN = 6
		};

		enum SCREEN_EFFECT {
			EFFECT_WATER        = 1,
			EFFECT_FORCE_32BIT  = 0x7FFFFFFF
		};

		dxCamera();
		dxCamera(D3DXVECTOR3 startPos);
		dxCamera( const dxCamera * pCamera );
		~dxCamera();

		bool createCamera(LPDIRECT3DDEVICE9 device, float nearView, float farView);
		void setLookAt(D3DXVECTOR3 newTarget);
		void setPosition(D3DXVECTOR3 newPosition);
		void resetView();
		
		void CalculateViewMatrix(D3DXMATRIX *viewMatrix);
		void SetYawPitchRoll(float yaw,float pitch, float roll);

		//GETS
		float GetYaw() const {return m_yaw;}
		float GetPitch() const {return m_pitch;}
		float GetRoll() const {return m_roll;}

		// Move operations
		void MoveForward(float amount) {m_position+=m_look*amount;}
		void MoveRight(float amount) {m_position+=m_right*amount;}
		void MoveUp(float amount) {m_position+=m_up*amount;}

		// Rotations
		void Yaw(float amount); // rotate around y axis
		void Pitch(float amount); // rotate around x axis
		void Roll(float amount); // rotate around z axis
	
		/////////////////////////////////////////////////////////////////////////////////////////////
		void                SetFOV           ( float FOV ) { m_fFOV = FOV; m_bProjDirty = true; }
		void                SetViewport      ( long Left, long Top, long Width, long Height, float NearClip, float FarClip, LPDIRECT3DDEVICE9 pDevice = NULL );
		void                UpdateRenderView ( LPDIRECT3DDEVICE9 pD3DDevice );
		void                UpdateRenderProj ( LPDIRECT3DDEVICE9 pD3DDevice );
		const D3DXMATRIX&   GetProjMatrix    ( );    
		float               GetFOV           ( ) const { return m_fFOV;  }
		float               GetNearClip      ( ) const { return m_fNearClip; }
		float               GetFarClip       ( ) const { return m_fFarClip; }
		const D3DVIEWPORT9& GetViewport      ( ) const { return m_Viewport; }
		CPlayer            *GetPlayer        ( ) const { return m_pPlayer;  }
		dxScene            *GetScene         ( ) const { return m_pScene;  }

		const D3DXVECTOR3&  GetPosition      ( ) const { return m_vecPos;   }
		const D3DXVECTOR3&  GetLook          ( ) const { return m_vecLook;  }
		const D3DXVECTOR3&  GetUp            ( ) const { return m_vecUp;    }
		const D3DXVECTOR3&  GetRight         ( ) const { return m_vecRight; }
		const D3DXMATRIX&   GetViewMatrix    ( );

		void                SetLook          ( const D3DXVECTOR3& Vector ) { m_vecLook  = Vector; }
		void                SetUp            ( const D3DXVECTOR3& Vector ) { m_vecUp    = Vector; }
		void                SetRight         ( const D3DXVECTOR3& Vector ) { m_vecRight = Vector; }
   
		void                SetVolumeInfo    ( const VOLUME_INFO& Volume );
		const VOLUME_INFO&  GetVolumeInfo    ( ) const;

		//void                RenderScreenEffect ( LPDIRECT3DDEVICE9 pD3DDevice, SCREEN_EFFECT Effect, ULONG Value );
		virtual void        AttachToPlayer   ( CPlayer * pPlayer );
		virtual void        AttachToScene    ( dxScene * pScene );
		virtual void        DetachFromPlayer ( );
		virtual void        DetachFromScene  ( );
		virtual void        SetPosition     ( const D3DXVECTOR3& Position ) { m_vecPos = Position; m_bViewDirty = true; m_bFrustumDirty = true; }
		virtual void        Move             ( const D3DXVECTOR3& vecShift ) { m_vecPos += vecShift; m_bViewDirty = true; m_bFrustumDirty = true; }
		virtual void        Rotate           ( float x, float y, float z )   {}
		virtual void        Update           ( float TimeScale, float Lag )  {}
		virtual void        SetCameraDetails ( const dxCamera * pCamera )     {}

		virtual CAMERA_MODE GetCameraMode    ( ) const = 0;
	    
		bool                BoundsInFrustum  ( const D3DXVECTOR3 & Min, const D3DXVECTOR3 & Max, const D3DXMATRIX * mtxWorld = NULL );

	private:
		D3DXMATRIX viewMatrix; //the view matrix
		D3DXMATRIX projectionMatrix; // the projection matrix
		D3DXVECTOR3 target;		// Camera Target
		D3DXVECTOR3 position;		// Position of camera
		float nearClip;
		float farClip;
		LPDIRECT3DDEVICE9 d3dDevice;
		
		D3DXVECTOR3 m_position; // camera position
		float m_yaw;	// rotation around the y axis
		float m_pitch;	// rotation around the x axis
		float m_roll;	// rotation around the z axis
		D3DXVECTOR3 m_up,m_look,m_right; // camera axis
		float RestrictAngleTo360Range(float angle) const;

		///////////////////////////////////////////////////////////////////////////////////
	protected:
		void            CalcFrustumPlanes( );

		CPlayer        *m_pPlayer;              // The player object we are attached to
		dxScene        *m_pScene;               // The scene object we are attached to
		VOLUME_INFO     m_Volume;               // Stores information about cameras collision volume
		D3DXMATRIX      m_mtxView;              // Cached view matrix
		D3DXMATRIX      m_mtxProj;              // Cached projection matrix
		D3DXPLANE       m_Frustum[6];           // The 6 planes of our frustum.

		bool            m_bViewDirty;           // View matrix dirty ?
		bool            m_bProjDirty;           // Proj matrix dirty ?
		bool            m_bFrustumDirty;        // Are the frustum planes dirty ?

		// Perspective Projection parameters
		float           m_fFOV;                 // FOV Angle.
		float           m_fNearClip;            // Near Clip Plane Distance
		float           m_fFarClip;             // Far Clip Plane Distance
		D3DVIEWPORT9    m_Viewport;             // The viewport details into which we are rendering.

		// Cameras current position & orientation
		D3DXVECTOR3     m_vecPos;               // Camera Position
		D3DXVECTOR3     m_vecUp;                // Camera Up Vector
		D3DXVECTOR3     m_vecLook;              // Camera Look Vector
		D3DXVECTOR3     m_vecRight;             // Camera Right Vector
};

class dxCamFPSFly : public dxCamera
{
	public:
		dxCamFPSFly( const dxCamera * pCamera );
		dxCamFPSFly();

		CAMERA_MODE         GetCameraMode    ( ) const { return MODE_FPS_FLY; }
		void                Rotate           ( float x, float y, float z );
		void                SetCameraDetails ( const dxCamera * pCamera );
};

class dxCam1stPerson : public dxCamera
{
	public:
		dxCam1stPerson( const dxCamera * pCamera );
		dxCam1stPerson();

		CAMERA_MODE         GetCameraMode    ( ) const { return MODE_FPS; }
		void                Rotate           ( float x, float y, float z );
		void                SetCameraDetails ( const dxCamera * pCamera );
};

class dxCam3rdPerson : public dxCamera
{
	public:
		dxCam3rdPerson( const dxCamera * pCamera );
		dxCam3rdPerson();

		CAMERA_MODE         GetCameraMode    ( ) const { return MODE_THIRDPERSON; }
		void                Move             ( const D3DXVECTOR3& vecShift ) {};
		void                Rotate           ( float x, float y, float z )   {};
		void                Update           ( float TimeScale, float Lag );
		void                SetCameraDetails ( const dxCamera * pCamera );
		void                SetLookAt        ( const D3DXVECTOR3& vecLookAt );
};

class dxCamSpaceCraft : public dxCamera
{
public:
    dxCamSpaceCraft( const dxCamera * pCamera );
    dxCamSpaceCraft();

    CAMERA_MODE         GetCameraMode    ( ) const { return MODE_SPACECRAFT; }
    void                Rotate           ( float x, float y, float z );
    void                SetCameraDetails ( const dxCamera * pCamera );

};

#endif