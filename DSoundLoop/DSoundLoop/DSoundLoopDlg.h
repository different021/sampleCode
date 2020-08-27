
// DSoundLoopDlg.h : header file
//

#pragma once

#include <memory>
#include <MMSystem.h>
#include <dsound.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "dsound.lib")

// CDSoundLoopDlg dialog
class CDSoundLoopDlg : public CDialogEx
{
	struct ArrayDeleter
	{
		template <class T>
		void operator()(T* p)
		{
			delete[] p;
		}
	};

// Construction
public:
	CDSoundLoopDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DSOUNDLOOP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	enum {Num_of_buffers = 4};
	BOOL m_bPlaying;
	std::shared_ptr<char> m_pData;
	DWORD m_nDataLen;
	DWORD m_nDataPos;

	CComPtr<IDirectSound8> m_lpDSound;
	CComPtr<IDirectSoundBuffer8> m_lpDSoundBuffer8;

	HANDLE m_hEvents[Num_of_buffers];
	MMRESULT m_timerID;
	WAVEFORMATEX m_wfx;

	int LoadWaveFile(LPTSTR szFileName);
	static void CALLBACK TimerProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnPlay();
};


