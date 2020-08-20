///////////////////////////////////////////////////////////////////////////////
// Filename: soundclass.cpp
///////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "soundclass.h"
#include <tchar.h>


#define EQUAL(a, b) ( _tcsicmp( (a), (b)) == 0 )
int _ReadLine(FILE* fp, TCHAR* buff)
{
	_tcscpy(buff, L"********************");		//���� Ŭ����. �񱳸� ���� �ӽ� �ڵ�
	_fgetts(buff, 256, fp);					//�о���� ������ ����

	return S_OK;
}

/////////////////////////////////////////////////
//	GLOBAL
////////////////////////////////////////////////

SoundClass* g_pSound = NULL;

/////////////////////////////////////////////////
//	�Լ�
////////////////////////////////////////////////
BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);



void InitSound_(HWND hWnd)
{
	g_pSound = new SoundClass;
	g_pSound->Initialize(hWnd);

}

void CleanupSound_()
{
	g_pSound->CleanUp();
}

bool PlaySound_(eSOUND num, DWORD dwFlag)
{
	if (num - 1 > eSOUND::eSOUND_MAX_)
		return FALSE;

	g_pSound->PlayWaveFile(num, dwFlag);

	return TRUE;
}



////////////////////////////////////////////////
// Member Func Area
///////////////////////////////////////////////

SoundClass::SoundClass()
{
	m_DirectSound = 0;
	m_primaryBuffer = 0;
	m_Vol = 0;
	pFileNameList = NULL;
}


SoundClass::SoundClass(const SoundClass& other)
{
}


SoundClass::~SoundClass()
{
	CleanUp();
}


bool SoundClass::Initialize(HWND hwnd)
{
	bool result;

	//��ġ���� Ȯ���Ѵ�. ��� �߸����� �ϴ� ����Ѵ�. �ֿܼ����� ��ġ�� ����Ʈ�� ������ش�.
	DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback, (LPVOID)&m_vSoundDevice);

	// Initialize direct sound and the primary sound buffer.
	result = InitializeDirectSound(hwnd);
	if (!result)
	{
		return false;
	}

	result = LoadWaveFileNameFromINI(L"../sound/sound.ini", &pFileNameList);
	if (!result)
	{
		return false;
	}

	TCHAR temp[256] = L"";


	for (int i = 0; i < m_iNumberOfFile; i++)
	{
		_tcscpy(temp, pFileNameList + 256 * i);
		result = LoadWaveFile(temp);
	}

	return true;
}


void SoundClass::CleanUp()
{
	m_vSoundDevice.clear();		//clear�� ������ �Ҹ��ڸ� ȣ���Ѵٰ� �˷����ִ�.
	m_SoundWaveList.clear();	// Release the secondary buffer.	�����������۴� ������ ���̺� ������ �����ִ�.

	// Shutdown the Direct Sound API.	primBuf, DS.
	ReleaseDirectSound();
	delete[] pFileNameList;

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

	m_SoundWaveList.reserve(32);

	return bResult;
}

bool SoundClass::CreateDevice(HWND hWnd)
{
	HRESULT hr;

	// Initialize the direct sound interface pointer for the default sound device.
	hr = DirectSoundCreate8(NULL, &m_DirectSound, NULL);														//ù��° ���ڰ� NULL�� ��� ����Ʈ ����̽�
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




bool SoundClass::PlayWaveFile(eSOUND NUM, DWORD dwFlag)
{
	HRESULT hr;

	m_SoundWaveList[NUM]->Play(dwFlag);

	return true;
}

bool SoundClass::StopWaveFile(eSOUND NUM)
{
	HRESULT hr = S_OK;



	return TRUE;
}

//target�� num��ŭ ���� ����.
void SoundClass::VolumeUp(eSOUND target, LONG num)
{
	m_SoundWaveList[target]->VolumeUp(num);
	//m_secondaryBuffer[eSOUND::eBGM_STAGE01_]->SetVolume(m_Vol);
}

//target�� num��ŭ ���� ����.
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

bool SoundClass::LoadWaveFileNameFromINI(const TCHAR* fileName, TCHAR** output)
{
	FILE* fp = _tfopen(fileName, L"rt");
	if (fp == NULL) return FALSE;

	TCHAR line[256] = L"";					//�������� �о����
	TCHAR token[256] = L"";					//�� �� Ű���带 �о����.

	int iTotalFileNum = 0;					//���ҽ� ����.
	TCHAR* pFileName = NULL;

	DWORD cntFile = 0;

	//�����ڵ� ���ڿ��� ����� �ѱ� ���� ������ ���� ������ ����
	setlocale(LC_ALL, "");				//�ü�� �⺻��

	while (1)
	{
		if (feof(fp))break;

		_tcscpy(token, L"********************");		//���� Ŭ����. �񱳸� ���� �ӽ� �ڵ�
		_fgetts(line, 256, fp);								//������ ���� ������ fscanf�� �� �� ����
		_stscanf(line, L"%s", token);
		//_ReadLine(fp, line);								//�ϴ� ���� �а�

		if (_tcsnicmp(token, L"//", 2) == 0) continue;		//�ּ��̸� ��ŵ

		if (_tcsnicmp(line, L"\n", 2) == 0) continue;

		if (EQUAL(token, L"*TOTAL_FILE_NUM"))
		{
			_stscanf(line, L"\t *TOTAL_FILE_NUM %d", &iTotalFileNum);		//�� �ҷ����� ���̺� ���� ���� ǥ���ص�.
			pFileName = new TCHAR[256 * iTotalFileNum];
			continue;
		}

		if (EQUAL(token, L"*FILENAME"))						//���� ã�� ���ڿ� �߰�.
		{
			_stscanf(line, L"%s", token);					//�ش簪 �б�
			//_tcscpy(FILENAME[cntFile], token);			//����
			_stscanf(line, _T("\t*FILENAME \"%[^\"]\""), pFileName + cntFile * 256);
			cntFile++;
			
			continue;
		}
	}

	//�� ���ڰ� Ʋ���� ���� �߸��ƴٴ� ŷ��������.
	if (cntFile != iTotalFileNum)
	{
		MessageBox(NULL, L"���ҽ� ���� ������ �����ʽ��ϴ�. \n[sound.ini]������ TOTAL_FILE_NUM ���ڿ� ���ҽ� ���� ������ Ȯ���ϼ���.", L"ERROR", MB_OK);
		return FALSE;
	}

	if (eSOUND::eSOUND_MAX_ != iTotalFileNum)
	{
		MessageBox(NULL, L"LOAD�� ���ҽ� ���ڿ� Header�� enum���� �����ʽ��ϴ�.", L"ERROR", MB_OK);
		return FALSE;
	}

	//������ ����.
	m_iNumberOfFile = iTotalFileNum;
	*output = pFileName;
	
	return TRUE;
}

bool SoundClass::LoadWaveFile(const TCHAR* szFileName)
{
	CSoundWave* pTempWave = new CSoundWave(m_DirectSound);
	bool bResult = pTempWave->LoadWaveFile(szFileName);
	m_SoundWaveList.push_back(pTempWave);							//���� Ŭ������ �߰�.

	return bResult;
}


void SoundClass::ReleaseWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	// Release the secondary sound buffer.
	if (*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}

	return;
}

void SoundClass::ReleaseDirectSound()
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