#pragma once


// CConnectSettingDlg ��ȭ �����Դϴ�.
#include "MiniSlider.h"
#include "FileServer.h"
#include "afxwin.h"

class CConnectSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CConnectSettingDlg)

public:
	CConnectSettingDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CConnectSettingDlg();

protected:
	void RefreshEnableControl( CFileServer::ServerTypeEnum Type );
	void RefreshProfileList();
// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_CONNECT_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
