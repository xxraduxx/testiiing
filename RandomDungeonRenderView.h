#pragma once



// CRandomDungeonRenderView 폼 뷰입니다.

#include "EtMatrixEx.h"
#include "Timer.h"
#include "InputReceiver.h"
class CRandomDungeonRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CRandomDungeonRenderView)

protected:
	CRandomDungeonRenderView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CRandomDungeonRenderView();

public:
	enum { IDD = IDD_RANDOMDUNGEONRENDERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	bool m_bRefreshScene;
	MatrixEx m_Camera;
	int m_nMouseFlag;
	CPoint m_MousePos;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;

	bool m_bDraw2DMap;
	int m_n2DMapType;
	int m_nDraw2DMapLevel;

	void RefreshScene() { m_bRefreshScene = true; }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual void OnInitialUpdate();
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnCustomRender( WPARAM wParam, LPARAM lParam );
	LRESULT OnOpenView( WPARAM wParam, LPARAM lParam );
	LRESULT OnCloseView( WPARAM wParam, LPARAM lParam );

	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


