/****************************************************************************
*                                                                           *
* dxAnimation.h -- Animation of model                                       *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/
#include "..\Includes\winMain.h"
#include "..\Includes\dxGameMgr.h"
#include "..\Common\dxDirectives.h"
#include "..\Common\dxTypes.h"
#include "..\Includes\dxUtility.h"

dxGameMgr    game_Mgr; 

dxWinMain::dxWinMain()
{
}

dxWinMain::~dxWinMain()
{
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int iCmdShow )
{
    int gameBegin;

	//instatiate the engine game begin
	if (!game_Mgr.InitInstance( hInstance, lpCmdLine, iCmdShow )) 
		return 0;

    gameBegin = game_Mgr.BeginGame();

    if ( !game_Mgr.ShutDown() )  
		MessageBox( 0, _T("Failed to shut system down correctly, please check file named 'debug.txt'.\r\n\r\nIf the problem persists, please contact technical support."), _T("Non-Fatal Error"), MB_OK | MB_ICONEXCLAMATION );

    return gameBegin;

}

dxGameMgr* GetGameMgr()
{
    return &game_Mgr;
}
