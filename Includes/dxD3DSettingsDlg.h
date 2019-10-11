/****************************************************************************
*                                                                           *
* dxD3DSettings.h -- settings D3D                                           *
*                                                                           *
* Copyright (c) INFORMATICHA.RO. All rights reserved(2009-2010).            *
*																			*
* Programmer: Ionescu Marius                                                *
*                                                                           *
* Designer: Ionescu Marius                                                  *
*                                                                           *
****************************************************************************/

#ifndef _DXD3DSETTINGSDLG_H_
#define _DXD3DSETTINGSDLG_H_

//-----------------------------------------------------------------------------
// dxD3DSettingsDlg Specific Includes
//-----------------------------------------------------------------------------
#include "..\Includes\dxD3DInitialize.h"		// Automatic Application Include
#include <tchar.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : dxD3DSettingsDlg (Class)
// Desc : Creates and controls a settings dialog for the d3d devices.
//-----------------------------------------------------------------------------
class dxD3DSettingsDlg
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         dxD3DSettingsDlg();
	virtual ~dxD3DSettingsDlg();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
	int				ShowDialog                ( dxD3DInitialize * pInitialize, dxD3DSettings *pSettings = NULL, HWND hWndParent = NULL );
    dxD3DSettings    GetD3DSettings            ( ) const { return m_D3DSettings; }
    
private:
    //-------------------------------------------------------------------------
	// Private Functions for This Class
	//-------------------------------------------------------------------------
	BOOL			SettingsDlgProc           ( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    void            AdapterChanged            (  );
    void            DeviceChanged             (  );
    void            WindowedFullscreenChanged (  );
    void            AdapterFormatChanged      (  );
    void            ResolutionChanged         (  );
    void            RefreshRateChanged        (  );
    void            BackBufferFormatChanged   (  );
    void            DepthStencilFormatChanged (  );
    void            MultisampleTypeChanged    (  );
    void            MultisampleQualityChanged (  );
    void            VertexProcessingChanged   (  );
    void            PresentIntervalChanged    (  );

    void            Combo_ItemAdd             ( ULONG ComboID, LPVOID pData, LPCTSTR pStrDesc );
    LPVOID          Combo_GetSelectedItemData ( ULONG ComboID );
    ULONG           Combo_GetItemCount        ( ULONG ComboID );
    void            Combo_SelectItem          ( ULONG ComboID, ULONG Index );
    void            Combo_SelectItemData      ( ULONG ComboID, LPVOID pData );
    void            Combo_Clear               ( ULONG ComboID );
    bool            Combo_FindText            ( ULONG ComboID, LPCTSTR pStrText );

    dxD3DSettings::Settings * GetSettings ( )  { return m_D3DSettings.GetSettings(); }

	//-------------------------------------------------------------------------
	// Private Static Functions For This Class
	//-------------------------------------------------------------------------
    static BOOL CALLBACK StaticDlgProc        (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

    //-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
	dxD3DInitialize *m_pInitialize;			// The class which stores enum objects
    HWND            m_hWndDlg;              // The main dialog window handle 
    dxD3DSettings    m_D3DSettings;          // The settings we are currently using

};

#endif