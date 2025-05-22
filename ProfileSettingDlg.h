#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CProfileSettingDlg dialog

class CProfileSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CProfileSettingDlg)

public:
	CProfileSettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProfileSettingDlg();

// Dialog Data
	enum { IDD = IDD_PROFILEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CComboBox m_Combo;
	CString m_LocalPath;
	CString m_LocalResPath;
	CString m_LoginInfoPath;
	CXTListCtrl m_List;
	CXTButton m_NewButton;
	CXTButton m_DelButton;
	CXTButton m_PropButton;
	CImageList m_ImageList;

	CMenu *m_pContextMenu;

	void RefreshProfileList( const char *szSelectProfileName );
	void RefreshProfileInfo();
public:

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton9();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnEditchangeCombo1();
	afx_msg void OnEnUpdateEdit1();
	afx_msg void OnEnUpdateEdit4();
	afx_msg void OnEnUpdateEdit6();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnProfileeditAddnation();
	afx_msg void OnProfileeditRemovenation();
	afx_msg void OnProfileeditModifynation();
};
