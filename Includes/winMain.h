/****************************************************************************
*                                                                           *
* winMain.h -- WINDOW MAIN KERNEL ENGINE                                    *
*                                                                           *
* Copyright (c) INFORMATICHA.RO. All rights reserved(2009-2011).            *
*																			*
* Programmer: Ionescu Marius                                                *
*                                                                           *
* Designer: Ionescu Marius                                                  *
*                                                                           *
****************************************************************************/
#ifndef DXWINMAIN_H
#define DXWINMAIN_H

#include "..\Resources\resource.h"
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <D3DX9.h>

#define VK_DOT      0xBE
#define VK_COMMA    0xBC
#define VK_LBRACKET 0xDB
#define VK_RBRACKET 0xDD

class dxGameMgr;

typedef struct _CALLBACK_FUNC
{
    LPVOID  pFunction;
    LPVOID  pContext;

} CALLBACK_FUNC;

dxGameMgr* GetGameMgr();

class dxWinMain
{
	public:
		dxWinMain(void);
		~dxWinMain(void);

	private:

};

#endif