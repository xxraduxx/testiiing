#pragma once


// CCreatePropPropertyDlg dialog
#include "PropEditWizardCommon.h"

class CCreatePropPropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreatePropPropertyDlg)

public:
	CCreatePropPropertyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreatePropPropertyDlg();

// Dialog Data
	enum { IDD = IDD_CREATEPROPPROPERTYDLG };

protected:
	CPropEditWizardCommon::VIEW_TYPE m_CurrentViewType;
	CWnd *m_pCurrentView;

	std::vector<CWnd *> m_pVecViewList;

public:
	void SetCurrentView( CPropEditWizardCommon::VIEW_TYPE Type, bool bPreview = false );

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
