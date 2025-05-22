#pragma once
#include "afxcmn.h"

#include "PropertyGridCreator.h"

// CEnviLightPaneView form view

class CEnviPaneView;
class CEnviLightPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CEnviLightPaneView)

protected:
	CEnviLightPaneView();           // protected constructor used by dynamic creation
	virtual ~CEnviLightPaneView();

public:
	enum { IDD = IDD_ENVILIGHTPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CEnviPaneView *m_pEnvi;
	CMenu *m_pContextMenu;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;
	CXTTreeCtrl m_Tree;
	CImageList m_ImageList;
	HTREEITEM m_hRoot;
	int m_nDirCount;
	int m_nSpotCount;
	int m_nPointCount;

	void CalcSelectItemList();
	void InsertLight( int nLightType );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void SetParentEnvi( CEnviPaneView *pPaneView ) { m_pEnvi = pPaneView; }
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	LRESULT OnNotifyGrid( WPARAM wParam, LPARAM lParam );
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSetModify( WPARAM wParam, LPARAM lParam );

	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnvilightAdddirectionlight();
	afx_msg void OnEnvilightAddpointlight();
	afx_msg void OnEnvilightAddspotlight();
	afx_msg void OnEnvilightRemovelight();
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


