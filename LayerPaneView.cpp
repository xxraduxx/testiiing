#include "stdafx.h"
#include "EtWorldPainter.h"
#include "LayerPaneView.h"

#include "LayerPrevView.h"
#include "LayerHeightView.h"
#include "LayerGrassView.h"
#include "LayerDecalView.h"
#include "UserMessage.h"
#include "UnionValueProperty.h"

#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "TEtWorldDecal.h"

// CLayerPaneView

IMPLEMENT_DYNCREATE(CLayerPaneView, CFormView)

CLayerPaneView::CLayerPaneView()
	: CFormView(CLayerPaneView::IDD)
{
	m_pLayerView = NULL;
	m_pLayerHeight = NULL;
	m_pLayerGrass = NULL;
	m_bActivate = false;
	m_nSelectLayer = 0;
}

CLayerPaneView::~CLayerPaneView()
{
}

void CLayerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CLayerPaneView, CFormView)
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_LAYERPANE_CHANGE_LAYER, OnSelectChangeLayer )
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CLayerPaneView 진단입니다.

#ifdef _DEBUG
void CLayerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CLayerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLayerPaneView 메시지 처리기입니다.

int CLayerPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_TabCtrl.Create(WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), this, 1001);
	m_TabCtrl.GetPaintManager()->m_bDisableLunaColors = TRUE;
	m_TabCtrl.SetAppearance(xtpTabAppearanceStateButtons);
	m_TabCtrl.GetPaintManager()->m_bHotTracking = TRUE;
	m_TabCtrl.GetPaintManager()->m_bRotateImageOnVerticalDraw = TRUE;
	m_TabCtrl.SetPosition( xtpTabPositionLeft );

	CCreateContext cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.m_pCurrentDoc = GetDocument();

	m_pLayerView = (CLayerPrevView*)RUNTIME_CLASS(CLayerPrevView)->CreateObject();	
	((CWnd *)m_pLayerView)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2005, &cc );

	m_pLayerHeight = (CLayerHeightView*)RUNTIME_CLASS(CLayerHeightView)->CreateObject();	
	((CWnd *)m_pLayerHeight)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2006, &cc );

	m_pLayerGrass = (CLayerGrassView*)RUNTIME_CLASS(CLayerGrassView)->CreateObject();	
	((CWnd *)m_pLayerGrass)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2007, &cc );

	m_pLayerDecal = (CLayerDecalView*)RUNTIME_CLASS(CLayerDecalView)->CreateObject();	
	((CWnd *)m_pLayerDecal)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2008, &cc );


	m_TabCtrl.InsertItem( 1, _T("Layer1"), m_pLayerView->m_hWnd );
	m_TabCtrl.InsertItem( 2, _T("Layer2"), m_pLayerView->m_hWnd );
	m_TabCtrl.InsertItem( 3, _T("Layer3"), m_pLayerView->m_hWnd );
	m_TabCtrl.InsertItem( 4, _T("Layer4"), m_pLayerView->m_hWnd );
	m_TabCtrl.InsertItem( 5, _T("Grass"), m_pLayerGrass->m_hWnd );
	m_TabCtrl.InsertItem( 6, _T("Decal"), m_pLayerDecal->m_hWnd );
	m_TabCtrl.InsertItem( 0, _T("Height"), m_pLayerHeight->m_hWnd );


	return 0;
}

void CLayerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CLayerPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( ::IsWindow(m_TabCtrl.m_hWnd) ) {
		m_TabCtrl.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}

}

BOOL CLayerPaneView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if( (UINT)wParam == 1001 ) {
		NMHDR* pNMHDR = (NMHDR*)lParam;

		switch(pNMHDR->code)
		{
		case TCN_SELCHANGING:
			break;

		case TCN_SELCHANGE:
			m_nSelectLayer = m_TabCtrl.GetCurSel();
			CGlobalValue::GetInstance().m_nSelectLayer = m_nSelectLayer;
			CGlobalValue::GetInstance().m_bCanDragTile = false;
			if( m_nSelectLayer > 0 && m_nSelectLayer < 5 ) {
				m_pLayerView->SetLayerIndex( m_nSelectLayer - 1 );
				m_pLayerView->SendMessage( UM_REFRESH_PANE_VIEW );
			}
			else if( m_nSelectLayer == 5 ) {
				m_pLayerGrass->SendMessage( UM_REFRESH_PANE_VIEW );
				CGlobalValue::GetInstance().m_bCanDragTile = true;
			}
			else if( m_nSelectLayer == 6 ) {
				m_pLayerDecal->SendMessage( UM_REFRESH_PANE_VIEW );
				CGlobalValue::GetInstance().m_bCanDragTile = true;
			}

			if( m_nSelectLayer != 6 ) {
				if( CGlobalValue::GetInstance().m_pSelectDecal ) {
					CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
					CGlobalValue::GetInstance().m_pSelectDecal = NULL;
				}
			}
			CGlobalValue::GetInstance().RefreshRender();
			break;
		}
	}
	return CView::OnNotify(wParam, lParam, pResult);
}

LRESULT CLayerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( wParam == 1 ) {
		m_pLayerView->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );
		return S_OK;;
	}
	switch( m_nSelectLayer ) {
		case 0:
			m_pLayerHeight->Invalidate();
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			m_pLayerView->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );
			break;
		case 5:
			m_pLayerGrass->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );
			break;
		case 6:
			m_pLayerDecal->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );
			break;
	}
	return S_OK;
}
BOOL CLayerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

//	return CFormView::OnEraseBkgnd(pDC);
	return TRUE;
}


LRESULT CLayerPaneView::OnSelectChangeLayer( WPARAM wParam, LPARAM lParam )
{
	int nLayer = (int)wParam;
	m_TabCtrl.SetCurSel( nLayer );
	m_nSelectLayer = nLayer;
	CGlobalValue::GetInstance().m_nSelectLayer = m_nSelectLayer;
	CGlobalValue::GetInstance().m_bCanDragTile = false;
	if( m_nSelectLayer > 0 && m_nSelectLayer < 5 ) {
		m_pLayerView->SetLayerIndex( m_nSelectLayer - 1 );
		m_pLayerView->SendMessage( UM_REFRESH_PANE_VIEW );
	}
	else if( m_nSelectLayer == 5 ) {
		m_pLayerGrass->SendMessage( UM_REFRESH_PANE_VIEW );
		CGlobalValue::GetInstance().m_bCanDragTile = true;
	}
	else if( m_nSelectLayer == 6 ) {
		m_pLayerDecal->SendMessage( UM_REFRESH_PANE_VIEW );
		CGlobalValue::GetInstance().m_bCanDragTile = true;
	}

	if( m_nSelectLayer != 6 ) {
		if( CGlobalValue::GetInstance().m_pSelectDecal ) {
			CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
			CGlobalValue::GetInstance().m_pSelectDecal = NULL;
		}
	}
	return S_OK;
}