#pragma once

#include "EventEditWizardCommon.h"
#include "ColorListCtrl.h"


// CEventEdit_SelectParameter form view

class CEventEdit_SelectParameter : public CFormView, public CEventEditWizardCommon
{
	DECLARE_DYNCREATE(CEventEdit_SelectParameter)

protected:
	CEventEdit_SelectParameter();           // protected constructor used by dynamic creation
	virtual ~CEventEdit_SelectParameter();

	bool m_bActivate;
public:
	enum { IDD = IDD_EVENTEDIT_SELECTPARAMETER };
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
	CColorListCtrl m_ListCtrl;

};


