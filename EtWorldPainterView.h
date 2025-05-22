// EtWorldPainterView.h : CEtWorldPainterView Ŭ������ �������̽�
//


#pragma once

class CRenderView;
class CDummyView;
class CEtWorldPainterView : public CView
{
protected: // serialization������ ��������ϴ�.
	CEtWorldPainterView();
	DECLARE_DYNCREATE(CEtWorldPainterView)

// Ư���Դϴ�.
public:
	CEtWorldPainterDoc* GetDocument() const;

// �۾��Դϴ�.
public:
	void Refresh();
	CDummyView *GetDummyView() { return m_pDummyView; }
	void EnableTabView( int nTabIndex, bool bEnable );
	void SelectTabView( int nTabIndex );
	void ChangeModify( int nTabIndex, bool bModify );

// �������Դϴ�.
public:

	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// �����Դϴ�.
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

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
};

#ifndef _DEBUG  // EtWorldPainterView.cpp�� ����� ����
inline CEtWorldPainterDoc* CEtWorldPainterView::GetDocument() const
   { return reinterpret_cast<CEtWorldPainterDoc*>(m_pDocument); }
#endif

