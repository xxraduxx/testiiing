#pragma once


// CParamInputDlg dialog

#include "EtTrigger.h"
#include "afxwin.h"
#include "UnionValueProperty.h"

#define UM_PARAMINPUT_PICK WM_USER + 9875

class CParamInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CParamInputDlg)

public:
	CParamInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParamInputDlg();

// Dialog Data
	enum { IDD = IDD_PARAMINPUTDLG };

protected:
	CPoint m_StartPos;
	CEtTrigger::ParamTypeEnum m_Type;
	CString m_szDesc;
	CXTFlatComboBox m_Combo;
	CXTEdit m_EditBox;
	CXTButton m_Button;
	CUnionValueProperty *m_pVariable;

public:
	void SetInitFlag( CEtTrigger::ParamTypeEnum Type, CPoint &p, CUnionValueProperty *pDefaultVariable );
	CUnionValueProperty *GetVariable() { return m_pVariable; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnBnClickedPick();
	afx_msg void OnBnClickedOK();
	afx_msg void OnBnClickedCancel();
};
