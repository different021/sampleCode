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
	g_pSound->CleanUp();
}

bool PlaySound_(eSOUND num, DWORD dwFlag)
{
	if (num - 1 > eSOUND::eSOUND_MAX_)
	{
		MessageBox(NULL, L"Play Sound enum Out of Range", L"ERROR", MB_OK);
		return FALSE;
	}

	g_pSound->PlayWaveFile(num, dwFlag);

	return TRUE;
}

bool StopSound_(eSOUND eTarget, DWORD dwFlag)
{
	g_pSound->PlayWaveFile(eTarget, dwFlag);
	return false;
}

bool OnOffSound_()
{
	static int flag = -1;
	flag *= -1;
	if(flag > 0)
		g_pSound->MasterVolumeMute();
	if (flag < 0)
		g_pSound->MasterVolumeMax();

	return TRUE;
}

void VolumeUp(eSOUND target, DWORD degree)
{
	if (target >= eSOUND_MAX_)
		MessageBox(NULL, L"TARGET SOUND Out of Range", L"ERROR", MB_OK);
	g_pSound->VolumeUp(target, degree);
}

void VolumeDown(eSOUND target, DWORD degree)
{
	if (target > eSOUND_MAX_)
		MessageBox(NULL, L"TARGET SOUND Out of Range", L"ERROR", MB_OK);
	g_pSound->VolumeDown(target, degree);
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

	//장치들을 확인한다. 사실 잘모르지만 일단 사용한다. 콘솔에서는 장치들 리스트를 출력해준다.
	DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback, (LPVOID)&m_vSoundDevice);

	// Initialize direct sound and the primary sound buffer.
	result = InitializeDirectSound(hwnd);
	if (!result)
	{
		return false;
	}
			
	result = LoadWaveFileNameFromINI(FILENAME_INI, &pFileNameList);
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
	m_vSoundDevice.clear();		//clear가 각각의 소멸자를 호출한다고 알려져있다.
	m_SoundWaveList.clear();	// Release the secondary buffer.	세컨더리버퍼는 각각의 웨이브 파일이 갖고있다.

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
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN ; 
	bufferDesc.dwBufferBytes = 0;	//prim버퍼는 0이여야만 한다.														
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

bool SoundClass::PlayWaveFile(eSOUND NUM, DWORD dwFlag)
{
	HRESULT hr;
	m_SoundWaveList[NUM]->Play(dwFlag);
	//m_primaryBuffer->Play(0, 0, 0);

	return true;
}

//NOT IMPLEMENT this part
bool SoundClass::StopWaveFile(eSOUND NUM)
{
	HRESULT hr = S_OK;



	return TRUE;
}

bool SoundClass::CopyToPrim(eSOUND track)
{
	//bool bResult = m_SoundWaveList[track]->CopyToPrim();
	//if (!bResult)
	//{
	//	MessageBox(NULL, L"Fail To Play PrimaryBuffer", L"ERORR : SOUND", MB_OK);
	//	return FALSE;
	//}

	DWORD dataSize =m_SoundWaveList[track]->GetDataSize();
	LPDIRECTSOUNDBUFFER buf = m_SoundWaveList[track]->GetBuffer();

	//Lock the prime buffer to write wave data into it.
	HRESULT	hr = (m_primaryBuffer)->Lock(0, dataSize, (void**)&buf, (DWORD*)&dataSize, NULL, 0, DSBLOCK_ENTIREBUFFER);
	
	if (FAILED(hr))
		return false;

	CopyMemory(m_primaryBuffer, buf, dataSize);				//걍써봄. 걍 memcpy쓸까한다.

	// UnLock the prime buffer
	hr = (m_primaryBuffer)->Unlock(m_primaryBuffer, dataSize, NULL, 0);
	if (FAILED(hr))
		return false;



	return TRUE;
}

bool SoundClass::PlayPrime()
{
	HRESULT hr = m_primaryBuffer->Play(0, 0, 0);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Play Prim Buffer", L"ERROR!", MB_OK);
		return FALSE;
	}

	return TRUE;
}

//target에 num만큼 볼륨 증가.
void SoundClass::VolumeUp(eSOUND target, LONG num)
{
	m_SoundWaveList[target]->VolumeUp(num);
}

//target에 num만큼 볼륨 감소.
void SoundClass::VolumeDown(eSOUND target, LONG num)
{
	m_SoundWaveList[target]->VolumeDown(num);
}

void SoundClass::MasterVolumeMute()
{
	m_primaryBuffer->SetVolume(DSBVOLUME_MIN);
}

void SoundClass::MasterVolumeMax()
{
	m_primaryBuffer->SetVolume(DSBVOLUME_MAX);
}

bool SoundClass::LoadWaveFileNameFromINI(const TCHAR* fileName, TCHAR** output)
{
	FILE* fp = _tfopen(fileName, L"rt");
	if (fp == NULL) return FALSE;

	TCHAR line[256] = L"";					//한줄한줄 읽어오자
	TCHAR token[256] = L"";					//줄 앞 키워드를 읽어오자.

	int iTotalFileNum = 0;					//리소스 갯수.
	TCHAR* pFileName = NULL;

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
			pFileName = new TCHAR[256 * iTotalFileNum];
			continue;
		}

		if (EQUAL(token, L"*FILENAME"))						//내가 찾던 문자열 발견.
		{
			_stscanf(line, L"%s", token);					//해당값 읽기
			//_tcscpy(FILENAME[cntFile], token);			//복사
			_stscanf(line, _T("\t*FILENAME \"%[^\"]\""), pFileName + cntFile * 256);
			cntFile++;

			continue;
		}
	}

	//두 숫자가 틀리면 뭔가 잘못됐다는 킹리적갓심.
	if (cntFile != iTotalFileNum)
	{
		MessageBox(NULL, L"리소스 파일 갯수가 맞지않습니다. \n[sound.ini]파일의 TOTAL_FILE_NUM 숫자와 리소스 파일 갯수를 확인하세요.", L"ERROR", MB_OK);
		return FALSE;
	}

	if (eSOUND::eSOUND_MAX_ != iTotalFileNum)
	{
		MessageBox(NULL, L"LOAD한 리소스 숫자와 Header의 enum수가 맞지않습니다.", L"ERROR", MB_OK);
		return FALSE;
	}

	//성공시 리턴.
	m_iNumberOfFile = iTotalFileNum;
	*output = pFileName;

	return TRUE;
}

bool SoundClass::LoadWaveFile(const TCHAR* szFileName)
{
	CSoundWave* pTempWave = new CSoundWave(m_DirectSound);
	pTempWave->SetPrimBuffer(m_primaryBuffer);
	bool bResult = pTempWave->LoadWaveFile(szFileName);
	m_SoundWaveList.push_back(pTempWave);							//벡터 클래스에 추가.

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