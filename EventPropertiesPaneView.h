#pragma once



#include "PropertyGridCreator.h"
// CEventPropertiesPaneView form view

class CTEtWorldEventArea;
class CEventPropertiesPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CEventPropertiesPaneView)

protected:
	CEventPropertiesPaneView();           // protected constructor used by dynamic creation
	virtual ~CEventPropertiesPaneView();

public:
	enum { IDD = IDD_EVENTPROPERTIESPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CTEtWorldEventArea *m_pSelectArea;

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


