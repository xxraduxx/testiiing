#pragma once


#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

// CNaviRenderView form view

class CNaviRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CNaviRenderView)

protected:
	CNaviRenderView();           // protected constructor used by dynamic creation
	virtual ~CNaviRenderView();

protected:
	int m_nMouseFlag;
	bool m_bRefreshScene;

	MatrixEx m_Camera;	
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	CActionElement *m_pCurAction;


protected:
	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();

	void ApplyTerrain( int nX, int nY );
public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );

public:
	enum { IDD = IDD_NAVIRENDERVIEW };
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
};



class CTEtWorldSector;
class CActionElementBrushAttribute : public CActionElement {
public:
	CActionElementBrushAttribute( CTEtWorldSector *pSector );
	virtual ~CActionElementBrushAttribute();

protected:
	CTEtWorldSector *m_pSector;
	char *m_pAttribute[2];
	CRect m_rcRect;

public:
	bool Redo();
	bool Undo();
	void BeginModify();
	bool FinishModify();
};
