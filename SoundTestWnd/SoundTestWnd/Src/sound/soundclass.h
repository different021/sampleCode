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


/////////////////////////////////////////////////////////////////////
//
// �� �κ��� �����ؼ� ���.
//
/////////////////////////////////////////////////////////////////////

enum eSOUND
{
	eBGM_STAGE01_ = 0,
	eBGM_STAGE02_,

	eSOUND_MAX_,
};



///////////////////////////////////////////////////////////////////
//���� �� ����
///////////////////////////////////////////////////////////////////

typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;
class SoundClass;




/////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////

extern SoundClass* g_pSound;

void InitSound_(HWND);
void CleanupSound_();
bool PlaySound_(eSOUND);




////////////////////////////////////////////
// Class name: SoundClass
////////////////////////////////////////////

class SoundClass
{
private:
	struct WaveHeaderType					//�� 44byte
	{
		char chunkId[4];					// RIFF : Resource InterChange File Format (Big Endian)
		unsigned long chunkSize;			// ����ũ�� - 8 :: sizeof(chunk_ID) -sizeof(chunk_Size).
		char format[4];						// 'WAVE' : ���̺� ������ ��� 
		char subChunkId[4];					// 'fmt ', fmt' ' �ڿ� ���� ��ĭ. = 4ĭ. 
		unsigned long subChunkSize;			//  
		unsigned short audioFormat;			// PCM = 1 (2byte) little-Endian
		unsigned short numChannels;			// ü�� ��. mono : 1 , Stereo : 2
		unsigned long sampleRate;			// 1�ʸ� ��� �������� ����ȭ. 
		unsigned long  bytesPerSecond;		// SampleRate * numChannels * BitsPerSample / 8; �Ҹ��� 1�� �� ����Ʈ �Ҹ�?
		unsigned short blockAlign;			// ��ü ä���� �����ϴ� �� ������ ũ��
		unsigned short bitsPerSample;		// �Ѱ��� ������ ��� ��Ʈ�� �����°�.
		char dataChunkId[4];				// 'data'	//���ڿ����� BigEndian
		unsigned long dataSize;				// ���� PMC ������. (BitsPerSample / 8) * NumChannels * ���� ���ü�.
	};

public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	bool Initialize(HWND);
	void Shutdown();
	bool PlayWaveFile(eSOUND track);
	bool StopWaveFile(eSOUND track);

	//Volume Control
	void VolumeUp(eSOUND target,LONG num);
	void VolumeDown(eSOUND target ,LONG num);
	void VolumeMute(eSOUND target);
	void VolumeMax(eSOUND target);
	

private:
	bool InitializeDirectSound(HWND);
	bool CreateDevice(HWND);
	bool CreateBuffer(IDirectSoundBuffer*&);
	void ShutdownDirectSound();

	bool LoadWaveFromINI(const TCHAR* fileName);
	bool LoadWaveFile(const TCHAR* szFileName);
	bool LoadWaveFile(const TCHAR*, IDirectSoundBuffer8**);
	void ShutdownWaveFile(IDirectSoundBuffer8**);

private:	
	std::vector<DeviceTuple> m_vSoundDevice;

	LONG m_Vol;

	IDirectSound8* m_DirectSound;
	IDirectSoundBuffer* m_primaryBuffer;
	//IDirectSoundBuffer8* m_secondaryBuffer[eSOUND_MAX_];

	//std::vector<CSoundWave*> m_SoundWaveList;
	CSoundWave* m_SoundWaveList[eSOUND_MAX_];
};



#endif




///////////////////////////////////////////////////////////////////
//
// ����: https://psychoria.tistory.com/212?category=556283
//		
///////////////////////////////////////////////////////////////////
