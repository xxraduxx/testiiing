#pragma once
#include "afxwin.h"


// CExportPropInfoDlg dialog

class CExportPropInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExportPropInfoDlg)

public:
	CExportPropInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExportPropInfoDlg();

public:
	CString GetMaxFolder() { return m_szMaxFolder; }
// Dialog Data
	enum { IDD = IDD_EXPORTPORPINFODLG };

protected:
	CString m_szMaxFolder;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CEdit m_MaxFolder;
};
