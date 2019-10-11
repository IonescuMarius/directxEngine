/****************************************************************************
*                                                                           *
* dxPlayer.h -- PLAYER                                                      *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXPLAYER_H_
#define _DXPLAYER_H_

//-----------------------------------------------------------------------------
// dxPlayer Specific Includes
//-----------------------------------------------------------------------------
#include <string>
#include <d3dx9.h>
#include "..\Includes\winMain.h"
#include "..\Includes\dxObject.h"
#include "..\Includes\dxAnimation.h"

#define MAX_ANIMATIONS          6 

typedef struct _VOLUME_INFO     
{
    D3DXVECTOR3 Min;            
    D3DXVECTOR3 Max;            

} VOLUME_INFO;

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CObjectC;
class dxCamera;
class CPlayer;
class dxMeshAnimation;
class dxTimer;

//-----------------------------------------------------------------------------
// Typedefs for update / collision callbacks.
//-----------------------------------------------------------------------------
typedef void (*UPDATEPLAYER)(LPVOID pContext, CPlayer * pPlayer, float TimeScale);
typedef void (*UPDATECAMERA)(LPVOID pContext, dxCamera * pCamera, float TimeScale);

class CPlayer
{
public:
    //-------------------------------------------------------------------------
    // Enumerators
    //-------------------------------------------------------------------------
    enum DIRECTION { 
        DIR_FORWARD     = 1, 
        DIR_BACKWARD    = 2, 
        DIR_LEFT        = 4, 
        DIR_RIGHT       = 8, 
        DIR_UP          = 16, 
        DIR_DOWN        = 32,

        DIR_FORCE_32BIT = 0x7FFFFFFF
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CPlayer();
    virtual ~CPlayer();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    bool                SetCameraMode      ( ULONG Mode );
    void                Update             ( float TimeScale );

    void                AddPlayerCallback    ( UPDATEPLAYER pFunc, LPVOID pContext );
    void                AddCameraCallback    ( UPDATECAMERA pFunc, LPVOID pContext );
    void                RemovePlayerCallback ( UPDATEPLAYER pFunc, LPVOID pContext );
    void                RemoveCameraCallback ( UPDATECAMERA pFunc, LPVOID pContext );

    void                Set3rdPersonObject ( CObjectC * pObject           ) { m_p3rdPersonObject = pObject; }
    void                SetFriction        ( float Friction              ) { m_fFriction = Friction; }
    void                SetGravity         ( const D3DXVECTOR3& Gravity  ) { m_vecGravity = Gravity; }
    void                SetMaxVelocityXZ   ( float MaxVelocity           ) { m_fMaxVelocityXZ = MaxVelocity; }
    void                SetMaxVelocityY    ( float MaxVelocity           ) { m_fMaxVelocityY = MaxVelocity; }
    void                SetVelocity        ( const D3DXVECTOR3& Velocity ) { m_vecVelocity = Velocity; }
    void                SetCamLag          ( float CamLag )                { m_fCameraLag = CamLag; }
    void                SetCamOffset       ( const D3DXVECTOR3& Offset );
    void                SetVolumeInfo      ( const VOLUME_INFO& Volume );
    const VOLUME_INFO&  GetVolumeInfo      ( ) const;

    dxCamera           *GetCamera          ( ) const { return m_pCamera; }
    const D3DXVECTOR3  &GetVelocity        ( ) const { return m_vecVelocity; }
    const D3DXVECTOR3  &GetCamOffset       ( ) const { return m_vecCamOffset; }

    const D3DXVECTOR3  &GetPosition        ( ) const { return m_vecPos; }
    const D3DXVECTOR3  &GetLook            ( ) const { return m_vecLook; }
    const D3DXVECTOR3  &GetUp              ( ) const { return m_vecUp; }
    const D3DXVECTOR3  &GetRight           ( ) const { return m_vecRight; }
    
    float               GetYaw             ( ) const { return m_fYaw; }
    float               GetPitch           ( ) const { return m_fPitch; }
    float               GetRoll            ( ) const { return m_fRoll; }

    void                SetPosition        ( const D3DXVECTOR3& Position ) { Move( Position - m_vecPos, false ); }
	void                SetAnimation	   ( int numAnim );


    void                Move               ( ULONG Direction, float Distance, bool Velocity = false );
    void                Move               ( const D3DXVECTOR3& vecShift, bool Velocity = false );
    void                Rotate             ( float x, float y, float z );

    void                Render             ( LPDIRECT3DDEVICE9 pDevice );
	void                RenderMesh         ( LPDIRECT3DDEVICE9 pDevice, float timeElapsed );
	void                RenderMeshObject   ( LPDIRECT3DDEVICE9 pDevice, float timeElapsed );
	void                BuildMeshAnimation ( );
	void                AnimateMesh		   ( dxTimer & Timer );

	bool LoadXFile(const std::string &filename,int startAnimation ,int model =0, LPDIRECT3DDEVICE9 pDevice = NULL, TCHAR * strFileName = NULL);
	long                AddObject            ( ULONG Count = 1 );
	long                AddActor             ( ULONG Count = 1 );
	long                AddMesh              ( ULONG Count = 1 );

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    dxCamera        *m_pCamera;            
    CObjectC        *m_p3rdPersonObject;   
    VOLUME_INFO     m_Volume;              
    ULONG           m_CameraMode;          

    D3DXVECTOR3     m_vecPos;               
    D3DXVECTOR3     m_vecUp;                
    D3DXVECTOR3     m_vecRight;             
    D3DXVECTOR3     m_vecLook;              
    D3DXVECTOR3     m_vecCamOffset;         
    float           m_fPitch;               
    float           m_fRoll;                
    float           m_fYaw;                 
    
    D3DXVECTOR3     m_vecVelocity;          
    D3DXVECTOR3     m_vecGravity;           
    float           m_fMaxVelocityXZ;       
    float           m_fMaxVelocityY;        
    float           m_fFriction;            
    float           m_fCameraLag;           

    CALLBACK_FUNC   m_pUpdatePlayer[255];   
    CALLBACK_FUNC   m_pUpdateCamera[255];   
    USHORT          m_nUpdatePlayerCount;   
    USHORT          m_nUpdateCameraCount;   

	CObject			  **m_pObject;                      
    ULONG               m_nObjectCount;                 
    
    CTriMesh          **m_pMesh;                        
    ULONG               m_nMeshCount;                   
    dxMeshAnimation   **m_pActor;                       
    ULONG               m_nActorCount;                  
	ULONG               m_nAttribCount; 

	LPTSTR              m_strDataPath; 
	CAnimation          m_pAnimList    [MAX_ANIMATIONS];

public:
	float m_entityYaw,m_entityPitch,m_entityRoll;
	D3DXVECTOR3 modelEntityPos;
	dxMeshAnimation *modelEntity;
};

#endif // _dxPlayer_H_