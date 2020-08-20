///////////////////////////////////////////////////////////////////////////////
// Filename: soundclass.cpp
///////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "soundclass.h"
#include <tchar.h>


#define EQUAL(a, b) ( _tcsicmp( (a), (b)) == 0 )
int _ReadLine(FILE* fp, TCHAR* buff)
{
	_tcscpy(buff, L"********************");		//버퍼 클리어. 비교를 위한 임시 코드
	_fgetts(buff, 256, fp);					//읽어들인 데이터 복사

	return S_OK;
}

/////////////////////////////////////////////////
//	GLOBAL
////////////////////////////////////////////////

SoundClass* g_pSound = NULL;

/////////////////////////////////////////////////
//	함수
////////////////////////////////////////////////
BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);



void InitSound_(HWND hWnd)
{
	g_pSound = new SoundClass;
	g_pSound->Initialize(hWnd);

}

void CleanupSound_()
{
	g_pSound->Shutdown();
}

bool PlaySound_(eSOUND num)
{
	if (num - 1 > eSOUND::eSOUND_MAX_)
		return FALSE;

	g_pSound->PlayWaveFile(num);

	return TRUE;
}



////////////////////////////////////////////////
// Member Func Area
///////////////////////////////////////////////

SoundClass::SoundClass()
{
	m_DirectSound = 0;
	m_primaryBuffer = 0;
	//m_Volume = DSBVOLUME_MAX;
	m_Vol = 0;
	//for (int i = 0; i < eSOUND_MAX_; i++)
	//{
	//	m_secondaryBuffer[i] = 0;
	//}
	//memset(m_WaveHeader, 0, sizeof(WaveHeaderType) * eSOUND_MAX_);
}


SoundClass::SoundClass(const SoundClass& other)
{
}


SoundClass::~SoundClass()
{
}


bool SoundClass::Initialize(HWND hwnd)
{
	bool result;


	DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback, (LPVOID)&m_vSoundDevice);

	// Initialize direct sound and the primary sound buffer.
	result = InitializeDirectSound(hwnd);
	if (!result)
	{
		return false;
	}

	LoadWaveFromINI(L"../data/sound.ini");

	/*
	// Load a wave audio file onto a secondary buffer.
	//result = LoadWaveFile(L"../data/sound01.wav", &m_secondaryBuffer[0]);
	result = LoadWaveFile(L"../data/sound01.wav");

	//result = LoadWaveFile()
	if (!result)
	{
		return false;
	}

	//result = LoadWaveFile(L"../data/sound02_10m.wav", &m_secondaryBuffer[1]);
	result = LoadWaveFile(L"../data/sound02_10m.wav");
	if (!result)
	{
		return false;
	}
	*/

	/*
	// Play the wave file now that it has been loaded.
	result = PlayWaveFile();
	if(!result)
	{
		return false;
	}
	*/

	return true;
}


void SoundClass::Shutdown()
{
	// Release the secondary buffer.
	for (int i = 0; i < 2; i++)
	{
		//ShutdownWaveFile(&m_secondaryBuffer[i]);
	}


	// Shutdown the Direct Sound API.
	ShutdownDirectSound();

	return;
}


bool SoundClass::InitializeDirectSound(HWND hwnd)
{
	bool bResult = TRUE;

	bResult = CreateDevice(hwnd);
	if (!bResult)
	{
		MessageBox(NULL, L"FAIL TO CREATE DEVICE", L"ERROR", MB_OK);
		return FALSE;
	}


	bResult = CreateBuffer(m_primaryBuffer);
	if (!bResult)
	{
		MessageBox(NULL, L"FAIL TO CREATE BUFFER01", L"ERROR", MB_OK);
		return FALSE;
	}


	return bResult;
}

bool SoundClass::CreateDevice(HWND hWnd)
{
	HRESULT hr;

	// Initialize the direct sound interface pointer for the default sound device.
	hr = DirectSoundCreate8(NULL, &m_DirectSound, NULL);														//첫번째 인자가 NULL인 경우 디폴트 디바이스
	if (FAILED(hr))
	{
		return FALSE;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.			 https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416818(v=vs.85)
	hr = m_DirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}

bool SoundClass::CreateBuffer(IDirectSoundBuffer*& pSoundBuffer)
{
	HRESULT hr = S_OK;
	DSBUFFERDESC bufferDesc;

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN; //| DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	hr = m_DirectSound->CreateSoundBuffer(&bufferDesc, &pSoundBuffer, NULL);
	if (FAILED(hr))
	{
		hr = pSoundBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)pSoundBuffer);
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	WAVEFORMATEX waveFormat;
	memset(&waveFormat, 0, sizeof(WAVEFORMATEX));
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	hr = pSoundBuffer->SetFormat(&waveFormat);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}


void SoundClass::ShutdownDirectSound()
{
	// Release the primary sound buffer pointer.
	if (m_primaryBuffer)
	{
		m_primaryBuffer->Release();
		m_primaryBuffer = 0;
	}

	// Release the direct sound interface pointer.
	if (m_DirectSound)
	{
		m_DirectSound->Release();
		m_DirectSound = 0;
	}

	return;
}


bool SoundClass::LoadWaveFile(const TCHAR* filename, IDirectSoundBuffer8** secondaryBuffer)
{
	int error;
	FILE* pFile;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* pTempBuffer;
	unsigned char* waveData;
	unsigned char* pBuffer;
	unsigned long bufferSize;


	// Open the wave file in binary.
	error = _wfopen_s(&pFile, filename, L"rb");
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
		return false;
	}

	// Check that the file format is the WAVE format.
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		return false;
	}

	// Check that the sub chunk ID is the fmt format.
	if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		return false;
	}

	// Check that the audio format is WAVE_FORMAT_PCM.
	if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		return false;
	}

	// Check that the wave file was recorded in stereo format.
	if (waveFileHeader.numChannels != 2)
	{
		return false;
	}

	// Check that the wave file was recorded at a sample rate of 44.1 KHz.
	if (waveFileHeader.sampleRate != 44100)
	{
		//return false;
	}

	// Ensure that the wave file was recorded in 16 bit format.
	if (waveFileHeader.bitsPerSample != 16)
	{
		return false;
	}

	// Check for the data chunk header.
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		return false;
	}

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	//waveFormat.nSamplesPerSec = 44100;
	waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &pTempBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = pTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)secondaryBuffer);
	if (FAILED(result))
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
	result = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&pBuffer, (DWORD*)&bufferSize, NULL, 0, DSBLOCK_ENTIREBUFFER);
	if (FAILED(result))
	{
		return false;
	}

	// Copy the wave data into the buffer.
	//memcpy(pBuffer, waveData, waveFileHeader.dataSize);
	memcpy_s(pBuffer, waveFileHeader.dataSize, waveData, waveFileHeader.dataSize);

	// Unlock the secondary buffer after the data has been written to it.
	result = (*secondaryBuffer)->Unlock((void*)pBuffer, bufferSize, NULL, 0);
	if (FAILED(result))
	{
		return false;
	}

	// Release the wave data since it was copied into the secondary buffer.
	delete[] waveData;
	waveData = 0;

	return true;
}


void SoundClass::ShutdownWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	// Release the secondary sound buffer.
	if (*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}

	return;
}


bool SoundClass::PlayWaveFile(eSOUND NUM)
{
	HRESULT hr;

	// Play the contents of the secondary sound buffer.		//Loop Play until STOP
	//m_SoundWaveList[NUM]->PlayOnce();
	m_SoundWaveList[NUM]->PlayLoop();

	return true;
}

bool SoundClass::StopWaveFile(eSOUND NUM)
{
	HRESULT hr = S_OK;

	//hr = m_secondaryBuffer[NUM]->Stop();
	//if (FAILED(hr))
	//	return hr;

	return TRUE;
}

//target에 num만큼 볼륨 증가.
void SoundClass::VolumeUp(eSOUND target, LONG num)
{
	m_SoundWaveList[target]->VolumeUp(num);
	//m_secondaryBuffer[eSOUND::eBGM_STAGE01_]->SetVolume(m_Vol);
}

//target에 num만큼 볼륨 감소.
void SoundClass::VolumeDown(eSOUND target, LONG num)
{
	m_SoundWaveList[target]->VolumeDown(num);
	//m_secondaryBuffer[eSOUND::eBGM_STAGE01_]->SetVolume(m_Vol);
}

void SoundClass::VolumeMute(eSOUND target)
{
	m_SoundWaveList[target]->SetVolume(DSBVOLUME_MIN);
}

void SoundClass::VolumeMax(eSOUND target)
{
	m_SoundWaveList[target]->SetVolume(DSBVOLUME_MAX);
}

bool SoundClass::LoadWaveFromINI(const TCHAR* fileName)
{
	TCHAR path[256] = L"";
	::GetCurrentDirectory(256, path);

	FILE* fp = _tfopen(fileName, L"rt");
	if (fp == NULL) return FALSE;

	TCHAR line[256] = L"";
	TCHAR token[256] = L"";

	int iTotalFileNum = 0;					//리소스 갯수.

	TCHAR FILENAME[20][256] = { L"", };
	DWORD cntFile = 0;

	//유니코드 문자열로 복사시 한글 개짐 방지를 위해 로케일 변경
	setlocale(LC_ALL, "");				//운영체제 기본값

	while (1)
	{
		if (feof(fp))break;

		_tcscpy(token, L"********************");		//버퍼 클리어. 비교를 위한 임시 코드
		_fgetts(line, 256, fp);								//라인의 공백 때문에 fscanf는 쑬 수 없당
		_stscanf(line, L"%s", token);
		//_ReadLine(fp, line);								//일단 한줄 읽고

		if (_tcsnicmp(token, L"//", 2) == 0) continue;		//주석이면 스킵

		if (_tcsnicmp(line, L"\n", 2) == 0) continue;

		if (EQUAL(token, L"*TOTAL_FILE_NUM"))
		{
			_stscanf(line, L"\t *TOTAL_FILE_NUM %d", &iTotalFileNum);		//총 불러들일 웨이브 파일 숫자 표시해둠.
			
			continue;
		}

		if (EQUAL(token, L"*FILENAME"))						//내가 찾던 문자열 발견.
		{
			_stscanf(line, L"%s", token);					//해당값 읽기
			//_tcscpy(FILENAME[cntFile], token);				//복사
			_stscanf(line, _T("\t*FILENAME \"%[^\"]\""), FILENAME[cntFile]);
			cntFile++;
			
			continue;
		}

	}

	return false;
}

bool SoundClass::LoadWaveFile(const TCHAR* szFileName)
{
	static int i = 0;
	//m_SoundWaveList
	CSoundWave* pTempWave = new CSoundWave(m_DirectSound);
	bool bResult = pTempWave->LoadWaveFile(szFileName);
	m_SoundWaveList[i] = (pTempWave);
	i++;

	return bResult;
}



BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext)
{
	auto vSoundDevice = reinterpret_cast<std::vector<DeviceTuple>*>(lpContext);
	std::wcout.imbue(std::locale(""));
	std::wcout << lpcstrDescription << std::endl;

	GUID guid;
	memset(&guid, 0x00, sizeof(guid));
	if (nullptr != lpGuid)
	{
		memcpy(&guid, lpGuid, sizeof(guid));
	}
	vSoundDevice->push_back(std::make_tuple(guid, std::wstring(lpcstrDescription), std::wstring(lpcstrModule)));

	return TRUE;
}