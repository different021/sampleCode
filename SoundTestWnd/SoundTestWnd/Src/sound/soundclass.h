///////////////////////////////////////////////////////////////////////////////
// Filename: soundclass.h
///////////////////////////////////////////////////////////////////////////////
#ifndef _SOUNDCLASS_H_
#define _SOUNDCLASS_H_


/////////////
// LINKING //
/////////////
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

#include "CSoundWave.h"

const static TCHAR* FILENAME_INI = L"../sound/sound.ini";

/////////////////////////////////////////////////////////////////////
//
// 이 부분을 수정해서 사용.
//
/////////////////////////////////////////////////////////////////////

enum eSOUND
{
	eBGM_STAGE01_ = 0,			
	eBGM_STAGE02_,
	eBGM_STAGE03_,

	eSOUND_MAX_,
};

/*
//약속된 값이라서 이렇게 하는게 하는게 맞나싶지만? enum이 제일 보기 좋을거 같기는 하다.
enum PLAYFLAG
{
	ePLAY_ONCE_ = 0,
	ePLAY_LOOP_ = DSBPLAY_LOOPING,
};
*/

///////////////////////////////////////////////////////////////////
//선언 및 정의
///////////////////////////////////////////////////////////////////

//실행 플래그
//반복
#define PLAY_LOOP DSBPLAY_LOOPING 
//한번만
#define PLAY_ONCE 0

typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;


/////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////
void InitSound_(HWND);							//초기화			 프로그램 시작시 호출 
void CleanupSound_();							//데이터 Release  프로그램 종료시 호출
//control 함수
bool PlaySound_(eSOUND, DWORD);
bool StopSound_(eSOUND, DWORD);
bool OnOffSound_();
void VolumeUp(eSOUND target, DWORD degree);
void VolumeDown(eSOUND target, DWORD degree);



////////////////////////////////////////////
// Class name: SoundClass
////////////////////////////////////////////

class SoundClass
{

public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	bool Initialize(HWND);
	void CleanUp();
	bool PlayWaveFile(eSOUND track, DWORD dwFlag);
	bool StopWaveFile(eSOUND track);

	bool CopyToPrim(eSOUND track);
	bool PlayPrime();

public:												//Volume Control
	void VolumeUp(eSOUND target,LONG num);
	void VolumeDown(eSOUND target ,LONG num);
	void MasterVolumeMute();
	void MasterVolumeMax();
	

private:
	bool InitializeDirectSound(HWND);
	bool CreateDevice(HWND);
	bool CreateBuffer(IDirectSoundBuffer*&);

	bool LoadWaveFileNameFromINI(const TCHAR* fileName, TCHAR** output);
	bool LoadWaveFile(const TCHAR* szFileName);

	void ReleaseWaveFile(IDirectSoundBuffer8**);	//이제 사용하지 않는다. 주체가 바뀌었다.
	void ReleaseDirectSound();

private:	
	std::vector<DeviceTuple> m_vSoundDevice;

	LONG m_Vol;

	IDirectSound8* m_DirectSound;						
	IDirectSoundBuffer* m_primaryBuffer;				//

	std::vector<CSoundWave*> m_SoundWaveList;

	TCHAR* pFileNameList;								//불러야하는 파일 네임 리스트. 파서를 분리할걸 그랫나?
	INT m_iNumberOfFile;
};



extern SoundClass* g_pSound;


#endif



///////////////////////////////////////////////////////////////////
//
// 참고: https://psychoria.tistory.com/212?category=556283
//		
///////////////////////////////////////////////////////////////////
