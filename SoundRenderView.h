#pragma once


#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "ActionCommander.h"

class CEtWorldProp;
class CTEtWorldSector;
class CActionElement;
class CTEtWorldProp;

// CSoundRenderView form view
class CSoundRenderView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CSoundRenderView)

protected:
	CSoundRenderView();           // protected constructor used by dynamic creation
	virtual ~CSoundRenderView();

protected:
	int m_nMouseFlag;
	bool m_bRefreshScene;

	MatrixEx m_Camera;	
	CPoint m_MousePos;
	CPoint m_PrevMousePos;
	LOCAL_TIME m_LocalTime, m_PrevLocalTime;
	bool m_bActivate;
	bool m_bCanceled;

	EtObjectHandle m_SpeakerHandle;

	CActionElement *m_pCurAction;

protected:
	void RefreshScene() { m_bRefreshScene = true; }
	void ResetCamera();
	void RefreshProerties();
	void ControlSound( int nX, int nY );


public:
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );

public:
	enum { IDD = IDD_SOUNDRENDERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CString GetUniqueSoundName();
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
class CTEtWorldSoundEnvi;
class CActionElementModifySound : public CActionElement {
public:
	CActionElementModifySound( char cType, CTEtWorldSoundEnvi *pSoundEnvi );
	virtual ~CActionElementModifySound();

protected:
	char m_cType;
	CTEtWorldSoundEnvi *m_pSoundEnvi;
	EtVector3 m_vModifyPos;
	float m_fModifyRange;
	float m_fModifyRollOff;
	float m_fModifyVolume;
	bool m_bModifyStream;

	EtVector3 m_vRedoPos;
	float m_fRedoRange;
	float m_fRedoRollOff;
	float m_fRedoVolume;
	bool m_bRedoStream;

public:
	bool Redo();
	bool Undo();
	void BeginModify();
	bool FinishModify();
};
