/****************************************************************************
*                                                                           *
* dxSound.h -- MANAGER SOUND                                                *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once
#include <dsound.h>
#include <string>
#include "dsutil.h"
#include <vector>

class dxSound
{
	public:
		dxSound();
		~dxSound();

		bool init(HWND wndHandle);
		int loadFile(std::string filename);
		void playSound(int bufferNumber);
		void loopSound(int bufferNumber);
		void stopSound(int bufferNumber);
		void Mp3Init(void);
		void Mp3Load(char filename[]);
		void Mp3Cleanup(void);
		void Mp3Play(void);
		void Mp3Pause(void);
		void Mp3Stop(void);
		void Mp3SetVolume(long val);
		long Mp3GetVolume(void);
		void Mp3SetBalance(long val);
		long Mp3GetBalance(void);

	private:
		LPDIRECTSOUND8 directSound;	// The DirectSound Device
		CSoundManager* directSoundManager;

		std::vector <CSound*> sounds; //holds our sound buffers

};