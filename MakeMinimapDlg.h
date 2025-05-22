#pragma once
#include "afxwin.h"


// CMakeMinimapDlg dialog

class CMakeMinimapDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeMinimapDlg)

public:
	CMakeMinimapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMakeMinimapDlg();

// Dialog Data
	enum { IDD = IDD_MAKEMINIMAP };

protected:
	CComboBox m_ResXCombo;
	CComboBox m_ResYCombo;
	CComboBox m_EnviCombo;
	CButton m_RenderTerrain;
	CButton m_RenderProp;
	CButton m_AttributeAlpha;
	BOOL m_bRenderTerrain;
	BOOL m_bRenderProp;
	BOOL m_bAttributeAlpha;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedOk();
};
