#pragma once



// CPropEdit_CreateType form view

#include "PropEditWizardCommon.h"
class CPropEdit_CreateType : public CFormView, public CPropEditWizardCommon
{
	DECLARE_DYNCREATE(CPropEdit_CreateType)

protected:
	CPropEdit_CreateType();           // protected constructor used by dynamic creation
	virtual ~CPropEdit_CreateType();

public:
	enum { IDD = IDD_PROPEDIT_CREATETYPE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	virtual VIEW_TYPE GetNextView() { return FINISH; }

	virtual bool IsVisivleNextButton() { return true; }
	virtual bool IsVisivlePrevButton() { return true; }
	virtual bool IsVisivleFinishButton() { return true; }
	virtual bool IsFinishOrCancel() { return false; }
	virtual void PreProcess();
	virtual bool PostProcess();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szSignalName;
	CComboBox m_Combo;
};


