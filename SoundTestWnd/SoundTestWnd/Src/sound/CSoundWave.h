#pragma once

#pragma comment(lib, "dsound.lib")

#include <Windows.h>
#include <dsound.h>

//기

class CSoundWave
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


private:
	IDirectSound8*		 m_DSDevice;			//device		
	IDirectSoundBuffer*  m_pPrimaryBuffer;		//디바이스와 함께 얻어온다. 이 버퍼에 쓸것.
	IDirectSoundBuffer8* m_pSecondaryBuffer;	//Buffer.

	DWORD	m_dataSize;
	INT		m_CurrentPosition;					//재생위치	->필요없나?
	LONG	m_Vol;								//볼륨

	WaveHeaderType m_WaveHeader;				//Header정보.

public:
	CSoundWave();
	CSoundWave(IDirectSound8* pDSDevice);
	~CSoundWave();
	
	
	bool LoadWaveFile(const TCHAR* fileName);

public:
	bool MakeDuple();
	bool Play(DWORD dwFlag);		//단순 실행. 사용하지말 것. 테스트용.(wave 파일이 제대로 로딩되었는가)

public:
	//
	void SetDevice(IDirectSound8* pDSDevice) { m_DSDevice = pDSDevice; };
	void SetPrimBuffer(IDirectSoundBuffer* pPrimBuf) { m_pPrimaryBuffer = pPrimBuf; };
	void VolumeUp(LONG num);
	void VolumeDown(LONG num);
	void SetVolume(LONG num) { m_Vol = num; };
	DWORD GetDataSize() { return m_dataSize; };
	IDirectSoundBuffer8* GetBuffer() { return m_pSecondaryBuffer; };

private:
	void Cleanup();

};

