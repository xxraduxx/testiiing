#pragma once


// CConnectSettingDlg 대화 상자입니다.
#include "MiniSlider.h"
#include "FileServer.h"
#include "afxwin.h"

class CConnectSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CConnectSettingDlg)

public:
	CConnectSettingDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CConnectSettingDlg();

protected:
	void RefreshEnableControl( CFileServer::ServerTypeEnum Type );
	void RefreshProfileList();
// 대화 상자 데이터입니다.
	enum { IDD = IDD_CONNECT_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

protected:
	bool m_bCreateEmptyProject;
public:
	CString m_szUserName;

	CString m_szLocalPath;
	CButton m_CreateProject;

	bool IsCreateEmptyProject() { return m_bCreateEmptyProject; }

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton10();
	CComboBox m_ProfileCombo;
};
