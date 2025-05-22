#pragma once



// CPropEdit_SelectEditType form view

#include "PropEditWizardCommon.h"
class CPropEdit_SelectEditType : public CFormView, public CPropEditWizardCommon
{
	DECLARE_DYNCREATE(CPropEdit_SelectEditType)

protected:
	CPropEdit_SelectEditType();           // protected constructor used by dynamic creation
	virtual ~CPropEdit_SelectEditType();

protected:
	int m_nEditType;

public:
	enum { IDD = IDD_PROPEDIT_SELEDITTYPE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();

	virtual VIEW_TYPE GetNextView();
	virtual bool IsVisivleNextButton();
	virtual bool IsVisivlePrevButton() { return false; }
	virtual bool IsVisivleFinishButton() { return true; }
	virtual bool IsFinishOrCancel() { return false; }
	virtual void PreProcess();
	virtual bool PostProcess();

	virtual void OnInitialUpdate();
};


