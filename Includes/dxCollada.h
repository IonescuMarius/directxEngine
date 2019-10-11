/****************************************************************************
*                                                                           *
* dxCollada.h -- COLLADA                                                    *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <d3d9.h>
#include <d3dx9tex.h>

class dxCollada
{
	public:
		dxCollada(void);
		~dxCollada(void);

		bool initLoader();
		void processCollada();

	private:

		
};
