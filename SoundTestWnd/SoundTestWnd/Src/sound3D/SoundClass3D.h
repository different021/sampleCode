#pragma once


#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")


//////////////
// INCLUDES //
//////////////
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <tuple>

#include "CSoundWave3D.h"

//---------------------------------------------------------
// INI
//---------------------------------------------------------
const static TCHAR* FILENAME_INI = L"../sound/sound.ini";
#define PLAY_LOOP	DSBPLAY_LOOPING //반복
#define PLAY_ONCE	0				  //한번만

enum eSOUND
{
	eSOUND_STAGE01_ = 0,
	eSOUND_SFX_MOVE_STEREO,
	eSOUND_SFX_MOVE,

	eSOUND_MAX_,
};

//---------------------------------------------------------
//GOLBAL
//---------------------------------------------------------
void InitSound3D(HWND);
void CleanUp3D();
void PlaySound3D(eSOUND track, DWORD flag);


void SetPos(float x, float y, float z);


class SoundClass3D
{
private:

public:
	SoundClass3D();
	SoundClass3D(const SoundClass3D&);
	~SoundClass3D();

	bool Initalize(HWND);
	void CleanUp();

private:
	bool InitializeDirectSound(HWND);
	void CreateDSBUFFERDESC();
	void CreateDS_2ndBUFFERDESC(DSBUFFERDESC& desc);

	bool LoadWaveFileFromINI(const TCHAR*, TCHAR** output);
	bool LoadWaveFile();
	bool LoadWaveFile(const TCHAR* szFileName);

	void CleanUpWaveFile(IDirectSoundBuffer8**, IDirectSound3DBuffer8**);

public:
	void SetPos(float x, float y, float z);
	void SetStartPos(eSOUND track, DWORD dwPos);
	void SetVolume(eSOUND track, LONG num);
	void PlayWaveFile(eSOUND track, DWORD dwFlag);

private:
	IDirectSound8*			m_pDS				 = NULL;
	IDirectSoundBuffer*		m_pPrimaryBuffer	 = NULL;
	IDirectSound3DListener* m_pListener			 = NULL;

	std::vector<CSoundWave3D*> m_SoundWaveList;
	int m_iNumberOfFile = 0;
	TCHAR* m_pFileNameList;
};

