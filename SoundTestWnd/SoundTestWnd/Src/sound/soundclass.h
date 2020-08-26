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
// �� �κ��� �����ؼ� ���.
//
/////////////////////////////////////////////////////////////////////

enum eSOUND
{
	eSOUND_STAGE01_ = 0,			
	eSOUND_STAGE02_,
	eSOUND_DRUM_,

	eSOUND_MAX_,
};


///////////////////////////////////////////////////////////////////
//���� �� ����
///////////////////////////////////////////////////////////////////

//���� �÷���

#define PLAY_LOOP	DSBPLAY_LOOPING //�ݺ�
#define PLAY_ONCE	0				  //�ѹ���



typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;

/////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////
void InitSound_(HWND);							//�ʱ�ȭ			 ���α׷� ���۽� ȣ�� 
void CleanupSound_();							//������ Release  ���α׷� ����� ȣ��

bool PlaySound_(eSOUND, DWORD);					//CONTROL �Լ�
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
	bool CreatePrimaryBuffer(IDirectSoundBuffer*&);

	bool LoadWaveFileNameFromINI(const TCHAR* fileName, TCHAR** output);
	bool LoadWaveFile(const TCHAR* szFileName);

	void ReleaseWaveFile(IDirectSoundBuffer8**);	//���� ������� �ʴ´�. ��ü�� �ٲ����.
	void ReleaseDirectSound();

private:	
	std::vector<DeviceTuple> m_vSoundDevice;

	LONG m_Vol;

	IDirectSound8* m_DirectSound;						
	IDirectSoundBuffer* m_primaryBuffer;				//

	std::vector<CSoundWave*> m_SoundWaveList;

	TCHAR* pFileNameList;								//�ҷ����ϴ� ���� ���� ����Ʈ. �ļ��� �и��Ұ� �׷���?
	INT m_iNumberOfFile;
};



extern SoundClass* g_pSound;


#endif



///////////////////////////////////////////////////////////////////
//
// ����: https://psychoria.tistory.com/212?category=556283
//		
///////////////////////////////////////////////////////////////////
