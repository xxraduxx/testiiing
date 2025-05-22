#pragma once
#include "afxcmn.h"



// CWaterPaneView form view

class CTEtWorldWaterSea;
class CTEtWorldWaterRiver;

class CWaterPaneView : public CFormView
{
	DECLARE_DYNCREATE(CWaterPaneView)

protected:
	CWaterPaneView();           // protected constructor used by dynamic creation
	virtual ~CWaterPaneView();

public:
	enum { IDD = IDD_WATERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	CXTTreeCtrl m_Tree;
	CImageList m_ImageList;
	CXTPToolBar m_wndToolBar;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;
	HTREEITEM m_hRoot;

	CTEtWorldWaterSea *m_pSea;
	CTEtWorldWaterRiver *m_pWater;

protected:
	void CalcSelectItemList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelectRiver( WPARAM wParam, LPARAM lParam );

	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBakewateralpha();
};


