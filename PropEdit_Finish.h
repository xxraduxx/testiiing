#pragma once



// CPropEdit_Finish form view

#include "PropEditWizardCommon.h"
class CPropEdit_Finish : public CFormView, public CPropEditWizardCommon
{
	DECLARE_DYNCREATE(CPropEdit_Finish)

protected:
	CPropEdit_Finish();           // protected constructor used by dynamic creation
	virtual ~CPropEdit_Finish();

public:
	enum { IDD = IDD_PROPEDIT_FINISH };
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
	virtual VIEW_TYPE GetNextView() { return UNKNOWN_TYPE; }

	virtual bool IsVisivleNextButton() { return false; }
	virtual bool IsVisivlePrevButton() { return false; }
	virtual bool IsVisivleFinishButton() { return true; }
	virtual bool IsFinishOrCancel() { return true; }
	virtual void PreProcess();
	virtual bool PostProcess();
	afx_msg void OnBnClickedCheck1();
	BOOL m_bCheckExport;
	CString m_szExportFileName;
	afx_msg void OnBnClickedButton1();

};


