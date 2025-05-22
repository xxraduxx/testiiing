#pragma once


// CNationDlg dialog
#include "FileServer.h"
#include "afxwin.h"

class CNationDlg : public CDialog
{
	DECLARE_DYNAMIC(CNationDlg)

public:
	CNationDlg(CWnd* pParent = NULL, CFileServer::ProfileStruct *pStruct=NULL );   // standard constructor
	virtual ~CNationDlg();

// Dialog Data
	enum { IDD = IDD_NATIONDLG };

protected:
	CFileServer::ProfileStruct *m_pProfile;
	CFileServer::NationStruct *m_pNation;

public:
	void SetNation( CFileServer::NationStruct *pNation ) { m_pNation = pNation; }


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString m_szNationName;
	CString m_szNationPath;
	CString m_szNationResource;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();
};
