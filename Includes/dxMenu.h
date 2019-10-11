/****************************************************************************
*                                                                           *
* dxMenu.h -- Menu                                                          *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#include <d3d9.h>
#include <d3dx9.h>

class dxInput;
class dxPlane;
class dxSound;

class dxMenu
{
	public:
		dxMenu(void);
		~dxMenu(void);

		bool init(dxInput* input, LPDIRECT3DDEVICE9 device);
		void update();
		void render(LPDIRECT3DDEVICE9 device);
		int getMessage();
		enum { NEW_GAME = 1, EXIT = 2, CONTINUE = 3, OPTIONS = 4 };

	private:

		dxInput* myInput;
		dxSound* sounds;
		dxPlane* background;
		dxPlane* continueGame;
		dxPlane* continueGameHighlighted;
		dxPlane* newGame;
		dxPlane* newGameHighlighted;
		dxPlane* exit;
		dxPlane* exitHighlighted;
		dxPlane* options;
		dxPlane* optionsHighlighted;
		
		int menuItemSelected;
		int message;
		
		bool menuContinue;
		bool menuNew;
		bool menuExit;
		bool menuOptions;
};

	
