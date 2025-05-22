#pragma once


#include "EventEditWizardCommon.h"
#include "afxwin.h"

// CEventEdit_CreateControl form view

class CEventEdit_CreateControl : public CFormView, public CEventEditWizardCommon
{
	DECLARE_DYNCREATE(CEventEdit_CreateControl)

protected:
	CEventEdit_CreateControl();           // protected constructor used by dynamic creation
	virtual ~CEventEdit_CreateControl();

public:
	enum { IDD = IDD_EVENTEDIT_CREATEMODIFY_EVENT };
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
	void OnSelEndOkBackColor();
	afx_msg void OnEnChangeEdit1();

	CString m_szSignalName;
	CXTColorPicker m_BackColorBtn;
	CXTColorPicker m_BackSideColorBtn;
	CXTColorPicker m_SelectColorBtn;
	CXTColorPicker m_SelectSideColorBtn;
	COLORREF m_BackgroundColor;
	COLORREF m_BackgroundSideColor;
	COLORREF m_SelectColor;
	COLORREF m_SelectSideColor;
};


