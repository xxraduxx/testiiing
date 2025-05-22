#pragma once

#include "afxcmn.h"
#include "PropertyGridCreator.h"



// CLayerPaneView �� ���Դϴ�.

class CLayerPrevView;
class CLayerHeightView;
class CLayerGrassView;
class CLayerDecalView;
class CLayerPaneView : public CFormView
{
	DECLARE_DYNCREATE(CLayerPaneView)

protected:
	CLayerPaneView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CLayerPaneView();

public:
	enum { IDD = IDD_LAYERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

protected:
	bool m_bActivate;
	int m_nSelectLayer;
	CLayerPrevView *m_pLayerView;
	CLayerHeightView *m_pLayerHeight;
	CLayerGrassView *m_pLayerGrass;
	CLayerDecalView *m_pLayerDecal;
	CXTPTabControl m_TabCtrl;

public:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelectChangeLayer( WPARAM wParam, LPARAM lParam );
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


