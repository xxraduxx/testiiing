#pragma once


// CCreateProjectDlg ��ȭ �����Դϴ�.

class CCreateProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateProjectDlg)

public:
	CCreateProjectDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CCreateProjectDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ADMIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	CString m_szCreateFolder;
};
