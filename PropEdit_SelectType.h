#pragma once



// CPropEdit_SelectType form view

#include "PropEditWizardCommon.h"
class CPropEdit_SelectType : public CFormView, public CPropEditWizardCommon
{
	DECLARE_DYNCREATE(CPropEdit_SelectType)

protected:
	CPropEdit_SelectType();           // protected constructor used by dynamic creation
	virtual ~CPropEdit_SelectType();

public:
	enum { IDD = IDD_PROPEDIT_SELECTTYPE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	bool m_bActivate;

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


