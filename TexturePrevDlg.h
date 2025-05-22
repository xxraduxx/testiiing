#pragma once


// CTexturePrevDlg

class CTexturePrevDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CTexturePrevDlg)

public:
	CTexturePrevDlg(BOOL bOpenFileDialog, // FileOpen은 TRUE, FileSaveAs는 FALSE입니다.
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CTexturePrevDlg();

protected:
	CDC *m_pmemDC[5];
	CBitmap *m_pdcBitmap[5];
	int m_nViewIndex;

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
	afx_msg void OnBnClickedColorRGB();
	afx_msg void OnBnClickedColorR();
	afx_msg void OnBnClickedColorG();
	afx_msg void OnBnClickedColorB();
	afx_msg void OnBnClickedColorA();
	virtual void OnFileNameChange();
	virtual BOOL OnFileNameOK();
	afx_msg void OnDestroy();
};


