#pragma once

#pragma comment(lib, "dsound.lib")

#include <Windows.h>
#include <dsound.h>


class CSoundWave3D
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
	IDirectSound8* m_pDS;							//device �ܺ� ����.		
	IDirectSoundBuffer8*	m_pSecondaryBuffer;		//Buffer.
	IDirectSoundBuffer8*	m_pSecondaryBuffer2[8];
	IDirectSound3DBuffer8*	m_pSecondary3DBuffer;	//3D

	DWORD	m_dataSize;
	INT		m_CurrentPosition;					//�����ġ	->�ʿ����?
	LONG	m_Vol;								//����

	WaveHeaderType m_WaveHeader;				//Header����.
	DSBUFFERDESC   m_DESC;

public:
	CSoundWave3D();
	CSoundWave3D(IDirectSound8* pDS);
	~CSoundWave3D();

	bool LoadWaveFile(const TCHAR* fileName);

public:
	bool SetStartMusicPos(DWORD dwNewPos);
	bool SetVolume(LONG);
	void SetPos(float x, float y, float z);
	bool Play(DWORD dwFlag);		//�ܼ� ����. ��������� ��. �׽�Ʈ��.(wave ������ ����� �ε��Ǿ��°�)
	bool Play(DWORD num, DWORD dwFlag);
	void Mute();

public:
	void SetDevice(IDirectSound8* pDS) { m_pDS = pDS; };

	DWORD GetDataSize() { return m_dataSize; };
	IDirectSoundBuffer8* GetBuffer() { return m_pSecondaryBuffer; };

private:
	void Cleanup();

};

