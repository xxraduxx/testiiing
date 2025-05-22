#pragma once

#include "PropertyGridCreator.h"

// CEnviBasePaneView form view

class CEnviPaneView;
class CEnviBasePaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CEnviBasePaneView)

protected:
	CEnviBasePaneView();           // protected constructor used by dynamic creation
	virtual ~CEnviBasePaneView();

public:
	enum { IDD = IDD_ENVIBASEPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnNotifyGrid( WPARAM wParam, LPARAM lParam );
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	afx_msg LRESULT OnSetModify( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSoundPaneDropMoveItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSoundPaneDropItem( WPARAM wParam, LPARAM lParam );
};


