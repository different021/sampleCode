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
// 이 부분을 수정해서 사용.
//
/////////////////////////////////////////////////////////////////////

enum eSOUND
{
	eBGM_STAGE01_ = 0,			//명확한 이름 추천. 첫번째는 0.
	eBGM_STAGE02_,
	eBGM_STAGE03_,

	eSOUND_MAX_,
};



///////////////////////////////////////////////////////////////////
//선언 및 정의
///////////////////////////////////////////////////////////////////

#define PLAY_LOOP DSBPLAY_LOOPING 
#define PLAY_ONCE 0

typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;
class SoundClass;




/////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////

extern SoundClass* g_pSound;

void InitSound_(HWND);
void CleanupSound_();
bool PlaySound_(eSOUND, DWORD);




////////////////////////////////////////////
// Class name: SoundClass
////////////////////////////////////////////

class SoundClass
{
private:
	struct WaveHeaderType					//총 44byte
	{
		char chunkId[4];					// RIFF : Resource InterChange File Format (Big Endian)
		unsigned long chunkSize;			// 파일크기 - 8 :: sizeof(chunk_ID) -sizeof(chunk_Size).
		char format[4];						// 'WAVE' : 웨이브 파일일 경우 
		char subChunkId[4];					// 'fmt ', fmt' ' 뒤에 공백 한칸. = 4칸. 
		unsigned long subChunkSize;			//  
		unsigned short audioFormat;			// PCM = 1 (2byte) little-Endian
		unsigned short numChannels;			// 체널 수. mono : 1 , Stereo : 2
		unsigned long sampleRate;			// 1초를 몇개의 조각으로 세분화. 
		unsigned long  bytesPerSecond;		// SampleRate * numChannels * BitsPerSample / 8; 소리가 1초 몇 바이트 소모?
		unsigned short blockAlign;			// 전체 채널을 포함하는 한 샘플의 크기
		unsigned short bitsPerSample;		// 한개의 샘플은 몇개의 비트로 나누는가.
		char dataChunkId[4];				// 'data'	//문자열들은 BigEndian
		unsigned long dataSize;				// 실제 PMC 데이터. (BitsPerSample / 8) * NumChannels * 실제 샘플수.
	};

public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	bool Initialize(HWND);
	void CleanUp();
	bool PlayWaveFile(eSOUND track, DWORD dwFlag);
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

	bool LoadWaveFileNameFromINI(const TCHAR* fileName, TCHAR** output);
	bool LoadWaveFile(const TCHAR* szFileName);
	bool LoadWaveFile(const TCHAR*, IDirectSoundBuffer8**);

	void ReleaseWaveFile(IDirectSoundBuffer8**);
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



#endif




///////////////////////////////////////////////////////////////////
//
// 참고: https://psychoria.tistory.com/212?category=556283
//		
///////////////////////////////////////////////////////////////////
