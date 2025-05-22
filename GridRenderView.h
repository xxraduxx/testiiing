#pragma once



// CGridRenderView 폼 뷰입니다.
#include "InputReceiver.h"
#include "EtMatrixEx.h"

class CEtWorldGrid;
class CGridRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CGridRenderView)

protected:
	CGridRenderView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CGridRenderView();

public:
	enum { IDD = IDD_RENDERVIEW };
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
	int m_nMouseFlag;
	MatrixEx m_Camera;
	CMenu *m_pContextMenu;

	D3DXVECTOR2 m_StartPos;
	D3DXVECTOR2 m_CurPos;

	LOCAL_TIME m_PrevLocalTime;
	bool m_bRefreshScene;
	bool m_bPrevCamPos;
	MatrixEx m_PrevCameraPos;

	void OnGridViewControl( int nReceiverState, float fDelta );
	bool IntersectTriangle( const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2, FLOAT* t, FLOAT* u, FLOAT* v );
	void CheckAndSelectGrid();
	void CheckAndSelectGrid( CEtWorldGrid *pGrid, EtVector3 &vOrig, EtVector3 &vDir, CString &szResultName, SectorIndex &ResultIndex );
	void ResetCamera();

	void RefreshScene() { m_bRefreshScene = true; }

public:
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnCustomRender( WPARAM wParam, LPARAM lParam );
	LRESULT OnOpenView( WPARAM wParam, LPARAM lParam );
	LRESULT OnCloseView( WPARAM wParam, LPARAM lParam );
	LRESULT OnChangeCamera( WPARAM wParam, LPARAM lParam );

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnGridcontextCheckin();
	afx_msg void OnGridcontextCheckout();
	afx_msg void OnGridcontextCreateEmptySector();
	afx_msg void OnGridcontextEmptySector();
	afx_msg void OnGridcontextNationCheckOut( UINT nID );
};


