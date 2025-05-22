#pragma once
#include "afxwin.h"
#include "PropertyGridCreator.h"
#include "ActionCommander.h"


// CLayerPrevView 폼 뷰입니다.

class CLayerPrevView : public CFormView
{
	DECLARE_DYNCREATE(CLayerPrevView)

protected:
	CLayerPrevView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CLayerPrevView();

public:
	enum { IDD = IDD_LAYERPREVVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

protected:
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;
	CXTPToolBar m_wndToolBar;

	bool m_bActivate;
	int m_nLayerIndex;
	int m_nSelectTextureIndex;
	int m_nTextureCount;
	int m_nSelectBlockIndex;
	int m_nDrawFlag;
	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSlotWidth;
	int m_nSlotHeight;

	void CreateMemDC();
	void DeleteMemDC();

	void DrawSlot( CDC *pDC, int nBlockIndex, bool bDrawImage = true );
	void DrawCannotView( CDC *pDC, char *szStr );
	CRect GetSlotRect( int nSlotIndex );
	void ChangeTexture( int nTextureIndex, char *szFileName );
	void ResetScroll();

public:
	void SetLayerIndex( int nIndex ) { m_nLayerIndex = nIndex; }


public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropMoveItem( WPARAM wParam, LPARAM lParam );
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnReload();
};

class CTEtWorldSector;
class CActionElementChangeTexture : public CActionElement {
public:
	CActionElementChangeTexture( CTEtWorldSector *pSector );
	virtual ~CActionElementChangeTexture();

protected:
	CTEtWorldSector *m_pSector;
	std::vector<int> m_nVecBlockList;
	std::vector<CString> m_szVecPrevTextureName;
	int m_nLayerIndex;
	int m_nTextureIndex;
	CString m_szTextureName;

public:
	bool Redo();
	bool Undo();
	void ChangeTexture( int nLayerIndex, int nTextureIndex, std::vector<int> &nVecList, std::vector<CString> szVecName, CString szTextureName );
};

