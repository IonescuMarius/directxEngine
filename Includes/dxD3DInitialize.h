/****************************************************************************
*                                                                           *
* dxD3DInitialize.h -- Initialize D3D                                       *
*                                                                           *
* Copyright (c) INFORMATICHA.RO. All rights reserved(2009-2010).            *
*																			*
* Programmer: Ionescu Marius                                                *
*                                                                           *
* Designer: Ionescu Marius                                                  *
*
****************************************************************************/

#ifndef _DXD3DINITIALIZE_H_
#define _DXD3DINITIALIZE_H_

#include <D3DX9.h>
#include <vector>

enum VERTEXPROCESSING_TYPE
{
    SOFTWARE_VP         = 1,        // Software Vertex Processing
    MIXED_VP            = 2,        // Mixed Vertex Processing
    HARDWARE_VP         = 3,        // Hardware Vertex Processing
    PURE_HARDWARE_VP    = 4         // Pure Hardware Vertex Processing
};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class dxD3DEnumDeviceOptions;
class dxD3DEnumDevice;
class dxD3DEnumAdapter;

//-----------------------------------------------------------------------------
// STL Vector Typedefs for Easy Access
//-----------------------------------------------------------------------------
typedef std::vector<D3DMULTISAMPLE_TYPE>    VectorMSType;
typedef std::vector<D3DFORMAT>              VectorFormat;
typedef std::vector<ULONG>                  VectorULONG;
typedef std::vector<VERTEXPROCESSING_TYPE>  VectorVPType;
typedef std::vector<D3DDISPLAYMODE>         VectorDisplayMode;
typedef std::vector<dxD3DEnumDeviceOptions*> VectorDeviceOptions;
typedef std::vector<dxD3DEnumDevice*>        VectorDevice;
typedef std::vector<dxD3DEnumAdapter*>       VectorAdapter;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : dxD3DEnumDeviceOptions (Support Class)
// Desc : Stores the various device options available for any device.
//-----------------------------------------------------------------------------
class dxD3DEnumDeviceOptions
{
public:
    ~dxD3DEnumDeviceOptions();

    ULONG                   AdapterOrdinal;
    D3DDEVTYPE              DeviceType;
    D3DCAPS9                Caps;
    D3DFORMAT               AdapterFormat;
    D3DFORMAT               BackBufferFormat;
    bool                    Windowed;
    VectorMSType            MultiSampleTypes;
    VectorULONG             MultiSampleQuality;
    VectorFormat            DepthFormats;
    VectorVPType            VertexProcessingTypes;
    VectorULONG             PresentIntervals;
};

//-----------------------------------------------------------------------------
// Name : dxD3DEnumDevice (Support Class)
// Desc : Stores the various capabilities etc for an individual device type.
//-----------------------------------------------------------------------------
class dxD3DEnumDevice
{
public:
    ~dxD3DEnumDevice();

    D3DDEVTYPE              DeviceType;
    D3DCAPS9                Caps;
    VectorDeviceOptions     Options;
};

//-----------------------------------------------------------------------------
// Name : dxD3DEnumAdapter (Support Class)
// Desc : Stores the various adapter modes for a single enumerated adapter.
//-----------------------------------------------------------------------------
class dxD3DEnumAdapter
{
public:
    ~dxD3DEnumAdapter();

    ULONG                   Ordinal;
    D3DADAPTER_IDENTIFIER9  Identifier;
    VectorDisplayMode       Modes;
    VectorDevice            Devices;
};

//-----------------------------------------------------------------------------
// Name : dxD3DSettings (Support Class)
// Desc : Allows us to set up the various options we will be using.
// Note : Also used internally by dxD3DSettingsDlg.
//-----------------------------------------------------------------------------
class dxD3DSettings 
{
public:
    
    struct Settings
    {
        ULONG                   AdapterOrdinal;
        D3DDISPLAYMODE          DisplayMode;
        D3DDEVTYPE              DeviceType;
        D3DFORMAT               BackBufferFormat;
        D3DFORMAT               DepthStencilFormat;
        D3DMULTISAMPLE_TYPE     MultisampleType;
        ULONG                   MultisampleQuality;
        VERTEXPROCESSING_TYPE   VertexProcessingType;
        ULONG                   PresentInterval;
    };

    bool        Windowed;
    Settings    Windowed_Settings;
    Settings    Fullscreen_Settings;

    Settings*   GetSettings() { return (Windowed) ? &Windowed_Settings : &Fullscreen_Settings; }

};

//-----------------------------------------------------------------------------
// Name : dxD3DInitialize (Class)
// Desc : Direct3D Initialization class. Detects supported formats, modes and
//        capabilities, and initializes the devices based on the chosen details
//-----------------------------------------------------------------------------
class dxD3DInitialize
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         dxD3DInitialize();
	virtual ~dxD3DInitialize();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    HRESULT                 Enumerate              ( LPDIRECT3D9 pD3D );
    
    HRESULT                 CreateDisplay          ( dxD3DSettings& D3DSettings, ULONG Flags = 0, HWND hWnd = NULL, WNDPROC pWndProc = NULL,
                                                     LPCTSTR Title = NULL, ULONG Width = CW_USEDEFAULT, ULONG Height = CW_USEDEFAULT, 
                                                     LPVOID lParam = NULL );

    HRESULT                 ResetDisplay           ( LPDIRECT3DDEVICE9 pD3DDevice, dxD3DSettings& D3DSettings, HWND hWnd = NULL );
    
    bool                    FindBestWindowedMode   ( dxD3DSettings & D3DSettings, bool bRequireHAL = false, bool bRequireREF = false );
    bool                    FindBestFullscreenMode ( dxD3DSettings & D3DSettings, D3DDISPLAYMODE * pMatchMode = NULL, bool bRequireHAL = false, bool bRequireREF = false );

    D3DPRESENT_PARAMETERS   BuildPresentParameters ( dxD3DSettings& D3DSettings, ULONG Flags = 0 );
    
    ULONG                   GetAdapterCount( ) const  { return m_vpAdapters.size(); }
    const dxD3DEnumAdapter  *GetAdapter( ULONG Index ) { return (Index > GetAdapterCount() ) ? NULL : m_vpAdapters[Index]; }
    HWND                    GetHWND( )                { return m_hWnd; }
    const LPDIRECT3D9       GetDirect3D( )            { return m_pD3D; }
    LPDIRECT3DDEVICE9       GetDirect3DDevice( );

    
    
private:
    //-------------------------------------------------------------------------
	// Private Functions for This Class
	//-------------------------------------------------------------------------
    HRESULT             EnumerateAdapters               ( );
    HRESULT             EnumerateDisplayModes           ( dxD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDevices                ( dxD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDeviceOptions          ( dxD3DEnumDevice  * pDevice, dxD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDepthStencilFormats    ( dxD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumerateMultiSampleTypes       ( dxD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumerateVertexProcessingTypes  ( dxD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumeratePresentIntervals       ( dxD3DEnumDeviceOptions * pDeviceOptions );

    //-------------------------------------------------------------------------
	// Private Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual bool        ValidateDisplayMode          ( const D3DDISPLAYMODE& Mode )                         { return true; }
    virtual bool        ValidateDevice               ( const D3DDEVTYPE& Type, const D3DCAPS9& Caps )       { return true; }
    virtual bool        ValidateDeviceOptions        ( const D3DFORMAT& BackBufferFormat, bool IsWindowed ) { return true; }
    virtual bool        ValidateDepthStencilFormat   ( const D3DFORMAT& DepthStencilFormat )                { return true; }
    virtual bool        ValidateMultiSampleType      ( const D3DMULTISAMPLE_TYPE& Type )                    { return true; }
    virtual bool        ValidateVertexProcessingType ( const VERTEXPROCESSING_TYPE& Type )                  { return true; }
    virtual bool        ValidatePresentInterval      ( const ULONG& Interval )                              { return true; }
    
    //-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
	LPDIRECT3D9		    m_pD3D;			    // Primary Direct3D Object.
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // Created Direct3D Device.
    HWND                m_hWnd;             // Created window handle
    VectorAdapter       m_vpAdapters;       // Enumerated Adapters
    
};

#endif