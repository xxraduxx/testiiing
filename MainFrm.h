// MainFrm.h : CMainFrame 클래스의 인터페이스
//


#pragma once

class CActionCommander;
class CEtResourceMng;
class CMainFrame : public CXTPFrameWnd
{
	
protected: // serialization에서만 만들어집니다.
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 특성입니다.
public:

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	CWnd *GetPaneFrame( int nID );
	int GetFocusPaneID();
	CXTPDockingPaneManager* XTPDockingPaneManager() {return &m_paneManager;}
	void SetFocus( int nID );

// 구현입니다.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CXTPStatusBar m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CProgressCtrl m_ProgressBar;

	CMap<UINT,UINT, CWnd* , CWnd*> m_mapPanes;
	CXTPDockingPaneManager m_paneManager;
	bool m_bUndoRedoAccel;

	CXTPDockingPane* CreatePane( int nID, CRuntimeClass* pNewViewClass, CString strFormat, XTPDockingPaneDirection direction, CXTPDockingPane* pNeighbour = NULL, CRect *pRect = NULL ) ;

	CXTPControlStatic* FindInfoControl(CXTPControlListBox* pControlListBox);
	CActionCommander *GetCurActionCmd();
	CEtResourceMng *m_pResMng;

public:
	CProgressCtrl *GetProgressBar() { return &m_ProgressBar; }
	CXTPStatusBar *GetStatusBar() { return &m_wndStatusBar; }

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnClose();

	afx_msg void OnUpdateProjectConnect(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProjectDisconnect(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProjectSave(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProjectSetting(CCmdUI *pCmdUI);
	afx_msg int OnCreateControl(LPCREATECONTROLSTRUCT lpCreateControl);

	afx_msg void OnUpdateUndo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateRedo(CCmdUI *pCmdUI);

	afx_msg void OnUndo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUndoSelChange(NMHDR* pNMHDR, LRESULT* pRes);
	afx_msg void OnUndoPoup(NMHDR* pNMHDR, LRESULT* pRes);

	afx_msg void OnRedo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRedoSelChange(NMHDR* pNMHDR, LRESULT* pRes);
	afx_msg void OnRedoPoup(NMHDR* pNMHDR, LRESULT* pRes);

	afx_msg void OnUndoAccel();
	afx_msg void OnRedoAccel();

	afx_msg void OnIncreaseBrushSize();
	afx_msg void OnDecreaseBrushSize();

	afx_msg void OnAttr1();
	afx_msg void OnAttr2();
	afx_msg void OnAttr3();
	afx_msg void OnAttr4();
	afx_msg void OnToggleTopView();
	afx_msg void OnUpdateToggleTopView(CCmdUI *pCmdUI);

	afx_msg void OnToggleSimul();
	afx_msg void OnUpdateSimulRun(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSimulStop(CCmdUI *pCmdUI);
	afx_msg void OnSimulRun();
	afx_msg void OnSimulStop();
	afx_msg void OnGridCreategrid();
	afx_msg void OnGridDeletegrid();
	afx_msg void OnGridMovegrid();
	afx_msg void OnUpdateGridCreategrid(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridDeletegrid(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridMovegrid(CCmdUI *pCmdUI);
	afx_msg void OnPropertiesDirectorysetting();
	afx_msg void OnPropertiesEventSetting();
	afx_msg void OnUpdatePropertiesEventSetting(CCmdUI *pCmdUI);

	afx_msg void OnUpdateToolsMakeMinimap(CCmdUI *pCmdUI);
	afx_msg void OnToolsMakeMinimap();
	afx_msg void OnUpdateToolsSaveCurrentLightmap(CCmdUI *pCmdUI);
	afx_msg void OnToolsSaveCurrentLightmap();

	afx_msg void OnUpdateToolsExportTerrainto3dsmaxscriptdata(CCmdUI *pCmdUI);
	afx_msg void OnUpdateToolsExportPropInfoto3dsmaxscriptdata(CCmdUI *pCmdUI);
	afx_msg void OnToolsExportTerrainto3dsmaxscriptdata();
	afx_msg void OnToolsExportPropInfoto3dsmaxscriptdata();

	afx_msg void OnUpdatePropertiesPropPropertySetting(CCmdUI *pCmdUI);
	afx_msg void OnPropertiesPropPropertySetting();

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnViewCameraLight();
	afx_msg void OnUpdateViewCameraLight(CCmdUI *pCmdUI);
};


