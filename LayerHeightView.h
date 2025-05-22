#pragma once
#include "afxwin.h"
#ifdef _TOOLCOMPILE
#include "ImagePrevDlg.h"
#endif



// CLayerHeightView 폼 뷰입니다.
#include "BrushRenderView.h"

class CLayerHeightView : public CFormView
{
	DECLARE_DYNCREATE(CLayerHeightView)

protected:
	CLayerHeightView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CLayerHeightView();

public:
	enum { IDD = IDD_LAYERHEIGHTVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	CStatic m_PrevStatic;

	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;
	CXTPToolBar m_wndToolBar;

	bool m_bDrawPoint;
	bool m_bLButtonDown;
	CPoint m_MousePos;
	CMenu *m_pContextMenu;

	CActionElement *m_pCurAction;

#ifdef _TOOLCOMPILE
	int m_nChannelOption;
	std::string m_szPathName;
	bool m_bSwapBit;
	int m_nCenter;
	int m_nMin;
	int m_nMax;
#endif

	void CreateMemDC();
	void DeleteMemDC();

	void ProcessMouseRect( CPoint point );
	void ApplyHeight();
	void FinishApplyHeight();
	void ImportHeight( int nChannelOption, LPCSTR szPathName, bool bSwapBit, int nCenter, int nMin, int nMax, CTEtWorldSector *pSector );
public:
	void UpdateHeightPreview();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnLayerheightImporttexture();
	afx_msg void OnLayerheightExporttexture();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnReload();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


