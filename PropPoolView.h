#pragma once



// CPropPoolView form view

class CPropPoolView : public CFormView
{
	DECLARE_DYNCREATE(CPropPoolView)

protected:
	CPropPoolView();           // protected constructor used by dynamic creation
	virtual ~CPropPoolView();

public:
	enum { IDD = IDD_PROPPOOLVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	CXTPToolBar m_wndToolBar;
	bool m_bActivate;
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

	CSize m_ScrollSize;
	CSize m_ScrollPos;
	float m_fScale;
	int m_nCircleRadius;
	bool m_bLBDown;
	bool m_bInScroll;
	int m_nIconSize;

	int m_nSelectItemIndex;
	int m_nDetailViewIndex;
	bool m_bRemoveArea;

	void CreateMemDC();
	void DeleteMemDC();

	void ResetScroll();


	void CalcScrollRect( bool bVert, CRect &rcResult );
	void DrawPropList( CDC *pDC );
	EtVector2 MousePosToOffset( CPoint vPos );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnDropItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropMoveItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDblClickItem( WPARAM wParam, LPARAM lParam );
	afx_msg void OnPoolEmpty();
	afx_msg void OnPoolSave();
	afx_msg void OnPoolLoadlist();
	afx_msg void OnUpdatePoolEmpty(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePoolSave(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePoolLoadlist(CCmdUI *pCmdUI);
};


