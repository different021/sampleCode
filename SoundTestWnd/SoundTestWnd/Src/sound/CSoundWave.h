#pragma once

#pragma comment(lib, "dsound.lib")

#include <Windows.h>
#include <dsound.h>

//��

class CSoundWave
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


private:
	IDirectSound8* m_DSDevice;				//device		
	INT m_CurrentPosition;					//�����ġ
	LONG m_Vol;								//����

	WaveHeaderType m_WaveHeader;			//Header����.
	IDirectSoundBuffer8* m_pSecondaryBuffer; //Buffer.

public:
	CSoundWave();
	CSoundWave(IDirectSound8* pDSDevice);
	~CSoundWave();
	
	
	bool LoadWaveFile(const TCHAR* fileName);

public:
	//Play
	bool Play(DWORD dwFlag);

public:
	//
	void SetDevice(IDirectSound8* pDSDevice) { m_DSDevice = pDSDevice; };
	void VolumeUp(LONG num);
	void VolumeDown(LONG num);
	void SetVolume(LONG num) { m_Vol = num; };

private:
	void Cleanup();

};

