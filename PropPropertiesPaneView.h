#pragma once



// CPropPropertiesPaneView form view
#include "PropertyGridCreator.h"

class CPropPropertiesPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CPropPropertiesPaneView)

protected:
	CPropPropertiesPaneView();           // protected constructor used by dynamic creation
	virtual ~CPropPropertiesPaneView();

public:
	enum { IDD = IDD_PROPPROPERTIESPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	int m_nCurrentSelectIndex;

public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

	int GetCurrentSelectIndex() { return m_nCurrentSelectIndex; }

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


