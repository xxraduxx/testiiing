// DummyView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "DummyView.h"
#include "GlobalValue.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"

#include "GridRenderView.h"
#include "BrushRenderView.h"
#include "PropRenderView.h"
#include "EventRenderView.h"
#include "RandomDungeonRenderView.h"
#include "SoundRenderView.h"
#include "TriggerRenderView.h"
#include "NaviRenderView.h"
#include "WaterRenderView.h"

#include "PerfCheck.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"


// CDummyView

IMPLEMENT_DYNCREATE(CDummyView, CFormView)

CDummyView::CDummyView()
	: CFormView(CDummyView::IDD)
{
	m_pTabViewItem = NULL;

}
 
CDummyView::~CDummyView()
{
	SAFE_DELETEA( m_pTabViewItem );
}

void CDummyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDummyView, CFormView)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CDummyView 진단입니다.

#ifdef _DEBUG
void CDummyView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDummyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CDummyView 메시지 처리기입니다.


void CDummyView::InitView( CXTPTabControl *pControl )
{
	CRect rcRect;
	GetClientRect( &rcRect );

	CCreateContext cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.m_pCurrentDoc = GetDocument();

	struct TabViewInitStruct {
		CGlobalValue::VIEW_TYPE nTabIndex;
		char *szString;
		CRuntimeClass *pRuntimeClass;
		CWnd *pWnd;
	};
	TabViewInitStruct TabViewInitList[] = {
		{ CGlobalValue::BRUSH, "Brush", RUNTIME_CLASS(CBrushRenderView), NULL },
		{ CGlobalValue::PROP, "Prop", RUNTIME_CLASS(CPropRenderView), NULL },
		{ CGlobalValue::WATER, "Water", RUNTIME_CLASS(CWaterRenderView), NULL },
		{ CGlobalValue::NAVIGATION, "Navigation", RUNTIME_CLASS(CNaviRenderView), NULL },
		{ CGlobalValue::EVENT, "Event", RUNTIME_CLASS(CEventRenderView), NULL },
		{ CGlobalValue::SOUND, "Sound", RUNTIME_CLASS(CSoundRenderView), NULL },
		{ CGlobalValue::TRIGGER, "Trigger", RUNTIME_CLASS(CTriggerRenderView), NULL },
		{ CGlobalValue::RDUNGEON, "Random Dungeon", RUNTIME_CLASS(CRandomDungeonRenderView), NULL },
		{ CGlobalValue::GRID, "Grid", RUNTIME_CLASS(CGridRenderView), NULL },
	};

	int nCount = sizeof(TabViewInitList) / sizeof(TabViewInitStruct);
	TabViewStruct Struct;
	m_pTabViewItem = new TabViewStruct[nCount];
	for( int i=0; i<nCount; i++ ) {
		if( TabViewInitList[i].pRuntimeClass ) {
			TabViewInitList[i].pWnd = (CWnd*)TabViewInitList[i].pRuntimeClass->CreateObject();
			TabViewInitList[i].pWnd->Create( NULL, NULL, WS_CHILD, rcRect, pControl, 2000 + i, &cc );
		}

		Struct.pItem = pControl->InsertItem( TabViewInitList[i].nTabIndex, TabViewInitList[i].szString, ( TabViewInitList[i].pWnd == NULL ) ? NULL : TabViewInitList[i].pWnd->m_hWnd );
		Struct.pView = TabViewInitList[i].pWnd;
		m_pTabViewItem[ TabViewInitList[i].nTabIndex ] = Struct;
	}

	pControl->SetCurSel(0);
}


void CDummyView::Refresh( int nActiveView, bool bForceRefresh )
{
	CWnd *pWnd = m_pTabViewItem[CGlobalValue::GetInstance().m_nActiveView].pView;
	if( pWnd ) 
		pWnd->SendMessage( UM_REFRESH_VIEW, (WPARAM)bForceRefresh );
//	Invalidate();

	g_nPerfCounter = (int)CRenderBase::GetInstance().GetLocalTime();
}

void CDummyView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
//	CRenderBase::GetInstance().Initialize( m_hWnd );
//	Invalidate();
}

void CDummyView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
}

void CDummyView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

}

BOOL CDummyView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return FALSE;
}



LRESULT CDummyView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( message >= WM_USER && m_pTabViewItem ) {
		int nIndex = CGlobalValue::GetInstance().m_nActiveView;
		if( m_pTabViewItem[nIndex].pView )
			m_pTabViewItem[nIndex].pView->SendMessage( message, wParam, lParam );
	}

	return CFormView::WindowProc(message, wParam, lParam);
}


void CDummyView::SetSelectChangeView( int nActiveView )
{
	CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = NULL;
	if( Sector != -1 )
		pSector = CTEtWorld::GetInstance().GetSector( Sector );

	int nPrevView = CGlobalValue::GetInstance().m_nActiveView;
	if( nPrevView != -1 && m_pTabViewItem[nPrevView].pView ) {
		m_pTabViewItem[nPrevView].pView->SendMessage( UM_SELECT_CHANGE_CLOSE_VIEW, nActiveView );
	}

	CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );

	if( pSector ) {
		int nLoadLevel = CGlobalValue::GetInstance().m_ViewTypeLoadLevel[nActiveView];
		int nCurLoadLevel = CGlobalValue::GetInstance().m_ViewTypeLoadLevel[nPrevView];
		if( nLoadLevel & CTEtWorldSector::SRL_EMPTY )
			pSector->FreeSector( nCurLoadLevel );

		if( (nLoadLevel & CTEtWorldSector::SRL_TERRAIN) != (nCurLoadLevel & CTEtWorldSector::SRL_TERRAIN) )
			pSector->FreeSector( CTEtWorldSector::SRL_TERRAIN );
		if( (nLoadLevel & CTEtWorldSector::SRL_PROP) != (nCurLoadLevel & CTEtWorldSector::SRL_PROP) && !CGlobalValue::GetInstance().m_bLoadedProp )
			pSector->FreeSector( CTEtWorldSector::SRL_PROP );
		if( (nLoadLevel & CTEtWorldSector::SRL_EVENT) != (nCurLoadLevel & CTEtWorldSector::SRL_EVENT) )
			pSector->FreeSector( CTEtWorldSector::SRL_EVENT );
		if( (nLoadLevel & CTEtWorldSector::SRL_SOUND) != (nCurLoadLevel & CTEtWorldSector::SRL_SOUND) )
			pSector->FreeSector( CTEtWorldSector::SRL_SOUND );
		if( (nLoadLevel & CTEtWorldSector::SRL_ATTRIBUTE) != (nCurLoadLevel & CTEtWorldSector::SRL_ATTRIBUTE) )
			pSector->FreeSector( CTEtWorldSector::SRL_ATTRIBUTE );
		if( (nLoadLevel & CTEtWorldSector::SRL_NAVIGATION) != (nCurLoadLevel & CTEtWorldSector::SRL_NAVIGATION) )
			pSector->FreeSector( CTEtWorldSector::SRL_NAVIGATION );
		if( (nLoadLevel & CTEtWorldSector::SRL_TRIGGER) != (nCurLoadLevel & CTEtWorldSector::SRL_TRIGGER) )
			pSector->FreeSector( CTEtWorldSector::SRL_TRIGGER );
		if( (nLoadLevel & CTEtWorldSector::SRL_WATER) != (nCurLoadLevel & CTEtWorldSector::SRL_WATER) )
			pSector->FreeSector( CTEtWorldSector::SRL_WATER );

		pSector->LoadSector( nLoadLevel );
		/*
		if( nLoadLevel & CTEtWorldSector::SRL_TERRAIN ) pSector->LoadSector( CTEtWorldSector::SRL_TERRAIN );
		if( nLoadLevel & CTEtWorldSector::SRL_PROP ) pSector->LoadSector( CTEtWorldSector::SRL_PROP );
		if( nLoadLevel & CTEtWorldSector::SRL_EVENT ) pSector->LoadSector( CTEtWorldSector::SRL_EVENT );
		if( nLoadLevel & CTEtWorldSector::SRL_SOUND ) pSector->LoadSector( CTEtWorldSector::SRL_SOUND );
		if( nLoadLevel & CTEtWorldSector::SRL_ATTRIBUTE ) pSector->LoadSector( CTEtWorldSector::SRL_ATTRIBUTE );
		if( nLoadLevel & CTEtWorldSector::SRL_NAVIGATION ) pSector->LoadSector( CTEtWorldSector::SRL_NAVIGATION );
		*/
	}


	CGlobalValue::GetInstance().m_nActiveView = nActiveView;
	if( nActiveView != -1 && m_pTabViewItem[nActiveView].pView ) m_pTabViewItem[nActiveView].pView->SendMessage( UM_SELECT_CHANGE_OPEN_VIEW, nPrevView );
	if( pSector ) pSector->ShowWarningMesasge();
}
