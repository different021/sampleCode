
// DSoundLoopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DSoundLoop.h"
#include "DSoundLoopDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDSoundLoopDlg dialog




CDSoundLoopDlg::CDSoundLoopDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDSoundLoopDlg::IDD, pParent), m_bPlaying(FALSE), m_pData(), m_nDataLen(0), m_nDataPos(0), m_lpDSound(nullptr), m_lpDSoundBuffer8(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Added code...
	memset(m_hEvents, 0x00, sizeof(HANDLE) * Num_of_buffers);
	m_timerID = MMSYSERR_NOERROR;
	memset(&m_wfx, 0x00, sizeof(WAVEFORMATEX));
}

void CDSoundLoopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDSoundLoopDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOAD, &CDSoundLoopDlg::OnBnClickedBtnLoad)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CDSoundLoopDlg::OnBnClickedBtnPlay)
END_MESSAGE_MAP()


// CDSoundLoopDlg message handlers

BOOL CDSoundLoopDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Added code...
	HRESULT hr = DirectSoundCreate8(NULL, &m_lpDSound, NULL);
	if (FAILED(hr))
	{
		AfxMessageBox(_T("DirectSoundCreate8 Fail."));
		EndDialog(-1);
	}
	else
	{
		hr = m_lpDSound->SetCooperativeLevel(m_hWnd, DSSCL_PRIORITY);

		for (int i = 0 ; i < Num_of_buffers ; i++)
			m_hEvents[i] = CreateEvent(NULL, FALSE, NULL, NULL);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDSoundLoopDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDSoundLoopDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDSoundLoopDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDSoundLoopDlg::OnBnClickedBtnLoad()
{
	if (TRUE == m_bPlaying)
		return ;

	CString szFilter = _T("Wave File(*.wav)|*.wav||");
	CFileDialog fileDlg(TRUE, _T("wav"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,  szFilter);

	if (IDOK == fileDlg.DoModal())
	{
		CString szPath = fileDlg.GetPathName();
		LPTSTR szFileName = new TCHAR[szPath.GetLength() + 1];
		szFileName[szPath.GetLength()] = 0;
		memcpy_s(szFileName, sizeof(TCHAR) * (szPath.GetLength() + 1), (LPCTSTR)fileDlg.GetPathName(), sizeof(TCHAR) * szPath.GetLength());

		LoadWaveFile(szFileName);

		delete[] szFileName;
	}
}

void CDSoundLoopDlg::OnBnClickedBtnPlay()
{
	if (FALSE == m_bPlaying)
	{
		if (nullptr == m_pData || 0 == m_nDataLen)
			return ;

		DSBUFFERDESC bufdesc;
		ZeroMemory(&bufdesc, sizeof(DSBUFFERDESC));
		bufdesc.dwSize = sizeof(DSBUFFERDESC);

		bufdesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
		bufdesc.dwBufferBytes = m_wfx.nAvgBytesPerSec / 2 * Num_of_buffers;
		bufdesc.lpwfxFormat = &m_wfx;

		CComPtr<IDirectSoundBuffer> pBuffer;
		HRESULT hr = m_lpDSound->CreateSoundBuffer(&bufdesc, &pBuffer, NULL);
		if (FAILED(hr))
		{
			return ;
		}

		m_lpDSoundBuffer8.Release();
		GUID IID_IDSoundBuffer = {0x6825a449, 0x7524, 0x4d82, 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e};
		pBuffer->QueryInterface(IID_IDSoundBuffer, (LPVOID*)&m_lpDSoundBuffer8);

		void *write1 = 0, *write2 = 0;
		unsigned long length1 = 0, length2 = 0;
		hr = m_lpDSoundBuffer8->Lock(0, bufdesc.dwBufferBytes, &write1, &length1, &write2, &length2, 0);
		if (FAILED(hr))
		{
			return ;
		}

		if (0 < length1)
		{
			memset(write1, 0x00, length1);
			memcpy_s(write1, length1, m_pData.get(), length1 >= m_nDataLen ? m_nDataLen : length1);
			m_nDataPos = length1 >= m_nDataLen ? m_nDataLen : length1;
		}

		hr = m_lpDSoundBuffer8->Unlock(write1, length1, write2, length2);

		GUID IID_IDSoundNotify = {0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16};

		// Make Notification Point.
		CComPtr<IDirectSoundNotify8> lpDsNotify;
		DSBPOSITIONNOTIFY PositionNotify[Num_of_buffers];

		hr = m_lpDSoundBuffer8->QueryInterface(IID_IDSoundNotify, (LPVOID*)&lpDsNotify);
		if (SUCCEEDED(hr))
		{ 
			for (int i = 0 ; i < Num_of_buffers ; i++)
			{
				PositionNotify[i].dwOffset = m_wfx.nAvgBytesPerSec / 2 * (i + 1) - 1;
				PositionNotify[i].hEventNotify = m_hEvents[i];
			}
			hr = lpDsNotify->SetNotificationPositions(Num_of_buffers, PositionNotify);

			m_timerID = timeSetEvent(50, 50, TimerProcess, (DWORD)this, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);

			m_bPlaying = TRUE;
			GetDlgItem(IDC_BTN_PLAY)->SetWindowText(_T("Stop"));
			m_nDataPos = 0;

			m_lpDSoundBuffer8->SetCurrentPosition(0);
			m_lpDSoundBuffer8->Play(0, 0, DSBPLAY_LOOPING);
		}
	}
	else
	{
		timeKillEvent(m_timerID);

		m_lpDSoundBuffer8->Stop();

		m_bPlaying = FALSE;
		GetDlgItem(IDC_BTN_PLAY)->SetWindowText(_T("Play"));
	}
}

int CDSoundLoopDlg::LoadWaveFile(LPTSTR szFileName)
{
	if (NULL == szFileName)
		return -1;

	HMMIO hmmio = mmioOpen(szFileName, NULL, MMIO_ALLOCBUF | MMIO_READ);
	if (NULL == hmmio)
		return -2;

	MMCKINFO mmckParent;
	mmckParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	MMRESULT mmResult = mmioDescend(hmmio, &mmckParent, NULL, MMIO_FINDRIFF);
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
	if(mmioRead(hmmio, (HPSTR)&m_wfx, nFormatLen) != nFormatLen)
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

	m_nDataLen = mmckChild.cksize;

	char* pTemp = NULL;

	try
	{
		pTemp = new char[m_nDataLen];
	}
	catch (CMemoryException* /*e*/){}
	catch (CException* /*e*/){}

	if (NULL == pTemp)
	{
		mmioClose(hmmio, 0);
		return -6;
	}

	if (m_nDataLen != mmioRead(hmmio, (HPSTR)pTemp, m_nDataLen))
	{
		delete[] pTemp;
		mmioClose(hmmio, 0);
		return -4;
	}
	m_pData.reset(pTemp, ArrayDeleter());
	mmioClose(hmmio, 0);

	return 0;
}

void CALLBACK CDSoundLoopDlg::TimerProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CDSoundLoopDlg* pDDS = (CDSoundLoopDlg*)dwUser;
	HRESULT hr;

	void *write1 = 0, *write2 = 0;
	unsigned long length1, length2;

	DWORD dwResult = WaitForMultipleObjects(Num_of_buffers, pDDS->m_hEvents, FALSE, 0);
	if ((dwResult - WAIT_OBJECT_0) < Num_of_buffers)
	{
		TRACE(_T("[LOG] Wait : %d\n"), dwResult);
		DWORD nIndex = dwResult - WAIT_OBJECT_0;
		hr = pDDS->m_lpDSoundBuffer8->Lock((pDDS->m_wfx.nAvgBytesPerSec / 2) * nIndex, pDDS->m_wfx.nAvgBytesPerSec / 2, &write1, &length1, &write2, &length2, 0);

		if (0 < length1)
		{
			int nSize = (length1 > pDDS->m_nDataLen - pDDS->m_nDataPos) ? pDDS->m_nDataLen - pDDS->m_nDataPos : length1;
			memset(write1, 0x00, length1);
			memcpy_s(write1, nSize, pDDS->m_pData.get() + pDDS->m_nDataPos, nSize);
			pDDS->m_nDataPos += nSize;
			if (pDDS->m_nDataLen <= pDDS->m_nDataPos)
				pDDS->m_nDataPos = 0;
		}

		pDDS->m_lpDSoundBuffer8->Unlock(write1, length1, write2, length2);
	}
}