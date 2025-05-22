#pragma once



// CBrushRenderView 폼 뷰입니다.
#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

class CEtWorldDecal;
class CBrushRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CBrushRenderView)

protected:
	CBrushRenderView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CBrushRenderView();

public:
	enum { IDD = IDD_BRUSHRENDERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	enum EDIT_MODE {
		EM_TERRAIN,
		EM_BLOCK,
	};
	EDIT_MODE m_EditMode;

	int m_nMouseFlag;

	MatrixEx m_Camera;	
	std::vector<int> m_nVecSelectBlockList;
	CPoint m_MousePos;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	bool m_bRefreshScene;
	bool m_bBrushOneBlock;
	int m_nBrushOneBlockIndex;
	CActionElement *m_pCurAction;
	bool m_bShowProp;
	EtVector2 m_vDecalOffset;

	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void ApplyTerrain( int nX, int nY, bool bClick );
	void FinishApplyTerrian();

	void ToggleEditMode();
	void CalcBlockIndex( int nX, int nY, char cFlag );
	void CalcSelectDecal( int nX, int nY, bool bDrag );
	void DrawBlockArea( int nBlockIndex );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	virtual void OnInitialUpdate();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnCustomRender( WPARAM wParam, LPARAM lParam );
	LRESULT OnOpenView( WPARAM wParam, LPARAM lParam );
	LRESULT OnCloseView( WPARAM wParam, LPARAM lParam );

	afx_msg void OnDkBrushview1();
	afx_msg void OnDkBrushview2();
	afx_msg void OnDkBrushview3();
	afx_msg void OnDkBrushview4();
	afx_msg void OnDkBrushview5();
	afx_msg void OnDkBrushviewH();
	afx_msg void OnDkBrushviewD();
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void DeleteDecal();
};


class CTEtWorldSector;
class CActionElementBrushHeight : public CActionElement {
public:
	CActionElementBrushHeight( CTEtWorldSector *pSector );
	virtual ~CActionElementBrushHeight();

protected:
	CTEtWorldSector *m_pSector;
	short *m_pHeight[2];
	CRect m_rcRect;

public:
	bool Redo();
	bool Undo();
	void BeginModify();
	bool FinishModify();
};

class CActionElementBrushAlpha : public CActionElement {
public:
	CActionElementBrushAlpha( CTEtWorldSector *pSector );
	virtual ~CActionElementBrushAlpha();

protected:
	CTEtWorldSector *m_pSector;
	DWORD *m_pAlpha[2];
	CRect m_rcRect;

public:
	bool Redo();
	bool Undo();
	void BeginModify();
	bool FinishModify();
};

class CActionElementBrushGrass : public CActionElement {
public:
	CActionElementBrushGrass( CTEtWorldSector *pSector );
	virtual ~CActionElementBrushGrass();

protected:
	CTEtWorldSector *m_pSector;
	char *m_pGrass[2];
	CRect m_rcRect;

public:
	bool Redo();
	bool Undo();
	void BeginModify();
	bool FinishModify();
};

class CActionElementBrushDecal : public CActionElement {
public:
	CActionElementBrushDecal( CTEtWorldSector *pSector, CEtWorldDecal *pDecal, bool bAddRemove );
	virtual ~CActionElementBrushDecal();

protected:
	CTEtWorldSector *m_pSector;
	bool m_bAddRemove;
	CEtWorldDecal *m_pDecal;
	bool m_bLastActionUndo;

public:
	bool Redo();
	bool Undo();
};

class CActionElementModifyDecal : public CActionElement {
public:
	CActionElementModifyDecal( CTEtWorldSector *pSector, CEtWorldDecal *pDecal, char cType );
	virtual ~CActionElementModifyDecal();

protected:
	char m_cType;
	CTEtWorldSector *m_pSector;
	CEtWorldDecal *m_pDecal;

	EtVector2 m_vModifyPos;
	float m_fModifyRadius;
	float m_fModifyRotate;
	EtColor m_ModifyColor;
	float m_fModifyAlpha;

	EtVector2 m_vRedoPos;
	float m_fRedoRadius;
	float m_fRedoRotate;
	EtColor m_RedoColor;
	float m_fRedoAlpha;

public:
	bool Redo();
	bool Undo();
};