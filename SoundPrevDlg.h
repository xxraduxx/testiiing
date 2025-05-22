#pragma once

#include "fmod.hpp"

// CSoundPrevDlg

class CSoundPrevDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CSoundPrevDlg)

public:
	CSoundPrevDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CSoundPrevDlg();

protected:
	static HANDLE s_hThreadHandle;
	static int s_nThreadStatus;
	unsigned m_dwThreadIndex;
	CString m_szFileInfo;
	FMOD::Sound *m_pSound;
	FMOD::Channel *m_pChannel;

	void RefreshPreview();
	CRect GetPrevRect();

	static UINT __stdcall BeginThread( void *pParam );

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnFileNameChange();
	virtual BOOL OnFileNameOK();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedPlay();

};


