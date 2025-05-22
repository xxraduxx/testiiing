#pragma once



// CLayerGrassView form view
#include "PropertyGridCreator.h"

class CLayerGrassView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CLayerGrassView)

protected:
	CLayerGrassView();           // protected constructor used by dynamic creation
	virtual ~CLayerGrassView();

public:
	enum { IDD = IDD_LAYERGRASSVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;

	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

protected:
	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSlotWidth;
	int m_nSlotHeight;
	int m_nSelectTextureIndex;
	int m_nTextureCount;
	int m_nSelectBlockIndex;
	int m_nDrawFlag;

	void CreateMemDC();
	void DeleteMemDC();

	void DrawSlot( CDC *pDC, int nBlockIndex, bool bDrawImage = true );
	void DrawCannotView( CDC *pDC, char *szStr );
	CRect GetSlotRect( int nSlotIndex );
	void ChangeTexture( char *szFileName );
	void ResetScroll();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropMoveItem( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnGridNotify(WPARAM, LPARAM);
	afx_msg void OnGrass1();
	afx_msg void OnGrass2();
	afx_msg void OnGrass3();
	afx_msg void OnGrass4();
	afx_msg void OnUpdateGrass1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGrass2(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGrass3(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGrass4(CCmdUI *pCmdUI);
	virtual void OnInitialUpdate();
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	afx_msg void OnReload();
};


