#include "..\Includes\dxTimer.h"

dxTimer::dxTimer()
{
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&dx_PerfFreq)) 
    { 
		dx_PerfHardware		= TRUE;
		QueryPerformanceCounter((LARGE_INTEGER *) &dx_LastTime); 
		dx_TimeScale			= 1.0f / dx_PerfFreq;
	} 
    else 
    { 
		dx_PerfHardware		= FALSE;
		dx_LastTime			= timeGetTime(); 
		dx_TimeScale		= 0.001f;
    }

    dx_SampleCount			= 0;
	dx_FrameRate			= 0;
	dx_FPSFrameCount		= 0;
	dx_FPSTimeElapsed		= 0.0f;
    dx_fSimulationSpeed		= 0.0f;
}

dxTimer::~dxTimer()
{

}

void dxTimer::init(int fps)
{
	QueryPerformanceFrequency(&timerFreq);
	QueryPerformanceCounter(&timeNow);
	QueryPerformanceCounter(&timePrevious);
	
	Requested_FPS=fps;

	intervalsPerFrame = ( (float)timerFreq.QuadPart / Requested_FPS );
}

int dxTimer::framesToUpdate()
{
	int framesToUpdate = 0;
	QueryPerformanceCounter(&timeNow);

	intervalsSinceLastUpdate = (float)timeNow.QuadPart - (float)timePrevious.QuadPart;
	
	framesToUpdate = (int)( intervalsSinceLastUpdate / intervalsPerFrame );

	if (framesToUpdate != 0)
	{
		QueryPerformanceCounter(&timePrevious);
	}

	return framesToUpdate;
}

void dxTimer::Tick( float fLockFPS )
{
    float fTimeElapsed; 

	if ( dx_PerfHardware ) 
    {
		QueryPerformanceCounter((LARGE_INTEGER *)&dx_CurrentTime);
	} 
    else 
    {
		dx_CurrentTime = timeGetTime();
	}

	fTimeElapsed = (dx_CurrentTime - dx_LastTime) * dx_TimeScale;

    if ( fLockFPS > 0.0f )
    {
        while ( fTimeElapsed < (1.0f / fLockFPS))
        {
	        if ( dx_PerfHardware ) 
            {
		        QueryPerformanceCounter((LARGE_INTEGER*)&dx_CurrentTime);
	        } 
            else 
            {
		        dx_CurrentTime = timeGetTime();
	        }

	        fTimeElapsed = (dx_CurrentTime - dx_LastTime) * dx_TimeScale;
        }
    }

	dx_LastTime = dx_CurrentTime;

    if ( fabsf(fTimeElapsed - dx_TimeElapsed) < 1.0f  )
    {

        memmove( &dx_FrameTime[1], dx_FrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float) );
        dx_FrameTime[ 0 ] = fTimeElapsed;
        if ( dx_SampleCount < MAX_SAMPLE_COUNT ) 
			dx_SampleCount++;
    }

	dx_FPSFrameCount++;
	dx_FPSTimeElapsed += fTimeElapsed;
	if ( dx_FPSTimeElapsed > 1.0f) 
    {
		dx_FrameRate		= dx_FPSFrameCount;
		dx_FPSFrameCount		= 0;
		dx_FPSTimeElapsed	= 0.0f;
	}

    dx_TimeElapsed = 0.0f;
    if ( dx_SampleCount > 0 )
    {
        for ( ULONG i = 0; i < dx_SampleCount; i++ ) 
			dx_TimeElapsed += dx_FrameTime[ i ];
        dx_TimeElapsed /= dx_SampleCount;

    }
}

unsigned long dxTimer::GetFrameRate( LPTSTR lpszString ) const
{
    if ( lpszString )
    {
        _itot( dx_FrameRate, lpszString, 10 );

        strcat( lpszString, _T(" FPS") );

    }

    return dx_FrameRate;
}

float dxTimer::GetTimeElapsed() const
{
    return dx_TimeElapsed * dx_fSimulationSpeed;
}