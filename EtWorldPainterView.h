// EtWorldPainterView.h : CEtWorldPainterView 클래스의 인터페이스
//


#pragma once

class CRenderView;
class CDummyView;
class CEtWorldPainterView : public CView
{
protected: // serialization에서만 만들어집니다.
	CEtWorldPainterView();
	DECLARE_DYNCREATE(CEtWorldPainterView)

// 특성입니다.
public:
	CEtWorldPainterDoc* GetDocument() const;

// 작업입니다.
public:
	void Refresh();
	CDummyView *GetDummyView() { return m_pDummyView; }
	void EnableTabView( int nTabIndex, bool bEnable );
	void SelectTabView( int nTabIndex );
	void ChangeModify( int nTabIndex, bool bModify );

// 재정의입니다.
public:

	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// 구현입니다.
public:
	virtual ~CEtWorldPainterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nActiveView;
	CXTPTabControl *m_pFlatTabCtrl;
	CDummyView *m_pDummyView;

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
};

#ifndef _DEBUG  // EtWorldPainterView.cpp의 디버그 버전
inline CEtWorldPainterDoc* CEtWorldPainterView::GetDocument() const
   { return reinterpret_cast<CEtWorldPainterDoc*>(m_pDocument); }
#endif

