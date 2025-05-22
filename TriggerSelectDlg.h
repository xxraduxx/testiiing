#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "ScriptParamStatic.h"
#include "SyntaxColorizer.h"

// CTriggerSelectDlg dialog

class CTEtTriggerElement;
class CTriggerSelectDlg : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CTriggerSelectDlg)

public:
	CTriggerSelectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTriggerSelectDlg();

protected:
	CComboBox m_Combo;
	CRichEditCtrl m_CustomScript;
	CScriptParamStatic m_ParamDesc;
	bool m_bModify;
	int m_nPrevComboIndex;
	int m_nPrevCheck;
	CSyntaxColorizer m_sc;

	CTEtTriggerElement *m_pElement;

public:
	void SetTriggerElement( CTEtTriggerElement *pElement );

protected:
// Dialog Data
	enum { IDD = IDD_TRIGGERSELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnClickParam( WPARAM wParam, LPARAM lParam );
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeRichedit21();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnDestroy();
};
