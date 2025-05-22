#pragma once

#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

// CEventRenderView form view
class CEtWorldProp;
class CTEtWorldSector;
class CActionElement;
class CTEtWorldProp;
class CEventRenderView : public CFormView , public CInputReceiver
{
	DECLARE_DYNCREATE(CEventRenderView)

protected:
	CEventRenderView();           // protected constructor used by dynamic creation
	virtual ~CEventRenderView();

protected:
	int m_nMouseFlag;
	bool m_bRefreshScene;

	MatrixEx m_Camera;	
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	CPoint m_PrevMousePos2;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	char *m_pCursor;
	int m_nControlType;

	CActionElement *m_pCurAction;

protected:
	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void DrawArea( EtVector3 vMin, EtVector3 vMax, DWORD dwColor, DWORD dwSideColor );
	void RefreshProerties();

	CString GetUniqueAreaName();

public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );

public:
	enum { IDD = IDD_EVENTRENDERVIEW };
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

public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};



class CActionElementAddRemoveEventArea : public CActionElement {
public:
	CActionElementAddRemoveEventArea( bool bAddRemove );
	virtual ~CActionElementAddRemoveEventArea();

protected:
	bool m_bAddRemove;

	int m_nControlUniqueID;
	CString m_szAreaName;
	EtVector3 m_vMin;
	EtVector3 m_vMax;
	int m_nCreateAreaUniqueCount;

public:
	bool Redo();
	bool Undo();
	void SetEventArea( CEtWorldEventArea *pArea );
};

class CActionElementModifyEventArea : public CActionElement {
public:
	CActionElementModifyEventArea( char cType );
	virtual ~CActionElementModifyEventArea();

protected:
	char m_cType;
	EtVector3 m_vCurMin;
	EtVector3 m_vCurMax;
	float m_fCurRotate;
	CEtWorldEventArea *m_pEventArea;
	CString m_szCurName;

public:
	void SetEventArea( CEtWorldEventArea *pArea );
	bool Redo();
	bool Undo();

};