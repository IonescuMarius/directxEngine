/****************************************************************************
*                                                                           *
* dxTimer.h -- The Frame timer                                              *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#ifndef _DXTIMER_H_
#define _DXTIMER_H_

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <D3DX9.h>

const ULONG MAX_SAMPLE_COUNT = 1;

class dxTimer
{
	public:
		dxTimer();
		virtual ~dxTimer();

		void init(int fps);
		int framesToUpdate();

		void	        Tick( float fLockFPS = 0.0f );
		unsigned long   GetFrameRate( LPTSTR lpszString = NULL ) const;
		float           GetTimeElapsed() const;
		void            SetSimulationSpeed( float fSpeed ) { dx_fSimulationSpeed = fSpeed; }

	private:
		
		bool            dx_PerfHardware;             // Has Performance Counter
		float           dx_TimeScale;                // Amount to scale counter
		float           dx_TimeElapsed;              // Time elapsed since previous frame
		__int64         dx_CurrentTime;              // Current Performance Counter
		__int64         dx_LastTime;                 // Performance Counter last frame
		__int64         dx_PerfFreq;                 // Performance Frequency

		float           dx_FrameTime[MAX_SAMPLE_COUNT];
		ULONG           dx_SampleCount;
		float           dx_fSimulationSpeed;

		unsigned long   dx_FrameRate;                // Stores current framerate
		unsigned long   dx_FPSFrameCount;            // Elapsed frames in any given second
		float           dx_FPSTimeElapsed;           // How much time has passed during FPS sample

		// timer variables
		LARGE_INTEGER timeNow;
		LARGE_INTEGER timePrevious;
		LARGE_INTEGER timerFreq;

		float fps;
		float intervals;
		float intervalsSinceLastUpdate;
		float framesInLastSecond;
		float intervalsPerFrame;
		int Requested_FPS;

};

#endif