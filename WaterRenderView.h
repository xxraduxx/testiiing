#pragma once

#include "InputReceiver.h"
#include "EtMatrixEx.h"


// CWaterRenderView form view

class CWaterRenderView : public CFormView, public CInputReceiver, public CEtCustomRender
{
	DECLARE_DYNCREATE(CWaterRenderView)

protected:
	CWaterRenderView();           // protected constructor used by dynamic creation
	virtual ~CWaterRenderView();

protected:

	int m_nMouseFlag;
	bool m_bRefreshScene;
	MatrixEx m_Camera;	
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	bool m_bActivate;
	LOCAL_TIME m_LastClickTime;
	bool m_bDrawSelectLayer;
	bool m_bReadyAddRiver;
	EtVector3 *m_pControlPoint;

	bool m_GlobalWaterShowBackupValue;

protected:
	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void RefreshProerties();

public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );

public:
	enum { IDD = IDD_WATERRENDERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void ControlPoint( int nX, int nY );
	void CalcPointIndex( int nX, int nY, char cFlag, bool bClick = false );
	void DeletePoint();
	void RefreshProperties();

public:
	virtual void OnInitialUpdate();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnCustomRender( WPARAM wParam, LPARAM lParam );
	LRESULT OnOpenView( WPARAM wParam, LPARAM lParam );
	LRESULT OnCloseView( WPARAM wParam, LPARAM lParam );
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual void RenderCustom( float fElapsedTime );
};


