#include "CSoundWave3D.h"

#include <stdio.h>

CSoundWave3D::CSoundWave3D()
{
}

CSoundWave3D::CSoundWave3D(IDirectSound8* pDS)
{
	m_pDS = pDS;
}

CSoundWave3D::~CSoundWave3D()
{
	Cleanup();
}

bool CSoundWave3D::LoadWaveFile(const TCHAR* fileName)
{

	FILE* pf = NULL;
	//int error = fopen_s(&pf, fileName, "rb");
	int error = _wfopen_s(&pf, fileName, L"rb");
	if (error != 0)
	{
		MessageBox(NULL, L"FAIL to Open Wave File", L"ERROR", MB_OK);
		return FALSE;
	}

	WaveHeaderType header;
	UINT count = fread(&header, sizeof(WaveHeaderType), 1, pf);
	if (count != 1)
	{
		//읽어들인 원소 갯수 체크
		return FALSE;
	}

	if (
		header.chunkId[0] != 'R' ||
		header.chunkId[1] != 'I' ||
		header.chunkId[2] != 'F' ||
		header.chunkId[3] != 'F'
		)
	{
		MessageBox(NULL, L"ChunkID가 RIFF가 아닙니다.", L"ERROR", MB_OK);
		return FALSE;
	}

	if (
		header.format[0] != 'W' ||
		header.format[1] != 'A' ||
		header.format[2] != 'V' ||
		header.format[3] != 'E'
		)
	{
		MessageBox(NULL, L"WAVE 파일이 아닙니다.", L"ERROR", MB_OK);
		return FALSE;
	}

	//fmt 포멧 chunk id 체크
	if (
		header.subChunkId[0] != 'f' ||
		header.subChunkId[1] != 'm' ||
		header.subChunkId[2] != 't' ||
		header.subChunkId[3] != ' '
		)
	{
		MessageBox(NULL, L"Check WaveFile subChunID", L"ERROR", MB_OK);
		//return FALSE;
	}

	//PCM 데이터인지 확인.
	if (header.audioFormat != WAVE_FORMAT_PCM)
	{
		MessageBox(NULL, L"Check WaveFile AudioFormat", L"ERROR", MB_OK);
		//return FALSE;
	}

	//웨이브 파일이 모노 형식인지 체크
	if (header.numChannels != 1)
	{
		MessageBox(NULL, L"Check WaveFile MONO FORMAT", L"ERROR", MB_OK);
		//return FALSE;

	}

	//44.1 KHz 샘플속도 인지 체크
	if (header.sampleRate != 44100)
	{
		MessageBox(NULL, L"Check WaveFile samepRate(Hz)", L"ERROR", MB_OK);
		//return FALSE;
	}

	//웨이브 파일이 16비트 형식으로 레코딩 되었는지 확인.
	if (header.bitsPerSample != 16)
	{
		MessageBox(NULL, L"Check WaveFile bitsPerSample(16)", L"ERROR", MB_OK);
		//return FALSE;
	}

	if (
		header.dataChunkId[0] != 'd' ||
		header.dataChunkId[1] != 'a' ||
		header.dataChunkId[2] != 't' ||
		header.dataChunkId[3] != 'a'
		)
	{
		MessageBox(NULL, L"Check WaveFile DataChunkID", L"ERROR", MB_OK);
		//return FALSE;
	}

	WAVEFORMATEX fmt;
	fmt.wFormatTag		= header.audioFormat;
	fmt.nSamplesPerSec	= header.sampleRate;
	fmt.wBitsPerSample	= header.bitsPerSample;
	fmt.nChannels		= header.numChannels;
	fmt.nBlockAlign		= (fmt.wBitsPerSample / 8) * fmt.nChannels;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

	//SecondaryBuffer
	DSBUFFERDESC desc;
	desc.dwSize			 = sizeof(DSBUFFERDESC);
	desc.dwFlags		 = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	desc.dwBufferBytes	 = header.dataSize;
	desc.dwReserved		 = 0;
	desc.lpwfxFormat	 = &fmt;
	desc.guid3DAlgorithm = GUID_NULL;

	m_DESC = desc;

	IDirectSoundBuffer* tempBuffer = nullptr;
	HRESULT hr = m_pDS->CreateSoundBuffer(&desc, &tempBuffer, NULL);
	if (FAILED(hr))
		return FALSE;

	hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_pSecondaryBuffer);
	if (FAILED(hr))
		return FALSE;

	//test
	for (int i = 0; i < 8; i++)
	{
		hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_pSecondaryBuffer2[i]);
		if (FAILED(hr))
			return FALSE;
	}
	
	tempBuffer->Release();
	tempBuffer = NULL;
	
	
	fseek(pf, sizeof(WaveHeaderType), SEEK_SET);

	unsigned char* waveData = new unsigned char[header.dataSize];
	if (!waveData)
	{
		MessageBox(NULL, L"Check the waveData [dataSize]", L"ERROR", MB_OK);
		return FALSE;
	}

	count = fread(waveData, 1, header.dataSize, pf);
	if (count != header.dataSize)
	{
		MessageBox(NULL, L"Check the waveData fread", L"ERROR", MB_OK);
		return FALSE;
	}

	error = fclose(pf);
	if (error != 0)
	{
		MessageBox(NULL, L"Check the waveData fclose", L"ERROR", MB_OK);
		return FALSE;
	}

	unsigned char* pBuffer = nullptr;
	unsigned long bufferSize = 0;

	hr = m_pSecondaryBuffer->Lock(0, header.dataSize, (LPVOID*)&pBuffer, (DWORD*)&bufferSize, NULL, 0, 0);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Lock secondaryBuffer", L"ERROR", NULL);
		return FALSE;
	}

	//memcpy(pBuffer, waveData, header.dataSize);
	memcpy_s(pBuffer, header.dataSize, waveData, header.dataSize);

	hr = m_pSecondaryBuffer->Unlock((LPVOID)pBuffer, bufferSize, NULL, 0);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to UnLock secondaryBuffer", L"ERROR", NULL);
		return FALSE;
	}

	delete[] waveData;
	waveData = NULL;

	hr = m_pSecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&m_pSecondary3DBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Get InterFace", L"ERROR", NULL);
		return FALSE;
	}


	return TRUE;
}

bool CSoundWave3D::SetStartMusicPos(DWORD dwNewPos)
{
	//사운드 버퍼의 시작 부분에 위치를 설정.
	HRESULT hr = m_pSecondaryBuffer->SetCurrentPosition(dwNewPos);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to SetCurrentPosition", L"ERROR", MB_OK);
		return FALSE;
	}
	
	return TRUE;
}

bool CSoundWave3D::SetVolume(LONG vol)
{
	m_Vol += vol;

	//DSBVOLUME_MAX 버퍼 볼륨을 100%
	HRESULT hr = m_pSecondaryBuffer->SetVolume(m_Vol);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to SetVolume", L"ERROR", MB_OK);
		return FALSE;
	}
	return TRUE;
}

void CSoundWave3D::SetPos(float x, float y, float z)
{
	m_pSecondary3DBuffer->SetPosition(x, y, z, DS3D_IMMEDIATE);
	
}

bool CSoundWave3D::Play(DWORD dwFlag)
{
	HRESULT hr = S_OK;
	hr = m_pSecondaryBuffer->Play(0, 0, dwFlag);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Play", L"ERROR", MB_OK);
		return FALSE;
	}

	return TRUE;
}

bool CSoundWave3D::Play(DWORD num, DWORD dwFlag)
{
	static int N = 0;

	HRESULT hr = S_OK;
	hr = m_pSecondaryBuffer2[N]->Play(0, 0, dwFlag);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Play", L"ERROR", MB_OK);
		return FALSE;
	}
	N++;
	N = N % num;

	return TRUE;

}

void CSoundWave3D::Mute()
{
	m_pSecondaryBuffer->SetVolume(DSBVOLUME_MIN);
}


void CSoundWave3D::Cleanup()
{
	if (m_pSecondaryBuffer != NULL)
	{
		m_pSecondaryBuffer->Release();
		m_pSecondaryBuffer = NULL;
	}

	if (m_pSecondary3DBuffer != NULL)
	{
		m_pSecondary3DBuffer->Release();
		m_pSecondary3DBuffer = NULL;
	}

}
