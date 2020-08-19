#include <Windows.h>
#include <dsound.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <mmsystem.h>

#include <tchar.h>

#pragma comment(lib, "dsound")
#pragma comment(lib, "WinMM")

typedef std::tuple<GUID, std::wstring, std::wstring> DeviceTuple;

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




int ReadWaveFile(LPWSTR szFileName)
{
	if (nullptr == szFileName)
		return -1;

	// Wave File Open
	HMMIO hmmio = nullptr;
	hmmio = mmioOpenW(szFileName, nullptr, MMIO_READ | MMIO_ALLOCBUF);
	//hmmio = mmioOpen(szFileName, nullptr, MMIO_READ | MMIO_ALLOCBUF);
	if (nullptr == hmmio)
		return -2;

	// Find RIFF Chunk
	MMCKINFO mmckParent;
	memset(&mmckParent, 0x00, sizeof(mmckParent));
	mmckParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	MMRESULT mmRes = mmioDescend(hmmio, &mmckParent, nullptr, MMIO_FINDRIFF);
	if (MMSYSERR_NOERROR != mmRes)
	{
		mmioClose(hmmio, 0);
		return -3;
	}

	// Find Format Chunk
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

	std::cout << "wFormatTag      : " << wfex.wFormatTag << std::endl;
	std::cout << "nChannels       : " << wfex.nChannels << std::endl;
	std::cout << "nSamplesPerSec  : " << wfex.nSamplesPerSec << std::endl;
	std::cout << "wBitsPerSample  : " << wfex.wBitsPerSample << std::endl;
	std::cout << "nBlockAlign     : " << wfex.nBlockAlign << std::endl;
	std::cout << "nAvgBytesPerSec : " << wfex.nAvgBytesPerSec << std::endl;

	// Find Data Chunk
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

	// Read Wave Data
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
 	mmioRead(hmmio, (HPSTR)pData, dwDataSize);
 	delete[] pData;	

	mmioClose(hmmio, 0);

	return 0;
}


//LPCWSTR multi_to_wide(const char* str)
//{
//	LPWSTR wide_str = new WCHAR[strlen(str) + 1];
//
//	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, strlen(str), wide_str, strlen(str));
//
//	wide_str[strlen(str)] = L'';
//
//	return const_cast<WCHAR*>(wide_str);
//}


int main()
{
	std::vector<DeviceTuple> vSoundDevice;
	DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback, (LPVOID)&vSoundDevice);
	WCHAR str[256] = L"../data/sound01.wav";
	
	ReadWaveFile(str);

	return 0;
}