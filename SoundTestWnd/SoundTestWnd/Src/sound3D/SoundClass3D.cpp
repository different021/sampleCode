#pragma warning(disable:4996)
#include "SoundClass3D.h"
#include <tchar.h>

#define EQUAL(a, b) ( _tcsicmp( (a), (b)) == 0 )

SoundClass3D* g_pSound3D = NULL;

enum eDSB_DESC
{
	eDSB_PRIMARY_ = 0,
	eDSB_SECONDARY,
	eDSB_MAX_,
};

DSBUFFERDESC g_DSB_STATE[eDSB_MAX_];


SoundClass3D::SoundClass3D()
{
}

SoundClass3D::SoundClass3D(const SoundClass3D&)
{
}

SoundClass3D::~SoundClass3D()
{
}

bool SoundClass3D::Initalize(HWND hWnd)
{
	bool bResult = TRUE;

	bResult = InitializeDirectSound(hWnd);
	if (!bResult)
		return FALSE;

	bResult = LoadWaveFileFromINI(FILENAME_INI, &m_pFileNameList);
	if (!bResult)
		return FALSE;
		
	bResult = LoadWaveFile();
	if (!bResult)
		return FALSE;


	return TRUE;
}

void SoundClass3D::CleanUp()
{
	if (m_pPrimaryBuffer != NULL)
	{
		m_pPrimaryBuffer->Release();
		m_pPrimaryBuffer = NULL;
	}

	if (m_pListener != NULL)
	{
		m_pListener->Release();
		m_pListener = NULL;
	}

	if (m_pDS != NULL)
	{
		m_pDS->Release();
		m_pDS = NULL;
	}
}

bool SoundClass3D::InitializeDirectSound(HWND hWnd)
{
	HRESULT hr = S_OK;

	

	//�⺻ ��ġ�� ���� �������̽� ȹ��
	hr = DirectSoundCreate8(NULL, &m_pDS, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Create DirectSound", L"ERROR", MB_OK);
		return FALSE;
	}

	hr = CoInitialize(0);

	//���� ���� ����.
	hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Set Co-op Level", L"ERROR", MB_OK);
		return FALSE;
	}


	CreateDSBUFFERDESC();

	//������ ���� ����
	hr = m_pDS->CreateSoundBuffer(&g_DSB_STATE[eDSB_PRIMARY_], &m_pPrimaryBuffer, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Create Primary Buffer", L"ERROR", MB_OK);
		return FALSE;
	}
	

	//�⺻ ���� ���� ���� ����
	// 16��Ʈ ���׸���  44100 ����/�� .wav ����
	// ���� ���Ͽ��� �о�;��ϴ� ����.
	WAVEFORMATEX fmt;
	fmt.wFormatTag		= WAVE_FORMAT_PCM;
	fmt.nSamplesPerSec	= 44100;
	fmt.wBitsPerSample	= 16;
	fmt.nChannels		= 1;
	fmt.nBlockAlign		= (fmt.wBitsPerSample / 8) * fmt.nChannels;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
	fmt.cbSize			= 0;

	hr = m_pPrimaryBuffer->SetFormat(&fmt);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Set Format", L"ERROR", MB_OK);
		return FALSE;
	}


	hr = m_pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener8, (LPVOID*)&m_pListener);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"FAIL to Get Listener InterFace", L"ERRO", MB_OK);
		return FALSE;
	}

	

	//�ʱ� ��ġ ����.
	SetPos(0.f, 0.f, 0.f);
	//m_pListener->SetPosition(0.f, 0.f, 0.f, DS3D_IMMEDIATE);


	return TRUE;
}


//CREATE Primary Buffer DESC 
void SoundClass3D::CreateDSBUFFERDESC()
{
	//------------------------------------------------------------------------
	//PRIMARY BUFFER  
	//------------------------------------------------------------------------

	DSBUFFERDESC desc;
	desc.dwSize			= sizeof(DSBUFFERDESC);
	desc.dwFlags		= DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	desc.dwBufferBytes	= 0;
	desc.dwReserved		= 0;
	desc.lpwfxFormat	= NULL;
	desc.guid3DAlgorithm = GUID_NULL;

	g_DSB_STATE[eDSB_PRIMARY_] = desc;

}

void SoundClass3D::CreateDS_2ndBUFFERDESC(DSBUFFERDESC &desc)
{
	g_DSB_STATE[eDSB_SECONDARY] = desc;
}


bool SoundClass3D::LoadWaveFileFromINI(const TCHAR* fileName, TCHAR** output)
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

	
	if (cntFile != iTotalFileNum)
	{
		MessageBox(NULL, L"���ҽ� ���� ������ �����ʽ��ϴ�. \n[sound.ini]������ TOTAL_FILE_NUM ���ڿ� ���ҽ� ���� ������ Ȯ���ϼ���.", L"ERROR", MB_OK);
		return FALSE;
	}

	iTotalFileNum;
	//if (eSOUND::eSOUND_MAX_ != iTotalFileNum)
	//{
	//	MessageBox(NULL, L"LOAD�� ���ҽ� ���ڿ� Header�� enum���� �����ʽ��ϴ�.", L"ERROR", MB_OK);
	//	return FALSE;
	//}

	////������ ����.
	m_iNumberOfFile = iTotalFileNum;
	*output = pFileName;

	return TRUE;
}

bool SoundClass3D::LoadWaveFile()
{
	bool bResult = TRUE;
	TCHAR temp[256] = L"";

	for (int i = 0; i < m_iNumberOfFile; i++)
	{
		_tcscpy(temp, m_pFileNameList + 256 * i);
		bResult = LoadWaveFile(temp);
		
		if (!bResult)
			return FALSE;
	}

	return TRUE;
}

bool SoundClass3D::LoadWaveFile(const TCHAR* szFileName)
{
	CSoundWave3D* pTempWave = new CSoundWave3D(m_pDS);
	bool bResult = pTempWave->LoadWaveFile(szFileName);
	pTempWave->SetPos(0.f, 0.f, -5.f);
	m_SoundWaveList.push_back(pTempWave);							//���� Ŭ������ �߰�.

	return bResult;
}

void SoundClass3D::CleanUpWaveFile(IDirectSoundBuffer8** pp2ndBuffer, IDirectSound3DBuffer8** pp2nd3DBuffer)
{
	if (*pp2ndBuffer != NULL)
	{
		(*pp2ndBuffer)->Release();
		*pp2ndBuffer = NULL;
	}

	if (*pp2nd3DBuffer != NULL)
	{
		(*pp2nd3DBuffer)->Release();
		*pp2nd3DBuffer = NULL;
	}

}

void SoundClass3D::SetPos(float x, float y, float z)
{
	m_pListener->SetPosition(0.f, 0.f, 0.f, DS3D_IMMEDIATE);
}

void SoundClass3D::SetStartPos(eSOUND track, DWORD dwPos)
{
	if (m_SoundWaveList[track] != NULL)
	{
		MessageBox(NULL, L"[Out Of Index] SetStartPos", L"ERROR", MB_OK);
	}
	m_SoundWaveList[track]->SetStartMusicPos(dwPos);
}

void SoundClass3D::SetVolume(eSOUND track, LONG num)
{
	if (m_SoundWaveList[track] != NULL)
	{
		MessageBox(NULL, L"[Out Of Index] SetVolume", L"ERROR", MB_OK);
	}
	m_SoundWaveList[track]->SetVolume(num);
}

void SoundClass3D::PlayWaveFile(eSOUND track, DWORD dwFlag)
{
	if (m_SoundWaveList[track] == NULL)
	{
		MessageBox(NULL, L"[Out Of Index] PlayWaveFile", L"ERROR", MB_OK);
	}
	m_SoundWaveList[track]->Play(8 ,dwFlag);

}

void InitSound3D(HWND hWnd)
{
	g_pSound3D = new SoundClass3D;
	g_pSound3D->Initalize(hWnd);
}

void CleanUp3D()
{
	g_pSound3D->CleanUp();
}

void PlaySound3D(eSOUND track, DWORD flag)
{
	g_pSound3D->PlayWaveFile(track, flag);
}

void SetPos(float x, float y, float z)
{
	g_pSound3D->SetPos(x, y, z);
}
