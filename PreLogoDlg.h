#pragma once


// CPreLogoDlg dialog
#include "Singleton.h"

class CPreLogoDlg : public CDialog, public CSingleton<CPreLogoDlg>
{
	DECLARE_DYNAMIC(CPreLogoDlg)

public:
	CPreLogoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreLogoDlg();

// Dialog Data
	enum { IDD = IDD_PRELOGO };

public:
	void UpdateDescription( const char *szStr );
	void Finish();

protected:
	DWORD m_dwFinishTime;
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

protected:
	void CreateMemDC();
	void DeleteMemDC();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
