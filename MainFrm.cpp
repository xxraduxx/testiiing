// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "DockingPaneAdobeTheme.h"
#include "FileServer.h"

#include "MainFrm.h"
#include "PaneDefine.h"
#include "InputDevice.h"
#include "ActionCommander.h"
#include "TdDummyActor.h"
#include "CreateGridDlg.h"
#include "DeleteGridDlg.h"
#include "EnviControl.h"
#include "PropertiesDlg.h"
#include "UserMessage.h"
#include "MoveToGridDlg.h"
#include "EtResourceMng.h"
#include "MakeMinimapDlg.h"
#include "CreateEventAreaDlg.h"
#include "CreatePropPropertyDlg.h"
#include "ExportPropInfoDlg.h"

#include "EventSignalManager.h"
#include "PropSignalManager.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "../../Common/BugReporter/BugReporter.h"
#include "PreLogoDlg.h"
#include "RenderBase.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CInputDevice g_InputDevice;
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CXTPFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTPFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	ON_UPDATE_COMMAND_UI(ID_PROJECT_CONNECT, &CMainFrame::OnUpdateProjectConnect)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_DISCONNECT, &CMainFrame::OnUpdateProjectDisconnect)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_SAVE, &CMainFrame::OnUpdateProjectSave)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_SETTING, &CMainFrame::OnUpdateProjectSetting)
	ON_UPDATE_COMMAND_UI(ID_UNDO, &CMainFrame::OnUpdateUndo)
	ON_UPDATE_COMMAND_UI(ID_REDO, &CMainFrame::OnUpdateRedo)

	ON_XTP_EXECUTE(ID_UNDO, OnUndo)
	ON_NOTIFY(XTP_LBN_SELCHANGE, ID_UNDO, OnUndoSelChange)
	ON_NOTIFY(XTP_LBN_POPUP, ID_UNDO, OnUndoPoup)

	ON_XTP_EXECUTE(ID_REDO, OnRedo)
	ON_NOTIFY(XTP_LBN_SELCHANGE, ID_REDO, OnRedoSelChange)
	ON_NOTIFY(XTP_LBN_POPUP, ID_REDO, OnRedoPoup)

	ON_COMMAND(ID_UNDOACCEL, &CMainFrame::OnUndoAccel)
	ON_COMMAND(ID_REDOACCEL, &CMainFrame::OnRedoAccel)
	ON_COMMAND(ID_TOGGLESIMUL, &CMainFrame::OnToggleSimul)
	ON_COMMAND(ID_ADDBRUSHSIZE, &CMainFrame::OnIncreaseBrushSize)
	ON_COMMAND(ID_MINUSBRUSHSIZE, &CMainFrame::OnDecreaseBrushSize)

	ON_COMMAND(ID_ATTR_1, &CMainFrame::OnAttr1)
	ON_COMMAND(ID_ATTR_2, &CMainFrame::OnAttr2)
	ON_COMMAND(ID_ATTR_3, &CMainFrame::OnAttr3)
	ON_COMMAND(ID_ATTR_4, &CMainFrame::OnAttr4)

	ON_COMMAND(ID_VIEW_TOGGLETOPVIEW, &CMainFrame::OnToggleTopView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLETOPVIEW, &CMainFrame::OnUpdateToggleTopView)

	ON_XTP_CREATECONTROL()
	ON_UPDATE_COMMAND_UI(ID_SIMUL_RUN, &CMainFrame::OnUpdateSimulRun)
	ON_UPDATE_COMMAND_UI(ID_SIMUL_STOP, &CMainFrame::OnUpdateSimulStop)
	ON_COMMAND(ID_SIMUL_RUN, &CMainFrame::OnSimulRun)
	ON_COMMAND(ID_SIMUL_STOP, &CMainFrame::OnSimulStop)
	ON_COMMAND(ID_GRID_CREATEGRID, &CMainFrame::OnGridCreategrid)
	ON_COMMAND(ID_GRID_DELETEGRID, &CMainFrame::OnGridDeletegrid)
	ON_COMMAND(ID_GRID_MOVEGRID, &CMainFrame::OnGridMovegrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_CREATEGRID, &CMainFrame::OnUpdateGridCreategrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_DELETEGRID, &CMainFrame::OnUpdateGridDeletegrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_MOVEGRID, &CMainFrame::OnUpdateGridMovegrid)
	ON_COMMAND(ID_PROPERTIES_DIRECTORYSETTING, &CMainFrame::OnPropertiesDirectorysetting)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_EVENTSETTING, &CMainFrame::OnUpdatePropertiesEventSetting)
	ON_COMMAND(ID_PROPERTIES_EVENTSETTING, &CMainFrame::OnPropertiesEventSetting)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MAKEMINIMAP, &CMainFrame::OnUpdateToolsMakeMinimap)
	ON_COMMAND(ID_TOOLS_MAKEMINIMAP, &CMainFrame::OnToolsMakeMinimap)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_SAVELIGHTMAP, &CMainFrame::OnUpdateToolsSaveCurrentLightmap)
	ON_COMMAND(ID_TOOLS_SAVELIGHTMAP, &CMainFrame::OnToolsSaveCurrentLightmap)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_PROPPROPERTYSETTING, &CMainFrame::OnUpdatePropertiesPropPropertySetting)
	ON_COMMAND(ID_PROPERTIES_PROPPROPERTYSETTING, &CMainFrame::OnPropertiesPropPropertySetting)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_EXPORTTERRAINTO3DSMAXSCRIPTDATA, &CMainFrame::OnUpdateToolsExportTerrainto3dsmaxscriptdata)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_EXPORTPROPINFOTO3DSMAXSCRIPTDATA, &CMainFrame::OnUpdateToolsExportPropInfoto3dsmaxscriptdata)
	ON_COMMAND(ID_TOOLS_EXPORTTERRAINTO3DSMAXSCRIPTDATA, &CMainFrame::OnToolsExportTerrainto3dsmaxscriptdata)
	ON_COMMAND(ID_TOOLS_EXPORTPROPINFOTO3DSMAXSCRIPTDATA, &CMainFrame::OnToolsExportPropInfoto3dsmaxscriptdata)
	ON_COMMAND(ID_VIEW_CAMERA_LIGHT, &CMainFrame::OnViewCameraLight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_LIGHT, &CMainFrame::OnUpdateViewCameraLight)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
	m_bUndoRedoAccel = true;
	m_pResMng = new CEtResourceMng( false );
	CEtResourceMng::GetInstance().AddResourcePath( ".\\Plugins" );
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETE( m_pResMng );
}

int CMainFrame::OnCreateControl(LPCREATECONTROLSTRUCT lpCreateControl)
{
	switch( lpCreateControl->nID ) {
		case ID_UNDO:
		case ID_REDO:
			{
				CXTPControlPopup* pButtonUndo = CXTPControlPopup::CreateControlPopup(xtpControlSplitButtonPopup);

				CXTPPopupToolBar* pUndoBar = CXTPPopupToolBar::CreatePopupToolBar(GetCommandBars());
				pUndoBar->EnableCustomization(FALSE);
				pUndoBar->SetBorders(CRect(2, 2, 2, 2));
				pUndoBar->DisableShadow();


				CXTPControlListBox* pControlListBox = (CXTPControlListBox*)pUndoBar->GetControls()->Add(new CXTPControlListBox(), lpCreateControl->nID);
				pControlListBox->SetWidth(200);
				pControlListBox->SetLinesMinMax(1, 6);
				pControlListBox->SetMultiplSel(TRUE);

				CXTPControlStatic* pControlListBoxInfo = (CXTPControlStatic*)pUndoBar->GetControls()->Add(new CXTPControlStatic(), lpCreateControl->nID);
				pControlListBoxInfo->SetWidth(200);


				pButtonUndo->SetCommandBar(pUndoBar);
				pUndoBar->InternalRelease();

				lpCreateControl->pControl = pButtonUndo;
			}
			return TRUE;
	}
	return FALSE;
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CXTPFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	if (!InitCommandBars()) return -1;

	AfxInitRichEdit();
	AfxInitRichEdit2();

	// 상태 바에 추가할것들.
	// Progress
	m_ProgressBar.Create(WS_CHILD|WS_VISIBLE|PBS_SMOOTH, CRect(0,0,0,0), &m_wndStatusBar, 0);
	m_wndStatusBar.AddIndicator( ID_INDICATOR_PROG, 1 );
	int nIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_PROG );
	m_wndStatusBar.SetPaneWidth(nIndex, 120);
	m_wndStatusBar.SetPaneStyle(nIndex, m_wndStatusBar.GetPaneStyle(nIndex) | SBPS_NOBORDERS);
	m_wndStatusBar.AddControl(&m_ProgressBar, ID_INDICATOR_PROG, FALSE);

	/////////////////////////

	// Init Thema
	XTPPaintManager()->SetTheme( xtpThemeNativeWinXP );

	XTP_COMMANDBARS_ICONSINFO* pIconsInfo = XTPPaintManager()->GetIconsInfo();
	pIconsInfo->bUseDisabledIcons = TRUE;
	pIconsInfo->bOfficeStyleDisabledIcons = TRUE;

	CXTPCommandBars* pCommandBars = GetCommandBars();
	CXTPCommandBar* pMenuBar = pCommandBars->SetMenu(_T("Menu Bar"), IDR_MAINFRAME); 

	CXTPToolBar *pCommandBar, *pActionBar,*pSimulBar;

//	XTPImageManager()->SetMaskColor(RGB(0, 255, 0));
	if (!(pCommandBar = (CXTPToolBar*)pCommandBars->Add(_T("Standard"), xtpBarTop)) ||
		!pCommandBar->LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	if (!(pActionBar = (CXTPToolBar*)pCommandBars->Add(_T("Action"), xtpBarTop)) ||
		!pActionBar->LoadToolBar(IDR_ACTIONTOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	DockRightOf(pActionBar, pCommandBar);

	if (!(pSimulBar = (CXTPToolBar*)pCommandBars->Add(_T("Simulation"), xtpBarTop)) ||
		!pSimulBar->LoadToolBar(IDR_SIMULATION))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	DockRightOf(pSimulBar, pActionBar);


	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize DockingPaneManager.." );
	XTPDockingPaneManager()->InstallDockingPanes(this);
//	XTPDockingPaneManager()->SetTheme( xtpPaneThemeNativeWinXP ); //NativeWinXP

	m_paneManager.SetCustomTheme(new CDockingPaneAdobeTheme);
	m_paneManager.UseSplitterTracker(FALSE);
	m_paneManager.SetDockingContext(new CDockingPaneAdobeContext);
	m_paneManager.SetDefaultPaneOptions(xtpPaneNoCloseable);
 
	int nCount = sizeof(g_PaneList) / sizeof(PaneCreateStruct);
	int *nIconList = new int[nCount];
	for( int i=0; i<nCount; i++ ) {
		CXTPDockingPane *pDockPane = NULL;
		nIconList[i] = g_PaneList[i].nPaneID;
		if( g_PaneList[i].nDockPaneID != -1 ) pDockPane = g_PaneList[g_PaneList[i].nDockPaneID].pThis;

		if( (int)g_PaneList[i].Direction == -1 ) {
			g_PaneList[i].pThis = CreatePane( g_PaneList[i].nPaneID, g_PaneList[i].pClass, CString(g_PaneList[i].szName), g_PaneList[i].Direction, pDockPane, &CRect( g_PaneList[i].nLeft, g_PaneList[i].nTop, g_PaneList[i].nRight, g_PaneList[i].nBottom ) );
			XTPDockingPaneManager()->FloatPane( g_PaneList[i].pThis, CRect( g_PaneList[i].nLeft, g_PaneList[i].nTop, g_PaneList[i].nRight, g_PaneList[i].nBottom ) );
		}
		else g_PaneList[i].pThis = CreatePane( g_PaneList[i].nPaneID, g_PaneList[i].pClass, CString(g_PaneList[i].szName), g_PaneList[i].Direction, pDockPane, &CRect( g_PaneList[i].nLeft, g_PaneList[i].nTop, g_PaneList[i].nRight, g_PaneList[i].nBottom ) );
		if( g_PaneList[i].nAttachPaneID != -1 ) {
			XTPDockingPaneManager()->AttachPane( g_PaneList[i].pThis, g_PaneList[g_PaneList[i].nAttachPaneID].pThis );
			XTPDockingPaneManager()->ShowPane( g_PaneList[g_PaneList[i].nAttachPaneID].pThis );
		}
	}
	// 일단 아이콘 막아놓는다.
	delete []nIconList;

#ifndef _DEBUG
	LoadCommandBars(_T("EtWorldPainterCommandBars"));

	CXTPDockingPaneLayout layoutNormal(&m_paneManager);
	if (layoutNormal.Load(_T("EtWorldPainterLayout")))
		m_paneManager.SetLayout(&layoutNormal);
#endif //_DEBUG

	for( int i=0; i<nCount; i++ ) {
		if( g_PaneList[i].bShow == FALSE ) {
			XTPDockingPaneManager()->ClosePane( g_PaneList[i].nPaneID );
		}
		else {
			XTPDockingPaneManager()->ShowPane( g_PaneList[i].nPaneID );
		}
	}

	// Etc Initialize

	CInputDevice::GetInstance().Initialize( GetSafeHwnd(), CInputDevice::MOUSE | CInputDevice::KEYBOARD );
	CFileServer::GetInstance().Initialize();
	XTPDockingPaneManager()->ShowPane( GRID_PANE );
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CXTPFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return TRUE;
}


// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CXTPFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CXTPFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame 메시지 처리기



CXTPDockingPane* CMainFrame::CreatePane( int nID, CRuntimeClass* pNewViewClass, CString strFormat, XTPDockingPaneDirection direction, CXTPDockingPane* pNeighbour, CRect *pRect ) 
{
	CRect Rect;
	if( pRect == NULL ) Rect = CRect( 0, 0, 200, 200 );
	else Rect = *pRect;
	CXTPDockingPane* pwndPane = m_paneManager.CreatePane(
		nID, Rect, direction, pNeighbour);

	CString strTitle;
	strTitle.Format(strFormat, nID);
	pwndPane->SetTitle(strTitle);

	CFrameWnd* pFrame = new CFrameWnd;

	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;
	context.m_pCurrentDoc = NULL;

	DWORD dwStyle;
	dwStyle = WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	pFrame->Create(NULL, NULL, dwStyle, CRect(0, 0, 0, 0), this, NULL, 0, &context);
	pFrame->ModifyStyleEx( WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_OVERLAPPEDWINDOW, 0 );
	pFrame->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);


	m_mapPanes.SetAt(nID, pFrame);

	return pwndPane;
}

LRESULT CMainFrame::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_DPN_SHOWWINDOW)
	{
		// get a pointer to the docking pane being shown.
		CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;

		if (!pPane->IsValid())
		{
			CWnd* pWnd = NULL;
			if (m_mapPanes.Lookup(pPane->GetID(), pWnd))
			{
				pPane->Attach(pWnd);
			}
		}	
		return TRUE; // handled
	}

	if (wParam == XTP_DPN_CLOSEPANE)
	{
		CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;

		CWnd* pWnd =NULL;

		if (!m_mapPanes.Lookup(pPane->GetID(), pWnd))
			return FALSE;

		return TRUE;
	}

	return FALSE;
}

CWnd *CMainFrame::GetPaneFrame( int nID )
{
	if( XTPDockingPaneManager() == NULL ) return NULL;
	if( XTPDockingPaneManager()->GetSite() == NULL ) return NULL;
	CXTPDockingPane* pPane = XTPDockingPaneManager()->FindPane( nID );
	if( pPane == NULL ) return NULL;

	CWnd *pWnd = NULL;
	if (!m_mapPanes.Lookup(pPane->GetID(), pWnd) ) return NULL;

	CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, pWnd);
	return (CWnd*)pFrame->GetWindow( GW_CHILD );
	//	return pWnd;
}

int CMainFrame::GetFocusPaneID()
{
	int nCount = sizeof(g_PaneList) / sizeof(PaneCreateStruct);
	for( int i=0; i<nCount; i++ ) {
		CXTPDockingPane* pPane = XTPDockingPaneManager()->FindPane( g_PaneList[i].nPaneID );
		if( pPane->IsFocus() ) return g_PaneList[i].nPaneID;
	}
	return -1;
}

void CMainFrame::OnDestroy()
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	CXTPFrameWnd::OnDestroy();

}

void CMainFrame::OnClose()
{
#ifndef _DEBUG
	SaveCommandBars(_T("EtWorldPainterCommandBars"));
	CXTPDockingPaneLayout layoutNormal(&m_paneManager);
	m_paneManager.GetLayout(&layoutNormal);
	layoutNormal.Save(_T("EtWorldPainterLayout"));
#endif //_DEBUG

	if( CGlobalValue::GetInstance().CheckSaveMessage( GetSafeHwnd(), true ) == false ) return;
	/*
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	pView->SelectTabView(0);
	Sleep(1000);
	*/
	CWnd *pWnd = GetPaneWnd( SKINMNG_PANE );
	((CSkinManagerPaneView*)pWnd)->KillThread();

	pWnd = GetPaneWnd( SOUNDMNG_PANE );
	((CSoundManagerPaneView*)pWnd)->KillThread();

	pWnd = GetPaneWnd( TILEMNG_PANE );
	((CTileManagerPaneView*)pWnd)->KillThread();

	CFileServer::GetInstance().Disconnect();

	CXTPFrameWnd::OnClose();
}


void CMainFrame::OnUpdateProjectConnect(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->Enable( !CFileServer::GetInstance().IsConnect() );
}

void CMainFrame::OnUpdateProjectDisconnect(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CMainFrame::OnUpdateProjectSave(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	bool bEnable = true;
	if( CGlobalValue::GetInstance().m_bModify ) {
		if( CGlobalValue::GetInstance().m_nVecModifyTabList.size() == 0 ) bEnable = false;
		else if( CGlobalValue::GetInstance().m_nVecModifyTabList.size() == 1 &&
			CGlobalValue::GetInstance().m_nVecModifyTabList[0] == 0 ) bEnable = false;
	}
	else bEnable = false;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnUpdateProjectSetting(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->Enable( !CFileServer::GetInstance().IsConnect() );
}

CActionCommander *CMainFrame::GetCurActionCmd()
{
	CActionCommander *pCommander = NULL;
	switch( CGlobalValue::GetInstance().m_nActiveView ) {
		case 0:	break;
		case CGlobalValue::BRUSH:	pCommander = &s_BrushActionCommander;	break;
		case CGlobalValue::PROP:	pCommander = &s_PropActionCommander;	break;
		case CGlobalValue::NAVIGATION: pCommander = &s_NaviActionCommander;	break;
		case CGlobalValue::SOUND: pCommander = &s_SoundActionCommander;	break;
		case CGlobalValue::EVENT: pCommander = &s_EventActionCommander; break;

	}
	return pCommander;
}

void CMainFrame::OnUpdateUndo(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) pCmdUI->Enable( FALSE );
	else {
		pCmdUI->Enable( pCommander->IsCanUndo() );
	}

}

void CMainFrame::OnUpdateRedo(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) pCmdUI->Enable( FALSE );
	else {
		pCmdUI->Enable( pCommander->IsCanRedo() );
	}
}

void CMainFrame::OnUndo(NMHDR* pNMHDR, LRESULT* pResult)
{
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;

	int nCount = 0;
	if( pNMHDR == NULL ) nCount = 1;
	else {
		if (((NMXTPCONTROL*)pNMHDR)->pControl->GetType() == xtpControlSplitButtonPopup) nCount = 1;
		CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
		if (pControlListBox) nCount = pControlListBox->GetListCtrl()->GetSelCount();
	}

	for( int i=0; i<nCount; i++ ) {
		if( pCommander->Undo() == false ) {
			MessageBox( "언두 할 수 없삼!!", "에러", MB_OK );
		}
	}
}

void CMainFrame::OnRedo(NMHDR* pNMHDR, LRESULT* pResult)
{
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;

	int nCount = 0;
	if( pNMHDR == NULL ) nCount = 1;
	else {
		if (((NMXTPCONTROL*)pNMHDR)->pControl->GetType() == xtpControlSplitButtonPopup) nCount = 1;
		CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
		if (pControlListBox) nCount = pControlListBox->GetListCtrl()->GetSelCount();
	}

	for( int i=0; i<nCount; i++ ) {
		if( pCommander->Redo() == false ) {
			MessageBox( "리두 할 수 없삼!!", "에러", MB_OK );
		}
	}
}

void CMainFrame::OnUndoSelChange(NMHDR* pNMHDR, LRESULT* pRes)
{
	CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
	if (pControlListBox)
	{

		CXTPControlStatic* pInfo = FindInfoControl(pControlListBox);
		if (pInfo)
		{
			CString str;
			str.Format(_T("Undo %i Actions"), pControlListBox->GetListCtrl()->GetSelCount());
			pInfo->SetCaption(str);
			pInfo->DelayRedrawParent();
		}

		*pRes = 1;
	}
}

void CMainFrame::OnUndoPoup(NMHDR* pNMHDR, LRESULT* pRes)
{
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;

	CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
	if( pControlListBox ) {
		CListBox* pListBox = pControlListBox->GetListCtrl();
		pListBox->ResetContent();

		int nCount = pCommander->GetUndoCount();
		for( int i=0; i<nCount; i++ ) {
			pListBox->AddString( pCommander->GetUndoDesc(i) );
		}

		CXTPControlStatic* pInfo = FindInfoControl(pControlListBox);
		if (pInfo)
		{
			CString str;
			pInfo->SetCaption(_T("Undo 0 Actions"));
			pInfo->DelayRedrawParent();
		}

		*pRes = 1;
	}

}

void CMainFrame::OnRedoSelChange(NMHDR* pNMHDR, LRESULT* pRes)
{
	CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
	if (pControlListBox)
	{

		CXTPControlStatic* pInfo = FindInfoControl(pControlListBox);
		if (pInfo)
		{
			CString str;
			str.Format(_T("Redo %i Actions"), pControlListBox->GetListCtrl()->GetSelCount());
			pInfo->SetCaption(str);
			pInfo->DelayRedrawParent();
		}

		*pRes = 1;
	}
}

void CMainFrame::OnRedoPoup(NMHDR* pNMHDR, LRESULT* pRes)
{
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;

	CXTPControlListBox* pControlListBox = DYNAMIC_DOWNCAST(CXTPControlListBox, ((NMXTPCONTROL*)pNMHDR)->pControl);
	if( pControlListBox ) {
		CListBox* pListBox = pControlListBox->GetListCtrl();
		pListBox->ResetContent();

		int nCount = pCommander->GetRedoCount();
		for( int i=0; i<nCount; i++ ) {
			pListBox->AddString( pCommander->GetRedoDesc(i) );
		}

		CXTPControlStatic* pInfo = FindInfoControl(pControlListBox);
		if (pInfo)
		{
			CString str;
			pInfo->SetCaption(_T("Redo 0 Actions"));
			pInfo->DelayRedrawParent();
		}

		*pRes = 1;
	}
}


CXTPControlStatic* CMainFrame::FindInfoControl(CXTPControlListBox* pControlListBox)
{
	CXTPCommandBar* pCommandBar = pControlListBox->GetParent();

	for (int i = 0; i < pCommandBar->GetControls()->GetCount(); i++)
	{
		CXTPControlStatic* pControlStatic = DYNAMIC_DOWNCAST(CXTPControlStatic, pCommandBar->GetControl(i));
		if (pControlStatic && pControlStatic->GetID() == pControlListBox->GetID())
		{
			return pControlStatic;
		}

	}
	return NULL;
}

// Accel 에서 오는 메세지 ON_MESSAGE 와 XTP 의 ON_XTP_EXECUTE 가 호환되서 돌아가지지 않는다.
// 하지만 단축키가 아닐 경우엔 메세지가 중복되어 들어오기 때문에 문제가 생겨서
// Accel 용 ID 를 따로 만들고 따로 빼놓기로 함!!
void CMainFrame::OnUndoAccel()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;
	if( !pCommander->IsCanUndo() ) return;
	OnUndo( NULL, NULL );
}

void CMainFrame::OnRedoAccel()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CActionCommander *pCommander = GetCurActionCmd();
	if( pCommander == NULL ) return;
	if( !pCommander->IsCanRedo() ) return;
	OnRedo( NULL, NULL );
}

void CMainFrame::OnUpdateSimulRun(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->SetCheck( CGlobalValue::GetInstance().m_bSimulation );
	pCmdUI->Enable( CGlobalValue::GetInstance().m_bCanSimulation );
}

void CMainFrame::OnUpdateSimulStop(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	if( CGlobalValue::GetInstance().m_bCanSimulation == true && CGlobalValue::GetInstance().m_bSimulation == true ) {
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( TRUE );
	}
	else {
		pCmdUI->SetCheck( TRUE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnSimulRun()
{
	CTdDummyActor::GetInstance().BeginActor();
//	if( CGlobalValue::GetInstance().m_bSimulation == true ) return;
//	CGlobalValue::GetInstance().m_bSimulation = true;
}

void CMainFrame::OnSimulStop()
{
	CTdDummyActor::GetInstance().EndActor();
//	CGlobalValue::GetInstance().m_bSimulation = false;
}

void CMainFrame::OnGridCreategrid()
{
	// TODO: Add your command handler code here
	CFileServer::GetInstance().UpdateLoginList();
	if( CFileServer::GetInstance().GetLoginUserList()->size() > 0 ) {
		if( MessageBox( "이 작업은 로그인한 유저가 없어야 안전합니다.\n그래두 걍 만들래?", "에러", MB_YESNO ) == IDNO )
			return;
	}

	CCreateGridDlg Dlg;
	Dlg.DoModal();

	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) SendMessage( UM_REFRESH_PANE_VIEW );
	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnGridDeletegrid()
{
	// TODO: Add your command handler code here
	CFileServer::GetInstance().UpdateLoginList();
	if( CFileServer::GetInstance().GetLoginUserList()->size() > 0 ) {
		if( MessageBox( "이 작업은 로그인한 유저가 없어야 안전합니다.\n그래두 걍 지울래?", "에러", MB_YESNO ) == IDNO )
			return;
	}

	CDeleteGridDlg Dlg;
	if( Dlg.DoModal() != IDOK ) return;

	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) SendMessage( UM_REFRESH_PANE_VIEW );
	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnGridMovegrid()
{
	// TODO: Add your command handler code here
	CFileServer::GetInstance().UpdateLoginList();
	if( CFileServer::GetInstance().GetLoginUserList()->size() > 0 ) {
		if( MessageBox( "이 작업은 로그인한 유저가 없어야 안전합니다.\n그래두 걍 한번 옮길래?", "에러", MB_YESNO ) == IDNO )
			return;
	}

	CMoveToGridDlg Dlg;
	if( Dlg.DoModal() != IDOK ) return;

	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) SendMessage( UM_REFRESH_PANE_VIEW );
	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnUpdateGridCreategrid(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnUpdateGridDeletegrid(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnUpdateGridMovegrid(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnPropertiesDirectorysetting()
{
	// TODO: Add your command handler code here
	CPropertiesDlg Dlg;
	Dlg.DoModal();
}

void CMainFrame::OnUpdatePropertiesEventSetting(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnPropertiesEventSetting()
{
	if( CEventSignalManager::GetInstance().IsCanCheckOut() == false ) {
		MessageBox( "다른사람이 설정을 변경중입니다.", "에러", MB_OK );
		return;
	}

	if( CEventSignalManager::GetInstance().CheckOut() == false ) {
		MessageBox( "체크아웃 실패!!", "에러", MB_OK );
		return;
	}
	CCreateEventAreaDlg Dlg;
	Dlg.SetCurrentView( CEventEditWizardCommon::SELECT_EDIT_TYPE );

	if( Dlg.DoModal() == IDOK ) {
		if( CEventSignalManager::GetInstance().Save() == false ) {
			MessageBox( "이벤트 파라메터 정보 저장 실패", "에러", MB_OK );
		}
	}

	if( CEventSignalManager::GetInstance().CheckIn() == false ) {
		MessageBox( "체크인 실패!!", "에러", MB_OK );
		return;
	}
}

void CMainFrame::SetFocus( int nID )
{
	CXTPDockingPane* pPane = XTPDockingPaneManager()->FindPane( nID );
	XTPDockingPaneManager()->ShowPane( pPane );
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( nChar == VK_ESCAPE ) {
	}
}

void CMainFrame::OnUpdateToolsMakeMinimap(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnToolsMakeMinimap()
{
	CMakeMinimapDlg Dlg(this);
	if( Dlg.DoModal() != IDOK ) return;
}

void CMainFrame::OnUpdateToolsSaveCurrentLightmap(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	if( CFileServer::GetInstance().IsConnect() )
	{
		if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::PROP )
		{
			bEnable = TRUE;
		}
	}
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnToolsSaveCurrentLightmap()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;
	pSector->SaveLightmap();
}

void CMainFrame::OnUpdatePropertiesPropPropertySetting(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnPropertiesPropPropertySetting()
{
	if( CPropSignalManager::GetInstance().IsCanCheckOut() == false ) {
		MessageBox( "다른사람이 설정을 변경중입니다.", "에러", MB_OK );
		return;
	}

	if( CPropSignalManager::GetInstance().CheckOut() == false ) {
		MessageBox( "체크아웃 실패!!", "에러", MB_OK );
		return;
	}
	CCreatePropPropertyDlg Dlg(this);
	Dlg.SetCurrentView( CPropEditWizardCommon::SELECT_EDIT_TYPE );

	if( Dlg.DoModal() == IDOK ) {
		if( CPropSignalManager::GetInstance().Save() == false ) {
			MessageBox( "이벤트 파라메터 정보 저장 실패", "에러", MB_OK );
		}
	}

	if( CPropSignalManager::GetInstance().CheckIn() == false ) {
		MessageBox( "체크인 실패!!", "에러", MB_OK );
		return;
	}
}

void CMainFrame::OnUpdateToolsExportTerrainto3dsmaxscriptdata(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnUpdateToolsExportPropInfoto3dsmaxscriptdata(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CFileServer::GetInstance().IsConnect() == false ) bEnable = FALSE;
	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::GRID ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CMainFrame::OnToolsExportTerrainto3dsmaxscriptdata()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	CString szFileList;
	TCHAR szFilter[] = _T( "3ds max Script File(*.ms)|*.ms||All Files (*.*)|*.*||" );

	CFileDialog Dlg( FALSE, _T("ms"), _T("*.ms"), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );
	if( Dlg.DoModal() != IDOK ) return;

	bool bLoad = false;

	if( !pSector->IsLoaded() || !pSector->IsExistRWLevel( CTEtWorldSector::SRL_TERRAIN ) ) {
		pSector->LoadSector( CTEtWorldSector::SRL_TERRAIN );
		bLoad = true;
	}

	if( pSector->ExportTerrainToMaxScript( Dlg.GetPathName() ) == false ) {
		MessageBox( "익수포트 에러났으..", "에러", MB_OK );
	}

	if( bLoad ) {
		pSector->FreeSector( CTEtWorldSector::SRL_TERRAIN );
	}

}

void CMainFrame::OnToolsExportPropInfoto3dsmaxscriptdata()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	CExportPropInfoDlg exportdlg;
	if( exportdlg.DoModal() != IDOK ) return;

	CString szFileList;
	TCHAR szFilter[] = _T( "3ds max Script File(*.ms)|*.ms||All Files (*.*)|*.*||" );

	CFileDialog Dlg( FALSE, _T("ms"), _T("*.ms"), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );
	if( Dlg.DoModal() != IDOK ) return;


	bool bLoad = false;

	if( !pSector->IsLoaded() || !pSector->IsExistRWLevel( CTEtWorldSector::SRL_PROP ) ) {
		pSector->PropInfoFile( CTEtWorldSector::IFT_READ, true );
		bLoad = true;
	}

	CString szStr = exportdlg.GetMaxFolder();
	if( *szStr.GetBuffer() + szStr.GetLength() - 1 != '\\' ) szStr += "\\";

	if( pSector->ExportPropInfoToMaxScript( Dlg.GetPathName(), szStr.GetBuffer() ) == false ) {
		MessageBox( "익수포트 에러났으..", "에러", MB_OK );
	}

	if( bLoad ) {
		pSector->FreeSector( CTEtWorldSector::SRL_TERRAIN );
	}
}


void CMainFrame::OnToggleSimul()
{
	if( CGlobalValue::GetInstance().m_bSimulation )
		CTdDummyActor::GetInstance().EndActor();
	else CTdDummyActor::GetInstance().BeginActor();

	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnIncreaseBrushSize()
{
	CGlobalValue::GetInstance().m_nBrushDiameter++;

	CWnd *pWnd = GetPaneWnd( BRUSH_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnDecreaseBrushSize()
{
	CGlobalValue::GetInstance().m_nBrushDiameter--;

	CWnd *pWnd = GetPaneWnd( BRUSH_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().RefreshRender();
}

void CMainFrame::OnAttr1()
{
	switch( CGlobalValue::GetInstance().m_nActiveView ) {
		case CGlobalValue::BRUSH:
			{
				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 1 );
			}
			break;
		case CGlobalValue::NAVIGATION:
			CGlobalValue::GetInstance().m_cSelectAttribute = 0x01;
			break;
	}
}

void CMainFrame::OnAttr2()
{
	switch( CGlobalValue::GetInstance().m_nActiveView ) {
		case CGlobalValue::BRUSH:
			{
				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 2 );
			}
			break;
		case CGlobalValue::NAVIGATION:
			CGlobalValue::GetInstance().m_cSelectAttribute = 0x02;
			break;
	}
}

void CMainFrame::OnAttr3()
{
	switch( CGlobalValue::GetInstance().m_nActiveView ) {
		case CGlobalValue::BRUSH:
			{
				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 3 );
			}
			break;
		case CGlobalValue::NAVIGATION:
			CGlobalValue::GetInstance().m_cSelectAttribute = 0x04;
			break;
	}
}

void CMainFrame::OnAttr4()
{
	switch( CGlobalValue::GetInstance().m_nActiveView ) {
		case CGlobalValue::BRUSH:
			{
				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 4 );
			}
			break;
		case CGlobalValue::NAVIGATION:
			CGlobalValue::GetInstance().m_cSelectAttribute = 0x08;
			break;
	}
}

void CMainFrame::OnToggleTopView()
{
	if( !CRenderBase::IsActive() ) return;
	CRenderBase::GetInstance().ToggleCameraTopView();
}

void CMainFrame::OnUpdateToggleTopView(CCmdUI *pCmdUI)
{
	bool bEnable = true;
	if( !CFileServer::GetInstance().IsConnect() ) bEnable = false;
	if( CGlobalValue::GetInstance().m_nActiveView == 0 ) bEnable = false;
	pCmdUI->Enable( bEnable );
}


void CMainFrame::OnViewCameraLight()
{
	CTEtWorldSector::ms_IsFiltering = !CTEtWorldSector::ms_IsFiltering;
	CTEtWorldSector::InitFilteringNames();

	if (CTEtWorldSector::ms_IsFiltering)
	{
		CTEtWorldSector::AddPropNameForFiltering("Light.skn");
		CTEtWorldSector::AddPropNameForFiltering("Camera.skn");
	}

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	
	switch( CGlobalValue::GetInstance().m_nActiveView ) 
	{
	case CGlobalValue::PROP:
	case CGlobalValue::WATER:
	case CGlobalValue::NAVIGATION:
	case CGlobalValue::EVENT:
	case CGlobalValue::SOUND:
	case CGlobalValue::TRIGGER:
	case CGlobalValue::RDUNGEON:
		{
			pSector->ShowProp(true);
			CGlobalValue::GetInstance().RefreshRender();
		}
		break;
	}
}

void CMainFrame::OnUpdateViewCameraLight(CCmdUI *pCmdUI)
{
	bool bEnable = true;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) bEnable = false;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) bEnable = false;

// 	if (CGlobalValue::PROP != CGlobalValue::GetInstance().m_nActiveView ) 
// 		bEnable = false;

	pCmdUI->Enable( bEnable );
	pCmdUI->SetCheck( CTEtWorldSector::ms_IsFiltering );
}
