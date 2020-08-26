#pragma once
#include <memory>
#include <iostream>
#include <Windows.h>
#include <mmsystem.h>	
#include <vector>
#include <tuple>
#include <string>

#include <dsound.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "dsound.lib")


class CSound
{
	IDirectSound8* m_pDS;


public:
	CSound();
	~CSound();

	BOOL Init(HWND hWnd);
	void CleanUp();
};


typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;

int ReadWaveFile(LPTSTR szFileName);
BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);
HRESULT CreateSoundBuffer(LPDIRECTSOUND8 lpDS, LPDIRECTSOUNDBUFFER8* ppDsb8);
int WavePlay(LPDIRECTSOUNDBUFFER8* ppDsb8);

void OutputDebugWAVEFORMATEX(WAVEFORMATEX wfex);

extern CSound* g_pSound;