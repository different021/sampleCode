#pragma once
#pragma warning(disable : 4996)

#include <atlbase.h>
#include <memory>
#include <mmsystem.h>	
#include <dsound.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "dsound.lib")

class CSound
{
public:
	struct ArrayDeleter
	{
		template <class T>
		void operator()(T* p)
		{
			delete[] p;
		}
	};

public:
	CSound();
	~CSound();
public:
	//Create DirectSound Device interface.
	int CreateDSound(HWND hWnd, LPCGUID deviceID = nullptr);

	//Read WaveFile.
	int ReadWaveFile(LPTSTR szFileName);

	//Play Sound

	int PlayTheSound(HWND hWnd, UINT nMsgID, LPTSTR szWaveFile);

	//Stop Sound
	void StopTheSound();

private:
	HRESULT CreateSoundBuffer();
	HRESULT SetNotification();
	static UINT WINAPI SoundCallback(LPVOID lpParam);

private:
	CComPtr<IDirectSound8> m_pDirectSound;
	CComPtr<IDirectSoundBuffer8> m_pDSoundBuffer;

	HANDLE m_hCallbackThread;
	HANDLE m_hCallbackEvent;
	BOOL   m_bThreadEnd;

	//Wave Data
	WAVEFORMATEX m_wfx;
	DWORD m_nLength;
	std::tr1::shared_ptr<char> m_RawData;

	//Callback Message
	HWND m_hCallbackWnd;
	UINT m_nMsgID;

};




//extern CSound* g_pSound;