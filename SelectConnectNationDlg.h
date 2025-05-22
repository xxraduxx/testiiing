#pragma once
#include "afxwin.h"


// CSelectConnectNationDlg dialog

class CSelectConnectNationDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectConnectNationDlg)

public:
	CSelectConnectNationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectConnectNationDlg();

// Dialog Data
	enum { IDD = IDD_CONNECTTODLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_Combo;
	afx_msg void OnBnClickedOk();
};
