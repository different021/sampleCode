#include "CSoundWave.h"
#include <stdio.h>
#include <iostream>

#include <vector>



CSoundWave::CSoundWave()
{
	m_DSDevice = 0;					//device		
	m_CurrentPosition = 0;			//Àç»ýÀ§Ä¡
	m_Vol = DSBVOLUME_MAX;		//º¼·ý
	m_pSecondaryBuffer = 0;			//Buffer.

	memset(&m_WaveHeader, 0, sizeof(WaveHeaderType) );			//HeaderÁ¤º¸.
	
}

CSoundWave::CSoundWave(IDirectSound8* pDSDevice)
{
	m_DSDevice = 0;					//device		
	m_CurrentPosition = 0;			//Àç»ýÀ§Ä¡
	m_Vol = DSBVOLUME_MAX;		//º¼·ý
	m_pSecondaryBuffer = 0;			//Buffer.
	SetDevice(pDSDevice);
}

CSoundWave::~CSoundWave()
{
	Cleanup();
}



bool CSoundWave::LoadWaveFile(const TCHAR* fileName)
{
	int error;
	FILE* pFile;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	IDirectSoundBuffer* pTempBuffer;
	unsigned char* waveData;
	unsigned char* pBuffer;
	unsigned long  bufferSize;


	// Open the wave file in binary.
	error = _wfopen_s(&pFile, fileName, L"rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the wave file header.
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, pFile);
	if (count != 1)
	{
		return false;
	}


	// Check that the chunk ID is the RIFF format.
	if ((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		MessageBox(NULL, L"RIFF Çü½ÄÀÌ ¾Æ´Õ´Ï´Ù.", L"ERROR!", MB_OK);
		return false;
	}

	// Check that the file format is the WAVE format.
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		MessageBox(NULL, L"Wave ÆÄÀÏÀÌ ¾Æ´Õ´Ï´Ù.", L"ERROR!", MB_OK);
		return false;
	}

	// Check for the data chunk header.
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		MessageBox(NULL, L"dataChunkId°¡ data°¡ ¾Æ´Õ´Ï´Ù.", L"ERROR!", MB_OK);
		return false;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec	= waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample	= waveFileHeader.bitsPerSample;
	waveFormat.nChannels		= waveFileHeader.numChannels;
	waveFormat.nBlockAlign		= (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec	= waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize			= 0;
	
	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize			= sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags			= DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	bufferDesc.dwBufferBytes	= waveFileHeader.dataSize;
	bufferDesc.dwReserved		= 0;
	bufferDesc.lpwfxFormat		= &waveFormat;
	bufferDesc.guid3DAlgorithm	= GUID_NULL;

	HRESULT hr = S_OK;
	hr = m_DSDevice->CreateSoundBuffer(&bufferDesc, &pTempBuffer, NULL);
	if (FAILED(hr))
	{
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	hr = pTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_pSecondaryBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	// Release the temporary buffer.
	pTempBuffer->Release();
	pTempBuffer = 0;

	// Move to the beginning of the wave data which starts at the end of the data chunk header.
	fseek(pFile, sizeof(WaveHeaderType), SEEK_SET);

	// Create a temporary buffer to hold the wave file data.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if (!waveData)
	{
		return false;
	}

	// Read in the wave file data into the newly created buffer.
	count = fread(waveData, 1, waveFileHeader.dataSize, pFile);
	if (count != waveFileHeader.dataSize)
	{
		return false;
	}

	// Close the file once done reading.
	error = fclose(pFile);
	if (error != 0)
	{
		return false;
	}

	// Lock the secondary buffer to write wave data into it.
	hr = (m_pSecondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&pBuffer, (DWORD*)&bufferSize, NULL, 0, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hr))
	{
		return false;
	}

	// Copy the wave data into the buffer.
	//memcpy(pBuffer, waveData, waveFileHeader.dataSize);
	memcpy_s(pBuffer, waveFileHeader.dataSize, waveData, waveFileHeader.dataSize);

	// Unlock the secondary buffer after the data has been written to it.
	hr = (m_pSecondaryBuffer)->Unlock((void*)pBuffer, bufferSize, NULL, 0);
	if (FAILED(hr))
	{
		return false;
	}

	m_dataSize = waveFileHeader.dataSize;

	return TRUE;
}

bool CSoundWave::Play(DWORD dwFlag)
{
	// Set position at the beginning of the sound buffer.
	m_pSecondaryBuffer->SetCurrentPosition(0);
	// Set volume of the buffer to 100%.
	m_pSecondaryBuffer->SetVolume(m_Vol);
	m_pSecondaryBuffer->Play(0, 0, dwFlag);
	
	return TRUE;
}

bool CSoundWave::CopyToPrim()
{
	HRESULT hr = S_OK;
	LPDIRECTSOUNDBUFFER dsb;
	// Lock the prime buffer to write wave data into it.
	//hr = (m_pPrimaryBuffer)->Lock(0, m_dataSize, (void**)&m_pPrimaryBuffer, (DWORD*)&m_dataSize, NULL, 0, DSBLOCK_ENTIREBUFFER);
	//if (FAILED(hr))
	//	return false;

	//CopyMemory(m_pPrimaryBuffer, m_pSecondaryBuffer, m_dataSize);				//°Á½áº½. °Á memcpy¾µ±îÇÑ´Ù.

	//// UnLock the prime buffer
	//hr = (m_pPrimaryBuffer)->Unlock(m_pPrimaryBuffer, m_dataSize, NULL, 0);
	//if (FAILED(hr))
	//	return false;


	return true;
}

void CSoundWave::VolumeUp(LONG num)
{
	m_Vol += num;
	m_Vol = min(m_Vol, DSBVOLUME_MAX);
	m_pSecondaryBuffer->SetVolume(m_Vol);
}

void CSoundWave::VolumeDown(LONG num)
{
	m_Vol -= num;
	m_Vol = max(m_Vol, DSBVOLUME_MIN);
	m_pSecondaryBuffer->SetVolume(m_Vol);
}

void CSoundWave::Cleanup()
{
	m_DSDevice = 0;
	m_pSecondaryBuffer->Release();
}
