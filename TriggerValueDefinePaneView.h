#pragma once


#include "EtTrigger.h"

// CTriggerValueDefinePaneView form view

class CTriggerValueDefinePaneView : public CFormView
{
	DECLARE_DYNCREATE(CTriggerValueDefinePaneView)

protected:
	CTriggerValueDefinePaneView();           // protected constructor used by dynamic creation
	virtual ~CTriggerValueDefinePaneView();

protected:
	bool m_bActivate;

protected:
	CXTListCtrl m_ListCtrl;
	CMenu *m_pContextMenu;

protected:
	void SetItemListCtrl( CEtTrigger::DefineValue *pValue, int nItemID = -1 );
public:
	enum { IDD = IDD_TRIGGERVALUEDEFINEPANEVIEW };
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnTriggerValueDefineAddValue();
	afx_msg void OnTriggerValueDefineDeleteValue();
	afx_msg void OnTriggerValueDefineModifyValue();

};


