#pragma once



// CWaterPropPaneView form view
#include "PropertyGridCreator.h"

class CTEtWorldWater;
class CTEtWorldWaterRiver;

class CWaterPropPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CWaterPropPaneView)

protected:
	CWaterPropPaneView();           // protected constructor used by dynamic creation
	virtual ~CWaterPropPaneView();

public:
	enum { IDD = IDD_WATERPROPPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;

	int m_nPropType;
	union {
		CTEtWorldWater *m_pWater;
		CTEtWorldWaterRiver *m_pRiver;
	};

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
	afx_msg LRESULT OnDropItem( WPARAM wParam, LPARAM lParam );

};


