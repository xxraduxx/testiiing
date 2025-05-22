#pragma once


// CSkinPrevDlg

class CSkinPrevDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CSkinPrevDlg)

public:
	CSkinPrevDlg(BOOL bOpenFileDialog, // FileOpen은 TRUE, FileSaveAs는 FALSE입니다.
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CSkinPrevDlg();

protected:
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

	static HANDLE s_hThreadHandle;
	static int s_nThreadStatus;
	unsigned m_dwThreadIndex;

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
};
