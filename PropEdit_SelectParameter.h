#pragma once



// CPropEdit_SelectParameter form view

#include "PropEditWizardCommon.h"
class CPropEdit_SelectParameter : public CFormView, public CPropEditWizardCommon
{
	DECLARE_DYNCREATE(CPropEdit_SelectParameter)

protected:
	CPropEdit_SelectParameter();           // protected constructor used by dynamic creation
	virtual ~CPropEdit_SelectParameter();

	bool m_bActivate;
public:
	enum { IDD = IDD_PROPEDIT_SELECTPARAMETER };
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
	virtual VIEW_TYPE GetNextView();

	virtual bool IsVisivleNextButton() { return true; }
	virtual bool IsVisivlePrevButton() { return true; }
	virtual bool IsVisivleFinishButton() { return true; }
	virtual bool IsFinishOrCancel() { return false; }
	virtual void PreProcess();
	virtual bool PostProcess();
	virtual void OnInitialUpdate();
	CXTListCtrl m_ListCtrl;
};


