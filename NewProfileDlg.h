#pragma once
#include "afxwin.h"


// CNewProfileDlg dialog

class CNewProfileDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewProfileDlg)

public:
	CNewProfileDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewProfileDlg();

protected:
	CString m_szDefaultProfileName;
// Dialog Data
	enum { IDD = IDD_NEWPROFILEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit m_ProfileName;
	CString m_szProfileName;

	void SetDefaultProfileName( CString szStr ) { m_szDefaultProfileName = szStr; }
	virtual BOOL OnInitDialog();
};
