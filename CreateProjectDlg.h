#pragma once


// CCreateProjectDlg 대화 상자입니다.

class CCreateProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateProjectDlg)

public:
	CCreateProjectDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CCreateProjectDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ADMIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	CString m_szCreateFolder;
};
