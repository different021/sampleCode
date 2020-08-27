#include "CSound.h"

//CSound* g_pSound = NULL;



CSound::CSound() : m_pDirectSound(nullptr), m_pDSoundBuffer(nullptr), m_hCallbackThread(nullptr), m_hCallbackEvent(nullptr), m_bThreadEnd(FALSE),
m_nLength(0), m_RawData(), m_hCallbackWnd(nullptr), m_nMsgID(0)
{
	memset(&m_wfx, 0x00, sizeof(m_wfx));
}

CSound::~CSound()
{
	if (nullptr != m_pDSoundBuffer)
	{
		m_pDSoundBuffer->Stop();
	}
	if (nullptr != m_pDirectSound)
	{
		m_bThreadEnd = TRUE;
		SetEvent(m_hCallbackEvent);
		WaitForSingleObject(m_hCallbackThread, 5000);
		CloseHandle(m_hCallbackEvent);
		CloseHandle(m_hCallbackThread);
	}

	//TRACE(_T("[PGLOG] DSound Destructor Called. \n"));
}

int CSound::CreateDSound(HWND hWnd, LPCGUID deviceID)
{
	if (nullptr != m_pDirectSound)
	{
		MessageBox(NULL, L" [ERROR] Create DS ", L"ERROR", MB_OK);
		return -1;
	}

	if (nullptr == hWnd)
	{
		MessageBox(NULL, L" [ERROR] hWnd = NULL ", L"ERROR", MB_OK);
		return -2;
	}

	HRESULT hr = DirectSoundCreate8(deviceID, &m_pDirectSound, nullptr);
	if (FAILED(hr))
	{
		MessageBox(NULL, L" [ERROR] FAIL to Create ", L"ERROR", MB_OK);
		return -3;
	}

	m_pDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

	m_hCallbackEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hCallbackThread = (HANDLE)_beginthreadex(nullptr, 0, CSound::SoundCallback, this, 0, 0);
	return 0;
}

int CSound::ReadWaveFile(LPTSTR szFileName)
{
	if (nullptr == szFileName)
	{
		return -1;
	}

	HMMIO hmmio = mmioOpen(szFileName, nullptr, MMIO_ALLOCBUF | MMIO_READ);
	if (nullptr == hmmio)
	{
		return -2;
	}

	MMCKINFO mmckParent;
	mmckParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	
	MMRESULT mmResult =	mmioDescend(hmmio, &mmckParent, nullptr, MMIO_FINDRIFF);
	if (MMSYSERR_NOERROR != mmResult)
	{
		mmioClose(hmmio, 0);
		return -3;
	}

	MMCKINFO mmckChild;
	mmckChild.ckid = mmioFOURCC('f', 'm', 't', ' ');

	mmResult = mmioDescend(hmmio, &mmckChild, &mmckParent, MMIO_FINDCHUNK);
	if (MMSYSERR_NOERROR != mmResult)
	{
		mmioClose(hmmio, 0);
		return -3;

	}

	LONG nFormatLen = mmckChild.cksize;
	if (mmioRead(hmmio, (HPSTR)&m_wfx, nFormatLen) != nFormatLen)
	{
		mmioClose(hmmio, 0);
		return -4;
	}

	mmResult = mmioAscend(hmmio, &mmckChild, 0);
	if (MMSYSERR_NOERROR != mmResult)
	{
		mmioClose(hmmio, 0);
		return -5;
	}

	mmckChild.ckid = mmioFOURCC('d', 'a', 't', 'a');

	mmResult = mmioDescend(hmmio, &mmckChild, &mmckParent, MMIO_FINDCHUNK);
	if (MMSYSERR_NOERROR != mmResult)
	{
		mmioClose(hmmio, 0);
		return -3;
	}

	m_nLength = mmckChild.cksize;

	char* pTemp = nullptr;
	pTemp = new char[m_nLength];

	//try
	//{
	//	pTemp = new char[m_nLength];
	//}
	//catch (CMemoryException* /*e*/)
	//{
	//}
	//catch (CException* /*e*/)
	//{
	//}

	if (nullptr == pTemp)
	{
		mmioClose(hmmio, 0);
		return -6;
	}

	if (m_nLength != mmioRead(hmmio, (HPSTR)pTemp, m_nLength))
	{
		delete[] pTemp;
		mmioClose(hmmio, 0);
		return -4;
	}

	m_RawData.reset(pTemp, ArrayDeleter());

	return 0;
}

int CSound::PlayTheSound(HWND hWnd, UINT nMsgID, LPTSTR szWaveFile)
{
	
	if (nullptr == m_pDirectSound)
	{
		MessageBox(NULL, L"PLAY ERROR", L"ERROR", MB_OK);
		return -1;
	}

	if (0 != ReadWaveFile(szWaveFile))
	{
		MessageBox(NULL, L"[ERROR : PLAY]\n ReadWaveFile", L"ERROR", MB_OK);
		return -2;
	}

	HRESULT hr = CreateSoundBuffer();
	if (FAILED(hr))
	{
		//TRACE(_T("[PGLOG] PlayTheSound CreateSoundBuffer Error. \n"));
		return -3;
	}

	hr = SetNotification();
	if (FAILED(hr))
	{
		//TRACE(_T("[PGLOG] PlayTheSound SetNotification Error. \n"));
		return -4;
	}

	m_hCallbackWnd = hWnd;
	m_nMsgID = nMsgID;

	m_pDSoundBuffer->SetCurrentPosition(0);
	m_pDSoundBuffer->Play(0, 0, 0);

	return 0;
}

void CSound::StopTheSound()
{
	if (nullptr != m_pDSoundBuffer)
	{
		if (FAILED(m_pDSoundBuffer->Stop()))
		{
		}
	}
}

HRESULT CSound::CreateSoundBuffer()
{
	DSBUFFERDESC dc;
	ZeroMemory(&dc, sizeof(DSBUFFERDESC));
	dc.dwSize = sizeof(DSBUFFERDESC);

	DWORD dwFlags = DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;

	dc.dwFlags = dwFlags;
	dc.dwBufferBytes = m_nLength;
	dc.lpwfxFormat = &m_wfx;

	m_pDSoundBuffer.Release();

	CComPtr<IDirectSoundBuffer> pBuffer;
	HRESULT hr = m_pDirectSound->CreateSoundBuffer(&dc, &pBuffer, nullptr);
	if (FAILED(hr))
	{
		return -3;
	}

	GUID IID_IDSoundBuffer = { 0x6825a449, 0x7524, 0x4d82, 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e };
	pBuffer->QueryInterface(IID_IDSoundBuffer, (LPVOID*)&m_pDSoundBuffer);
	pBuffer.Release();

	void* write1 = 0;
	void* write2 = 0;

	unsigned long length1 = 0, length2 = 0;

	hr = m_pDSoundBuffer->Lock(0, m_nLength, &write1, &length1, &write2, &length2, 0);
	if (FAILED(hr))
	{
		return -4;
	}

	if (length1 > 0)
		memcpy_s(write1, length1, m_RawData.get(), length1);

	if (length2 > 0)
		memcpy_s(write2, length2, m_RawData.get() + length1, length2);

	hr = m_pDSoundBuffer->Unlock(write1, length1, write2, length2);
	if (FAILED(hr))
	{
		return -5;
	}

	return hr;
}

HRESULT CSound::SetNotification()
{
	GUID IID_IDSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 };

	CComPtr<IDirectSoundNotify8> lpDsNotify;
	DSBPOSITIONNOTIFY PositionNotify;

	HRESULT hr = m_pDSoundBuffer->QueryInterface(IID_IDSoundNotify, (void**)&lpDsNotify);
	if (SUCCEEDED(hr))
	{
		PositionNotify.dwOffset = DSBPN_OFFSETSTOP;
		PositionNotify.hEventNotify = m_hCallbackEvent;
		hr = lpDsNotify->SetNotificationPositions(1, &PositionNotify);
		lpDsNotify.Release();
	}

	return hr;
}

UINT __stdcall CSound::SoundCallback(LPVOID lpParam)
{
	CSound* pParam = (CSound*)lpParam;

	while (TRUE != pParam->m_bThreadEnd)
	{
		DWORD dwWait = ::WaitForSingleObject(pParam->m_hCallbackEvent, INFINITE);
		if (WAIT_OBJECT_0 == dwWait)
		{
			if (TRUE == pParam->m_bThreadEnd)
			{
				break;
			}
			else
			{
				UINT msgID = pParam->m_nMsgID;
				HWND hCallback = pParam->m_hCallbackWnd;
				if (0 != msgID || nullptr != hCallback)
				{
					::PostMessage(hCallback, msgID, 0, 0);
				}
			}
		}
		else
		{
			//TRACE(_T(""));
		}
	}

	return 0;
}






