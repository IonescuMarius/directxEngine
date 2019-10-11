//-----------------------------------------------------------------------------
// File: CActor.cpp
//
// Desc: The main classes used for the actor object(s).
//
// Copyright (c) 1997-2005 GameInstitute.com. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CActor Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\dxActor.h"
#include "..\\Includes\\dxObject.h"

//-----------------------------------------------------------------------------
// Name : CActor () (Constructor)
// Desc : CActor Class Constructor
//-----------------------------------------------------------------------------
CActor::CActor()
{
	// Reset / Clear all required values
    m_pFrameRoot        = NULL;
    m_pAnimController   = NULL;
    m_pD3DDevice        = NULL;
    m_nOptions          = 0;
    
    m_pSWMatrices       = NULL;
    m_nMaxSWMatrices    = 0;
    m_nSkinMethod       = SKINMETHOD_AUTODETECT;

    m_pActions          = NULL;
    m_nActionCount      = 0;
    m_pActionStatus     = NULL;

    // Setup the defaults for the maximum controller properties
    // (Defaults to the same defaults D3DX provides).
    m_nMaxAnimSets      = 1; // <-- Initially determined by the file but default to 1 to prevent failure
    m_nMaxAnimOutputs   = 1; // <-- Initially determined by the file but default to 1 to prevent failure
    m_nMaxTracks        = 2;
    m_nMaxEvents        = 30;
    m_nMaxCallbackKeys  = 1024; // Max callback keys collected by the Registered callback function

    ZeroMemory( m_strActorName, MAX_PATH * sizeof(TCHAR) );
    D3DXMatrixIdentity( &m_mtxWorld );
    
    // Clear structures
    for ( ULONG i = 0; i < CALLBACK_COUNT; ++i ) ZeroMemory( &m_CallBack[i], sizeof(CALLBACK_FUNC) );
}

//-----------------------------------------------------------------------------
// Name : ~CActor () (Destructor)
// Desc : CActor Class Destructor
//-----------------------------------------------------------------------------
CActor::~CActor( )
{
    // Release the actor objects
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release any and all objects, data etc.
//-----------------------------------------------------------------------------
void CActor::Release()
{
    CAllocateHierarchy Allocator( this );

    // Release any active callback data
    ReleaseCallbackData( );

    // Release objects (notice the specific method for releasing the root frame)
    if ( m_pActionStatus   ) m_pActionStatus->Release();
    if ( m_pActions        ) delete []m_pActions;
    if ( m_pSWMatrices     ) delete []m_pSWMatrices;
    if ( m_pFrameRoot      ) D3DXFrameDestroy( m_pFrameRoot, &Allocator );
    if ( m_pAnimController ) m_pAnimController->Release();
    if ( m_pD3DDevice      ) m_pD3DDevice->Release();

    // Reset / Clear all required values
    m_pFrameRoot      = NULL;
    m_pAnimController = NULL;
    m_pD3DDevice      = NULL;
    m_nOptions        = 0;
    m_pSWMatrices     = NULL;
    m_nMaxSWMatrices  = 0;
    m_pActions        = NULL;
    m_nActionCount    = 0;
    m_pActionStatus   = NULL;

    ZeroMemory( m_strActorName, MAX_PATH * sizeof(TCHAR) );

    // Since 'Release' is called just prior to loading, we should not
    // clear ANYTHING which is designed to be setup and stored prior
    // to the user making that load call. This includes things like
    // the world matrix, and the callbacks.
}

//-----------------------------------------------------------------------------
// Name : ReleaseCallbackData ()
// Desc : Release all of the callback data items stored.
//-----------------------------------------------------------------------------
void CActor::ReleaseCallbackData( )
{
    HRESULT             hRet;
    ULONG               i, SetCount;
    ID3DXAnimationSet * pSet = NULL;

    if ( !m_pAnimController ) return;

    // For each animation set, release the callback data.
    for ( i = 0, SetCount = m_pAnimController->GetNumAnimationSets(); i < SetCount; ++i )
    {
        // Retrieve the animation set
        hRet = m_pAnimController->GetAnimationSet( i, &pSet );
        if ( FAILED(hRet) ) continue;

        // Release them for this set
        ReleaseCallbackData( pSet );

        // Release the set itself
        pSet->Release();

    } // Next Animation Set
}

//-----------------------------------------------------------------------------
// Name : ReleaseCallbackData ()
// Desc : Release the callback data items for the set specified
//-----------------------------------------------------------------------------
void CActor::ReleaseCallbackData( ID3DXAnimationSet * pSet )
{
    double     Position  = 0.0;
    ULONG      Flags     = 0;
    IUnknown * pCallback = NULL;

    // Keep searching until we run out of callbacks
    for ( ; SUCCEEDED(pSet->GetCallback( Position, Flags, &Position, (void**)&pCallback )) ; )
    {
        // Ensure we only get the first available (GetCallback will loop forever).
        if ( Position >= pSet->GetPeriod() ) break;

        // Release the callback (it is required that this be an 
        // instance of an IUnknown derived class)
        if ( pCallback ) pCallback->Release();

        // Exclude this position from now on
        Flags = D3DXCALLBACK_SEARCH_EXCLUDING_INITIAL_POSITION; 

    } // Next Callback

}

//-----------------------------------------------------------------------------
// Name : RegisterCallback () (Destructor)
// Desc : Registers a callback function for one of the callback types.
//-----------------------------------------------------------------------------
bool CActor::RegisterCallback( CALLBACK_TYPE Type, LPVOID pFunction, LPVOID pContext )
{
    // Validate Parameters
    if ( Type > CALLBACK_COUNT ) return false;

    // You cannot set the functions to anything other than NULL
    // if hierarchy data already exists (i.e. it's too late to change your mind :)
    if ( pFunction != NULL && IsLoaded() ) return false;

    // Store function pointer and context
    m_CallBack[ Type ].pFunction = pFunction;
    m_CallBack[ Type ].pContext  = pContext;

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : SetSkinningMethod ()
// Desc : Specify the preferred skinning method. This is used on any subsequent
//        LoadActorFromX call.
//-----------------------------------------------------------------------------
void CActor::SetSkinningMethod( ULONG SkinMethod )
{
    // Store the preferred method.
    m_nSkinMethod = SkinMethod;
}

//-----------------------------------------------------------------------------
// Name : GetSkinningMethod ()
// Desc : Retrieve the preferred skinning method from the class.
//-----------------------------------------------------------------------------
ULONG CActor::GetSkinningMethod( ) const
{
    return m_nSkinMethod;
}

//-----------------------------------------------------------------------------
// Name : LoadActorFromX ()
// Desc : Load an actor from file.
//-----------------------------------------------------------------------------
HRESULT CActor::LoadActorFromX( LPCTSTR FileName, ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice )
{
    HRESULT hRet;
    CAllocateHierarchy Allocator( this );

    // Validate parameters
    if ( !FileName || !pD3DDevice ) return D3DERR_INVALIDCALL;

    // Release previous data!
    Release();

    // Store the D3D Device here
    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    // Store options
    m_nOptions = Options;

    // Load the mesh heirarchy
    hRet = D3DXLoadMeshHierarchyFromX( FileName, Options, pD3DDevice, &Allocator, NULL, &m_pFrameRoot, NULL );
    if ( FAILED(hRet) ) return hRet;

    // Copy the filename over
    _tcscpy( m_strActorName, FileName );

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : LoadActorFromX ()
// Desc : Load an actor from file.
//-----------------------------------------------------------------------------
HRESULT CActor::LoadActorFromX2( LPCTSTR FileName, ULONG Options, LPDIRECT3DDEVICE9 pD3DDevice, bool bApplyCustomSets /* = true */, void * pCallbackData /* = NULL */ )
{
    HRESULT hRet;
    CAllocateHierarchy Allocator( this );

    // Validate parameters
    if ( !FileName || !pD3DDevice ) return D3DERR_INVALIDCALL;

    // Release previous data!
    Release();

    // Store the D3D Device here
    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    // Store options
    m_nOptions = Options;

    // Load the mesh heirarchy and the animation data etc.
    hRet = D3DXLoadMeshHierarchyFromX( FileName, Options, pD3DDevice, &Allocator, NULL, &m_pFrameRoot, &m_pAnimController );
	//hRet = D3DXLoadMeshHierarchyFromX( FileName, Options, pD3DDevice, &Allocator, NULL, &m_pFrameRoot, NULL );
    if ( FAILED(hRet) ) return hRet;

    // Build the bone matrix tables for all skinned meshes stored here
    if ( m_pFrameRoot )
    {
        hRet = BuildBoneMatrixPointers( m_pFrameRoot );
        if ( FAILED(hRet) ) return hRet;
    
    } // End if no hierarchy

    // Copy the filename over
    _tcscpy( m_strActorName, FileName );

    // Apply our derived animation sets
    if ( m_pAnimController )
    {
        // Apply our default limits if they were set before we loaded
        // Specifying no parameters will ensure that the last set details are re-used.
        SetActorLimits( );

        // If there is a callback key function registered, collect the callbacks
        if ( m_CallBack[CALLBACK_CALLBACKKEYS].pFunction != NULL ) ApplyCallbacks( pCallbackData );

        // Apply the custom animation set classes if we are requested to do so.
        if ( bApplyCustomSets == true )
        {
            hRet = ApplyCustomSets();
            if ( FAILED(hRet) ) return hRet;

        } // End if swap sets

    } // End if any animation data

    // Success!!
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
HRESULT CActor::SaveActorToX( LPCTSTR FileName, ULONG Format )
{
    HRESULT hRet;

    // Validate parameters
    if ( !FileName ) return D3DERR_INVALIDCALL;

    // If we are NOT managing our own attributes, fail
    if ( GetCallback( CActor::CALLBACK_ATTRIBUTEID ).pFunction != NULL ) return D3DERR_INVALIDCALL;

    // Save the hierarchy back out to file
    hRet = D3DXSaveMeshHierarchyToFile( FileName, Format, m_pFrameRoot, m_pAnimController, NULL );
    if ( FAILED(hRet) ) return hRet;

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : LoadActionDefinitions ()
// Desc : Loads and processes the definitions for the pre-defined actions
//        used via the 'ApplyAction' function.
//-----------------------------------------------------------------------------
HRESULT CActor::LoadActionDefinitions( LPCTSTR FileName )
{
    ULONG   ActionCount = 0, i;
    HRESULT hRet;

    // Clear any previous action definitions
    if ( m_pActions ) delete []m_pActions;
    m_pActions     = NULL;
    m_nActionCount = 0;

    // Clear any previous action status
    if ( m_pActionStatus ) m_pActionStatus->Release();
    m_pActionStatus = NULL;

    // Retrieve the number of actions stored in the file
    ActionCount = ::GetPrivateProfileInt( _T("General"), _T("ActionCount"), 0, FileName );
    if ( ActionCount == 0 ) return D3DERR_NOTFOUND;

    // Allocate the new actions array
    m_pActions = new CActionDefinition[ ActionCount ];
    if ( !m_pActions ) return E_OUTOFMEMORY;

    // Allocate the new status structure
    m_pActionStatus = new CActionStatus;
    if ( !m_pActionStatus ) return E_OUTOFMEMORY;

    // Setup max tracks for action status
    m_pActionStatus->SetMaxTrackCount( (USHORT)GetMaxNumTracks() );

    // Store the count
    m_nActionCount = ActionCount;

    // Process the actions
    for ( i = 0; i < ActionCount; ++i )
    {
        // Load the definition from file
        hRet = m_pActions[i].LoadAction( i + 1, FileName );
        if ( FAILED(hRet) ) return hRet;

    } // Next Action in file

    // Success!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : ApplAction ()
// Desc : Given the name of a specific action, this function set's up the
//        animation controller to carry out that specified action.
//-----------------------------------------------------------------------------
HRESULT CActor::ApplyAction( LPCTSTR ActionName )
{
    ULONG   i;
    HRESULT hRet;

    // Bail if there is no active controller
    if ( !m_pAnimController || !m_pActionStatus ) return D3DERR_INVALIDCALL;

    // If this action is already set, ignore
    CActionDefinition * pCurrentAction = m_pActionStatus->GetCurrentAction();
    if ( pCurrentAction && _tcsicmp( pCurrentAction->GetName(), ActionName ) == 0 ) return D3D_OK;

    // Loop through each action
    for ( i = 0; i < m_nActionCount; ++i )
    {
        CActionDefinition * pAction = &m_pActions[i];

        // Compare the name, and apply if it matches
        if ( _tcsicmp( ActionName, pAction->GetName() ) == 0 )
        {
            // Apply the action
            hRet = pAction->ApplyAction( this );
            if ( FAILED(hRet) ) return hRet;

            // Store the action
            m_pActionStatus->SetCurrentAction( pAction );

            // Success!
            return D3D_OK;

        }  // End if names match
    
    } // Next action

    // Unable to find this action
    return D3DERR_NOTFOUND;
}

//-----------------------------------------------------------------------------
// Name : GetCurrentAction ()
// Desc : Returns the currently active action definition.
//-----------------------------------------------------------------------------
CActionDefinition * CActor::GetCurrentAction( ) const
{
    // Return the current action if status is available
    if ( !m_pActionStatus ) return NULL;
    return m_pActionStatus->GetCurrentAction();
}

//-----------------------------------------------------------------------------
// Name : GetActionStatus ()
// Desc : Retrieve the status structure, which describes the state of each
//        track, specifically regarding the action system.
//-----------------------------------------------------------------------------
CActionStatus * CActor::GetActionStatus( ) const
{
    // Validate requirements
    if ( !m_pActionStatus ) return NULL;

    // Add a reference to the action status, we have a floating pointer 
    m_pActionStatus->AddRef();

    // Return the pointer
    return m_pActionStatus;
}

//-----------------------------------------------------------------------------
// Name : ApplyCustomSets ()
// Desc : Replaces all animation sets with our own custom versions.
//-----------------------------------------------------------------------------
HRESULT CActor::ApplyCustomSets( )
{
    ULONG   i;
    HRESULT hRet;
    CAnimationSet       * pNewAnimSet = NULL;
    ID3DXAnimationSet   * pAnimSet    = NULL, ** ppSetList = NULL;

    // Validate
    if ( !m_pAnimController || !m_pAnimController->GetNumAnimationSets( ) ) return D3DERR_INVALIDCALL;

    // Store set count
    ULONG SetCount = m_pAnimController->GetNumAnimationSets( );

    // Allocate temporary storage for our new animation sets here
    // We have to store them, rather than doing it on the fly because
    // when calling "RegisterAnimationSet", they aren't necessarily added
    // to the 'tail' end of an internal array, they can end up anywhere
    // in the list (presumably they are sorted by name). This means we 
    // must unregister all sets first and then store the new ones in one go
    // via a separate pass.
    ppSetList = new ID3DXAnimationSet*[ SetCount ];
    if ( !ppSetList ) return E_OUTOFMEMORY;

    // Clear the array
    ZeroMemory( ppSetList, SetCount * sizeof(ID3DXAnimationSet*) );

    // Loop through each animation set and copy into our set list array. Take care to
    // store the set count first as the value in the animation controller
    // will decrement when we unregister
    for ( i = 0; i < SetCount; ++i )
    {
        // Keep retrieving index 0 (remember we unregister each time)
        hRet = m_pAnimController->GetAnimationSet( 0, &pAnimSet );
        if ( FAILED( hRet ) ) continue;

        // Store this set in our list, and AddRef
        ppSetList[ i ] = pAnimSet;
        pAnimSet->AddRef();

        // Unregister the old animation sets first otherwise we won't have enough
        // 'slots' for the new ones.
        m_pAnimController->UnregisterAnimationSet( pAnimSet );

        // Release this set, leaving only the item in the set list.
        pAnimSet->Release();

    } // Next Set

    // Loop through each animation set and rebuild a new custom one.
    for ( i = 0; i < SetCount; ++i )
    {
        // Skip if we didn't retrieve anything
        if ( ppSetList[i] == NULL ) continue;

        // Allocate a new animation set, duplicating the one we just retreived
        // Note : Because this is done in the constructor, we catch the
        // exceptions because no return value is available.
        try
        {
            // Duplicate
            pNewAnimSet = new CAnimationSet( ppSetList[i] );

        } // End Try Block
        catch ( HRESULT & )
        {
            // Release the new animation set
            delete pNewAnimSet;

            // Just skip this animation set (leave it in the array)
            continue;

        } // End Catch block
        catch ( ... )
        {
            // Just skip this animation set (leave it in the array)
            continue;

        } // Catch all other errors (inc. out of memory)

        // Release this set, memory will now be freed
        ppSetList[i]->Release();

        // Store the new animation set in our temporary array
        ppSetList[i] = pNewAnimSet;

    } // Next Set

    // Second pass, register all new animation sets
    for ( i = 0; i < SetCount; ++i )
    {
        // Register our brand new set. (AddRef is called)
        m_pAnimController->RegisterAnimationSet( ppSetList[i] );

        // Release our hold on the custom animation set
        ppSetList[i]->Release();

    } // Next Set

    // Free up any temporary memory
    delete []ppSetList;

    // Set the first animation set into the first track, otherwise
    // nothing will play, since all previous tracks have been unregistered.
    SetTrackAnimationSetByIndex( 0, 0 );

    // We're all done
    return D3D_OK;

}

//-----------------------------------------------------------------------------
// Name : DetachController ()
// Desc : Retrieve and detach the animation controller from this actor.
//-----------------------------------------------------------------------------
LPD3DXANIMATIONCONTROLLER CActor::DetachController( CActionStatus ** ppActionStatus /* = NULL */ )
{
    LPD3DXANIMATIONCONTROLLER pController = m_pAnimController;
    
    // Detach from us
    m_pAnimController = NULL;

    // If the user requested to detach action status information, copy it over
    if ( ppActionStatus )
    {
        *ppActionStatus = m_pActionStatus;
        m_pActionStatus = NULL;
    
    } // End if requested action status

    // Note, we would AddRef here before we returned, but also
    // release our reference, so this is a no-op. Just return the pointer.
    return pController;

}

//-----------------------------------------------------------------------------
// Name : AttachController ()
// Desc : Attach a new animation controller to this actor. 
//-----------------------------------------------------------------------------
void CActor::AttachController( LPD3DXANIMATIONCONTROLLER pController, bool bSyncOutputs /* = true */, CActionStatus * pActionStatus /* = NULL */ )
{
    // Release our current controller. If the user want's to
    // maintain this object it must be detached first.
    if ( m_pAnimController ) m_pAnimController->Release();
    m_pAnimController = NULL;

    // Release our current action status (if provided). 
    // If the user want's to maintain this object it must be detached first.
    if ( m_pActionStatus ) m_pActionStatus->Release();
    m_pActionStatus = NULL;

    // If there is no controller, we will assume that they are just 
    // using 'AttachController( NULL )' to destroy the animation controller 
    // already stored here.
    if ( pController )
    {
        // Store the new controller
        m_pAnimController = pController;

        // Add ref, we're storing a pointer to it
        m_pAnimController->AddRef();

        // Synchronise our frame matrices if requested
        if ( bSyncOutputs ) m_pAnimController->AdvanceTime( 0.0f, NULL );

    } // End if controller specified

    // Similarly for the action status
    if ( pActionStatus )
    {
        // Store the new status information
        m_pActionStatus = pActionStatus;

        // Add ref, we're storing a pointer to it
        m_pActionStatus->AddRef();
    
    } // End if status specified
}

//-----------------------------------------------------------------------------
// Name : ApplyCallbacks ()
// Desc : Retrieves all callbacks from the application as requested and 
//        clones the animation sets, storing this passed data.
// Note : Until there is a function for cloning animation sets, or storing
//        callbacks after the fact, we have to go through this pain.
//-----------------------------------------------------------------------------
HRESULT CActor::ApplyCallbacks( void * pCallbackData )
{
    ULONG   i, j;
    HRESULT hRet;
    ID3DXKeyframedAnimationSet * pNewAnimSet = NULL, *pKeySet = NULL;
    ID3DXAnimationSet          * pAnimSet    = NULL, **ppSetList = NULL;
    D3DXKEY_CALLBACK           * pCallbacks  = NULL;

    // Validate
    if ( !m_pAnimController || !m_pAnimController->GetNumAnimationSets( ) ) return D3DERR_INVALIDCALL;

    // Store set count
    ULONG SetCount = m_pAnimController->GetNumAnimationSets( );

    // Allocate our temporary key buffer
    pCallbacks = new D3DXKEY_CALLBACK[ m_nMaxCallbackKeys ];
    if ( !pCallbacks ) return E_OUTOFMEMORY;

    // Allocate temporary storage for our new animation sets here
    // We have to store them, rather than doing it on the fly because
    // when calling "RegisterAnimationSet", they aren't necessarily added
    // to the 'tail' end of an internal array, they can end up anywhere
    // in the list (presumably they are sorted by name). This means we 
    // must unregister all sets first and then store the new ones in one go
    // via a separate pass.
    ppSetList = new ID3DXAnimationSet*[ SetCount ];
    if ( !ppSetList ) { delete pCallbacks; return E_OUTOFMEMORY; }

    // Clear the array
    ZeroMemory( ppSetList, SetCount * sizeof(ID3DXAnimationSet*) );

    // Loop through each animation set and copy into our set list array. Take care to
    // store the set count first as the value in the animation controller
    // will decrement when we unregister
    for ( i = 0; i < SetCount; ++i )
    {
        // Keep retrieving index 0 (remember we unregister each time)
        hRet = m_pAnimController->GetAnimationSet( 0, &pAnimSet );
        if ( FAILED( hRet ) ) continue;

        // Store this set in our list, and AddRef
        ppSetList[ i ] = pAnimSet;
        pAnimSet->AddRef();

        // Unregister the old animation sets first otherwise we won't have enough
        // 'slots' for the new ones.
        m_pAnimController->UnregisterAnimationSet( pAnimSet );

        // Release this set, leaving only the item in the set list.
        pAnimSet->Release();

    } // Next Set

    // Loop through each animation set and rebuild a new one containing the callbacks.
    for ( i = 0; i < SetCount; ++i )
    {
        // Skip if we didn't manage to retrieve
        if ( ppSetList[i] == NULL ) continue;

        // Retrieve the set we're working on
        pAnimSet = ppSetList[i];

        // Query the interface to get a keyframed animation set
        hRet = pAnimSet->QueryInterface( IID_ID3DXKeyframedAnimationSet, (void**)&pKeySet );
        if ( FAILED(hRet) ) continue;

        // Retrieve our application's registered function pointer
        COLLECTCALLBACKS CollectCallbacks = (COLLECTCALLBACKS)m_CallBack[ CALLBACK_CALLBACKKEYS ].pFunction;

        // Request that our key buffer be filled
        ULONG KeyCount = CollectCallbacks( m_CallBack[ CALLBACK_CALLBACKKEYS ].pContext, m_strActorName, pCallbackData, pKeySet, pCallbacks );

        // If no keys are to be added, leave the existing one alone.
        if ( KeyCount == 0 ) { pKeySet->Release(); continue; }

        // Create a new animation set for us to copy data into
        hRet = D3DXCreateKeyframedAnimationSet( pKeySet->GetName(), pKeySet->GetSourceTicksPerSecond(), pKeySet->GetPlaybackType(),
            pKeySet->GetNumAnimations(), KeyCount, pCallbacks, &pNewAnimSet );
        if ( FAILED(hRet) ) { pKeySet->Release(); continue; }

        // Copy over the data from the old animation set (we've already stored the keys)
        for ( j = 0; j < pKeySet->GetNumAnimations(); ++j )
        {
            LPCTSTR               strName = NULL;
            ULONG                 ScaleKeyCount, RotateKeyCount, TranslateKeyCount;
            D3DXKEY_VECTOR3     * pScaleKeys = NULL, * pTranslateKeys = NULL;
            D3DXKEY_QUATERNION  * pRotateKeys = NULL;

            // Get the old animation sets details
            ScaleKeyCount     = pKeySet->GetNumScaleKeys( j );
            RotateKeyCount    = pKeySet->GetNumRotationKeys( j );
            TranslateKeyCount = pKeySet->GetNumTranslationKeys( j );
            pKeySet->GetAnimationNameByIndex( j, &strName );

            // Allocate any memory required
            if ( ScaleKeyCount     ) pScaleKeys     = new D3DXKEY_VECTOR3[ ScaleKeyCount ];
            if ( RotateKeyCount    ) pRotateKeys    = new D3DXKEY_QUATERNION[ RotateKeyCount ];
            if ( TranslateKeyCount ) pTranslateKeys = new D3DXKEY_VECTOR3[ TranslateKeyCount ];

            // Retrieve the keys
            if ( pScaleKeys     ) pKeySet->GetScaleKeys( j, pScaleKeys );
            if ( pRotateKeys    ) pKeySet->GetRotationKeys( j, pRotateKeys );
            if ( pTranslateKeys ) pKeySet->GetTranslationKeys( j, pTranslateKeys );

            // Register the keys here
            hRet = pNewAnimSet->RegisterAnimationSRTKeys( strName, ScaleKeyCount, RotateKeyCount, TranslateKeyCount,
                pScaleKeys, pRotateKeys, pTranslateKeys, NULL );

            // Delete the temporary key buffers
            if ( pScaleKeys     ) delete []pScaleKeys;
            if ( pRotateKeys    ) delete []pRotateKeys;
            if ( pTranslateKeys ) delete []pTranslateKeys;

            // If we failed, break out
            if ( FAILED(hRet) ) break;

        } // Next Animation 'item'

        // If we didn't reach the end, this is a failure
        if ( j != pKeySet->GetNumAnimations() )
        { 
            // Release new set, and reference to old
            pKeySet->Release(); 
            pNewAnimSet->Release();
            continue;

        } // End if failed

        // Release our duplicated set pointer
        pKeySet->Release();

        // Release the actual animation set, memory will now be freed
        pAnimSet->Release();

        // Store the new animation set in our temporary array
        ppSetList[i] = pNewAnimSet;

    } // Next Set

    // Second pass, register all new animation sets
    for ( i = 0; i < SetCount; ++i )
    {
        // Nothing stored here?
        if ( !ppSetList[i] ) continue;

        // Register our brand new set. (AddRef is called)
        m_pAnimController->RegisterAnimationSet( ppSetList[i] );

        // Release our hold on the custom animation set
        ppSetList[i]->Release();

    } // Next Set

    // Free up any temporary memory
    delete []ppSetList;
    delete []pCallbacks;

    // Set the first animation set into the first track, otherwise
    // nothing will play, since all previous tracks have potentially been unregistered.
    SetTrackAnimationSetByIndex( 0, 0 );

    // We're all done
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : ApplySplitDefinitions ()
// Desc : Using the split definition entries provided, split up the animation
//        sets into one or more duplicates, to allow the user to take
//        an input .x file which may only have one set, and separate out frames
//-----------------------------------------------------------------------------
HRESULT CActor::ApplySplitDefinitions( ULONG nDefCount, AnimSplitDefinition pDefList[] )
{
    HRESULT                     hRet;
    LPCTSTR                     strName;
    ULONG                       i, j, k;
    LPD3DXANIMATIONSET          pAnimSet;
    LPD3DXKEYFRAMEDANIMATIONSET pSourceAnimSet, pDestAnimSet;
    D3DXKEY_VECTOR3            *ScaleKeys     = NULL, *NewScaleKeys = NULL;
    D3DXKEY_VECTOR3            *TranslateKeys = NULL, *NewTranslateKeys = NULL;
    D3DXKEY_QUATERNION         *RotateKeys    = NULL, *NewRotateKeys = NULL;
    ULONG                       ScaleKeyCount, RotationKeyCount, TranslationKeyCount, AnimationCount;
    ULONG                       NewScaleKeyCount, NewRotationKeyCount, NewTranslationKeyCount;
    D3DXVECTOR3                 StartScale, EndScale, StartTranslate, EndTranslate;
    D3DXQUATERNION              StartRotate, EndRotate;

    // Validate pre-requisites
    if ( !m_pAnimController || !IsLoaded() || nDefCount == 0 ) return D3DERR_INVALIDCALL;

    // Clone our animation controller if there are not enough set slots available
    if ( m_pAnimController->GetMaxNumAnimationSets() < nDefCount )
    {
        LPD3DXANIMATIONCONTROLLER pNewController = NULL;

        // Clone the animation controller
        hRet = m_pAnimController->CloneAnimationController( m_pAnimController->GetMaxNumAnimationOutputs(),
            nDefCount, m_pAnimController->GetMaxNumTracks(),
            m_pAnimController->GetMaxNumEvents(), &pNewController );
        if ( FAILED(hRet) ) return hRet;

        // Release our old controller
        m_pAnimController->Release();

        // Store the new controller
        m_pAnimController = pNewController;

    } // End if too small!

    // We're now going to start building for each definition
    for ( i = 0; i < nDefCount; ++i )
    {
        // First retrieve the animation set
        if ( FAILED(m_pAnimController->GetAnimationSet( pDefList[i].SourceSet, &pAnimSet )) ) continue;

        // We have to query this to determine if it's key framed
        if ( FAILED( pAnimSet->QueryInterface( IID_ID3DXKeyframedAnimationSet, (LPVOID*)&pSourceAnimSet ) ) )
        {
            // Release animation set and continue
            pAnimSet->Release();
            continue;

        } // End if failed to retrieve key framed set

        // We can release the original animation set now, we've 'queried' it
        pAnimSet->Release();

        // Create a new 'destination' keyframed animation set
        hRet = D3DXCreateKeyframedAnimationSet( pDefList[i].SetName, pSourceAnimSet->GetSourceTicksPerSecond(), pSourceAnimSet->GetPlaybackType(), pSourceAnimSet->GetNumAnimations(), 0, NULL, &pDestAnimSet );
        if ( FAILED(hRet) ) { pSourceAnimSet->Release(); continue; }

        // Loop through the animations stored in the set
        AnimationCount = pSourceAnimSet->GetNumAnimations();
        for ( j = 0; j < AnimationCount; ++j )
        {
            // Get name
            pSourceAnimSet->GetAnimationNameByIndex( j, &strName );

            // Retrieve all the key counts etc
            ScaleKeyCount           = pSourceAnimSet->GetNumScaleKeys( j );
            RotationKeyCount        = pSourceAnimSet->GetNumRotationKeys( j );
            TranslationKeyCount     = pSourceAnimSet->GetNumTranslationKeys( j );
            NewScaleKeyCount        = 0;
            NewRotationKeyCount     = 0;
            NewTranslationKeyCount  = 0;

            // Allocate enough memory for the keys
            if ( ScaleKeyCount ) ScaleKeys = new D3DXKEY_VECTOR3[ ScaleKeyCount ];
            if ( TranslationKeyCount ) TranslateKeys = new D3DXKEY_VECTOR3[ TranslationKeyCount ];
            if ( RotationKeyCount ) RotateKeys = new D3DXKEY_QUATERNION[ RotationKeyCount ];

            // Allocate enough memory (total) for our new keys, + 2 for potential start and end keys
            if ( ScaleKeyCount ) NewScaleKeys = new D3DXKEY_VECTOR3[ ScaleKeyCount + 2 ];
            if ( TranslationKeyCount ) NewTranslateKeys = new D3DXKEY_VECTOR3[ TranslationKeyCount + 2 ];
            if ( RotationKeyCount ) NewRotateKeys = new D3DXKEY_QUATERNION[ RotationKeyCount + 2 ];

            // Retrieve the physical keys
            if ( ScaleKeyCount ) pSourceAnimSet->GetScaleKeys( j, ScaleKeys );
            if ( TranslationKeyCount ) pSourceAnimSet->GetTranslationKeys( j, TranslateKeys );
            if ( RotationKeyCount ) pSourceAnimSet->GetRotationKeys( j, RotateKeys );

            // Get the SRT data at the start and end ticks (just in case we need to insert them)
            pSourceAnimSet->GetSRT( (double)pDefList[i].StartTicks / pSourceAnimSet->GetSourceTicksPerSecond(), j, &StartScale, &StartRotate, &StartTranslate );
            pSourceAnimSet->GetSRT( (double)pDefList[i].EndTicks / pSourceAnimSet->GetSourceTicksPerSecond(), j, &EndScale, &EndRotate, &EndTranslate );

            // Swap 'winding', we're adding these directly backto the anim controller, not using directly.
            D3DXQuaternionConjugate( &StartRotate, &StartRotate );
            D3DXQuaternionConjugate( &EndRotate, &EndRotate );

            // ******************************************************************
            // * SCALE KEYS
            // ******************************************************************
            for ( k = 0; k < ScaleKeyCount; ++k )
            {
                D3DXKEY_VECTOR3 * pScale = &ScaleKeys[k];

                // Skip all keys prior to the start time
                if ( pScale->Time < pDefList[i].StartTicks ) continue;

                // Have we hit a key past our last time?
                if ( pScale->Time > pDefList[i].EndTicks ) break;

                // If we got here we're within range. If this is the first key, we may need to add one
                if ( NewScaleKeyCount == 0 && pScale->Time != pDefList[i].StartTicks )
                {
                    // Insert the interpolated start key
                    NewScaleKeys[ NewScaleKeyCount ].Time  = 0.0f;
                    NewScaleKeys[ NewScaleKeyCount ].Value = StartScale;
                    NewScaleKeyCount++;

                } // End if insert of interpolated key is required

                // Copy over the key and subtract start time
                NewScaleKeys[ NewScaleKeyCount ] = *pScale;
                NewScaleKeys[ NewScaleKeyCount ].Time -= (float)pDefList[i].StartTicks;
                NewScaleKeyCount++;

            } // Next Key

            // Last key matched end time?
            if ( NewScaleKeys[NewScaleKeyCount - 1].Time != (float)(pDefList[i].EndTicks - pDefList[i].StartTicks) )
            {
                // Insert the interpolated end key
                NewScaleKeys[ NewScaleKeyCount ].Time  = (float)(pDefList[i].EndTicks - pDefList[i].StartTicks);
                NewScaleKeys[ NewScaleKeyCount ].Value = EndScale;
                NewScaleKeyCount++;

            } // End if insert of interpolated key is required

            // ******************************************************************
            // * TRANSLATION KEYS
            // ******************************************************************
            for ( k = 0; k < TranslationKeyCount; ++k )
            {
                D3DXKEY_VECTOR3 * pTranslate = &TranslateKeys[k];

                // Skip all keys prior to the start time
                if ( pTranslate->Time < pDefList[i].StartTicks ) continue;

                // Have we hit a key past our last time?
                if ( pTranslate->Time > pDefList[i].EndTicks ) break;

                // If we got here we're within range. If this is the first key, we may need to add one
                if ( NewTranslationKeyCount == 0 && pTranslate->Time != pDefList[i].StartTicks )
                {
                    // Insert the interpolated start key
                    NewTranslateKeys[ NewTranslationKeyCount ].Time  = 0.0f;
                    NewTranslateKeys[ NewTranslationKeyCount ].Value = StartTranslate;
                    NewTranslationKeyCount++;

                } // End if insert of interpolated key is required

                // Copy over the key and subtract start time
                NewTranslateKeys[ NewTranslationKeyCount ] = *pTranslate;
                NewTranslateKeys[ NewTranslationKeyCount ].Time -= (float)pDefList[i].StartTicks;
                NewTranslationKeyCount++;

            } // Next Key

            // Last key matched end time?
            if ( NewTranslateKeys[NewTranslationKeyCount - 1].Time != (float)(pDefList[i].EndTicks - pDefList[i].StartTicks) )
            {
                // Insert the interpolated end key
                NewTranslateKeys[ NewTranslationKeyCount ].Time  = (float)(pDefList[i].EndTicks - pDefList[i].StartTicks);
                NewTranslateKeys[ NewTranslationKeyCount ].Value = EndTranslate;
                NewTranslationKeyCount++;

            } // End if insert of interpolated key is required

            // ******************************************************************
            // * ROTATION KEYS
            // ******************************************************************
            for ( k = 0; k < RotationKeyCount; ++k )
            {
                D3DXKEY_QUATERNION * pRotate = &RotateKeys[k];

                // Skip all keys prior to the start time
                if ( pRotate->Time < pDefList[i].StartTicks ) continue;

                // Have we hit a key past our last time?
                if ( pRotate->Time > pDefList[i].EndTicks ) break;

                // If we got here we're within range. If this is the first key, we may need to add one
                if ( NewRotationKeyCount == 0 && pRotate->Time != pDefList[i].StartTicks )
                {
                    // Insert the interpolated start key
                    NewRotateKeys[ NewRotationKeyCount ].Time  = 0.0f;
                    NewRotateKeys[ NewRotationKeyCount ].Value = StartRotate;
                    NewRotationKeyCount++;

                } // End if insert of interpolated key is required

                // Copy over the key and subtract start time
                NewRotateKeys[ NewRotationKeyCount ] = *pRotate;
                NewRotateKeys[ NewRotationKeyCount ].Time -= (float)pDefList[i].StartTicks;
                NewRotationKeyCount++;

            } // Next Key

            // Last key matched end time?
            if ( NewRotateKeys[NewRotationKeyCount - 1].Time != (float)(pDefList[i].EndTicks - pDefList[i].StartTicks) )
            {
                // Insert the interpolated end key
                NewRotateKeys[ NewRotationKeyCount ].Time  = (float)(pDefList[i].EndTicks - pDefList[i].StartTicks);
                NewRotateKeys[ NewRotationKeyCount ].Value = EndRotate;
                NewRotationKeyCount++;

            } // End if insert of interpolated key is required

            // Register this with our destination set
            pDestAnimSet->RegisterAnimationSRTKeys( strName, NewScaleKeyCount, NewRotationKeyCount, NewTranslationKeyCount, NewScaleKeys, NewRotateKeys, NewTranslateKeys, NULL );

            // Clean up
            if ( ScaleKeys ) delete []ScaleKeys; ScaleKeys = NULL;
            if ( TranslateKeys ) delete []TranslateKeys; TranslateKeys = NULL;
            if ( RotateKeys ) delete []RotateKeys; RotateKeys = NULL;

            if ( NewScaleKeys ) delete []NewScaleKeys; NewScaleKeys = NULL;
            if ( NewTranslateKeys ) delete []NewTranslateKeys; NewTranslateKeys = NULL;
            if ( NewRotateKeys ) delete []NewRotateKeys; NewRotateKeys = NULL;

        } // Next Animation

        // We're done with the source 'keyframed' animation set
        pSourceAnimSet->Release();

        // Store the dest animation set for use later
        pDefList[i].Reserved = (LPVOID)pDestAnimSet;

    } // Next Definition

    // Release all animation sets from the animation controller 
    // (Note: be careful because the GetNumAnimationSets result, and the values passed in to GetAnimationSet alter ;)
    for ( ; m_pAnimController->GetNumAnimationSets() > 0; )
    {
        // Retrieve the animation set we want to remove
        m_pAnimController->GetAnimationSet( 0, &pAnimSet );

        // Unregister it (this will release inside anim controller)
        m_pAnimController->UnregisterAnimationSet( pAnimSet );

        // Now release the last known copy
        pAnimSet->Release();

    } // Next Animation Set

    // Now add all the new animation sets back into the controller
    for ( i = 0; i < nDefCount; ++i )
    {
        // Retrieve back from the structure
        pDestAnimSet = (LPD3DXKEYFRAMEDANIMATIONSET)pDefList[i].Reserved;
        if ( !pDestAnimSet ) continue;

        // Register with the controller (addrefs internally)
        m_pAnimController->RegisterAnimationSet( pDestAnimSet );

        // Release our local copy and clear the list item just in case
        pDestAnimSet->Release();
        pDefList[i].Reserved = NULL;

    } // Next Definition

    // Set the first set in track 0 for default
    m_pAnimController->GetAnimationSet( 0, &pAnimSet );
    m_pAnimController->SetTrackAnimationSet( 0, pAnimSet );
    pAnimSet->Release();

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSet ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackAnimationSet( ULONG TrackIndex, LPD3DXANIMATIONSET pAnimSet )
{
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set anim set
    return m_pAnimController->SetTrackAnimationSet( TrackIndex, pAnimSet );
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSetByIndex ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackAnimationSetByIndex( ULONG TrackIndex, ULONG SetIndex )
{
    HRESULT hRet;
    LPD3DXANIMATIONSET pAnimSet = NULL;

    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Retrieve animation set by index
    hRet = m_pAnimController->GetAnimationSet( SetIndex, &pAnimSet );
    if ( FAILED(hRet) ) return hRet;

    // Set anim set
    hRet = m_pAnimController->SetTrackAnimationSet( TrackIndex, pAnimSet );

    // Release the one we retrieved
    pAnimSet->Release();

    // Return result
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SetTrackAnimationSetByName ()
// Desc : Allows the application to set an animation set to the specified track.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackAnimationSetByName( ULONG TrackIndex, LPCTSTR SetName )
{
    HRESULT hRet;
    LPD3DXANIMATIONSET pAnimSet = NULL;

    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Retrieve animation set by name
    hRet = m_pAnimController->GetAnimationSetByName( SetName, &pAnimSet );
    if ( FAILED(hRet) ) return hRet;

    // Set anim set
    hRet = m_pAnimController->SetTrackAnimationSet( TrackIndex, pAnimSet );

    // Release the one we retrieved
    pAnimSet->Release();

    // Return result
    return hRet;
}

//-----------------------------------------------------------------------------
// Name : SetTrackPosition ()
// Desc : Set the position within the specified track
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackPosition( ULONG TrackIndex, DOUBLE Position )
{
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the position within the track.
    return m_pAnimController->SetTrackPosition( TrackIndex, Position );
}


//-----------------------------------------------------------------------------
// Name: GetAnimationSetCount ()
// Desc: Retrieve the number of animation sets stored within the anim controller.
//-----------------------------------------------------------------------------
ULONG CActor::GetAnimationSetCount( ) const
{
    // Validate
    if ( !m_pAnimController ) return 0;

    // Return the count
    return m_pAnimController->GetNumAnimationSets( );
}

//-----------------------------------------------------------------------------
// Name: GetAnimationSet ()
// Desc: Retrieve the animation set itself by index.
// Note: Automatically adds a reference which you must release!
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET CActor::GetAnimationSet( ULONG Index ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_pAnimController ) return 0;

    // Get the animation set and return it
    if ( FAILED( m_pAnimController->GetAnimationSet( Index, &pAnimSet ) )) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetAnimationSetByName ()
// Desc: Retrieve the animation set itself by name.
// Note: Automatically adds a reference which you must release!
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET CActor::GetAnimationSetByName( LPCTSTR strName ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_pAnimController ) return 0;

    // Get the animation set and return it
    if ( FAILED( m_pAnimController->GetAnimationSetByName( strName, &pAnimSet ) )) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumTracks ()
// Desc: Get the maximum number of mixer tracks currently supported by the
//       animation controller. This can be used to determine if there are
//       enough available for your needs, or whether you need to increase the
//       supported track count by making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG CActor::GetMaxNumTracks( ) const
{
    if ( !m_pAnimController ) return 0;
    return m_pAnimController->GetMaxNumTracks();
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumAnimationSets ()
// Desc: Get the maximum number of animation sets supported by the animation
//       controller. This can be used to determine if there are enough
//       available for your needs, or whether you need to increase the
//       supported set count by making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG CActor::GetMaxNumAnimationSets( ) const
{
    if ( !m_pAnimController ) return 0;
    return m_pAnimController->GetMaxNumAnimationSets( );
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumEvents ()
// Desc: Get the maximum number of events supported by the animation controller
//       This can be used to determine if there are enough available for your 
//       needs, or whether you need to increase the supported set count by 
//       making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG CActor::GetMaxNumEvents( ) const
{
    if ( !m_pAnimController ) return 0;
    return m_pAnimController->GetMaxNumEvents( );
}

//-----------------------------------------------------------------------------
// Name: GetMaxNumAnimationOutputs ()
// Desc: Get the maximum number of outputs supported by the animation controller
//       This can be used to determine if there are enough available for your 
//       needs, or whether you need to increase the supported set count by 
//       making a call to 'SetControllerProperties'.
//-----------------------------------------------------------------------------
ULONG CActor::GetMaxNumAnimationOutputs( ) const
{
    if ( !m_pAnimController ) return 0;
    return m_pAnimController->GetMaxNumAnimationOutputs( );
}

//-----------------------------------------------------------------------------
// Name: SetTrackDesc ()
// Desc: Called to manually pass in all track setup details in one call, rather
//       than making many different calls to the SetTrack* functions.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackDesc( ULONG TrackIndex, D3DXTRACK_DESC * pDesc )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetTrackDesc( TrackIndex, pDesc );
}

//-----------------------------------------------------------------------------
// Name: SetTrackEnable ()
// Desc: Enable or disable the specified track.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackEnable( ULONG TrackIndex, BOOL Enable )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetTrackEnable( TrackIndex, Enable );
}

//-----------------------------------------------------------------------------
// Name: SetTrackPriority ()
// Desc: Specify the current priority type of this track, i.e. low or high etc.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackPriority( ULONG TrackIndex, D3DXPRIORITY_TYPE Priority )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetTrackPriority( TrackIndex, Priority );
}

//-----------------------------------------------------------------------------
// Name: SetTrackSpeed ()
// Desc: Speed up or slow down the playing animation set in this track.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackSpeed( ULONG TrackIndex, float Speed )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetTrackSpeed( TrackIndex, Speed );
}

//-----------------------------------------------------------------------------
// Name: SetTrackWeight ()
// Desc: When mixing is in use, this specifies how 'much' of this track's
//       resulting output data will be used in the mixing process.
//-----------------------------------------------------------------------------
HRESULT CActor::SetTrackWeight( ULONG TrackIndex, float Weight )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetTrackWeight( TrackIndex, Weight );

}

//-----------------------------------------------------------------------------
// Name: SetPriorityBlend ()
// Desc: Sets the global priority blending weight for the controller. This
//       determines how the High and Low priority tracks are blended together.
//-----------------------------------------------------------------------------
HRESULT CActor::SetPriorityBlend( float BlendWeight )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Set the details
    return m_pAnimController->SetPriorityBlend( BlendWeight );

}

//-----------------------------------------------------------------------------
// Name: GetTrackAnimationSet ()
// Desc: Retrieve the current animation set currently selected into a track.
// Note: Automatically adds a reference to the set, so it must be released
//       by the caller.
//-----------------------------------------------------------------------------
LPD3DXANIMATIONSET CActor::GetTrackAnimationSet( ULONG TrackIndex ) const
{
    LPD3DXANIMATIONSET pAnimSet;

    // Validate
    if ( !m_pAnimController ) return NULL;

    // Get the track details (calls AddRef) and return it
    if ( FAILED(m_pAnimController->GetTrackAnimationSet( TrackIndex, &pAnimSet )) ) return NULL;
    return pAnimSet;
}

//-----------------------------------------------------------------------------
// Name: GetTrackDesc ()
// Desc: Retrieve all of the properties associated with this track.
//-----------------------------------------------------------------------------
HRESULT CActor::GetTrackDesc( ULONG TrackIndex, D3DXTRACK_DESC * pDesc ) const
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Get the details
    return m_pAnimController->GetTrackDesc( TrackIndex, pDesc );
}

//-----------------------------------------------------------------------------
// Name: GetPriorityBlend ()
// Desc: Get the current priority blend weight value for the controller.
//-----------------------------------------------------------------------------
float CActor::GetPriorityBlend( ) const
{
    // Validate
    if ( !m_pAnimController ) return 0.0f;

    // Get the details
    return m_pAnimController->GetPriorityBlend( );
}

//-----------------------------------------------------------------------------
// Name: KeyPriorityBlend ()
// Desc: Insert / sequence an event key for transitioning the priority blending
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::KeyPriorityBlend( float NewBlendWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->KeyPriorityBlend( NewBlendWeight, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackEnable ()
// Desc: Insert / sequence an event key for enabling / disabling a track
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::KeyTrackEnable( ULONG TrackIndex, BOOL NewEnable, double StartTime )
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->KeyTrackEnable( TrackIndex, NewEnable, StartTime );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackPosition ()
// Desc: Insert / sequence an event key for switching to a new position within
//       the specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::KeyTrackPosition( ULONG TrackIndex, double NewPosition, double StartTime )
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->KeyTrackPosition( TrackIndex, NewPosition, StartTime );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackSpeed ()
// Desc: Insert / sequence an event key for altering the playing speed of the
//       specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::KeyTrackSpeed( ULONG TrackIndex, float NewSpeed, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->KeyTrackSpeed( TrackIndex, NewSpeed, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: KeyTrackWeight ()
// Desc: Insert / sequence an event key for transitioning the weight used for
//       mixing the animated output of the specified track.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::KeyTrackWeight( ULONG TrackIndex, float NewWeight, double StartTime, double Duration, D3DXTRANSITION_TYPE Transition )
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->KeyTrackWeight( TrackIndex, NewWeight, StartTime, Duration, Transition );
}

//-----------------------------------------------------------------------------
// Name: UnkeyPriorityBlends ()
// Desc: Remove all sequenced priority blend keys.
//-----------------------------------------------------------------------------
HRESULT CActor::UnkeyAllPriorityBlends( )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_pAnimController->UnkeyAllPriorityBlends( );
}

//-----------------------------------------------------------------------------
// Name: UnkeyAllTrackEvents ()
// Desc: Remove all sequenced track related event keys for the specified track.
//-----------------------------------------------------------------------------
HRESULT CActor::UnkeyAllTrackEvents( ULONG TrackIndex )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_pAnimController->UnkeyAllTrackEvents( TrackIndex );
}

//-----------------------------------------------------------------------------
// Name: UnkeyEvent ()
// Desc: Remove the specified event only.
//-----------------------------------------------------------------------------
HRESULT CActor::UnkeyEvent( D3DXEVENTHANDLE hEvent )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_pAnimController->UnkeyEvent( hEvent );
}

//-----------------------------------------------------------------------------
// Name: ValidateEvent ()
// Desc: Ensures that the specified event handle is valid, and that it has not
//       yet been triggered and removed. (This will validate running events
//       successfully, but all those which have finished will fail).
//-----------------------------------------------------------------------------
HRESULT CActor::ValidateEvent( D3DXEVENTHANDLE hEvent )
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_pAnimController->ValidateEvent( hEvent );
}

//-----------------------------------------------------------------------------
// Name: GetCurrentTrackEvent ()
// Desc: Retrieve the handle of any event currently running on the specified
//       track, of the specified type.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::GetCurrentTrackEvent( ULONG TrackIndex, D3DXEVENT_TYPE EventType ) const
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->GetCurrentTrackEvent( TrackIndex, EventType );
}

//-----------------------------------------------------------------------------
// Name: GetCurrentPriorityBlend ()
// Desc: Get the event handle of any priority blend currently running on the
//       global timeline.
//-----------------------------------------------------------------------------
D3DXEVENTHANDLE CActor::GetCurrentPriorityBlend( ) const
{
    // Validate
    if ( !m_pAnimController ) return NULL;

    // Send the details
    return m_pAnimController->GetCurrentPriorityBlend( );
}

//-----------------------------------------------------------------------------
// Name: GetEventDesc ()
// Desc: Retrieve the descriptor for the event.
//-----------------------------------------------------------------------------
HRESULT CActor::GetEventDesc( D3DXEVENTHANDLE hEvent, LPD3DXEVENT_DESC pDesc ) const
{
    // Validate
    if ( !m_pAnimController ) return D3DERR_INVALIDCALL;

    // Send the details
    return m_pAnimController->GetEventDesc( hEvent, pDesc );
}

//-----------------------------------------------------------------------------
// Name : DrawActor ()
// Desc : Render the currently loaded actor.
//-----------------------------------------------------------------------------
void CActor::DrawActor( )
{
    if ( !IsLoaded() ) return;

    // Draw the frame heirarchy
    DrawFrame( m_pFrameRoot );
}

//-----------------------------------------------------------------------------
// Name : DrawBones ()
// Desc : Triggered to render the bones for debug purposes.
//-----------------------------------------------------------------------------
void CActor::DrawBones( LPD3DXFRAME pFrame /* = NULL */ )
{
    // Temporary lit vertex structure
    struct TLITVERTEX
    {
        D3DXVECTOR3 Pos;
        ULONG       Color;
    };

    TLITVERTEX Points[18];
    ULONG      i, j;
    float      fSize = 1.0f;

    // Swap frame if NULL
    if ( !pFrame ) pFrame = m_pFrameRoot;
    if ( !pFrame ) return;

    D3DXMATRIX mtx;
    D3DXMatrixIdentity( &mtx );
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &mtx );

    // If this has a mesh container (or set of containers), we'll check for skinning etc.
    D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;
    for ( ; pContainer; pContainer = (D3DXMESHCONTAINER_DERIVED*)pContainer->pNextMeshContainer )
    {
        // Skinned mesh?
        if ( pContainer->pSkinInfo )
        {
            LPD3DXSKININFO pSkinInfo = pContainer->pSkinInfo;

            // Loop through each of the bones
            for ( i = 0; i < pSkinInfo->GetNumBones(); ++i )
            {
                // Get the name of the bone
                LPCTSTR strName = pSkinInfo->GetBoneName( i );

                // Retrieve our custom matrix for this bone
                D3DXFRAME_MATRIX * pBone = (D3DXFRAME_MATRIX*)GetFrameByName( strName );

                // Skip if none found, or if there is no terminator
                if ( !pBone || !pBone->pFrameFirstChild ) continue;

                // Retrieve the bone matrix
                D3DXMATRIX &mtxBegin = pBone->mtxCombined;
                D3DXMATRIX &mtxEnd   = ((D3DXFRAME_MATRIX*)pBone->pFrameFirstChild)->mtxCombined;

                D3DXVECTOR3 vecStart = D3DXVECTOR3( mtxBegin._41, mtxBegin._42, mtxBegin._43 );
                D3DXVECTOR3 vecEnd   = D3DXVECTOR3( mtxEnd._41, mtxEnd._42, mtxEnd._43 );

                // Build the points
                float fLength = D3DXVec3Length( &(vecEnd - vecStart) );
                Points[0].Pos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
                Points[1].Pos = D3DXVECTOR3( 0.0f, 0.0f, fLength );

                // Fins
                Points[2].Pos = D3DXVECTOR3( 0.0f, fSize, 0.0f );
                Points[3].Pos = Points[1].Pos;
                Points[4].Pos = D3DXVECTOR3( fSize, 0.0f, 0.0f );
                Points[5].Pos = Points[1].Pos;
                Points[6].Pos = D3DXVECTOR3( 0.0f, -fSize, 0.0f );
                Points[7].Pos = Points[1].Pos;
                Points[8].Pos = D3DXVECTOR3( -fSize, 0.0f, 0.0f );
                Points[9].Pos = Points[1].Pos;

                // Base
                Points[10].Pos = Points[2].Pos;
                Points[11].Pos = Points[4].Pos;
                Points[12].Pos = Points[4].Pos;
                Points[13].Pos = Points[6].Pos;
                Points[14].Pos = Points[6].Pos;
                Points[15].Pos = Points[8].Pos;
                Points[16].Pos = Points[8].Pos;
                Points[17].Pos = Points[2].Pos;

                // Set color
                for ( j = 0; j < 2; ++j ) Points[j].Color = 0xFFFF0000;
                for ( j = 2; j < 10; ++j ) Points[j].Color = 0xFF0000FF;
                for ( j = 10; j < 18; ++j ) Points[j].Color = 0xFF00FF00;

                D3DXVECTOR3 vecUp;
                D3DXVec3Normalize( &vecUp, &(vecEnd - vecStart) );
                if ( fabsf(D3DXVec3Dot( &vecUp, &D3DXVECTOR3( 0, 1, 0) )) > 0.9 )
                    vecUp = D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
                else
                    vecUp = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

                // Build the matrix and set it
                D3DXMatrixLookAtLH( &mtx, &vecStart, &vecEnd, &vecUp );
                D3DXMatrixInverse( &mtx, NULL, &mtx );
                m_pD3DDevice->SetTransform( D3DTS_WORLD, &mtx );

                // Setup states
                m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
                m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
                m_pD3DDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

                // Draw the line
                m_pD3DDevice->DrawPrimitiveUP( D3DPT_LINELIST, 9, (LPVOID)Points, sizeof(Points[0]) );

                // Reset the states
                m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
                m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

            } // Next Bone

        } // End if skinned mesh

    } // Next Container

    // Move onto next sibling frame
    if ( pFrame->pFrameSibling ) DrawBones( pFrame->pFrameSibling );

    // Move onto first child frame 
    if ( pFrame->pFrameFirstChild ) DrawBones( pFrame->pFrameFirstChild );

}

//-----------------------------------------------------------------------------
// Name : DrawActorSubset ()
// Desc : Render the specified subset of the currently loaded actor.
//-----------------------------------------------------------------------------
void CActor::DrawActorSubset( ULONG AttributeID )
{
    if ( !IsLoaded() ) return;

    // Draw the frame heirarchy
    DrawFrame( m_pFrameRoot, (long)AttributeID );
}

//-----------------------------------------------------------------------------
// Name : DrawFrame () (Private)
// Desc : Iteratively called to render a frame in the hierarchy
//-----------------------------------------------------------------------------
void CActor::DrawFrame( LPD3DXFRAME pFrame, long AttributeID /* = -1 */ )
{
    LPD3DXMESHCONTAINER pMeshContainer;
    D3DXFRAME_MATRIX  * pMtxFrame  = (D3DXFRAME_MATRIX*)pFrame;

    // Set the frames combined matrix
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &pMtxFrame->mtxCombined );

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
void CActor::DrawMeshContainer( LPD3DXMESHCONTAINER pMeshContainer, LPD3DXFRAME pFrame, long AttributeID /* = -1 */ )
{
    ULONG                       i, j, MatrixIndex;
    D3DXFRAME_MATRIX          * pMtxFrame  = (D3DXFRAME_MATRIX*)pFrame;
    D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer;
    CTriMesh                  * pMesh      = pContainer->pMesh;
    LPD3DXSKININFO              pSkinInfo  = pContainer->pSkinInfo;
    D3DXMATRIX                  mtxEntry;

    // Validate requirements
    if ( !pMesh ) return;

    // Is this a skinned mesh ?
    if ( pSkinInfo != NULL)
    {
        if ( pContainer->SkinMethod  == SKINMETHOD_INDEXED )
        {
            // Here is an ideal place to set the device's FVF flags. However,
            // since ID3DXMesh::DrawSubset does this for us, we need only do this
            // if we are implementing the mesh classes / systems for ourselves.
            //m_pD3DDevice->SetFVF( pMesh->GetFVF() );

            // If the hardware doesn't support indexed vertex processing
            // then we have to switch to software processing here.
            if ( pContainer->SoftwareVP ) m_pD3DDevice->SetSoftwareVertexProcessing( TRUE );

            // Set the number of blending indices to be used
            if ( pContainer->InfluenceCount == 1 )
                m_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS );
            else
                m_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, pContainer->InfluenceCount - 1 );

            // Enable indexed blending
            if ( pContainer->InfluenceCount ) m_pD3DDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );

            // Get a usable pointer to the combination table
            LPD3DXBONECOMBINATION pBoneComb = (LPD3DXBONECOMBINATION)pContainer->pBoneCombination->GetBufferPointer();

            // Loop through each of the attribute groups and calculate the matrices we need.
            for ( i = 0; i < pContainer->AttribGroupCount; ++i )
            {
                // Not entirely healthy for us to have set up all these states etc, only to batch
                // by material explicitly, but we provide support nonethless.
                if ( AttributeID >= 0 && pBoneComb[i].AttribId != (ULONG)AttributeID ) continue;

                // First calculate the world matrices
                for ( j = 0; j < pContainer->PaletteEntryCount; ++j )
                {
                    // Get the index
                    MatrixIndex = pBoneComb[i].BoneId[j];

                    // If it's valid, set this index entry
                    if ( MatrixIndex != UINT_MAX )
                    {
                        // Generate the final matrix
                        D3DXMatrixMultiply( &mtxEntry, &pContainer->pBoneOffset[ MatrixIndex ], pContainer->ppBoneMatrices[ MatrixIndex ] );

                        // Set it to the device
                        m_pD3DDevice->SetTransform( D3DTS_WORLDMATRIX( j ), &mtxEntry );

                    } // End if valid matrix entry

                } // Next Palette Entry

                // Draw the mesh subset
                pMesh->DrawSubset( i, pBoneComb[i].AttribId );

            } // Next attribute group

            // Reset the blending states
            m_pD3DDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
            m_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, 0 );

            // Reset back to hardware vertex processing if required
            if ( pContainer->SoftwareVP ) m_pD3DDevice->SetSoftwareVertexProcessing( FALSE );

        } // End if indexed skinning
        else if ( pContainer->SkinMethod == SKINMETHOD_NONINDEXED )
        {
            // Get a usable pointer to the combination table
            LPD3DXBONECOMBINATION pBoneComb = (LPD3DXBONECOMBINATION)pContainer->pBoneCombination->GetBufferPointer();

            // Here is an ideal place to set the device's FVF flags. However,
            // since ID3DXMesh::DrawSubset does this for us, we need only do this
            // if we are implementing the mesh classes / systems for ourselves.
            //m_pD3DDevice->SetFVF( pMesh->GetFVF() );

            // Loop through each of the attribute groups and calculate the matrices we need.
            // At this point we are rendering using the default vertex processing mode (typically Hardware)
            for ( i = 0; i < pContainer->AttribGroupCount; ++i )
            {
                // Retrieve the remapped index
                ULONG iAttribID = pContainer->pVPRemap[ i ];

                // Enable software processing if we reached our SW begin index
                if ( i == pContainer->SWRemapBegin ) m_pD3DDevice->SetSoftwareVertexProcessing( TRUE );

                // Not entirely healthy for us to have set up all these states etc, only to batch
                // by material explicitly, but we provide support nonethless.
                if ( AttributeID >= 0 && pBoneComb[ iAttribID ].AttribId != (ULONG)AttributeID ) continue;

                ULONG MaxBoneIndex = UINT_MAX;
                // Count the number of bones required for this attribute group
                for ( j = 0; j < pContainer->InfluenceCount; ++j )
                {
                    // If this is not an 'empty' bone, increase our max bone index
                    if ( pBoneComb[ iAttribID ].BoneId[ j ] != UINT_MAX ) MaxBoneIndex = j;

                } // Next Influence Item

                // First calculate the world matrices
                for ( j = 0; j <= MaxBoneIndex; ++j )
                {
                    // Get the index
                    MatrixIndex = pBoneComb[ iAttribID ].BoneId[j];

                    // If it's valid, set this index entry
                    if ( MatrixIndex != UINT_MAX )
                    {
                        // Generate the final matrix
                        D3DXMatrixMultiply( &mtxEntry, &pContainer->pBoneOffset[ MatrixIndex ], pContainer->ppBoneMatrices[ MatrixIndex ] );

                        // Set it to the device
                        m_pD3DDevice->SetTransform( D3DTS_WORLDMATRIX( j ), &mtxEntry );

                    } // End if valid matrix entry

                } // Next Palette Entry

                // Set the blending count to however many are required / in use
                m_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, MaxBoneIndex );

                // Draw the mesh subset
                pMesh->DrawSubset( iAttribID, pBoneComb[ iAttribID ].AttribId );

            } // Next attribute group

            // Reset the blending states
            m_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, 0 );

            // Disable software processing if it was enabled
            if ( pContainer->SWRemapBegin < pContainer->AttribGroupCount ) m_pD3DDevice->SetSoftwareVertexProcessing( FALSE );

        } // End if Non-Indexed
        else if ( pContainer->SkinMethod == SKINMETHOD_SOFTWARE )
        {
            D3DXMATRIX  Identity;
            ULONG       BoneCount = pSkinInfo->GetNumBones();
            PBYTE       pbVerticesSrc;
            PBYTE       pbVerticesDest;

            // Get the actual mesh
            LPD3DXBASEMESH pSrcMesh = pMesh->GetMesh();
            if ( !pSrcMesh ) pMesh->GetPMesh();

            // If the data has been invalidated since last rendering, update our SW mesh
            if ( pContainer->Invalidated )
            {
                // Loop through and setup the bone matrices
                for ( i = 0; i < BoneCount; ++i )
                {
                    // Concatenate matrices into our temporary storage
                    D3DXMatrixMultiply( &m_pSWMatrices[i], &pContainer->pBoneOffset[i], pContainer->ppBoneMatrices[i] );

                } // Next Bone

                // Lock the input and output vertex buffers
                pSrcMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pbVerticesSrc);
                pContainer->pSWMesh->LockVertexBuffer( 0, (LPVOID*)&pbVerticesDest);

                // Generate the newly skinned mesh data
                pSkinInfo->UpdateSkinnedMesh( m_pSWMatrices, NULL, pbVerticesSrc, pbVerticesDest );

                // Unlock the buffers
                pSrcMesh->UnlockVertexBuffer();
                pContainer->pSWMesh->UnlockVertexBuffer();

            } // End if update required

            // It's important that nothing else in the heirarchy has a chance to
            // influence our position, as the world transform is already taken into account
            // via the bone matrices themselves.
            D3DXMatrixIdentity( &Identity );
            m_pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );

            // Attach the SW mesh to our CTriMesh (attach only) for rendering
            pMesh->Attach( pContainer->pSWMesh, NULL, true );

            // Render the mesh or subset
            if ( AttributeID >= 0 )
            {
                // Draw the subset
                pMesh->DrawSubset( (ULONG)AttributeID );

            } // End if attribute specified    
            else
            {
                // Draw the mesh as a whole
                pMesh->Draw( );

            } // End if no attribute specified

            // Re-attach our source mesh
            pMesh->Attach( pSrcMesh, NULL, true );

            // Now we can release our source mesh
            pSrcMesh->Release();


        } // End if software skinned

    } // End if skinned
    else
    {
        // Render the mesh
        if ( AttributeID >= 0 )
        {
            // Here is an ideal place to set the device's FVF flags. However,
            // since ID3DXMesh::DrawSubset does this for us, we need only do this
            // if we are implementing the mesh classes / systems for ourselves.
            //m_pD3DDevice->SetFVF( pMesh->GetFVF() );

            // Draw the subset
            pMesh->DrawSubset( (ULONG)AttributeID );

        } // End if attribute specified    
        else
        {
            pMesh->Draw( );

        } // End if no attribute specified

    } // End if not skinned

}

//-----------------------------------------------------------------------------
// Name : UpdateFrameMatrices () (Private)
// Desc : Updates the frame matrices (computes our combined matrix)
//-----------------------------------------------------------------------------
void CActor::UpdateFrameMatrices( LPD3DXFRAME pFrame, const D3DXMATRIX * pParentMatrix )
{
    D3DXFRAME_MATRIX * pMtxFrame = (D3DXFRAME_MATRIX*)pFrame;

    if ( pParentMatrix != NULL)
        D3DXMatrixMultiply( &pMtxFrame->mtxCombined, &pMtxFrame->TransformationMatrix, pParentMatrix);
    else
        pMtxFrame->mtxCombined = pMtxFrame->TransformationMatrix;

    // Move onto sibling frame
    if ( pMtxFrame->pFrameSibling ) UpdateFrameMatrices( pMtxFrame->pFrameSibling, pParentMatrix );

    // Move onto first child frame
    if ( pMtxFrame->pFrameFirstChild ) UpdateFrameMatrices( pMtxFrame->pFrameFirstChild, &pMtxFrame->mtxCombined );

    // If this has a mesh container (or set of containers), we must invalidate them
    D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pMtxFrame->pMeshContainer;
    for ( ; pContainer; pContainer = (D3DXMESHCONTAINER_DERIVED*)pContainer->pNextMeshContainer )
    {
        // Flag as invalid
        pContainer->Invalidated = true;

    } // Next Container
}

//-----------------------------------------------------------------------------
// Name : GetCallback ()
// Desc : Retrieve a currently registered callback.
//-----------------------------------------------------------------------------
CALLBACK_FUNC CActor::GetCallback( CALLBACK_TYPE Type ) const
{
    return m_CallBack[Type];
}

//-----------------------------------------------------------------------------
// Name : GetOptions ()
// Desc : Retrieve the options requested via Load*.
//-----------------------------------------------------------------------------
ULONG CActor::GetOptions( ) const
{
    return m_nOptions;
}

//-----------------------------------------------------------------------------
// Name : SetWorldMatrix ()
// Desc : Updates the actor using the specified matrix.
// Note : You can pass NULL here. Doing so will assume identity.
//-----------------------------------------------------------------------------
void CActor::SetWorldMatrix( const D3DXMATRIX * mtxWorld /* = NULL */, bool UpdateFrames /* = false */ )
{
    // Store the currently set world matrix
    if ( mtxWorld )
        m_mtxWorld = *mtxWorld;
    else
        D3DXMatrixIdentity( &m_mtxWorld );

    // Update the frame matrices
    if ( IsLoaded() && UpdateFrames ) UpdateFrameMatrices( m_pFrameRoot, mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: AdvanceTime ()
// Desc: Progresses the time forward by the specified amount
//-----------------------------------------------------------------------------
void CActor::AdvanceTime( double fTimeElapsed, bool UpdateFrames /* = true */, LPD3DXANIMATIONCALLBACKHANDLER pCallbackHandler /* = NULL */ )
{
    if ( !IsLoaded() ) return;

    // Set the current time if applicable
    if ( m_pAnimController ) m_pAnimController->AdvanceTime( fTimeElapsed, pCallbackHandler );

    // Update the frame matrices
    if ( UpdateFrames ) UpdateFrameMatrices( m_pFrameRoot, &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: ResetTime ()
// Desc: Reset the global time to 0.0, whilst retaining the tracks periodic
//       position.
//-----------------------------------------------------------------------------
void CActor::ResetTime( bool UpdateFrames /* = false */ )
{
    if ( !IsLoaded() ) return;

    // Set the current time if applicable
    if ( m_pAnimController ) m_pAnimController->ResetTime( );

    // Update the frame matrices
    if ( UpdateFrames ) UpdateFrameMatrices( m_pFrameRoot, &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name: SetActorLimits ()
// Desc: Call this to setup the maximum amount of various different controller
//       properties, such as tracks and animation sets.
//-----------------------------------------------------------------------------
HRESULT CActor::SetActorLimits( ULONG MaxAnimSets /*= 0*/, ULONG MaxTracks /* = 0*/, ULONG MaxAnimOutputs /* = 0 */, ULONG MaxEvents /* = 0 */, ULONG MaxCallbackKeys /* = 0 */ )
{
    HRESULT hRet;
    bool    bCloneController = false;
    ULONG   Value;

    // Cache the current details
    if ( MaxAnimSets     > 0 ) m_nMaxAnimSets     = MaxAnimSets;
    if ( MaxTracks       > 0 ) m_nMaxTracks       = MaxTracks;
    if ( MaxAnimOutputs  > 0 ) m_nMaxAnimOutputs  = MaxAnimOutputs;
    if ( MaxEvents       > 0 ) m_nMaxEvents       = MaxEvents;
    if ( MaxCallbackKeys > 0 ) m_nMaxCallbackKeys = MaxCallbackKeys;

    // Store back in our temp variables for defaults (if 0 was specified, we'll re-use)
    MaxAnimSets     = m_nMaxAnimSets;
    MaxTracks       = m_nMaxTracks;
    MaxAnimOutputs  = m_nMaxAnimOutputs;
    MaxEvents       = m_nMaxEvents;
    MaxCallbackKeys = m_nMaxCallbackKeys; // This limit can shrink, but should never be 0

    // If we have no controller yet, we'll take this no further
    if ( !m_pAnimController ) return D3D_OK;

    // Otherwise we must clone if any of the details are below our thresholds
    Value = m_pAnimController->GetMaxNumAnimationOutputs();
    if ( Value < MaxAnimOutputs ) bCloneController = true; else MaxAnimOutputs = Value;
    Value = m_pAnimController->GetMaxNumAnimationSets();
    if ( Value < MaxAnimSets ) bCloneController = true; else MaxAnimSets = Value;
    Value = m_pAnimController->GetMaxNumTracks();
    if ( Value < MaxTracks ) bCloneController = true; else MaxTracks = Value;
    Value = m_pAnimController->GetMaxNumEvents();
    if ( Value < MaxEvents ) bCloneController = true; else MaxEvents = Value;

    // Clone our animation controller if there are not enough set slots available
    if ( bCloneController )
    {
        LPD3DXANIMATIONCONTROLLER pNewController = NULL;

        // Clone the animation controller
        hRet = m_pAnimController->CloneAnimationController( MaxAnimOutputs, MaxAnimSets, MaxTracks, MaxEvents, &pNewController );
        if ( FAILED(hRet) ) return hRet;

        // Release our old controller
        m_pAnimController->Release();

        // Store the new controller
        m_pAnimController = pNewController;

    } // End if requires clone

    // Setup max tracks for action status (if available)
    if ( m_pActionStatus ) m_pActionStatus->SetMaxTrackCount( (USHORT)MaxTracks );

    // Success
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name: GetTime ()
// Desc: Retrieve the current time of the animation.
//-----------------------------------------------------------------------------
double CActor::GetTime( ) const
{
    if ( !m_pAnimController) return 0.0f;
    return m_pAnimController->GetTime();
}

//-----------------------------------------------------------------------------
// Name: GetActorName ()
// Desc: Retrieve the filename used to load this actor.
//-----------------------------------------------------------------------------
LPCTSTR CActor::GetActorName( ) const
{
    return m_strActorName;
}

//-----------------------------------------------------------------------------
// Name: GetDevice ()
// Desc: Retrieve the D3D device being used by the actor
// Note: Adds a reference when it returns, so you must release when you're done
//-----------------------------------------------------------------------------
LPDIRECT3DDEVICE9 CActor::GetDevice( ) const
{
    m_pD3DDevice->AddRef();
    return m_pD3DDevice;
}

//-----------------------------------------------------------------------------
// Name: GetRootFrame ()
// Desc: Return the root frame.
//-----------------------------------------------------------------------------
LPD3DXFRAME CActor::GetRootFrame( ) const
{
    return m_pFrameRoot;
}

//-----------------------------------------------------------------------------
// Name: GetFrameByName ()
// Desc: Retrieve the filename used to load this actor.
//-----------------------------------------------------------------------------
LPD3DXFRAME CActor::GetFrameByName( LPCTSTR strName, LPD3DXFRAME pFrame /* = NULL */ ) const
{
    LPD3DXFRAME pRetFrame = NULL;

    // Any start frame passed in?
    if ( !pFrame ) pFrame = m_pFrameRoot;

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
// Name: BuildBoneMatrixPointers ()
// Desc: Setup the bone matrix pointers for all mesh containers in the 
//       heirarchy.
//-----------------------------------------------------------------------------
HRESULT CActor::BuildBoneMatrixPointers( LPD3DXFRAME pFrame )
{
    HRESULT hRet;

    // Has a mesh container?
    if ( pFrame->pMeshContainer )
    {
        D3DXMESHCONTAINER_DERIVED * pContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;
        LPD3DXSKININFO              pSkinInfo  = pContainer->pSkinInfo;

        // if there is a skinmesh, then setup the bone matrices
        if ( pSkinInfo != NULL )
        {
            ULONG BoneCount = pSkinInfo->GetNumBones(), i;

            // Allocate space for the bone matrix pointers
            pContainer->ppBoneMatrices = new D3DXMATRIX*[BoneCount];
            if ( pContainer->ppBoneMatrices == NULL ) return E_OUTOFMEMORY;

            // Store the bone pointers
            for ( i = 0; i < BoneCount; ++i )
            {
                // Find the matching frame for this bone
                D3DXFRAME_MATRIX * pMtxFrame = (D3DXFRAME_MATRIX*)D3DXFrameFind( m_pFrameRoot, pSkinInfo->GetBoneName(i) );
                if ( pFrame == NULL ) return E_FAIL;

                // Store the matrix pointer
                pContainer->ppBoneMatrices[ i ] = &pMtxFrame->mtxCombined;

            } // Next Bone

            // If we are in software skinning mode, we need to allocate our temporary
            // storage. If there is not enough room, grow our temporary array.
            if ( pContainer->SkinMethod == SKINMETHOD_SOFTWARE && m_nMaxSWMatrices < BoneCount )
            {
                // Release previous memory.
                if ( m_pSWMatrices ) delete []m_pSWMatrices;
                m_pSWMatrices = NULL;

                // Allocate new memory
                m_pSWMatrices = new D3DXMATRIX[ BoneCount ];
                m_nMaxSWMatrices = BoneCount;

                // Success ?
                if ( !m_pSWMatrices ) return E_OUTOFMEMORY;

            } // End if grow SW storage

        } // End if skinned mesh

    } // End if has mesh container

    // Has a sibling frame?
    if (pFrame->pFrameSibling != NULL)
    {
        hRet = BuildBoneMatrixPointers( pFrame->pFrameSibling );
        if ( FAILED(hRet) ) return hRet;

    } // End if has sibling

    // Has a child frame?
    if (pFrame->pFrameFirstChild != NULL)
    {
        hRet = BuildBoneMatrixPointers( pFrame->pFrameFirstChild );
        if ( FAILED(hRet) ) return hRet;

    } // End if has child

    // Success!!
    return D3D_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : CAllocateHierarchy () (Constructor)
// Desc : Class Constructor
//-----------------------------------------------------------------------------
CAllocateHierarchy::CAllocateHierarchy( CActor * pActor )
{
    // Store / Clear details.
    m_pActor         = pActor;
    m_pAttribRemap   = NULL;
    m_nMaterialCount = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CAllocateHierarchy () (Destructor)
// Desc : Class Destructor
//-----------------------------------------------------------------------------
CAllocateHierarchy::~CAllocateHierarchy( )
{
    // Release any memory
    if ( m_pAttribRemap ) delete []m_pAttribRemap;

    // Clear details.
    m_pActor         = NULL;
    m_pAttribRemap   = NULL;
    m_nMaterialCount = 0;
}

//-----------------------------------------------------------------------------
// Name : GetAttributeRemap ()
// Desc : Allows external sources to query how attributes were remapped (if at
//        all) after the call to CreateMeshContainer()
//-----------------------------------------------------------------------------
ULONG CAllocateHierarchy::GetAttributeRemap( ULONG AttributeID )
{
    // Bounds Test
    if ( AttributeID >= m_nMaterialCount ) return 0;

    // If no remapping occured, just return the ID passed in
    if ( !m_pAttribRemap ) return AttributeID;

    // Return the new attribute ID for this attribute
    return m_pAttribRemap[ AttributeID ];
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
    CTriMesh                  *pNewMesh       = NULL;
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
    ManageAttribs = (m_pActor->GetCallback( CActor::CALLBACK_ATTRIBUTEID ).pFunction == NULL);

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
            Callback = m_pActor->GetCallback( CActor::CALLBACK_TEXTURE );
            if ( Callback.pFunction )
            {
                COLLECTTEXTURE CollectTexture = (COLLECTTEXTURE)Callback.pFunction;
                pAttribData[i].Texture = CollectTexture( Callback.pContext, pMaterials[i].pTextureFilename );

                // Add reference. We are now using this
                if ( pAttribData[i].Texture ) pAttribData[i].Texture->AddRef();

            } // End if callback available

            // Request effect pointer via callback
            Callback = m_pActor->GetCallback( CActor::CALLBACK_EFFECT );
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
            Callback = m_pActor->GetCallback( CActor::CALLBACK_ATTRIBUTEID );
            if ( Callback.pFunction )
            {
                D3DMATERIAL9 Material = pMaterials[i].MatD3D;

                // Note : The X File specification contains no ambient material property.
                //        We should ideally set this to full intensity to allow us to 
                //        control ambient brightness via the D3DRS_AMBIENT renderstate.
                Material.Ambient = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

                // Collect the attribute ID
                COLLECTATTRIBUTEID CollectAttributeID = (COLLECTATTRIBUTEID)Callback.pFunction;
                AttribID = CollectAttributeID( Callback.pContext, pMaterials[i].pTextureFilename,
                    &Material, &pEffectInstances[i] );

                // Store this in our attribute remap table
                pAttribRemap[i] = AttribID;

                // Determine if any changes are required so far
                if ( AttribID != i ) RemapAttribs = true;

            } // End if callback available

        } // End if we don't manage attributes

    } // Next Material

    // Copy over adjacency information if any
    if ( pAdjacency )
    {
        pMeshContainer->pAdjacency = new DWORD[ pMesh->GetNumFaces() * 3 ];
        if ( !pMeshContainer->pAdjacency ) { hRet = E_OUTOFMEMORY; goto ErrorOut; }
        memcpy( pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * pMesh->GetNumFaces() * 3 );
    
    } // End if adjacency provided

    // Is this a 'skinned' mesh ?
    if (pSkinInfo != NULL)
    {
        LPD3DXMESH pSkinMesh;

        // first save off the SkinInfo and original mesh data
        pMeshContainer->pSkinInfo = pSkinInfo;
        pSkinInfo->AddRef();

        // Allocate a set of bone offset matrices, we need to store these to transform
        // any vertices from 'character space' into 'bone space'
        ULONG BoneCount = pSkinInfo->GetNumBones();
        pMeshContainer->pBoneOffset = new D3DXMATRIX[BoneCount];
        if ( !pMeshContainer->pBoneOffset ) { hRet = E_OUTOFMEMORY; goto ErrorOut; }

        // Retrieve the bone offset matrices here.
        for ( i = 0; i < BoneCount; ++i )
        {
            // Store the bone
            pMeshContainer->pBoneOffset[i] = *pSkinInfo->GetBoneOffsetMatrix(i);

        } // Next Bone

        // Build the skinned mesh
        hRet = BuildSkinnedMesh( pMeshContainer, pMesh, &pSkinMesh );
        if ( FAILED(hRet) ) goto ErrorOut;

        // Replace the old mesh with this one (note: unlike previous usage of attach
        // where the texture information etc will become invalid, we specify here that
        // we do not want the management data (textures etc) to be released by specifying
        // true to the bReplaceMeshOnly parameter).
        pNewMesh->Attach( pSkinMesh, NULL, true );
        pMesh = pSkinMesh;

    } // End if skin info provided

    // Remap attributes if required
    if ( pAttribRemap != NULL && RemapAttribs == true )
    {
        // Is a skinned mesh ?
        if ( !pSkinInfo || pMeshContainer->SkinMethod == CActor::SKINMETHOD_SOFTWARE )
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

            // In the software skinning case, we need to reflect any changes made into our SW Mesh
            if ( pSkinInfo && pMeshContainer->SkinMethod == CActor::SKINMETHOD_SOFTWARE )
            {
                ULONG * pSWAttributes = NULL;

                // Lock the attribute buffer and copy the contents over
                hRet = pMeshContainer->pSWMesh->LockAttributeBuffer( 0, &pSWAttributes );
                if ( SUCCEEDED(hRet) )
                {
                    memcpy( pSWAttributes, pAttributes, pMesh->GetNumFaces() * sizeof(ULONG) );
                    pMeshContainer->pSWMesh->UnlockAttributeBuffer( );
                
                } // End if succeeded                

            } // End if software skinning

            // Finish up
            pMesh->UnlockAttributeBuffer( );

        } // End if not skinned
        else
        {
            // Because the attribute grouping is significant with skinned meshes
            // we can only remap the bone combination attribute ID mapping information
            LPD3DXBONECOMBINATION pBoneComb = (LPD3DXBONECOMBINATION)pMeshContainer->pBoneCombination->GetBufferPointer();
            for ( i = 0; i < pMeshContainer->AttribGroupCount; ++i )
            {
                // Retrieve the current attribute / material ID for this bone combination
                AttribID = pBoneComb[i].AttribId;

                // Replace it with the remap value
                pBoneComb[i].AttribId = pAttribRemap[AttribID];

            } // Next Attribute Group

        } // End if Skinned mesh


    } // End if remap attributes

    // Release old remap data if it exists, and store the current for querying later.
    if ( m_pAttribRemap ) delete []m_pAttribRemap;
    m_pAttribRemap   = pAttribRemap;
    m_nMaterialCount = NumMaterials;

    // If this is a skinned mesh, we must release our overriden copy
    // only the CTriMesh will now reference a copy.
    if ( pSkinInfo ) pMesh->Release();

    // Attempt to optimize the new mesh.
    // Note : We can only do this if our mesh is not being used for software skinning
    // as the SkinInfo would not be updated automatically. We can use the SkinInfo::Remap
    // method, but it's not vitally important for us in software mode.
    if ( !(pSkinInfo && pMeshContainer->SkinMethod == CActor::SKINMETHOD_SOFTWARE) )
    {
        // Optimize
        pNewMesh->WeldVertices( 0 );
        pNewMesh->OptimizeInPlace( D3DXMESHOPT_VERTEXCACHE );

    } // End if not software skinned mesh

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
// Name : BuildSkinnedMesh ()
// Desc : Wraps up the functionality for building the skinned mesh information
//        from the data provided.
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::BuildSkinnedMesh( D3DXMESHCONTAINER_DERIVED * pMeshContainer, LPD3DXMESH pMesh, LPD3DXMESH * pMeshOut )
{
    D3DCAPS9            Caps;
    HRESULT             hRet;
    ULONG               i, j, k;
    LPD3DXSKININFO      pSkinInfo  = pMeshContainer->pSkinInfo;
    LPDIRECT3DDEVICE9   pDevice    = NULL;

    // Retrieve the Direct3D Device and poll the capabilities
    pDevice = m_pActor->GetDevice();
    pDevice->GetDeviceCaps( &Caps );
    pDevice->Release();

    // Calculate the best skinning method and store
    pMeshContainer->SkinMethod = DetectSkinningMethod( pSkinInfo, pMesh );

    // Built differently based on the type of skinning required
    if ( pMeshContainer->SkinMethod == CActor::SKINMETHOD_INDEXED )
    {
        ULONG MaxFaceInfluences;
        ULONG Flags = D3DXMESHOPT_VERTEXCACHE;

        LPDIRECT3DINDEXBUFFER9 pIB;
        hRet = pMesh->GetIndexBuffer(&pIB);
        if (FAILED(hRet)) return hRet;

        // Retrieve the maximum number of influences for the mesh index buffer
        hRet = pSkinInfo->GetMaxFaceInfluences( pIB, pMesh->GetNumFaces(), &MaxFaceInfluences);
        pIB->Release();
        if (FAILED(hRet)) return hRet;

        // In Direct3D, the most weights / indices we can have in a single vertex is 4. Therefore
        // a single triangle can never be influenced by more than 12 matrices. Any indices or weights
        // that are referenced in the SkinInfo above and beyond this number (3 vertices in a face * 4 weights)
        // will be discarded by ConvertToIndexedBlendedMesh ANYWAY. This line ensures that the test case
        // (to determine whether the capabilities of the card are up to the job), only tests against this
        // 'post' build maximum, rather than the number of weights / indices that happened to be stored 
        // in the skin info.
        MaxFaceInfluences = min( MaxFaceInfluences, 12 );

        // If normals are present in the vertex data that needs to be blended for lighting, then 
        // the number of matrices is half the number specified by MaxVertexBlendMatrixIndex.
        ULONG MaxSupportedIndices = Caps.MaxVertexBlendMatrixIndex + 1;
        if ( pMesh->GetFVF() & D3DFVF_NORMAL) MaxSupportedIndices /= 2;

        // Test to see if we can support this in hardware
        if ( MaxSupportedIndices < MaxFaceInfluences )
        {
            // HW does not support indexed vertex blending. Use SW instead
            pMeshContainer->PaletteEntryCount = min(256, pSkinInfo->GetNumBones());
            pMeshContainer->SoftwareVP = true;
            Flags |= D3DXMESH_SOFTWAREPROCESSING | D3DXMESH_SYSTEMMEM;
        
        } // End if unsupported in hardware
        else
        {
            // Using hardware - determine palette size from caps and number of bones
            pMeshContainer->PaletteEntryCount = min( MaxSupportedIndices, pSkinInfo->GetNumBones() );
            pMeshContainer->SoftwareVP = false;
            Flags |= D3DXMESH_MANAGED;
        
        } // End if supported in hardware

        // Convert the mesh
        hRet = pSkinInfo->ConvertToIndexedBlendedMesh( pMesh, Flags, pMeshContainer->PaletteEntryCount, 
            pMeshContainer->pAdjacency, NULL, NULL, NULL, 
            &pMeshContainer->InfluenceCount,
            &pMeshContainer->AttribGroupCount, 
            &pMeshContainer->pBoneCombination, 
            pMeshOut );
        if ( FAILED(hRet) ) return hRet;

    } // End if Indexed Skinning
    else if ( pMeshContainer->SkinMethod == CActor::SKINMETHOD_NONINDEXED )
    {
        ULONG CurrentIndex = 0;

        // Convert the mesh
        hRet = pSkinInfo->ConvertToBlendedMesh( pMesh, D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE, 
            pMeshContainer->pAdjacency, NULL, NULL, NULL, 
            &pMeshContainer->InfluenceCount,
            &pMeshContainer->AttribGroupCount, 
            &pMeshContainer->pBoneCombination,
            pMeshOut );
        if ( FAILED(hRet) ) return hRet;

        // If the device can only do 2 matrix blends, ConvertToBlendedMesh cannot approximate all meshes to it
        // Thus we split the mesh in two parts: The part that uses at most 2 matrices and the rest. The first is
        // drawn using the device's HW vertex processing and the rest is drawn using SW vertex processing.
        LPD3DXBONECOMBINATION pBoneComb = (LPD3DXBONECOMBINATION)pMeshContainer->pBoneCombination->GetBufferPointer();

        // Allocate space for the vertex processing remap table
        pMeshContainer->pVPRemap = new ULONG[ pMeshContainer->AttribGroupCount ];
        if ( !pMeshContainer->pVPRemap ) { (*pMeshOut)->Release(); return E_OUTOFMEMORY; }

        // Loop through each of the attributes and determine how many bones influence
        // the vertices in this set, and determine whether it is software or hardware.
        for ( i = 0; i < 2; ++i )
        {
            // First pass is for hardware, second for software
            for ( j = 0; j < pMeshContainer->AttribGroupCount; ++j )
            {
                ULONG InfluenceCount = 0;

                // Loop through for each of our maximum 'influence' items
                for ( k = 0; k < pMeshContainer->InfluenceCount; ++k )
                {
                    // If there is a bone used here, increase our max influence count
                    if ( pBoneComb[ j ].BoneId[ k ] != UINT_MAX) InfluenceCount++;

                } // Next influence entry

                // If the maximum number of influencing bones exceeded the hardware capabilities
                // we know this is software, so in pass 0 we should skip, but in pass 1 we should store
                if ( InfluenceCount > Caps.MaxVertexBlendMatrices )
                {
                    // If it exceeds, we only store on the software pass
                    if ( i == 1 ) pMeshContainer->pVPRemap[ CurrentIndex++ ] = j;

                } // End if exceeds HW capabilities
                else
                {
                    // If it is within HW caps, we only store on the first pass
                    if ( i == 0 )
                    {
                        pMeshContainer->pVPRemap[ CurrentIndex++ ] = j;
                        pMeshContainer->SWRemapBegin++;

                    } // End if HW pass

                } // End if within HW capabilities

            } // Next attribute group

            // If all were found to be supported by hardware, no need to test for SW
            if ( pMeshContainer->SWRemapBegin >= pMeshContainer->AttribGroupCount ) break;

        } // Next Pass

        // If there are entries which require software processing, we must clone our mesh
        // to ensure that software processing is enabled.
        if ( pMeshContainer->SWRemapBegin < pMeshContainer->AttribGroupCount )
        {
            LPD3DXMESH pCloneMesh;

            // Clone the mesh including our flag
            pDevice = m_pActor->GetDevice();
            hRet = (*pMeshOut)->CloneMeshFVF( D3DXMESH_SOFTWAREPROCESSING | (*pMeshOut)->GetOptions(), (*pMeshOut)->GetFVF(), pDevice, &pCloneMesh );
            pDevice->Release();

            // Validate result
            if ( FAILED( hRet ) ) return hRet;

            // Release the old output mesh and store again
            (*pMeshOut)->Release();
            *pMeshOut = pCloneMesh;

        } // End if software elements required

    } // End if non indexed skinning
    else if ( pMeshContainer->SkinMethod == CActor::SKINMETHOD_SOFTWARE )
    {
        // Clone the mesh that we'll be using for rendering software skinned
        pDevice = m_pActor->GetDevice();
        hRet = pMesh->CloneMeshFVF( D3DXMESH_MANAGED, pMesh->GetFVF(), pDevice, &pMeshContainer->pSWMesh );
        pDevice->Release();

        // Validate result
        if ( FAILED( hRet ) ) return hRet;

        // Add ref the mesh, we're going to pass it straight back out
        pMesh->AddRef();
        *pMeshOut = pMesh;

    } // End if Software Skinning

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : DetectSkinningMethod ()
// Desc : Taking the flags specified by the application, determine the best
//        skinning method for us to use.
//-----------------------------------------------------------------------------
ULONG CAllocateHierarchy::DetectSkinningMethod( ID3DXSkinInfo * pSkinInfo, LPD3DXMESH pMesh ) const
{
    HRESULT hRet;
    ULONG   SkinMethod = m_pActor->GetSkinningMethod();

    // Autodetecting?
    if ( !(SkinMethod & CActor::SKINMETHOD_AUTODETECT) )
    {
        // We are not to autodetect, so just return it
        return SkinMethod;

    } // End if no auto-detect
    else
    {
        LPDIRECT3DINDEXBUFFER9  pIB;
        D3DCAPS9                Caps;
        LPDIRECT3DDEVICE9       pDevice;
        ULONG                   MaxInfluences;
        bool                    bHWIndexed = false, bHWNonIndexed = false;

        // Retrieve the Direct3D Device and poll the capabilities
        pDevice = m_pActor->GetDevice();
        pDevice->GetDeviceCaps( &Caps );
        pDevice->Release();

        // First of all we will test for HW indexed support
        bHWIndexed = false;


        // Is indexing supported?
        if ( Caps.MaxVertexBlendMatrixIndex > 0 )
        {
            // Retrieve the mesh index buffer
            hRet = pMesh->GetIndexBuffer(&pIB);
            if ( !FAILED(hRet) )
            {
                // Retrieve the maximum number of influences for the mesh index buffer
                hRet = pSkinInfo->GetMaxFaceInfluences( pIB, pMesh->GetNumFaces(), &MaxInfluences );
                pIB->Release();

                // Validate
                if ( !FAILED(hRet) && Caps.MaxVertexBlendMatrixIndex + 1 >= MaxInfluences ) bHWIndexed = true;

            } // End if no failure

        } // End if indexing supported

        // Now we will test for HW non-indexed support
        bHWNonIndexed = false;

        // It should be safe to assume that if the maximum number of vertex influences
        // is larger than the card capabilities, then at least one of the resulting attribute
        // groups generated by ConvertToBlendedMesh will require SoftwareVP
        hRet = pSkinInfo->GetMaxVertexInfluences( &MaxInfluences );

        // Validate
        if ( !FAILED(hRet) && Caps.MaxVertexBlendMatrices >= MaxInfluences ) bHWNonIndexed = true;

        // Return the detected mode
        if ( bHWIndexed == true && bHWNonIndexed == false )
        {
            // Hardware indexed is supported in full
            return CActor::SKINMETHOD_INDEXED;

        } // End if only indexed is supported in full in hardware
        else if ( bHWNonIndexed == true && bHWIndexed == false )
        {
            // Hardware non-indexed is supported in full
            return CActor::SKINMETHOD_NONINDEXED;

        } // End if only non-indexed is supported in full in hardware
        else if ( bHWNonIndexed == true && bHWIndexed == true )
        {
            // What hardware method do we prefer since they are both supported?
            if ( SkinMethod & CActor::SKINMETHOD_PREFER_HW_NONINDEXED )
                return CActor::SKINMETHOD_NONINDEXED;
            else
                return CActor::SKINMETHOD_INDEXED;
        }
        else 
        {
            // What software method do we prefer since neither are supported?
            if ( SkinMethod & CActor::SKINMETHOD_PREFER_SW_INDEXED )
                return CActor::SKINMETHOD_INDEXED;
            else
                return CActor::SKINMETHOD_NONINDEXED;

        } // End if both provide only software methods

    } // End if auto detect
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

    // Release data
    if ( pContainer->pBoneCombination ) pContainer->pBoneCombination->Release();
    if ( pContainer->pSWMesh          ) pContainer->pSWMesh->Release();
    if ( pContainer->pBoneOffset      ) delete []pContainer->pBoneOffset;
    if ( pContainer->ppBoneMatrices   ) delete []pContainer->ppBoneMatrices;
    if ( pContainer->pAdjacency       ) delete []pContainer->pAdjacency;
    if ( pContainer->pVPRemap         ) delete []pContainer->pVPRemap;

    // Release other data
    if ( pContainer->MeshData.pMesh   ) pContainer->MeshData.pMesh->Release();
    if ( pContainer->Name             ) free( pContainer->Name );   // '_tcsdup' allocated.
    if ( pContainer->pMesh            ) delete pContainer->pMesh;
    delete pContainer;

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : CAnimationSet () (Constructor)
// Desc : Class Constructor.
//-----------------------------------------------------------------------------
CAnimationSet::CAnimationSet( ID3DXAnimationSet * pAnimSet )
{
    ULONG   i;
    HRESULT hRet;
    ID3DXKeyframedAnimationSet * pKeySet = NULL;

    // Query to ensure we have a keyframed animation set
    hRet = pAnimSet->QueryInterface( IID_ID3DXKeyframedAnimationSet, (LPVOID*)&pKeySet );
    if ( FAILED(hRet) || !pKeySet ) throw hRet;

    // Duplicate the name if available
    LPCTSTR strName = pKeySet->GetName( );
    m_strName = (strName) ? _tcsdup( strName ) : NULL;

    // Store total length and ticks per second
    m_fLength           = pKeySet->GetPeriod();
    m_fTicksPerSecond   = pKeySet->GetSourceTicksPerSecond( );

    // Copy callback keys
    m_pCallbacks     = NULL;
    m_nCallbackCount = pKeySet->GetNumCallbackKeys( );
    if ( m_nCallbackCount > 0 )
    {
        // Allocate memory to hold the callback keys
        m_pCallbacks = new D3DXKEY_CALLBACK[ m_nCallbackCount ];
        if ( !m_pCallbacks ) throw E_OUTOFMEMORY;

        // Copy the callback keys over
        hRet = pKeySet->GetCallbackKeys( m_pCallbacks );
        if ( FAILED(hRet) ) throw hRet;

    } // End if any callback keys

    // Allocate memory for animation items
    m_pAnimations = NULL;
    m_nAnimCount  = pKeySet->GetNumAnimations();
    if ( m_nAnimCount )
    {
        // Allocate memory to hold animation items
        m_pAnimations = new CAnimationItem[ m_nAnimCount ];
        if ( !m_pAnimations ) throw E_OUTOFMEMORY;

        // Build the animation items
        for ( i = 0; i < m_nAnimCount; ++i )
        {
            hRet = m_pAnimations[ i ].BuildItem( pKeySet, i );
            if ( FAILED(hRet) ) throw hRet;

        } // Next animation

    } // End if any animations

    // Release our keyframed friend
    pKeySet->Release();

    // *************************************
    // *** VERY IMPORTANT
    // *************************************
    // Set our initial reference count to 1.
    m_nRefCount = 1;

}

//-----------------------------------------------------------------------------
// Name : ~CAnimationSet () (Destructor)
// Desc : Class Destructor.
//-----------------------------------------------------------------------------
CAnimationSet::~CAnimationSet( )
{
    // Release any memory
    if ( m_pCallbacks  ) delete []m_pCallbacks; m_pCallbacks = NULL;
    if ( m_pAnimations ) delete []m_pAnimations; m_pAnimations = NULL;
    if ( m_strName     ) free( m_strName ); m_strName = NULL;

}

//-----------------------------------------------------------------------------
// Name : AddRef ()
// Desc : Increments the internal reference count for this object, to inform
//        us that someone else is sharing a reference to us.
//-----------------------------------------------------------------------------
ULONG CAnimationSet::AddRef( )
{
    return m_nRefCount++;
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Decrements the internal reference count for this object, to inform
//        us that a reference to us is no longer needed. Once the reference
//        count == 0, we release our own memory.
//-----------------------------------------------------------------------------
ULONG CAnimationSet::Release( )
{
    // Decrement ref count
    m_nRefCount--;

    // If the reference count has got down to 0, delete us
    if ( m_nRefCount == 0 )
    {
        // Delete us (cast just to be safe, so that any non virtual destructor
        // is called correctly)
        delete (CAnimationSet*)this;

        // WE MUST NOT REFERENCE ANY MEMBER VARIABLES FROM THIS POINT ON!!!
        // For this reason, we must simply return 0 here, rather than dropping
        // out of the if statement, since m_nRefCount references memory which
        // has been released (and would cause a protection fault).
        return 0;

    } // End if ref count == 0

    // Return the reference count
    return m_nRefCount;
}

//-----------------------------------------------------------------------------
// Name : QueryInterface ()
// Desc : Called via our base class to determine what type of object we are.
//        We must also cast back and return the pointer to ensure that the
//        correct vtable items are assumed.
//-----------------------------------------------------------------------------
HRESULT CAnimationSet::QueryInterface( REFIID iid, LPVOID *ppv )
{
    // We support three interfaces, the base ID3DXAnimationSet, IUnknown and ourselves.
    // It's important that we cast these items, so that the compiler knows we possibly
    // need to switch to an alternate VTable
    if ( iid == IID_IUnknown )

        *ppv = (LPVOID)((IUnknown*)this);

    else if ( iid == IID_ID3DXAnimationSet )

        *ppv = (LPVOID)((ID3DXAnimationSet*)this);

    else if ( iid == IID_CAnimationSet )

        *ppv = (LPVOID)((CAnimationSet*)this);

    else
    {
        // The specified interface is not supported!
        *ppv = NULL;
        return E_NOINTERFACE;

    } // End iid switch

    // Call addref since a pointer is being copied
    AddRef();

    // We're supported!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : GetName ()
// Desc : Return the name of the animation set
//-----------------------------------------------------------------------------
LPCTSTR CAnimationSet::GetName( )
{
    return m_strName;
}

//-----------------------------------------------------------------------------
// Name : GetPeriod ()
// Desc : Get the total length of this animation set in seconds.
//-----------------------------------------------------------------------------
DOUBLE CAnimationSet::GetPeriod( )
{
    return m_fLength;
}

//-----------------------------------------------------------------------------
// Name : GetPeriodicPosition ()
// Desc : Map the position specified into our local set time.
//-----------------------------------------------------------------------------
DOUBLE CAnimationSet::GetPeriodicPosition( DOUBLE Position )
{
    // To map, we'll just loop the position round
    double Pos = fmod( Position, m_fLength );
    if ( Pos < 0 ) Pos += m_fLength;
    return Pos;
}

//-----------------------------------------------------------------------------
// Name : GetNumAnimations ()
// Desc : Return the total number of indivdual animation items we are storing.
//-----------------------------------------------------------------------------
UINT CAnimationSet::GetNumAnimations( )
{
    return m_nAnimCount;
}

//-----------------------------------------------------------------------------
// Name : GetAnimationNameByIndex ()
// Desc : Return the name of the animation item, using the specified index to
//        look up the item from our internal array.
//-----------------------------------------------------------------------------
HRESULT CAnimationSet::GetAnimationNameByIndex ( UINT Index, LPCTSTR *ppName )
{
    // Validate
    if ( !ppName || !*ppName || Index >= m_nAnimCount ) return D3DERR_INVALIDCALL;

    // Copy the selected item's name (it must always exist).
    _tcscpy( (LPTSTR)*ppName, m_pAnimations[ Index ].GetName() );

    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : GetAnimationIndexByName ()
// Desc : Return the index of the animation item, using the specified name to
//        loop up the item from our internal array.
//-----------------------------------------------------------------------------
HRESULT CAnimationSet::GetAnimationIndexByName ( LPCTSTR pName, UINT *pIndex )
{
    ULONG i;

    // Validate
    if ( !pIndex || !pName ) return D3DERR_INVALIDCALL;

    // Search through our animation sets
    for ( i = 0; i < m_nAnimCount; ++i )
    {
        // Does the name match ?
        if ( _tcsicmp( pName, m_pAnimations[i].GetName() ) == 0 ) 
        {
            // We found it, store the index and return
            *pIndex = i;
            return D3D_OK;

        } // End if names match

    } // Next Item

    // We didn't find this item!
    return D3DERR_NOTFOUND;
}

//-----------------------------------------------------------------------------
// Name : GetSRT ()
// Desc : The core of the animation system. Given a time based 'position', that
//        has already been mapped into our local timeframe, retrieve the
//        interpolated results for each component, from the specified animation
//        item.
//-----------------------------------------------------------------------------
HRESULT CAnimationSet::GetSRT( DOUBLE PeriodicPosition, UINT Animation, D3DXVECTOR3 *pScale, D3DXQUATERNION *pRotation, D3DXVECTOR3 *pTranslation )
{
    // Validate
    if ( Animation > m_nAnimCount ) return D3DERR_INVALIDCALL;

    // Return the interpolated result
    return m_pAnimations[ Animation ].GetSRT( PeriodicPosition, pScale, pRotation, pTranslation );
}

//-----------------------------------------------------------------------------
// Name : GetCallback ()
// Desc : Search for a callback using the specified search method.
// Note : This wraps the callbacks so that one set of keys, within the periodic
//        position will keep repeating, appearing to the controller such that
//        the exist duplicated within the track timeline. This allows the
//        internaly callback key caching mechanism to function correctly.
//-----------------------------------------------------------------------------
HRESULT CAnimationSet::GetCallback( DOUBLE Position, DWORD Flags, DOUBLE *pCallbackPosition, LPVOID *ppCallbackData )
{
    long                i;   // Must be signed long for the backwards search
    D3DXKEY_CALLBACK    Key;
    bool                bFoundKey = false;
    double              fPeriodic, fLoopTime;

    // Are there any keys at all?
    if ( m_nCallbackCount == 0 ) return D3DERR_NOTFOUND;

    // Modulate position back into our range (and convert to ticks)
    fPeriodic = fmod( Position, m_fLength );
    if ( fPeriodic < 0 ) fPeriodic += m_fLength;
    fPeriodic *= m_fTicksPerSecond;
    fLoopTime = (Position * m_fTicksPerSecond) - fPeriodic;

    // Searching forwards or backwards?
    if ( !(Flags & D3DXCALLBACK_SEARCH_BEHIND_INITIAL_POSITION) )
    {
        // Find the first callback equal to, or greater than the periodic position
        for ( i = 0; i < (signed)m_nCallbackCount ; ++i )
        {
            if ( m_pCallbacks[i].Time >= fPeriodic ) break;

        } // Next Callback

        // If nothing was found, then this HAS to be the first key (for the next loop)
        if ( i == m_nCallbackCount )
        {
            // Increase the loop time and use the first key
            fLoopTime += (m_fLength * m_fTicksPerSecond);
            i = 0;

        } // End if wrap

        // Get the key
        Key = m_pCallbacks[ i ];

        // Do we ignore keys that share the same position?
        bool bExclude = Flags & D3DXCALLBACK_SEARCH_EXCLUDING_INITIAL_POSITION;
        if ( (bExclude && Key.Time == fPeriodic) ) 
        {
            // If we're going to wrap onto the next loop, increase our loop time
            // and also wrap 'i' around such that i+1 is an index to the first key in the array
            if ( (i + 1) == m_nCallbackCount ) { fLoopTime += (m_fLength * m_fTicksPerSecond); i = -1; }

            // Get the next key
            Key = m_pCallbacks[ i + 1 ];

        } // End if skip to the next key

    } // End if searching forwards
    else
    {
        // Find the first callback equal to, or greater than the periodic position
        for ( i = (signed)m_nCallbackCount - 1; i >= 0; --i )
        {
            if ( m_pCallbacks[i].Time <= fPeriodic ) break;

        } // Next Callback

        // If nothing was found, then this HAS to be the last key (for the next loop)
        if ( i == -1 )
        {
            // Decrease the loop time and use the last key
            fLoopTime -= (m_fLength * m_fTicksPerSecond);
            i = (signed)m_nCallbackCount - 1;

        } // End if wrap    

        // Get the key
        Key = m_pCallbacks[ i ];

        // Do we ignore keys that share the same position?
        bool bExclude = Flags & D3DXCALLBACK_SEARCH_EXCLUDING_INITIAL_POSITION;
        if ( (bExclude && Key.Time == fPeriodic) ) 
        {
            // If we're going to wrap onto the previous loop, decrease our loop time
            // and also wrap 'i' around such that i-1 is an index to the last key in the array
            if ( (i - 1) < 0 ) { fLoopTime -= (m_fLength * m_fTicksPerSecond); i = m_nCallbackCount; }

            // Get the next key
            Key = m_pCallbacks[ i - 1 ];

        } // End if skip to the next key

    } // End if searching backwards

    // Return the time, mapped to the track position (converted back to seconds)
    if ( pCallbackPosition ) *pCallbackPosition = (Key.Time + fLoopTime) / m_fTicksPerSecond;

    // Return the callback data
    if ( ppCallbackData ) *ppCallbackData = Key.pCallbackData;

    // Success!!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : CAnimationItem () (Constructor)
// Desc : Class Constructor.
//-----------------------------------------------------------------------------
CAnimationItem::CAnimationItem(  )
{
    // Reset required variables
    m_strName               = NULL;
    m_pScaleKeys            = NULL;
    m_pTranslateKeys        = NULL;
    m_pRotateKeys           = NULL;
    m_nScaleKeyCount        = 0;
    m_nTranslateKeyCount    = 0;
    m_nRotateKeyCount       = 0;

    // Index caching
    m_nLastScaleIndex       = 0;
    m_nLastRotateIndex      = 0;
    m_nLastTranslateIndex   = 0;
    m_fLastPosRequest       = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CAnimationItem () (Destructor)
// Desc : Class Destructor.
//-----------------------------------------------------------------------------
CAnimationItem::~CAnimationItem( )
{
    // Release any memory items
    if ( m_strName        ) free( m_strName );
    if ( m_pScaleKeys     ) delete []m_pScaleKeys;
    if ( m_pTranslateKeys ) delete []m_pTranslateKeys;
    if ( m_pRotateKeys    ) delete []m_pRotateKeys;

    // Reset required variables
    m_strName               = NULL;
    m_pScaleKeys            = NULL;
    m_pTranslateKeys        = NULL;
    m_pRotateKeys           = NULL;
    m_nScaleKeyCount        = 0;
    m_nTranslateKeyCount    = 0;
    m_nRotateKeyCount       = 0;
}

//-----------------------------------------------------------------------------
// Name : GetName ()
// Desc : Return the name of the frame of which this animation item is
//        'animating'.
//-----------------------------------------------------------------------------
LPCTSTR CAnimationItem::GetName( ) const
{
    return m_strName;
}

//-----------------------------------------------------------------------------
// Name : GetSRT ()
// Desc : The core of the animation system. Given a time based 'position', that
//        has already been mapped into our local timeframe, retrieve the
//        interpolated results for each keyed component stored.
//-----------------------------------------------------------------------------
HRESULT CAnimationItem::GetSRT( DOUBLE PeriodicPosition, D3DXVECTOR3 *pScale, D3DXQUATERNION *pRotation, D3DXVECTOR3 *pTranslation )
{
    ULONG                i;
    D3DXQUATERNION       q1, q2;
    double               fInterpVal, fTicks;
    LPD3DXKEY_VECTOR3    pKeyVec1 , pKeyVec2;
    LPD3DXKEY_QUATERNION pKeyQuat1, pKeyQuat2;

    // Validate parameters
    if ( !pScale || !pRotation || !pTranslation ) return D3DERR_INVALIDCALL;

    // Clear them out as D3D does for us :)
    *pScale       = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
    *pTranslation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXQuaternionIdentity( pRotation );

    // Reset index caching if we've wrapped around (Note: this caching method is
    // designed primarily with a traditional forward playing animation in mind.
    // Although this does not prevent the animation from playing backwards, the
    // index caching method employed here will not optimize for this situation).
    // Another good optimization which could be utilised instead of this simple 
    // cache, is to perform a binary search on the key data (i.e. divide and compare)
    // This will not however be as effective an optimization, although it is more
    // generic and will improve performance with both forward and backward playing 
    // animations (such as the 'Ping-Pong' playing style).
    if ( PeriodicPosition < m_fLastPosRequest )
    {
        m_nLastScaleIndex       = 0;
        m_nLastRotateIndex      = 0;
        m_nLastTranslateIndex   = 0;

    } // End if wrapped / move backwards

    // Store the last requested position
    m_fLastPosRequest = PeriodicPosition;

    // Now calculate the 'TimeStamp' value.
    fTicks = PeriodicPosition * m_fTicksPerSecond;

    // ******************************************************************
    // * SCALE KEYS
    // ******************************************************************
    if ( m_nScaleKeyCount )
    {
        pKeyVec1 = pKeyVec2 = NULL;
        for ( i = m_nLastScaleIndex; i < m_nScaleKeyCount - 1; ++i )
        {
            LPD3DXKEY_VECTOR3 pKey     = &m_pScaleKeys[i];
            LPD3DXKEY_VECTOR3 pNextKey = &m_pScaleKeys[i + 1];

            // Do these keys  bound the requested time ?
            if ( fTicks >= pKey->Time && fTicks <= pNextKey->Time )
            {
                // Update last index
                m_nLastScaleIndex = i;

                // Store the two bounding keys
                pKeyVec1 = pKey;
                pKeyVec2 = pNextKey;
                break;

            } // End if found keys

        } // Next Scale Key

        // Make sure we found keys
        if ( pKeyVec1 && pKeyVec2 ) 
        {
            // Calculate interpolation
            fInterpVal  = fTicks - pKeyVec1->Time;
            fInterpVal /= (pKeyVec2->Time - pKeyVec1->Time);

            // Interpolate!
            D3DXVec3Lerp( pScale, &pKeyVec1->Value, &pKeyVec2->Value, (float)fInterpVal );

        } // End if keys were found
        else
        {
            // Scale is the same as the last scale key found
            if ( m_nScaleKeyCount ) *pScale = m_pScaleKeys[ m_nScaleKeyCount - 1 ].Value;

            // Inform cache that it should no longer search unless cache is invalidated
            m_nLastScaleIndex = m_nScaleKeyCount - 1;


        } // End if no keys found

    } // End if any scale keys

    // ******************************************************************
    // * ROTATION KEYS
    // ******************************************************************
    if ( m_nRotateKeyCount )
    {
        pKeyQuat1 = pKeyQuat2 = NULL;
        for ( i = m_nLastRotateIndex; i < m_nRotateKeyCount- 1; ++i )
        {
            LPD3DXKEY_QUATERNION pKey     = &m_pRotateKeys[i];
            LPD3DXKEY_QUATERNION pNextKey = &m_pRotateKeys[i + 1];

            // Do these keys bound the requested time ?
            if ( fTicks >= pKey->Time && fTicks <= pNextKey->Time )
            {
                // Update last index
                m_nLastRotateIndex = i;

                // Store the two bounding keys
                pKeyQuat1 = pKey;
                pKeyQuat2 = pNextKey;
                break;

            } // End if found keys

        } // Next Rotation Key

        // Make sure we found keys
        if ( pKeyQuat1 && pKeyQuat2 )
        {
            // Reverse 'winding' of these values
            D3DXQuaternionConjugate( &q1, &pKeyQuat1->Value );
            D3DXQuaternionConjugate( &q2, &pKeyQuat2->Value );

            // Calculate interpolation
            fInterpVal  = fTicks - pKeyQuat1->Time;
            fInterpVal /= (pKeyQuat2->Time - pKeyQuat1->Time);

            // Interpolate!
            D3DXQuaternionSlerp( pRotation, &q1, &q2, (float)fInterpVal );

        } // End if keys were found
        else
        {
            // Rotation is the same as the last key found
            if ( m_nRotateKeyCount ) D3DXQuaternionConjugate( pRotation,  &m_pRotateKeys[ m_nRotateKeyCount - 1 ].Value );

            // Inform cache that it should no longer search unless cache is invalidated
            m_nLastRotateIndex = m_nRotateKeyCount - 1;

        } // End if no keys found

    } // End if any rotation keys

    // ******************************************************************
    // * TRANSLATION KEYS
    // ******************************************************************
    if ( m_nTranslateKeyCount )
    {
        pKeyVec1 = pKeyVec2 = NULL;
        for ( i = m_nLastTranslateIndex; i < m_nTranslateKeyCount - 1; ++i )
        {
            LPD3DXKEY_VECTOR3 pKey     = &m_pTranslateKeys[i];
            LPD3DXKEY_VECTOR3 pNextKey = &m_pTranslateKeys[i + 1];

            // Do these keys bound the requested time ?
            if ( fTicks >= pKey->Time && fTicks <= pNextKey->Time )
            {
                // Update last index
                m_nLastTranslateIndex = i;

                // Store the two bounding keys
                pKeyVec1 = pKey;
                pKeyVec2 = pNextKey;
                break;

            } // End if found keys

        } // Next Translation Key

        // Make sure we found keys
        if ( pKeyVec1 && pKeyVec2 )
        {
            // Calculate interpolation
            fInterpVal  = fTicks - pKeyVec1->Time;
            fInterpVal /= (pKeyVec2->Time - pKeyVec1->Time);

            // Interpolate!
            D3DXVec3Lerp( pTranslation, &pKeyVec1->Value, &pKeyVec2->Value, (float)fInterpVal );

        } // End if keys were found
        else
        {
            // Rotation is the same as the last key found
            if ( m_nTranslateKeyCount ) *pTranslation = m_pTranslateKeys[ m_nTranslateKeyCount - 1 ].Value;

            // Inform cache that it should no longer search unless cache is invalidated
            m_nLastTranslateIndex = m_nTranslateKeyCount - 1;

        } // End if no keys found

    } // End if any translation keys

    // We're done
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : BuildItem ()
// Desc : Build the animation item from the set / index specified.
//-----------------------------------------------------------------------------
HRESULT CAnimationItem::BuildItem( ID3DXKeyframedAnimationSet *pAnimSet, ULONG ItemIndex )
{
    LPCTSTR strName = NULL;

    // Validate
    if ( !pAnimSet || ItemIndex > pAnimSet->GetNumAnimations( ) ) return D3DERR_INVALIDCALL;

    // Get the name and duplicate it
    pAnimSet->GetAnimationNameByIndex( ItemIndex, &strName );
    m_strName = _tcsdup( strName );

    // Store any secondary values
    m_fTicksPerSecond    = pAnimSet->GetSourceTicksPerSecond( );
    m_nScaleKeyCount     = pAnimSet->GetNumScaleKeys( ItemIndex );
    m_nRotateKeyCount    = pAnimSet->GetNumRotationKeys( ItemIndex );
    m_nTranslateKeyCount = pAnimSet->GetNumTranslationKeys( ItemIndex );

    // Allocate any memory required
    if ( m_nScaleKeyCount     ) m_pScaleKeys     = new D3DXKEY_VECTOR3[ m_nScaleKeyCount ];
    if ( m_nRotateKeyCount    ) m_pRotateKeys    = new D3DXKEY_QUATERNION[ m_nRotateKeyCount ];
    if ( m_nTranslateKeyCount ) m_pTranslateKeys = new D3DXKEY_VECTOR3[ m_nTranslateKeyCount ];

    // Retrieve the keys
    if ( m_pScaleKeys     ) pAnimSet->GetScaleKeys( ItemIndex, m_pScaleKeys );
    if ( m_pRotateKeys    ) pAnimSet->GetRotationKeys( ItemIndex, m_pRotateKeys );
    if ( m_pTranslateKeys ) pAnimSet->GetTranslationKeys( ItemIndex, m_pTranslateKeys );

    // We're done!
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Name : CActionDefinition () (Constructor)
// Desc : Class Constructor.
//-----------------------------------------------------------------------------
CActionDefinition::CActionDefinition( )
{
    // Clear required variables
    m_strName[0]            = _T('\0');
    m_nSetDefinitionCount   = 0;
    m_pSetDefinitions       = NULL;
}

//-----------------------------------------------------------------------------
// Name : CActionDefinition () (Destructor)
// Desc : Class Destructor.
//-----------------------------------------------------------------------------
CActionDefinition::~CActionDefinition( )
{
    // Delete any flat arrays
    if ( m_pSetDefinitions ) delete []m_pSetDefinitions;

    // Clear required variables.
    m_strName[0]            = _T('\0');
    m_nSetDefinitionCount   = 0;
    m_pSetDefinitions       = NULL;
}

//-----------------------------------------------------------------------------
// Name : LoadAction ()
// Desc : Performs the brunt of the actual loading of an individual action,
//        from the specified '.ACT' file.
//-----------------------------------------------------------------------------
HRESULT CActionDefinition::LoadAction( ULONG nActionIndex, LPCTSTR ActFileName )
{
    TCHAR strSectionName[128];
    TCHAR strKeyName[128];
    TCHAR strBuffer[128];
    ULONG i;

    // First build the action section name
    _stprintf( strSectionName, _T("Action%i"), nActionIndex );

    // Remove any previous set definitions
    if ( m_pSetDefinitions ) delete []m_pSetDefinitions;
    m_pSetDefinitions     = NULL;
    m_nSetDefinitionCount = 0;

    // Retrieve the set definition count
    m_nSetDefinitionCount = ::GetPrivateProfileInt( strSectionName, _T("DefinitionCount"), 0, ActFileName );

    // If there are no sets, we were most likely unable to find the action in the file
    if ( m_nSetDefinitionCount == 0 ) return D3DERR_NOTFOUND;

    // Allocate the setdefinition array
    m_pSetDefinitions = new SetDefinition[ m_nSetDefinitionCount ];
    if ( !m_pSetDefinitions ) return E_OUTOFMEMORY;

    // Retrieve the name of the action
    ::GetPrivateProfileString( strSectionName, _T("Name"), _T(""), m_strName, 127, ActFileName );

    // Loop through each set defined in the file
    for ( i = 0; i < m_nSetDefinitionCount; ++i )
    {
        // Retriev a pointer to the definition for easy access
        SetDefinition * pDefinition = &m_pSetDefinitions[i];

        // Get the set name
        _stprintf( strKeyName, _T("Definition[%i].SetName"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T(""), pDefinition->strSetName, 127, ActFileName );

        // Get the group name
        _stprintf( strKeyName, _T("Definition[%i].GroupName"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T(""), pDefinition->strGroupName, 127, ActFileName );

        // Get the blend mode flags
        _stprintf( strKeyName, _T("Definition[%i].BlendMode"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T("Off"), strBuffer, 127, ActFileName );
        if ( _tcsicmp( strBuffer, _T("MixFadeGroup") ) == 0 )
            pDefinition->BlendMode = ActBlendMode::MixFadeGroup;
        else 
            pDefinition->BlendMode = ActBlendMode::Off;

        // Get the time mode flags
        _stprintf( strKeyName, _T("Definition[%i].TimeMode"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T("Begin"), strBuffer, 127, ActFileName );
        if ( _tcsicmp( strBuffer, _T("MatchGroup") ) == 0 )
            pDefinition->TimeMode = ActTimeMode::MatchGroup;
        else if ( _tcsicmp( strBuffer, _T("MatchSpecifiedGroup") ) == 0 )
            pDefinition->TimeMode = ActTimeMode::MatchSpecifiedGroup;
        else 
            pDefinition->TimeMode = ActTimeMode::Begin;

        // Retrieve and scan the weight float var
        _stprintf( strKeyName, _T("Definition[%i].Weight"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T("1.0"), strBuffer, 127, ActFileName );
        _stscanf( strBuffer, _T("%g"), &pDefinition->fWeight );

        // Retrieve and scan the speed float var
        _stprintf( strKeyName, _T("Definition[%i].Speed"), i );
        ::GetPrivateProfileString( strSectionName, strKeyName, _T("1.0"), strBuffer, 127, ActFileName );
        _stscanf( strBuffer, _T("%g"), &pDefinition->fSpeed );

        // If we're blending, there will be an additional parameter
        if ( pDefinition->BlendMode == ActBlendMode::MixFadeGroup )
        {
            // Retrieve and scan the various floats
            _stprintf( strKeyName, _T("Definition[%i].MixLength"), i );
            ::GetPrivateProfileString( strSectionName, strKeyName, _T("0.0"), strBuffer, 127, ActFileName );
            _stscanf( strBuffer, _T("%g"), &pDefinition->fMixLength );

            // If we returned a zero value, turn off mix fading of the group
            if ( pDefinition->fMixLength == 0.0f ) pDefinition->BlendMode = ActBlendMode::Off;

        } // End if fading

        // If we're timing against a specified group, there will be an additional parameter
        if ( pDefinition->TimeMode == ActTimeMode::MatchSpecifiedGroup )
        {
            // Retrieve and scan the various floats
            _stprintf( strKeyName, _T("Definition[%i].TimeGroup"), i );
            ::GetPrivateProfileString( strSectionName, strKeyName, pDefinition->strGroupName, pDefinition->strTimeGroup, 127, ActFileName );

        } // End if matching against alternate group

    } // Next new set definition

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : ApplyAction ()
// Desc : Call this function when this action should be applied to the
//        specified animation controller.
//-----------------------------------------------------------------------------
HRESULT CActionDefinition::ApplyAction( CActor * pActor )
{
    SetDefinition                    * pDefinition;
    CActionStatus                    * pStatus;
    CActionStatus::TrackActionStatus * pTrackStatus;
    ULONG                              i, j, nCount;
    D3DXTRACK_DESC                     TrackDesc;

    // Retrieve the actors action status
    pStatus = pActor->GetActionStatus();
    if ( !pStatus ) return D3DERR_INVALIDCALL;

    // Mark disabled tracks as free for use.
    nCount = pActor->GetMaxNumTracks();
    for ( j = 0; j < nCount; ++j )
    {
        // Retrieve action status information for this track.
        pTrackStatus = pStatus->GetTrackStatus( j );

        // Get the current track description.
        pActor->GetTrackDesc( j, &TrackDesc );

        // Flag as unused if the track was disabled
        if ( TrackDesc.Enable == FALSE )
        {
            pTrackStatus->bInUse            = false;
            pTrackStatus->bTransitioningOut = false;

        } // End if track is disabled

    } // Next track

    // Kill any sets belonging to groups that no longer exist
    nCount = pActor->GetMaxNumTracks();
    for ( j = 0; j < nCount; ++j )
    {
        // Retrieve action status information for this track.
        pTrackStatus = pStatus->GetTrackStatus( j );

        // Skip if this track is not in use or is currently transitioning out (we should only ever
        // find no one more than one track, assigned to this group, which is active at this point)
        if ( pTrackStatus->bInUse == false || pTrackStatus->bTransitioningOut == true ) continue;

        // Loop through each of our set definitions
        for ( i = 0; i < m_nSetDefinitionCount; ++i )
        {
            // Retrieve the definition pointer for easy access
            pDefinition = &m_pSetDefinitions[i];
            
            // Does this belong to this group?
            if ( _tcsicmp( pTrackStatus->strGroupName, pDefinition->strGroupName ) == 0 ) break;

        } // Next definition

        // If we reached the end, then this is a dead track
        if ( i == m_nSetDefinitionCount )
        {
            // Kill any track events already setup
            pActor->UnkeyAllTrackEvents( j );
            pActor->SetTrackEnable( j, FALSE );

            // Update status
            pTrackStatus->bInUse            = false;
            pTrackStatus->bTransitioningOut = false;
        
        } // End if dead track

    } // Next Track

    // Loop through each of our set definitions
    for ( i = 0; i < m_nSetDefinitionCount; ++i )
    {
        // Retrieve the definition pointer for easy access
        pDefinition = &m_pSetDefinitions[i];

        // Reset sensitive variables
        pDefinition->bNewTrack = false;
        pDefinition->fNewTime  = 0.0f;

        // Get timing data from actor if required
        if ( pDefinition->TimeMode != ActTimeMode::Begin )
        {
            // Choose which group name to pull timing from.
            TCHAR * pGroupName = pDefinition->strGroupName;
            if ( pDefinition->TimeMode == ActTimeMode::MatchSpecifiedGroup ) pGroupName = pDefinition->strTimeGroup;

            // Find the track in this group not currently transitioning out
            nCount = pActor->GetMaxNumTracks();
            for ( j = 0; j < nCount; ++j )
            {
                // Retrieve action status information for this track.
                pTrackStatus = pStatus->GetTrackStatus( j );

                // Skip if this track is not in use or is currently transitioning out (we should only ever
                // find no one more than one track, assigned to this group, which is active at this point)
                if ( pTrackStatus->bInUse == false || pTrackStatus->bTransitioningOut == true ) continue;

                // Does this belong to this group?
                if ( _tcsicmp( pTrackStatus->strGroupName, pGroupName ) == 0 )
                {
                    // Get the current track description.
                    pActor->GetTrackDesc( j, &TrackDesc );

                    // Store the position and break
                    pDefinition->fNewTime = TrackDesc.Position;
                    break;

                } // End if matching group

            } // Next Track

        } // End if need timing data

		long nSetMatched = -1;

		// First search for an exact match for this group / set combination
		nCount = pActor->GetMaxNumTracks();
		for ( j = 0; j < nCount; ++j )
		{
			// Retrieve action status information for this track.
			pTrackStatus = pStatus->GetTrackStatus( j );

			// Skip if this track is not in use
			if ( pTrackStatus->bInUse == false ) continue;

			// Does this belong to this group?
			if ( _tcsicmp( pTrackStatus->strGroupName, pDefinition->strGroupName ) == 0 )
			{
				// Is it physically the same set?
				if ( _tcsicmp( pTrackStatus->strSetName, pDefinition->strSetName ) == 0 ) nSetMatched = (signed)j;
				if ( nSetMatched >= 0 ) break;

			} // End if matching group

		} // Next Track

        // Does this definition require mix blending?
        if ( pDefinition->BlendMode == ActBlendMode::MixFadeGroup )
        {
            // Did we find an exact match for this group / set combination?
            if ( nSetMatched >= 0 )
            {
                // This is the same set as we're trying to blend in, first thing is to
                // kill all events currently queued for this track
                pActor->UnkeyAllTrackEvents( nSetMatched );

                // Clear status
                pTrackStatus = pStatus->GetTrackStatus( nSetMatched );
                pTrackStatus->bInUse            = true;
                pTrackStatus->bTransitioningOut = false;
                pTrackStatus->pActionDefinition = this;
                pTrackStatus->nSetDefinition    = i;

                // Queue up a weight event, and bring us back to the blend weight we requested
                pActor->KeyTrackWeight( nSetMatched, pDefinition->fWeight, pActor->GetTime(), pDefinition->fMixLength, D3DXTRANSITION_EASEINEASEOUT );

                // Get the current track description.
                pActor->GetTrackDesc( nSetMatched, &TrackDesc );

                // We are recovering from an outbound transition, so we will retain the timing from
                // this set, rather than the one we chose above, if they specified only 'MatchGroup' mode.
                if ( pDefinition->TimeMode != ActTimeMode::MatchGroup ) TrackDesc.Position = pDefinition->fNewTime;

                // Setup other track properties
                TrackDesc.Speed = pDefinition->fSpeed;

                // Set back to track
                pActor->SetTrackDesc( nSetMatched, &TrackDesc );

            } // End if found exact match
            else
            {
                // This definition is now due to be assigned to a new track
                pDefinition->bNewTrack = true;
            
            } // End if no exact match

            // Find all tracks in this group that are not currently transitioning out, and fade them
            nCount = pActor->GetMaxNumTracks();
            for ( j = 0; j < nCount; ++j )
            {
                // Skip this track if it's our matched track
                // (Remember, if we didn't find a match exactly, nSetMatched will still be -1)
                if ( (signed)j == nSetMatched ) continue;

                // Retrieve action status information for this track.
                pTrackStatus = pStatus->GetTrackStatus( j );

                // Skip if this track is not in use or is currently transitioning out
                if ( pTrackStatus->bInUse == false || pTrackStatus->bTransitioningOut == true ) continue;

                // Does this belong to this group?
                if ( _tcsicmp( pTrackStatus->strGroupName, pDefinition->strGroupName ) == 0 )
                {
                    // Kill any track events already setup
                    pActor->UnkeyAllTrackEvents( j );

                    // Queue up a weight event to fade out the track
                    pActor->KeyTrackWeight( j, 0.0f, pActor->GetTime(), pDefinition->fMixLength, D3DXTRANSITION_EASEINEASEOUT );
                    pActor->KeyTrackEnable( j, FALSE, pActor->GetTime() + pDefinition->fMixLength );

                    // Update status
                    pTrackStatus->bInUse            = true;
                    pTrackStatus->bTransitioningOut = true;

                } // End if matching group

            } // Next Track

        } // End if requires blending
        else
        {
            // Did we find an exact match for this group / set combination?
            if ( nSetMatched >= 0 )
            {
                // This is the same set as we're trying to apply, first thing is to
                // kill all events currently queued for this track
                pActor->UnkeyAllTrackEvents( nSetMatched );

                // Clear status
                pTrackStatus = pStatus->GetTrackStatus( nSetMatched );
                pTrackStatus->bInUse            = true;
                pTrackStatus->bTransitioningOut = false;
                pTrackStatus->pActionDefinition = this;
                pTrackStatus->nSetDefinition    = i;

                // Get the current track description.
                pActor->GetTrackDesc( nSetMatched, &TrackDesc );

                // The user specified a set which is already active, so we will retain the timing from
                // this set, rather than the one we chose above, if they specified only 'MatchGroup' mode.
                if ( pDefinition->TimeMode != ActTimeMode::MatchGroup ) TrackDesc.Position = pDefinition->fNewTime;

                // Setup other track properties
                TrackDesc.Speed  = pDefinition->fSpeed;
                TrackDesc.Weight = pDefinition->fWeight;

                // Set back to track
                pActor->SetTrackDesc( nSetMatched, &TrackDesc );
            
            } // End if found exact match
            else
            {
                // This definition is now due to be assigned to a new track
                pDefinition->bNewTrack = true;

            } // End if no exact match

            // Find all tracks in this group, and kill them dead
            nCount = pActor->GetMaxNumTracks();
            for ( j = 0; j < nCount; ++j )
            {
                // Skip this track if it's our matched track
                // (Remember, if we didn't find a match exactly, nSetMatched will still be -1)
                if ( (signed)j == nSetMatched ) continue;

                // Retrieve action status information for this track.
                pTrackStatus = pStatus->GetTrackStatus( j );
                
                // Skip if this track is not in use
                if ( pTrackStatus->bInUse == false ) continue;

                // Does this belong to this group?
                if ( _tcsicmp( pTrackStatus->strGroupName, pDefinition->strGroupName ) == 0 )
                {
                    // Kill any track events already setup
                    pActor->UnkeyAllTrackEvents( j );
                    pActor->SetTrackEnable( j, FALSE );

                    // Update status
                    pTrackStatus->bInUse            = false;
                    pTrackStatus->bTransitioningOut = false;

                } // End if matching group

            } // Next Track

        } // End if does not require blending

    } // Next Definition

    // Loop through each of our set definitions and assign to tracks if required
    for ( i = 0; i < m_nSetDefinitionCount; ++i )
    {
        // Retrieve the definition pointer for easy access
        pDefinition = &m_pSetDefinitions[i];

        // Skip if not required to assign to new track
        if ( !pDefinition->bNewTrack ) continue;

        // Find a free track
        long TrackIndex = -1;
        nCount = pActor->GetMaxNumTracks();
        for ( j = 0; j < nCount; ++j )
        {
            // Retrieve action status information for this track.
            pTrackStatus = pStatus->GetTrackStatus( j );

            // Free track?
            if ( pTrackStatus->bInUse == false ) { TrackIndex = (signed)j; break; }

        } // Next Track

        // Build track description
        TrackDesc.Position = pDefinition->fNewTime;
        TrackDesc.Weight   = pDefinition->fWeight;
        TrackDesc.Speed    = pDefinition->fSpeed;
        TrackDesc.Priority = D3DXPRIORITY_LOW;
        TrackDesc.Enable   = TRUE;

        // Update track action status
        pTrackStatus = pStatus->GetTrackStatus( TrackIndex );
        pTrackStatus->bInUse            = true;
        pTrackStatus->bTransitioningOut = false;
        pTrackStatus->pActionDefinition = this;
        pTrackStatus->nSetDefinition    = i;
        _tcscpy( pTrackStatus->strSetName, pDefinition->strSetName );
        _tcscpy( pTrackStatus->strGroupName, pDefinition->strGroupName );

        // Apply animation set
        pActor->SetTrackAnimationSetByName( TrackIndex, pDefinition->strSetName );

        // If we are blending, key events and override starting weight
        if ( pDefinition->BlendMode == ActBlendMode::MixFadeGroup )
        {
            TrackDesc.Weight = 0.0f;
            pActor->KeyTrackWeight( TrackIndex, pDefinition->fWeight, pActor->GetTime(), pDefinition->fMixLength, D3DXTRANSITION_EASEINEASEOUT );

        } // End if blending

        // Set track description
        pActor->SetTrackDesc( TrackIndex, &TrackDesc );

    } // Next Definition

    // Success!!
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : CActionStatus () (Constructor)
// Desc : Class Constructor.
//-----------------------------------------------------------------------------
CActionStatus::CActionStatus( )
{
    // Clear required variables
    m_pCurrentAction     = NULL;
    m_pTrackActionStatus = NULL;
    m_nMaxTrackCount     = 0;

    // *************************************
    // *** VERY IMPORTANT
    // *************************************
    // Set our initial reference count to 1.
    m_nRefCount = 1;
}

//-----------------------------------------------------------------------------
// Name : CActionStatus () (Destructor)
// Desc : Class Destructor.
//-----------------------------------------------------------------------------
CActionStatus::~CActionStatus( )
{
    // Delete any flat arrays
    if ( m_pTrackActionStatus ) delete []m_pTrackActionStatus;

    // Clear required variables.
    m_pCurrentAction     = NULL;
    m_pTrackActionStatus = NULL;
    m_nMaxTrackCount     = 0;
}

//-----------------------------------------------------------------------------
// Name : SetMaxTrackCount ()
// Desc : Resize the internal status tracking variables, based on the
//        maximum number of tracks specified.
//-----------------------------------------------------------------------------
HRESULT CActionStatus::SetMaxTrackCount( USHORT Count )
{
    TrackActionStatus * pBuffer = NULL;

    // If we're clearing out.
    if ( Count == 0 )
    {
        // Just release and return
        if ( m_pTrackActionStatus ) delete []m_pTrackActionStatus;
        m_pTrackActionStatus = NULL;
        m_nMaxTrackCount     = 0;

        // Success
        return S_OK;

    } // End if count == 0

    // Allocate enough room for the specified tracks
    pBuffer = new TrackActionStatus[ Count ];
    if ( !Count ) return E_OUTOFMEMORY;

    // Clear the buffer
    memset( pBuffer, 0, Count * sizeof(TrackActionStatus) );

    // Was there any previous data?
    if ( m_nMaxTrackCount > 0 && m_pTrackActionStatus )
    {
        // Copy over as much data as we can
        memcpy( pBuffer, m_pTrackActionStatus, min(m_nMaxTrackCount, Count) * sizeof(TrackActionStatus) );

    } // End if existing data

    // Release previous buffer
    if ( m_pTrackActionStatus ) delete []m_pTrackActionStatus;

    // Store buffer, and count
    m_pTrackActionStatus = pBuffer;
    m_nMaxTrackCount     = Count;

    // Success!
    return S_OK;

}

//-----------------------------------------------------------------------------
// Name : SetCurrentAction ()
// Desc : Sets the current action state to that indicated.
//-----------------------------------------------------------------------------
void CActionStatus::SetCurrentAction( CActionDefinition * pActionDefinition )
{
    // Just store the current action
    m_pCurrentAction = pActionDefinition;
}

//-----------------------------------------------------------------------------
// Name : AddRef ()
// Desc : Increments the internal reference count for this object, to inform
//        us that someone else is sharing a reference to us.
//-----------------------------------------------------------------------------
ULONG CActionStatus::AddRef( )
{
    return m_nRefCount++;
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Decrements the internal reference count for this object, to inform
//        us that a reference to us is no longer needed. Once the reference
//        count == 0, we release our own memory.
//-----------------------------------------------------------------------------
ULONG CActionStatus::Release( )
{
    // Decrement ref count
    m_nRefCount--;

    // If the reference count has got down to 0, delete us
    if ( m_nRefCount == 0 )
    {
        // Delete us (cast just to be safe, so that any non virtual destructor
        // is called correctly)
        delete (CActionStatus*)this;

        // WE MUST NOT REFERENCE ANY MEMBER VARIABLES FROM THIS POINT ON!!!
        // For this reason, we must simply return 0 here, rather than dropping
        // out of the if statement, since m_nRefCount references memory which
        // has been released (and would cause a protection fault).
        return 0;

    } // End if ref count == 0

    // Return the reference count
    return m_nRefCount;
}