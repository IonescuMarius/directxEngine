//-----------------------------------------------------------------------------
// File: CActor.h
//
// Desc: The main classes used for the actor object(s).
//
// Copyright (c) 1997-2005 GameInstitute.com. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _DXACTOR_H_
#define _DXACTOR_H_

//-----------------------------------------------------------------------------
// CActor Specific Includes
//-----------------------------------------------------------------------------
#include "..\Includes\winMain.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CTriMesh;
class CAnimationItem;
class CActionDefinition;
class CActionStatus;

//-----------------------------------------------------------------------------
// Typedefs, structures & enumerators
//-----------------------------------------------------------------------------
struct D3DXFRAME_MATRIX : public D3DXFRAME  // Derived frame, stores an additional 'combined' matrix
{
    D3DXMATRIX  mtxCombined;    // Combined matrix for this frame.
};

struct D3DXMESHCONTAINER_DERIVED : public D3DXMESHCONTAINER // Derived Mesh Container, stores our wrapper mesh.
{
    CTriMesh   * pMesh;					// Our wrapper mesh.
	bool         Invalidated;           // Boolean flag informing us when the frame data has changed
    ULONG        SkinMethod;            // The method used to skin this mesh if applicable.
    D3DXMATRIX * pBoneOffset;           // Array of bone offset matrices
    D3DXMATRIX** ppBoneMatrices;        // Pointers to the bone matrices themselves
    DWORD        AttribGroupCount;      // Number of attribute groups
    DWORD        InfluenceCount;        // Total number of influences
    LPD3DXBUFFER pBoneCombination;      // The bone combination table
    ULONG        PaletteEntryCount;     // Number of entries in the palette
    bool         SoftwareVP;            // Requires the use of software vertex processing?
    
    ULONG      * pVPRemap;              // For non indexed skinning, we build a remap table for the VP modes
    ULONG        SWRemapBegin;          // The index in the VPRemap table where the sw modes begin

    LPD3DXMESH   pSWMesh;               // Cloned mesh used in software skinning.
};

typedef struct _AnimSplitDefinition     // Split definition structure
{
    TCHAR   SetName[128];               // The name of the new animation set
    ULONG   SourceSet;                  // The set from which we're sourcing the data
    double  StartTicks;                 // Ticks at which this new set starts
    double  EndTicks;                   // Ticks at which this new set ends
    LPVOID  Reserved;                   // Reserved for use internally, do not use.

} AnimSplitDefinition;
//-----------------------------------------------------------------------------
// Typedefs for collecting callbackkeys
//-----------------------------------------------------------------------------
typedef ULONG (*COLLECTCALLBACKS )( LPVOID pContext, LPCTSTR strActorFile, void * pCallbackData, LPD3DXKEYFRAMEDANIMATIONSET pAnimSet, D3DXKEY_CALLBACK pKeys[] );

//-----------------------------------------------------------------------------
// IIDs for our own internal animation set (for query interface).
//-----------------------------------------------------------------------------
const GUID IID_CAnimationSet = {0x5B23B100, 0xE885, 0x4F63, {0xB0, 0x2E, 0x50, 0xFC, 0x99, 0xBA, 0x3, 0xDD}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CActor (Class)
// Desc : Animatable objects, and mesh heirarchies are managed and stored here.
//-----------------------------------------------------------------------------
class CActor
{
public:
    //-------------------------------------------------------------------------
	// Public Enumerators for This Class.
	//-------------------------------------------------------------------------
    enum CALLBACK_TYPE { CALLBACK_TEXTURE = 0, CALLBACK_EFFECT = 1, CALLBACK_ATTRIBUTEID = 2, CALLBACK_CALLBACKKEYS = 3, CALLBACK_COUNT = 4 };
    enum SKINMETHOD    { SKINMETHOD_INDEXED = 1, SKINMETHOD_NONINDEXED = 2, SKINMETHOD_SOFTWARE = 3, 
                         SKINMETHOD_AUTODETECT = 4, SKINMETHOD_PREFER_HW_NONINDEXED = 8, SKINMETHOD_PREFER_SW_INDEXED = 16 };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CActor( );
    virtual ~CActor( );
             
    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    bool            RegisterCallback    ( CALLBACK_TYPE Type, LPVOID pFunction, LPVOID pContext );
    
    // Loading / Clearing functions
    HRESULT				LoadActorFromX				( LPCTSTR FileName, ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice );
    HRESULT             LoadActorFromX2             ( LPCTSTR FileName, ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice, bool ApplyCustomSets = true, void * pCallbackData = NULL );
    HRESULT             SaveActorToX                ( LPCTSTR FileName, ULONG Format );
    HRESULT             ApplySplitDefinitions       ( ULONG nDefCount, AnimSplitDefinition pDefList[] );
    HRESULT             SetActorLimits              ( ULONG MaxAnimSets = 0, ULONG MaxTracks = 0, ULONG MaxAnimOutputs = 0, ULONG MaxEvents = 0, ULONG MaxCallbackKeys = 0 );
    void                SetSkinningMethod           ( ULONG SkinMethod );
    HRESULT             LoadActionDefinitions       ( LPCTSTR FileName );
	virtual void        Release                     ( );


    // Rendering and update functionality
	virtual void        SetWorldMatrix              ( const D3DXMATRIX * mtxWorld = NULL, bool UpdateFrames = false );
    virtual void        ResetTime                   ( bool UpdateFrames = false );
    virtual void        AdvanceTime                 ( double fTimeElapsed, bool UpdateFrames = true, LPD3DXANIMATIONCALLBACKHANDLER pCallbackHandler = NULL );
    virtual void        DrawActor                   (  );
    virtual void        DrawActorSubset             ( ULONG AttributeID );
    virtual void        DrawBones                   ( LPD3DXFRAME pFrame = NULL );
    virtual HRESULT     ApplyAction                 ( LPCTSTR ActionName );
    
    // Accessor functions
    double              GetTime                     ( ) const;
    LPCTSTR             GetActorName                ( ) const;
    CALLBACK_FUNC       GetCallback                 ( CALLBACK_TYPE Type ) const;
    LPDIRECT3DDEVICE9   GetDevice                   ( ) const;
    LPD3DXFRAME         GetFrameByName              ( LPCTSTR strName, LPD3DXFRAME pFrame = NULL ) const;
    LPD3DXFRAME         GetRootFrame                ( ) const;
    ULONG               GetOptions                  ( ) const;
    ULONG               GetAnimationSetCount        ( ) const;
    LPD3DXANIMATIONSET  GetAnimationSet             ( ULONG Index ) const;
    LPD3DXANIMATIONSET  GetAnimationSetByName       ( LPCTSTR strName ) const;
    ULONG               GetMaxNumTracks             ( ) const;
    ULONG               GetMaxNumAnimationSets      ( ) const;
    ULONG               GetMaxNumEvents             ( ) const;
    ULONG               GetMaxNumAnimationOutputs   ( ) const;
    ULONG               GetSkinningMethod           ( ) const;
    CActionDefinition * GetCurrentAction            ( ) const;
    CActionStatus     * GetActionStatus             ( ) const;

	// Mixer / Track functions
    HRESULT             SetTrackAnimationSet        ( ULONG TrackIndex, LPD3DXANIMATIONSET pAnimSet );
    HRESULT             SetTrackAnimationSetByIndex ( ULONG TrackIndex, ULONG SetIndex );
    HRESULT             SetTrackAnimationSetByName  ( ULONG TrackIndex, LPCTSTR SetName );
    HRESULT             SetTrackPosition            ( ULONG TrackIndex, DOUBLE Position );
    HRESULT             SetTrackDesc                ( ULONG TrackIndex, D3DXTRACK_DESC * pDesc );
    HRESULT             SetTrackEnable              ( ULONG TrackIndex, BOOL Enable );
    HRESULT             SetTrackPriority            ( ULONG TrackIndex, D3DXPRIORITY_TYPE Priority );
    HRESULT             SetTrackSpeed               ( ULONG TrackIndex, float Speed );
    HRESULT             SetTrackWeight              ( ULONG TrackIndex, float Weight );
    HRESULT             SetPriorityBlend            ( float BlendWeight );
    HRESULT             GetTrackDesc                ( ULONG TrackIndex, D3DXTRACK_DESC * pDesc ) const;
    LPD3DXANIMATIONSET  GetTrackAnimationSet        ( ULONG TrackIndex ) const;
    float               GetPriorityBlend            ( ) const;

	// Event Sequencing Functions
    D3DXEVENTHANDLE     KeyPriorityBlend            ( float NewBlendWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition );
    D3DXEVENTHANDLE     KeyTrackEnable              ( ULONG TrackIndex, BOOL NewEnable, double StartTime );
    D3DXEVENTHANDLE     KeyTrackPosition            ( ULONG TrackIndex, double NewPosition, double StartTime );
    D3DXEVENTHANDLE     KeyTrackSpeed               ( ULONG TrackIndex, float NewSpeed, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition );
    D3DXEVENTHANDLE     KeyTrackWeight              ( ULONG TrackIndex, float NewWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition );
    HRESULT             UnkeyAllPriorityBlends      ( );
    HRESULT             UnkeyAllTrackEvents         ( ULONG TrackIndex );
    HRESULT             UnkeyEvent                  ( D3DXEVENTHANDLE hEvent );
    HRESULT             ValidateEvent               ( D3DXEVENTHANDLE hEvent );
    D3DXEVENTHANDLE     GetCurrentTrackEvent        ( ULONG TrackIndex, D3DXEVENT_TYPE EventType ) const;
    D3DXEVENTHANDLE     GetCurrentPriorityBlend     ( ) const;
    HRESULT             GetEventDesc                ( D3DXEVENTHANDLE hEvent, LPD3DXEVENT_DESC pDesc ) const;

    // Controller assignment functions
    LPD3DXANIMATIONCONTROLLER   DetachController    ( CActionStatus ** ppActionStatus = NULL );
    void                        AttachController    ( LPD3DXANIMATIONCONTROLLER pController, bool bSyncOutputs = true, CActionStatus * pActionStatus = NULL );

    // Utility Functions
    bool                IsLoaded                    ( ) const { return (m_pFrameRoot != NULL); }

protected:
    //-------------------------------------------------------------------------
    // Protected Functions for This Class
    //-------------------------------------------------------------------------
    void    DrawFrame               ( LPD3DXFRAME pFrame, long AttributeID = -1 );
    void    DrawMeshContainer       ( LPD3DXMESHCONTAINER pMeshContainer, LPD3DXFRAME pFrame, long AttributeID = -1 );
    void    UpdateFrameMatrices     ( LPD3DXFRAME pFrame, const D3DXMATRIX * pParentMatrix );
    HRESULT ApplyCustomSets         ( );
    HRESULT ApplyCallbacks          ( void * pCallbackData );
    void    ReleaseCallbackData     ( );
    void    ReleaseCallbackData     ( ID3DXAnimationSet * pSet );
    HRESULT BuildBoneMatrixPointers ( LPD3DXFRAME pFrame );

    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    LPD3DXFRAME                 m_pFrameRoot;                   // Root 'frame'
    LPD3DXANIMATIONCONTROLLER   m_pAnimController;              // Animation controller
    LPDIRECT3DDEVICE9           m_pD3DDevice;                   // Direct3D Device to use.
    CALLBACK_FUNC               m_CallBack[CALLBACK_COUNT];     // References the various callbacks
    TCHAR                       m_strActorName[MAX_PATH];       // The filename used to load the actor or later on for referencing
    D3DXMATRIX                  m_mtxWorld;                     // Currently active world matrix.
    ULONG                       m_nOptions;                     // The requested mesh options.
    ULONG                       m_nSkinMethod;                  // The preferred skinning method

	// 'Action' set data
    CActionDefinition          *m_pActions;                     // An array which stores the action data we have loaded from the .ACT file
    ULONG                       m_nActionCount;                 // The number of distinct actions we loaded.
    CActionStatus              *m_pActionStatus;                // Stores the status of all tracks for the action system

    // Cached property values
    ULONG                       m_nMaxTracks;
    ULONG                       m_nMaxAnimSets;
    ULONG                       m_nMaxAnimOutputs;
    ULONG                       m_nMaxEvents;
    ULONG                       m_nMaxCallbackKeys;

    // Temporary matrix buffer area for SKINMETHOD_SOFTWARE
    D3DXMATRIX                 *m_pSWMatrices;                  // Storage for matrices
    ULONG                       m_nMaxSWMatrices;               // Maximum software matrices required
};

//-----------------------------------------------------------------------------
// Name : CAllocateHierarchy (Class)
// Desc : A custom, derived, version of ID3DXAllocateHeirarchy. This class
//        contains custom methods that will be called by D3DX during the
//        loading of any mesh heirarchy.
//-----------------------------------------------------------------------------
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CAllocateHierarchy( CActor * pActor );
	~CAllocateHierarchy();
    
    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    STDMETHOD(CreateFrame)          (THIS_ LPCTSTR Name, LPD3DXFRAME *ppNewFrame);    
    STDMETHOD(CreateMeshContainer)  (THIS_ LPCTSTR Name, CONST D3DXMESHDATA * pMeshData,
                                     CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances,
                                     DWORD NumMaterials, CONST DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, 
                                     LPD3DXMESHCONTAINER *ppNewMeshContainer);
    STDMETHOD(DestroyFrame)         (THIS_ LPD3DXFRAME pFrameToFree);
    STDMETHOD(DestroyMeshContainer) (THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);

    //-------------------------------------------------------------------------
    // Public Custom Functions for This Class
    //-------------------------------------------------------------------------
    HRESULT BuildSkinnedMesh        ( D3DXMESHCONTAINER_DERIVED * pMeshContainer, LPD3DXMESH pMesh, LPD3DXMESH * pMeshOut );
    ULONG   DetectSkinningMethod    ( ID3DXSkinInfo * pSkinInfo, LPD3DXMESH pMesh ) const;
    ULONG   GetAttributeRemap       ( ULONG AttributeID );

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    CActor  * m_pActor;         // Actor we are allocating for

private:

    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    ULONG * m_pAttribRemap;     // Storage for any attribute re-mapping which took place
    ULONG   m_nMaterialCount;   // Used only for bounds checking in the new GetAttributeRemap() function

};

//-----------------------------------------------------------------------------
// Name : CAnimationSet (Class)
// Desc : This is our derived key frame animation set, designed to overcome
//        certain problems with the current (9.0b+) animation set causing
//        'jitters' on certain animations.
//-----------------------------------------------------------------------------
class CAnimationSet: public ID3DXAnimationSet
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CAnimationSet( ID3DXAnimationSet * pAnimSet );
    ~CAnimationSet( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    // Name
    STDMETHOD_(LPCSTR, GetName)(THIS);

    // Period
    STDMETHOD_(DOUBLE, GetPeriod)(THIS);
    STDMETHOD_(DOUBLE, GetPeriodicPosition)(THIS_ DOUBLE Position);    // Maps position into animation period

    // Animation names
    STDMETHOD_(UINT, GetNumAnimations)(THIS);
    STDMETHOD(GetAnimationNameByIndex)(THIS_ UINT Index, LPCTSTR *ppName);
    STDMETHOD(GetAnimationIndexByName)(THIS_ LPCTSTR pName, UINT *pIndex);

    // SRT
    STDMETHOD(GetSRT)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        UINT Animation,                     // Animation index
        D3DXVECTOR3 *pScale,                // Returns the scale
        D3DXQUATERNION *pRotation,          // Returns the rotation as a quaternion
        D3DXVECTOR3 *pTranslation);         // Returns the translation

    // Callbacks
    STDMETHOD(GetCallback)(THIS_ 
        DOUBLE Position,                    // Position from which to find callbacks
        DWORD Flags,                        // Callback search flags
        DOUBLE *pCallbackPosition,          // Returns the position of the callback
        LPVOID *ppCallbackData);            // Returns the callback data pointer

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    ULONG               m_nRefCount;        // Reference counter
    LPTSTR              m_strName;          // The name of this animation set
    CAnimationItem     *m_pAnimations;      // The physical animation data is stored in these items
    ULONG               m_nAnimCount;       // Number of animation items stored in the array.
    D3DXKEY_CALLBACK   *m_pCallbacks;       // List of all callbacks stored
    ULONG               m_nCallbackCount;   // Number of callbacks in the above list.
    double              m_fLength;          // Total length of this animation set (cached for simplicity)
    double              m_fTicksPerSecond;  // The number of source ticks per second
};

//-----------------------------------------------------------------------------
// Name : CAnimationItem (Class)
// Desc : The set's individual 'interpolator' or 'animation' object stores
//        all the keyframes for a frame in the heirarchy, as well as performing
//        the actual interpolation itself.
//-----------------------------------------------------------------------------
class CAnimationItem
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CAnimationItem( );
    ~CAnimationItem( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    LPCTSTR GetName     ( ) const;
    HRESULT GetSRT      ( DOUBLE PeriodicPosition, D3DXVECTOR3 *pScale, D3DXQUATERNION *pRotation, D3DXVECTOR3 *pTranslation );
    HRESULT BuildItem   ( ID3DXKeyframedAnimationSet *pAnimSet, ULONG ItemIndex );

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    LPTSTR              m_strName;              // The name of this animation set
    D3DXKEY_VECTOR3    *m_pScaleKeys;           // The scale keys
    D3DXKEY_VECTOR3    *m_pTranslateKeys;       // The translation keys
    D3DXKEY_QUATERNION *m_pRotateKeys;          // The rotation keys
    ULONG               m_nScaleKeyCount;       // Number of scale keys stored
    ULONG               m_nTranslateKeyCount;   // Number of translation keys stored
    ULONG               m_nRotateKeyCount;      // Number of rotation keys stored
    double              m_fTicksPerSecond;      // The number of source ticks per second

    ULONG               m_nLastScaleIndex;      // Cache for last 'start' scale key retrieved
    ULONG               m_nLastRotateIndex;     // Cache for last 'start' rotation key retrieved
    ULONG               m_nLastTranslateIndex;  // Cache for last 'start' translation key retrieved
    double              m_fLastPosRequest;      // Cache for last periodic position requested.
};

//-----------------------------------------------------------------------------
// Name : CActionDefinition (Class)
// Desc : Stores the definitions for an individual action.
//-----------------------------------------------------------------------------
class CActionDefinition
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerators for This Class.
    //-------------------------------------------------------------------------
    enum ActBlendMode                       // Flags for SetDefinition::BlendMode
    {
        Off          = 0,                   // No blending is to occur when this set is applied
        MixFadeGroup = 1                    // Mix this, and the previous group sets, so that the latter fades out, and the former fades in.
    };

    enum ActTimeMode                        // Flags for SetDefinition::TimeMode
    {
        Begin               = 0,            // Animation set will play from the beginning when it is applied
        MatchGroup          = 1,            // Animation set will match it's time to that of a set with the same group if currently applied, or the beginning if none are found.
        MatchSpecifiedGroup = 2             // Animation set will match the time to that of the specified group referenced in 'TimeGroup'. Allows easier syncing (for example) arms to legs.
    };

    //-------------------------------------------------------------------------
    // Public Structures for This Class.
    //-------------------------------------------------------------------------
    struct SetDefinition                    // This structure describes the animation set, and how it should be applied
    {
        TCHAR        strSetName[128];       // The name of the animation set
        TCHAR        strGroupName[128];     // The 'group' to which this belongs. This allows the system to distinguish blending and timing between similar animation sets
        float        fWeight;               // The 'strength' of which this animation set should be applied
        float        fSpeed;                // The speed of the set itself
        ActBlendMode BlendMode;             // When this set is applied, this variable describes how should it be blended in (if at all)
        ActTimeMode  TimeMode;              // Describes how the set's timing should be set-up when applied.
        float        fMixLength;            // If the blend mode dictates that the set should be mixed in, this defines how long a period that should take.
        TCHAR        strTimeGroup[128];     // If the time mode specified that we should match the timing to a specific alternate group, that group name is stored here.

        // Internal Temporary Vars
        bool         bNewTrack;             // Assign this set to a new track
        double       fNewTime;              // Use this time for the new track.
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CActionDefinition( );
    virtual ~CActionDefinition( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         LoadAction              ( ULONG nActionIndex, LPCTSTR ActFileName );
    HRESULT         ApplyAction             ( CActor * pActor );

    // Accessor functions
    LPCTSTR         GetName                 ( ) const             { return m_strName; }
    ULONG           GetSetDefinitionCount   ( ) const             { return m_nSetDefinitionCount; }
    SetDefinition * GetSetDefinition        ( ULONG Index ) const { return ( Index < m_nSetDefinitionCount ) ? &m_pSetDefinitions[ Index ] : NULL; }

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    TCHAR           m_strName[128];         // The name of this action definition
    USHORT          m_nSetDefinitionCount;  // The number of set definitions stored here
    SetDefinition * m_pSetDefinitions;      // An array of set definition structures.
};

//-----------------------------------------------------------------------------
// Name : CActionStatus (Class)
// Desc : Stores the various status flags required for tracking the action data
//-----------------------------------------------------------------------------
class CActionStatus
{
public:
    //-------------------------------------------------------------------------
    // Public Structures for This Class.
    //-------------------------------------------------------------------------
    struct TrackActionStatus                                        // Stores the status of a particular track, with regards the action system
    {
        bool                      bInUse;                           // This track is in use
        bool                      bTransitioningOut;                // The track is currently transitioning out
        TCHAR                     strGroupName[127];                // The name of the group assigned to this track
        TCHAR                     strSetName[127];                  // The name of the set assigned to this track
        const CActionDefinition * pActionDefinition;                // The actual action definition to which this track applies
        ULONG                     nSetDefinition;                   // The set definition index, of the above action, to which this track applies
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CActionStatus( );
    virtual ~CActionStatus( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    ULONG               AddRef           ( );
    ULONG               Release          ( );
    HRESULT             SetMaxTrackCount ( USHORT Count );
    void                SetCurrentAction ( CActionDefinition * pActionDefinition );
    USHORT              GetMaxTrackCount ( ) const             { return m_nMaxTrackCount; }
    TrackActionStatus * GetTrackStatus   ( ULONG Index ) const { return ( Index < m_nMaxTrackCount ) ? &m_pTrackActionStatus[ Index ] : NULL; }
    CActionDefinition * GetCurrentAction ( ) const             { return m_pCurrentAction; }

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    USHORT              m_nMaxTrackCount;           // The maximum number of tracks for this controller
    TrackActionStatus * m_pTrackActionStatus;       // An array of track status structures.
    CActionDefinition * m_pCurrentAction;           // The current action applied to the actor.
    ULONG               m_nRefCount;                // Reference count variable
};

#endif // !_CACTOR_H_