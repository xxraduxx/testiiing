#pragma once


// CCreateEventAreaDlg dialog
#include "EventEditWizardCommon.h"

class CCreateEventAreaDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateEventAreaDlg)

public:
	CCreateEventAreaDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateEventAreaDlg();

// Dialog Data
	enum { IDD = IDD_CREATEEVENTDLG };

protected:
	CEventEditWizardCommon::VIEW_TYPE m_CurrentViewType;
	CWnd *m_pCurrentView;

	std::vector<CWnd *> m_pVecViewList;

public:
	void SetCurrentView( CEventEditWizardCommon::VIEW_TYPE Type, bool bPreview = false );

	CButton m_BackButton;
	CButton m_NextButton;
	CButton m_FinishButton;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
