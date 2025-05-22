// EtWorldPainterView.cpp : CEtWorldPainterView 클래스의 구현
//

#include "stdafx.h"
#include "EtWorldPainter.h"

#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"

#include "RenderBase.h"
#include "FileServer.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "DummyView.h"
#include "UserMessage.h"
#include "TdDummyActor.h"
#include "PropertiesDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEtWorldPainterView

IMPLEMENT_DYNCREATE(CEtWorldPainterView, CView)

BEGIN_MESSAGE_MAP(CEtWorldPainterView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CEtWorldPainterView 생성/소멸

CEtWorldPainterView::CEtWorldPainterView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	m_pFlatTabCtrl = NULL;
	m_pDummyView = NULL;
	m_nActiveView = 0;
}

CEtWorldPainterView::~CEtWorldPainterView()
{
	/*
	CWnd *pWnd = GetPaneWnd( SKINMNG_PANE );
	((CSkinManagerPaneView*)pWnd)->KillThread();

	pWnd = GetPaneWnd( SOUNDMNG_PANE );
	((CSoundManagerPaneView*)pWnd)->KillThread();

	pWnd = GetPaneWnd( TILEMNG_PANE );
	((CTileManagerPaneView*)pWnd)->KillThread();
	*/


	CRenderBase::GetInstance().Finalize();
	SAFE_DELETE( m_pFlatTabCtrl );
}

BOOL CEtWorldPainterView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CEtWorldPainterView 그리기

void CEtWorldPainterView::OnDraw(CDC* /*pDC*/)
{
	CEtWorldPainterDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CEtWorldPainterView 인쇄

BOOL CEtWorldPainterView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CEtWorldPainterView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CEtWorldPainterView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CEtWorldPainterView 진단

#ifdef _DEBUG
void CEtWorldPainterView::AssertValid() const
{
	CView::AssertValid();
}

void CEtWorldPainterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CEtWorldPainterDoc* CEtWorldPainterView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEtWorldPainterDoc)));
	return (CEtWorldPainterDoc*)m_pDocument;
}
#endif //_DEBUG

int CEtWorldPainterView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_pFlatTabCtrl = new CXTPTabControl;
	m_pFlatTabCtrl->GetPaintManager()->m_bDisableLunaColors = FALSE;
	m_pFlatTabCtrl->SetAppearance(xtpTabAppearancePropertyPage2003);
	m_pFlatTabCtrl->GetPaintManager()->m_bHotTracking = TRUE;
	m_pFlatTabCtrl->GetPaintManager()->m_bOneNoteColors = TRUE;
	m_pFlatTabCtrl->GetPaintManager()->m_bBoldSelected = TRUE;
	m_pFlatTabCtrl->GetPaintManager()->SetColor( xtpTabColorOffice2003 );

	CCreateContext cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.m_pCurrentDoc = GetDocument();

	CRect rcRect;
	GetClientRect( &rcRect );
	// Create  the flat tab control.
	if (!m_pFlatTabCtrl->Create(WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, rcRect, this, 1001))
	{
		TRACE0( "Failed to create flattab control\n" );
		return 0;
	}

	m_pDummyView = (CDummyView*)RUNTIME_CLASS(CDummyView)->CreateObject();
	((CWnd *)m_pDummyView)->Create( NULL, NULL, WS_CHILD|WS_VISIBLE, rcRect, m_pFlatTabCtrl, 1995, &cc );

	m_pDummyView->InitView( m_pFlatTabCtrl );

	return 0;
}

void CEtWorldPainterView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_pFlatTabCtrl && ::IsWindow(m_pFlatTabCtrl->m_hWnd))
	{
		m_pFlatTabCtrl->SetWindowPos(NULL, 0,0,cx,cy, SWP_FRAMECHANGED);
		CXTPTabManagerItem *pItem = m_pFlatTabCtrl->GetItem( m_pFlatTabCtrl->GetCurSel() );

		CRenderBase::GetInstance().Lock();
		CRenderBase::GetInstance().Reconstruction();
		CRenderBase::GetInstance().Unlock();
	}
}

void CEtWorldPainterView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CGlobalValue::GetInstance().m_pParentView = this;
	EnableTabView( -1, false );

	char szStr1[2048] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ShaderFolder", szStr1, 2048 );

	if( strlen( szStr1 ) == 0 ) {
		CPropertiesDlg Dlg;
		Dlg.m_bDisableCancel = true;
		Dlg.DoModal();
	}

	CRenderBase::GetInstance().Initialize( m_hWnd );
}

BOOL CEtWorldPainterView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return FALSE;
}

void CEtWorldPainterView::Refresh()
{
	if( m_pDummyView == NULL ) return;
	// 각 Context 별 view 가 하나밖에 없기 때문에 ActiveView 인덱스를 넘겨서 RenderBase 에서 처리해준다.
	m_pDummyView->Refresh( m_nActiveView );
}

void CEtWorldPainterView::EnableTabView( int nTabIndex, bool bEnable ) 
{
	if( nTabIndex == -1 ) {
		for( int i=0; i<CGlobalValue::VIEW_TYPE_AMOUNT; i++ ) {
			m_pDummyView->GetTabViewStruct(i)->pItem->SetEnabled( bEnable );
		}
	}
	else {
		m_pDummyView->GetTabViewStruct(nTabIndex)->pItem->SetEnabled( bEnable );
	}
	m_pDummyView->GetTabViewStruct(CGlobalValue::RDUNGEON)->pItem->SetEnabled( false );
}

void CEtWorldPainterView::SelectTabView( int nTabIndex )
{
	if( m_pFlatTabCtrl ) m_pFlatTabCtrl->SetCurSel( nTabIndex );
	if( m_pDummyView ) m_pDummyView->SetSelectChangeView( nTabIndex );
	m_nActiveView = nTabIndex;
}

BOOL CEtWorldPainterView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if( (UINT)wParam == 1001 ) {
		NMHDR* pNMHDR = (NMHDR*)lParam;

		switch(pNMHDR->code) {
			case TCN_SELCHANGING:
				break;

			case TCN_SELCHANGE:
				{
					if( m_pFlatTabCtrl->GetCurSel() == m_nActiveView ) break;
					m_nActiveView = m_pFlatTabCtrl->GetCurSel();
					if( m_pDummyView ) m_pDummyView->SetSelectChangeView( m_nActiveView );
					if( !((CMainFrame*)AfxGetMainWnd()) ) break;

					int nCount = sizeof(g_PaneList) / sizeof(PaneCreateStruct);
					for( int i=0; i<nCount; i++ ) {
						if( g_PaneList[i].bAutoShowHide == TRUE ) {
							((CMainFrame*)AfxGetMainWnd())->XTPDockingPaneManager()->ClosePane( g_PaneList[i].nPaneID );
						}
					}
					// Manager Explorer 리셋시켜준다.
					CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
					if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

					CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
					CXTPDockingPaneManager *pManager = pFrame->XTPDockingPaneManager();

					CTdDummyActor::GetInstance().EndActor();

					switch( m_nActiveView ) {
						case CGlobalValue::GRID:
							CGlobalValue::GetInstance().m_bCanSimulation = false;
							pManager->ShowPane( GRIDLIST_PANE );
							pManager->ShowPane( GRID_PANE );
							pManager->ShowPane( LOGINLIST_PANE );
							break;
						case CGlobalValue::BRUSH:	// Brush
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( ENVI_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( DECALLIST_PANE );
								pManager->ShowPane( BRUSH_PANE );
								pManager->ShowPane( SECTOR_PANE );
								pManager->ShowPane( LAYER_PANE );
								pManager->ShowPane( TILEMNG_PANE );
								pManager->ShowPane( MNG_EXPLORER_PANE );

								// Manager Explorer Attach
								CXTPDockingPane *pPane1 = pManager->FindPane( MNG_EXPLORER_PANE );
								CXTPDockingPane *pPane2 = pManager->FindPane( TILEMNG_PANE );
								pManager->DockPane( pPane1, xtpPaneDockLeft, pPane2 );

								CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
								pWnd = GetPaneWnd( TILEMNG_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

								// Brush Pane Update Using Button
								pWnd = GetPaneWnd( BRUSH_PANE );
								if( pWnd ) pWnd->SendMessage( UM_BRUSHPANE_UPDATE_USE_BUTTON, 0 );
								// Environment Refresh
								pWnd = GetPaneWnd( ENVI_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
							}
							break;
						case CGlobalValue::PROP:	// Prop
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( ENVI_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( PROPLIST_PANE );
								pManager->ShowPane( SKINMNG_PANE );
								pManager->ShowPane( MNG_EXPLORER_PANE );
								pManager->ShowPane( BRUSH_PANE );
								pManager->ShowPane( PROPPROP_PANE );
								pManager->ShowPane( PROPPOOL_PANE );
								pManager->ShowPane( IGNOREPROP_PANE );

								// Manager Explorer Attach
								CXTPDockingPane *pPane1 = pManager->FindPane( MNG_EXPLORER_PANE );
								CXTPDockingPane *pPane2 = pManager->FindPane( SKINMNG_PANE );
								//CXTPDockingPane *pPane3 = pManager->FindPane( IGNOREPROP_PANE );
								//pManager->DockPane( pPane3, xtpPaneDockBottom, pPane2 );
								pManager->DockPane( pPane1, xtpPaneDockLeft, pPane2 );

								CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2 );
								pWnd = GetPaneWnd( SKINMNG_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)"Prop" );

								// Brush Pane Update Using Button
								pWnd = GetPaneWnd( BRUSH_PANE );
								if( pWnd ) pWnd->SendMessage( UM_BRUSHPANE_UPDATE_USE_BUTTON, 1 );
								// Environment Refresh
								pWnd = GetPaneWnd( ENVI_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
								// Ignore Prop Folder Refresh
								pWnd = GetPaneWnd( IGNOREPROP_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
							}
							break;
						case CGlobalValue::WATER:	// Water
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;

								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( ENVI_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( TILEMNG_PANE );
								pManager->ShowPane( MNG_EXPLORER_PANE );

								pManager->ShowPane( WATER_PANE );
								pManager->ShowPane( WATERPROP_PANE );

								// Manager Explorer Attach
								CXTPDockingPane *pPane1 = pManager->FindPane( MNG_EXPLORER_PANE );
								CXTPDockingPane *pPane2 = pManager->FindPane( TILEMNG_PANE );
								pManager->DockPane( pPane1, xtpPaneDockLeft, pPane2 );

								CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
								pWnd = GetPaneWnd( TILEMNG_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
							}
							break;
						case CGlobalValue::NAVIGATION:	// Navigation
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( BRUSH_PANE );
								pManager->ShowPane( NAVIPROP_PANE );

								CWnd *pWnd = GetPaneWnd( NAVIPROP_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
							}
							break;
						case CGlobalValue::EVENT:	// Event
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( ENVI_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( EVENT_PANE );
								pManager->ShowPane( EVENTPROP_PANE );

								CWnd *pWnd = GetPaneWnd( EVENT_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

							}
							break;
						case CGlobalValue::SOUND: // Sound
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( RENDEROPTION_PANE );
								pManager->ShowPane( ENVI_PANE );
								pManager->ShowPane( CAMERA_PANE );
								pManager->ShowPane( SOUNDLIST_PANE );
								pManager->ShowPane( SOUNDPROP_PANE );
								pManager->ShowPane( SOUNDMNG_PANE );
								pManager->ShowPane( MNG_EXPLORER_PANE );

								// Manager Explorer Attach
								CXTPDockingPane *pPane1 = pManager->FindPane( MNG_EXPLORER_PANE );
								CXTPDockingPane *pPane2 = pManager->FindPane( SOUNDMNG_PANE );
								pManager->DockPane( pPane1, xtpPaneDockLeft, pPane2 );

								CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 3 );
								pWnd = GetPaneWnd( SOUNDMNG_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

								// Environment Refresh
								pWnd = GetPaneWnd( ENVI_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

								pWnd = GetPaneWnd( SOUNDLIST_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

							}
							break;
						case CGlobalValue::TRIGGER:
							{
								CGlobalValue::GetInstance().m_bCanSimulation = true;
								pManager->ShowPane( PROPLIST_PANE );
								pManager->ShowPane( EVENT_PANE );
								pManager->ShowPane( TRIGGER_VALUE_PANE );
								pManager->ShowPane( TRIGGER_PANE );
								pManager->ShowPane( TRIGGER_ATTR_PANE );
								pManager->ShowPane( TRIGGER_PROP_PANE );

								CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

								pWnd = GetPaneWnd ( EVENT_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

							}
							break;
						case CGlobalValue::RDUNGEON:	// Random Dungeon
							CGlobalValue::GetInstance().m_bCanSimulation = true;

							pManager->ShowPane( RANDOM_DEFAULT_PANE );

							break;
					}
					CGlobalValue::GetInstance().RefreshRender();
				}
				break;
		}
	}
	return CView::OnNotify(wParam, lParam, pResult);
}


void CEtWorldPainterView::ChangeModify( int nTabIndex, bool bModify )
{
	if( !m_pFlatTabCtrl ) return;
	CString szStr;
	CDummyView::TabViewStruct *pStruct = m_pDummyView->GetTabViewStruct( nTabIndex );
	if( pStruct->pItem == NULL ) return;
	szStr = pStruct->pItem->GetCaption();
	if( bModify == true ) {
		if( strstr( szStr, "* " ) != NULL ) return;
		CString szTemp;
		szTemp.Format( "* %s", szStr );
		pStruct->pItem->SetCaption( szTemp );
	}
	else {
		if( strstr( szStr, "* " ) == NULL ) return;
		char szTemp[64] = { 0, };
		strncpy_s( szTemp, szStr.GetBuffer() + 2, szStr.GetLength() - 2 );
		pStruct->pItem->SetCaption( szTemp );
	}
}


LRESULT CEtWorldPainterView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_pDummyView->Refresh( m_nActiveView, ( wParam == 0 ) ? false : true );
	return S_OK;
}