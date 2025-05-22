#pragma once


#include "PropertyGridCreator.h"

// CNaviPropertyPaneView form view

class CNaviPropertyPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CNaviPropertyPaneView)

protected:
	CNaviPropertyPaneView();           // protected constructor used by dynamic creation
	virtual ~CNaviPropertyPaneView();

public:
	enum { IDD = IDD_NAVIPROPERTYPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;

public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnGridNotify(WPARAM, LPARAM);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAttr1();
	afx_msg void OnAttr2();
	afx_msg void OnAttr3();
	afx_msg void OnAttr4();
	afx_msg void OnUpdateAttr1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAttr2(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAttr3(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAttr4(CCmdUI *pCmdUI);

	afx_msg void OnAttrMakeNavi();
	afx_msg void OnAttrGenAttr4();

	afx_msg void OnUpdateMakeNavi(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGenAttr4(CCmdUI *pCmdUI);
};


