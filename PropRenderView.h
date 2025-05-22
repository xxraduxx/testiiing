#pragma once

#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

// CPropRenderView 폼 뷰입니다.

class CEtWorldProp;
class CTEtWorldSector;
class CActionElement;
class CTEtWorldProp;
class CEtOutlineFilter;
class CPropRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CPropRenderView)

protected:
	CPropRenderView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPropRenderView();

protected:
	enum EDIT_MODE {
		EM_BRUSH,
		EM_MODIFY,
	};

	EDIT_MODE m_EditMode;
	int m_nMouseFlag;
	bool m_bRefreshScene;
	bool m_bLockSelect;

	CActionElement *m_pCurAction;

	MatrixEx m_Camera;	
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	bool m_bEnableCopy;

	CEtOutlineFilter	*m_pOutlineFilter;
	EtOutlineHandle m_hOutline;

protected:

	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void ToggleEditMode();
	void RefreshProperties();

	void CalcPropIndex( int nX, int nY, char cFlag, bool bClick = false );
	void ApplyProp( int nX, int nY );
	void ControlProp( int nX, int nY );

public:
	void DeleteProp();
	void ClearOutline();
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );


public:
	enum { IDD = IDD_PROPRENDERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

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


class CActionElementBrushProp : public CActionElement {
public:
	CActionElementBrushProp( bool bAddRemove );
	virtual ~CActionElementBrushProp();

protected:
	bool m_bAddRemove;
	std::vector<CEtWorldProp *> m_pVecProp;
	bool m_bLastActionUndo;

public:
	bool Redo();
	bool Undo();
	void AddProp( CEtWorldProp *pProp );
};

class CActionElementModifyProp : public CActionElement {
public:
	CActionElementModifyProp( char cType );
	virtual ~CActionElementModifyProp();

protected:
	char m_cType;
	std::vector<CEtWorldProp *> m_pVecProp;
	std::vector<EtVector3> m_vVecModify;
	std::vector<EtVector3> m_vVecRedo;

public:
	void AddProp( CEtWorldProp *pProp );
	bool Redo();
	bool Undo();
};