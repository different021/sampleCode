#include "CSound.h"

CSound* g_pSound = NULL;

//mono stero만 가능.
int ReadWaveFile(LPTSTR szFileName)
{
	if (nullptr == szFileName)
		return -1;

	//WAVE File Open
	HMMIO hmmio = nullptr;
	hmmio = mmioOpen(szFileName, nullptr, MMIO_READ | MMIO_ALLOCBUF);
	if (nullptr == hmmio)
		return -2;

	//Find RIFF Chunk
	MMCKINFO mmckParent;
	memset(&mmckParent, 0x00, sizeof(mmckParent));
	mmckParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	HRESULT mmRes = mmioDescend(hmmio, &mmckParent, nullptr, MMIO_FINDRIFF);
	if (MMSYSERR_NOERROR != mmRes)
	{
		mmioClose(hmmio, 0);
		return -3;
	}

	//Find Format Chunk
	MMCKINFO mmckChild;
	memset(&mmckChild, 0x00, sizeof(mmckChild));
	mmckChild.fccType = mmioFOURCC('f', 'm', 't', ' ');

	mmRes = mmioDescend(hmmio, &mmckChild, &mmckParent, MMIO_FINDCHUNK);
	if (MMSYSERR_NOERROR != mmRes)
	{
		mmioClose(hmmio, 0);
		return -4;
	}

	WAVEFORMATEX wfex;
	memset(&wfex, 0x00, sizeof(wfex));
	mmioRead(hmmio, (HPSTR)&wfex, mmckChild.cksize);

	OutputDebugWAVEFORMATEX(wfex);

	//std::cout << "wFormatTag      : " << wfex.wFormatTag << std::endl;
	//std::cout << "nChannels       : " << wfex.nChannels << std::endl;
	//std::cout << "nSamplesPerSec  : " << wfex.nSamplesPerSec << std::endl;
	//std::cout << "wBitsPerSample  : " << wfex.wBitsPerSample << std::endl;
	//std::cout << "nBlockAlign     : " << wfex.nBlockAlign << std::endl;
	//std::cout << "nAvgBytesPerSec : " << wfex.nAvgBytesPerSec << std::endl;

	//Find Data Chunk
	mmioAscend(hmmio, &mmckChild, 0);

	mmckChild.ckid = mmioFOURCC('d', 'a', 't', 'a');

	mmRes = mmioDescend(hmmio, &mmckChild, &mmckParent, MMIO_FINDCHUNK);
	if (MMSYSERR_NOERROR != mmRes)
	{
		mmioClose(hmmio, 0);
		return -5;
	}

	DWORD dwDataSize = mmckChild.cksize;
	std::cout << "Data Size       : " << dwDataSize << std::endl;

	//Read Wave Data
	char* pData = nullptr;
	try
	{
		pData = new char[dwDataSize];
	}
	catch (std::bad_alloc e)
	{
		mmioClose(hmmio, 0);
		return -6;
	}

	int err = mmioRead(hmmio, (HPSTR)pData, dwDataSize);
	if (err == -1)
	{
		mmioClose(hmmio, 0);
		delete[] pData;
		return -7;
	}

	delete[] pData;

	mmioClose(hmmio, 0);

	return 0;

}





BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext)
{
	auto vSoundDevice = reinterpret_cast<std::vector<DeviceTuple>*>(lpContext);

#ifdef DEBUG
	TCHAR str[256] = L"";
	wsprintf(str, lpcstrDescription);
	OutputDebugString(str);
	OutputDebugString(L"\n");
#endif // DEBUG

	GUID guid;
	memset(&guid, 0x00, sizeof(guid));
	if (nullptr != lpGuid)
	{
		memcpy(&guid, lpGuid, sizeof(guid));
	}
	vSoundDevice->push_back(std::make_tuple(guid, std::wstring(lpcstrDescription), std::wstring(lpcstrModule)));

	return TRUE;
}

HRESULT CreateSoundBuffer(LPDIRECTSOUND8 lpDS, LPDIRECTSOUNDBUFFER8* ppDsb8)
{
	WAVEFORMATEX wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels		= 1;
	wfx.wBitsPerSample	= 16;
	wfx.cbSize			= 0;
	wfx.nSamplesPerSec	= 16000;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * (wfx.wBitsPerSample / 8);
	wfx.nBlockAlign		= (wfx.wBitsPerSample / 8) * wfx.nChannels;

	DSBUFFERDESC dc;
	memset(&dc, 0, sizeof(dc));
	dc.dwSize		 = sizeof(DSBUFFERDESC);
	dc.dwFlags		 = DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	dc.dwBufferBytes = 3 * wfx.nAvgBytesPerSec;
	dc.lpwfxFormat	 = &wfx;

	HRESULT hr = S_OK;
	LPDIRECTSOUNDBUFFER pDsb = NULL;
	hr = lpDS->CreateSoundBuffer(&dc, &pDsb, NULL);
	if (SUCCEEDED(hr))
	{
		hr = pDsb->QueryInterface(IID_IDirectSoundBuffer8, (void**)ppDsb8);
		pDsb->Release();
	}

	return hr;
}

int WavePlay(LPDIRECTSOUNDBUFFER8 pDsb8)
{
	LPVOID lpWrite;
	DWORD dwLength;

	LONG result = DS_OK;
	//Get Lock
	result = pDsb8->Lock(0, 0, &lpWrite, &dwLength, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (DS_OK != result)
	{
		MessageBox(NULL, L"FAIL to Lock a SecondaryBuffer", L"ERROR", MB_OK);
		return -1;
	}

	//Copy PCM Data
	///memcpy_s(lpWrite, dwLength, PCM데이터 주소, dwLength);

	//Release Lock
	pDsb8->Unlock(lpWrite, dwLength, NULL, 0);
	
	//Set Position to 0
	pDsb8->SetCurrentPosition(0);

	//Play
	result = pDsb8->Play(0, 0, 0);
	if (FAILED(result))
	{
		MessageBox(NULL, L"FAIL to Play SecondaryBuffer", L"ERROR", MB_OK);
		return -2;
	}

	return 0;
}








//제대로된 포맷인지 궁굼할때. 물론 출력창에 뜬다.
void OutputDebugWAVEFORMATEX(WAVEFORMATEX wfex)
{
	TCHAR str[256] = L"";
	wsprintf(str, L"wFormatTag      : %d\n", wfex.wFormatTag);
	OutputDebugString(str);

	wsprintf(str, L"nChannels       : %d\n", wfex.nChannels);
	OutputDebugString(str);

	wsprintf(str, L"nSamplesPerSec  : %d\n", wfex.nSamplesPerSec);
	OutputDebugString(str);

	wsprintf(str, L"wBitsPerSample  : %d\n", wfex.wBitsPerSample);
	OutputDebugString(str);

	wsprintf(str, L"nBlockAlign     : %d\n", wfex.nBlockAlign);
	OutputDebugString(str);

	wsprintf(str, L"nAvgBytesPerSec : %d\n", wfex.nAvgBytesPerSec);
	OutputDebugString(str);
}




CSound::CSound()
{
}

CSound::~CSound()
{
	CleanUp();
}

BOOL CSound::Init(HWND hWnd)
{
	HRESULT hr = S_OK;
	hr = DirectSoundCreate8(NULL, &m_pDS, NULL);
	if (FAILED(hr))
	{	
		MessageBox(NULL, L"FAIL to Create Direct Sound", L"ERROR", MB_OK);
		return FALSE;
	}

	//DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_WRITEPRIMARY     //Primary Buffer 제어 관련
	hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Set Coop Level", L"ERROR", MB_OK);
		return FALSE;
	}


	return TRUE;
}

void CSound::CleanUp()
{
	if (m_pDS != NULL)
	{
		m_pDS->Release();
		m_pDS = NULL;
	}
}




