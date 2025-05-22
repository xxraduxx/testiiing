#pragma once
#include "afxwin.h"


// CCreateEnviDlg dialog

class CCreateEnviDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateEnviDlg)

public:
	CCreateEnviDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateEnviDlg();

// Dialog Data
	enum { IDD = IDD_CREATEENVIDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_szName;
	CString m_szCopyTo;
	CComboBox m_Combo;

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
