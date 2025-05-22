#pragma once

#include "PropertyGridCreator.h"


// CSoundPropertyPaneView form view

class CSoundPropertyPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CSoundPropertyPaneView)

protected:
	CSoundPropertyPaneView();           // protected constructor used by dynamic creation
	virtual ~CSoundPropertyPaneView();

public:
	enum { IDD = IDD_SOUNDPROPERTYPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	int m_nSelectType;

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
};


