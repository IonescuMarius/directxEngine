#include "..\Includes\dxSound.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>

#pragma comment(lib,"strmiids.lib")
IGraphBuilder* g_pGraphBuilder = NULL;
IMediaControl* g_pPlaybackControl = NULL;
IBasicAudio*   g_pVolumeControl = NULL;
int			   g_bReady = 0;

dxSound::dxSound()
{

}


dxSound::~dxSound()
{
	if (directSoundManager)
	{
		delete directSoundManager;
		directSoundManager = NULL;
	}

	if (!sounds.empty()){
		for (unsigned int i=0; i<sounds.size(); i++)
		{
			if (sounds[i])
				delete sounds[i];
		}
	}
}
	
bool dxSound::init(HWND wndHandle)
{

	//create the direct Sound Manager
	directSoundManager = new CSoundManager();
	HRESULT hr;
	//initialize the sound manager
	hr = directSoundManager->Initialize( wndHandle, DSSCL_PRIORITY );

	if FAILED (hr){
		return false;
	}

	return true;
}

int dxSound::loadFile(std::string filename)
{
	CSound* tempSound = NULL; //holds our sound buffer
	HRESULT hr = NULL; //result variable

	//make the filename usable by the sound manager
	char* file = (char*)filename.c_str();

	//set the buffer format
	directSoundManager->SetPrimaryBufferFormat( 2, 22050, 16 );

	//create the sound buffer from the file
	directSoundManager->Create( &tempSound, file, 0, GUID_NULL );

	//save our loaded buffer into the sounds vector
	sounds.push_back((CSound*) tempSound);

	//return the index number of the saved buffer.
	return (int)(sounds.size() - 1);
}

void dxSound::playSound(int bufferNumber)
{
	sounds[bufferNumber]->Play( 0, 0, 0);
}

void dxSound::loopSound(int bufferNumber)
{
	sounds[bufferNumber]->Play( 0, 0, DSBPLAY_LOOPING);
}

void dxSound::stopSound(int bufferNumber)
{
	sounds[bufferNumber]->Stop();
}

void dxSound::Mp3Init(void) 
{
	CoInitialize(NULL);
}

void dxSound::Mp3Load(char filename[])
{
	// DirectShow requires unicode (16bit) characters rather than 8bit ascii
	WCHAR wFile[MAX_PATH];

	// convert ascii to unicode
	MultiByteToWideChar(CP_ACP, 0, filename, -1, wFile, MAX_PATH);

	// create a graph build to allow us to make the audio signal path
	if( SUCCEEDED(CoCreateInstance(CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void**)&g_pGraphBuilder)))
	{
		// create a simple controller for the sound file
		g_pGraphBuilder->QueryInterface( IID_IMediaControl, (void **)&g_pPlaybackControl );

		// create a simple interface for volume and balance
		g_pGraphBuilder->QueryInterface( IID_IBasicAudio  , (void **)&g_pVolumeControl );
		 
		// load and start playback of the specified sound file
		if( SUCCEEDED( g_pGraphBuilder->RenderFile(wFile, NULL) ) )
		{
			g_bReady = 1;
		}
	}
}

void dxSound::Mp3Cleanup(void)
{
	
	// stop playback of file if playing
	if( g_pPlaybackControl )
	{
		g_pPlaybackControl->Stop();
	}

	// release the volume controller
	if( g_pVolumeControl )
	{
		g_pVolumeControl->Release();
		g_pVolumeControl = NULL;
	}
	
	// release the playback controller
	if( g_pPlaybackControl )
	{
		g_pPlaybackControl->Release();
		g_pPlaybackControl = NULL;
	}
	
	// release the graph builder 
	if( g_pGraphBuilder )
	{
		g_pGraphBuilder->Release();
		g_pGraphBuilder = NULL;
	}
	
	// Unload DirectShow
	CoUninitialize();
}

void dxSound::Mp3Play(void)
{
	if(g_bReady)
	{
		g_pPlaybackControl->Run();
	}
}

void dxSound::Mp3Pause(void)
{
	if(g_bReady)
	{
		g_pPlaybackControl->Pause();
	}
}

void dxSound::Mp3Stop(void)
{
	if(g_bReady)
	{
		g_pPlaybackControl->Stop();
	}
}

void dxSound::Mp3SetVolume(long val)
{
	if(g_bReady)
	{
		g_pVolumeControl->put_Volume(val);
	}
}

long dxSound::Mp3GetVolume(void)
{
	long val=0;
	if(g_bReady)
	{
		g_pVolumeControl->get_Volume(&val);
	}
	return val;
}

void dxSound::Mp3SetBalance(long val)
{
	if(g_bReady)
	{
		g_pVolumeControl->put_Balance(val);
	}
}

long dxSound::Mp3GetBalance(void)
{
	long val=0;
	if(g_bReady)
	{
		g_pVolumeControl->get_Balance(&val);
	}
	return val;
}


