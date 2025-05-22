#pragma once


#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

class CEtWorldProp;
class CTEtWorldSector;
class CActionElement;
class CTEtWorldProp;

// CTriggerRenderView form view

class CTriggerRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CTriggerRenderView)

protected:
	CTriggerRenderView();           // protected constructor used by dynamic creation
	virtual ~CTriggerRenderView();

protected:
	int m_nMouseFlag;
	bool m_bRefreshScene;
	MatrixEx m_Camera;	
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	bool m_bActivate;
	LOCAL_TIME m_LastClickTime;

protected:
	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void RefreshProerties();

public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );

public:
	enum { IDD = IDD_TRIGGERRENDERVIEW };
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
	virtual void OnInitialUpdate();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnCustomRender( WPARAM wParam, LPARAM lParam );
	LRESULT OnOpenView( WPARAM wParam, LPARAM lParam );
	LRESULT OnCloseView( WPARAM wParam, LPARAM lParam );
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


