#include "..\Includes\dxMenu.h"
#include "..\Includes\dxPlane.h"
#include "..\Includes\dxInput.h"
#include "..\Includes\dxSound.h"
#include "..\Includes\dxUtility.h"
#include "..\Common\dxDirectives.h"
#include "..\Includes\dxGameMgr.h"
using namespace std;

dxMenu::dxMenu()
{
	myInput = NULL;
	sounds = NULL;
	background = NULL;
	continueGame = NULL;
	continueGameHighlighted = NULL;
	newGame = NULL;
	newGameHighlighted = NULL;
	options = NULL;
	optionsHighlighted = NULL;
	exit = NULL;
	exitHighlighted = NULL;
	menuContinue = false;
	menuNew = true;
	menuExit = false;
	menuOptions = false;
}

dxMenu::~dxMenu()
{
	if (myInput != NULL){delete myInput;}
	if (background != NULL){delete background;}
	if (continueGame != NULL){delete continueGame;}
	if (continueGameHighlighted != NULL){delete continueGameHighlighted;}
	if (newGame != NULL){delete newGame;}
	if (newGameHighlighted != NULL){delete newGameHighlighted;}
	if (exit != NULL){delete exit;}
	if (exitHighlighted != NULL){delete exitHighlighted;}
	if (options != NULL){delete options;}
	if (optionsHighlighted != NULL){delete optionsHighlighted;}
}

bool dxMenu::init(dxInput* input, LPDIRECT3DDEVICE9 device)
{
	myInput = input;
	menuItemSelected = 0;
	message = 0;
	
	sounds = new dxSound();
	//sounds->init(hWnd);

	//load background menubackground.jpg
	background = new dxPlane();
	background->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/menubackground.jpg", true);

	//load new game image
	newGame = new dxPlane();
	newGame->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/newgame.jpg", false);
	newGame->setPosition(207, 200);
	newGameHighlighted = new dxPlane();
	newGameHighlighted->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/newgameh.jpg", false);
	newGameHighlighted->setPosition(207, 200);
	
	//load continue game image
	continueGame = new dxPlane();
	continueGame->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/continue.jpg", false);
	continueGame->setPosition(210,300);
	continueGameHighlighted = new dxPlane();
	continueGameHighlighted->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/continueh.jpg", false);
	continueGameHighlighted->setPosition(210,300);
	
	//load options game image
	options = new dxPlane();
	options->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/options.jpg", false);
	options->setPosition(200,400);
	optionsHighlighted = new dxPlane();
	optionsHighlighted->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/optionsh.jpg", false);
	optionsHighlighted->setPosition(200,400);
	
	//load exit image
	exit = new dxPlane();
	exit->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/exit.jpg", false); 
	exit->setPosition(210,500);
	exitHighlighted = new dxPlane();
	exitHighlighted->loadPlane(device, CUtility::GetTheCurrentDirectory()+"/data/exith.jpg", false); 
	exitHighlighted->setPosition(210,500);
	
	return true;
}

void dxMenu::update()
{
	myInput->getInput();
	
	if (myInput->keyPress(DIK_DOWN))
	{
		if (menuNew == true)
		{			
			menuContinue = true;
			menuOptions = false;
			menuExit = false;
			menuNew = false;
			menuItemSelected = 1;
		} 
		else if(menuContinue == true)
		{	
			menuOptions = true;		
			menuExit = false;
			menuContinue = false;
			menuNew = false;
			menuItemSelected = 2;			
		}
		else if(menuOptions == true)
		{			
			menuExit = true;
			menuNew = false;
			menuContinue = false;
			menuOptions = false;
			menuItemSelected = 3;			
		}
		else if(menuExit == true) 
		{		
			menuNew = true;
			menuContinue = false;
			menuOptions = false;
			menuExit = false;
			menuItemSelected = 0;
		}
	}

	if (myInput->keyPress(DIK_UP))
	{
		if (menuNew == true)
		{
			menuExit = true;
			menuOptions = false;
			menuContinue = false;
			menuNew = false;
			menuItemSelected = 3;
		} 
		else if(menuExit == true)
		{	
			menuOptions = true;		
			menuExit = false;
			menuContinue = false;
			menuNew = false;
			menuItemSelected = 2;			
		}
		else if(menuOptions == true)
		{	
			menuContinue = true;		
			menuNew = false;
			menuExit = false;
			menuOptions = false;
			menuItemSelected = 1;			
		}
		else if(menuContinue == true) 
		{		
			menuNew = true;		
			menuExit = false;
			menuOptions = false;
			menuContinue = false;
			menuItemSelected = 0;
		}
	}

	if (myInput->keyPress(DIK_RETURN))
	{
		switch (menuItemSelected)
		{
			case 0: 
				message = NEW_GAME;
			break;
			case 1: 
				message = CONTINUE;
			break;
			case 2: 
				message = OPTIONS;
			break;
			case 3: 
				message = EXIT;
			break;
		}	
	}
}

void dxMenu::render(LPDIRECT3DDEVICE9 device)
{
	background->render(device);
	
	if (menuNew == true)
	{
		newGameHighlighted->render(device);
		continueGame->render(device);
		options->render(device);
		exit->render(device);
	} 
	else if(menuContinue == true)
	{
		newGame->render(device);
		continueGameHighlighted->render(device);
		options->render(device);
		exit->render(device);
	}
	else if(menuOptions == true)
	{
		optionsHighlighted->render(device);
		newGame->render(device);
		continueGame->render(device);
		exit->render(device);
	}
	else if(menuExit == true)
	{
		newGame->render(device);
		continueGame->render(device);
		options->render(device);
		exitHighlighted->render(device);
	}
}

int dxMenu::getMessage()
{
	return message;
}