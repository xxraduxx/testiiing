#pragma once


#include "PropertyGridCreator.h"
// CLayerDecalView form view

class CLayerDecalView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CLayerDecalView)

protected:
	CLayerDecalView();           // protected constructor used by dynamic creation
	virtual ~CLayerDecalView();

public:
	enum { IDD = IDD_LAYERDECALVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSlotWidth;
	int m_nSlotHeight;
	int m_nSelectTextureIndex;
	int m_nTextureCount;
	int m_nDrawFlag;

	void CreateMemDC();
	void DeleteMemDC();

	void DrawSlot( CDC *pDC, int nSelectDecal, bool bDrawImage = true );
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

	virtual void OnInitialUpdate();
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
};


