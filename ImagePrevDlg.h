#pragma once


// CImagePrevDlg

class CImagePrevDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CImagePrevDlg)

public:
	CImagePrevDlg(BOOL bOpenFileDialog, // FileOpen은 TRUE, FileSaveAs는 FALSE입니다.
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CImagePrevDlg();

	int GetChannelOption() { return m_nChannelOption; }
	bool IsSwapBit() { return m_bSwapBit; }
	int GetCenter() { return m_nCenter; }
	int GetMin() { return m_nMin; }
	int GetMax() { return m_nMax; }
	void SetExportMode() { m_bExportMode = true; }
protected:
	CDC *m_pmemDC[5];
	CBitmap *m_pdcBitmap[5];
	int m_nViewIndex;
	int m_nChannelOption;
	bool m_bSwapBit;
	int m_nCenter;
	int m_nMin;
	int m_nMax;
	bool m_bExportMode;

	static HANDLE s_hThreadHandle;
	static int s_nThreadStatus;
	unsigned m_dwThreadIndex;
	static UINT __stdcall BeginThread( void *pParam );

	void RefreshPreview();
	CRect GetPrevRect();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnPaint();
	afx_msg void OnBnClickedColorRGB();
	afx_msg void OnBnClickedColorR();
	afx_msg void OnBnClickedColorG();
	afx_msg void OnBnClickedColorB();
	afx_msg void OnBnClickedColorA();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedRadio7();
	afx_msg void OnBnClickedRadio8();
	afx_msg void OnBnClickedRadio9();
	afx_msg void OnBnClickedRadio10();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit7();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
protected:
	virtual void OnFileNameChange();
	virtual BOOL OnFileNameOK();
	virtual void OnOK();
public:
	afx_msg void OnDestroy();
};


