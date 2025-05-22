// LayerGrassView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "LayerGrassView.h"
#include "GlobalValue.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "PaneDefine.h"
#include "resource.h"
#include "MainFrm.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "UserMessage.h"


// CLayerGrassView
PropertyGridBaseDefine g_GrassInfoPropertyDefine[] = {
	{ "Common", "Shake Min", CUnionValueProperty::Float, "흔들림 최소값|1.f|100.f|1.f", TRUE },
	{ "Common", "Shake Max", CUnionValueProperty::Float, "흔들림 최대값|1.f|100.f|1.f", TRUE },
	{ "Grass #1", "Width", CUnionValueProperty::Float, "1번풀 가로사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #1", "Height - Min", CUnionValueProperty::Float, "1번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #1", "Height - Max", CUnionValueProperty::Float, "1번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #2", "Width", CUnionValueProperty::Float, "2번풀 가로사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #2", "Height - Min", CUnionValueProperty::Float, "2번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #2", "Height - Max", CUnionValueProperty::Float, "2번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #3", "Width", CUnionValueProperty::Float, "3번풀 가로사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #3", "Height - Min", CUnionValueProperty::Float, "3번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #3", "Height - Max", CUnionValueProperty::Float, "3번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #4", "Width", CUnionValueProperty::Float, "4번풀 가로사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #4", "Height - Min", CUnionValueProperty::Float, "4번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ "Grass #4", "Height - Max", CUnionValueProperty::Float, "4번풀 세로 최소 사이즈|1.f|1000.f|1.f", TRUE },
	{ NULL },
};


IMPLEMENT_DYNCREATE(CLayerGrassView, CFormView)

CLayerGrassView::CLayerGrassView()
	: CFormView(CLayerGrassView::IDD)
{
	m_bActivate = false;

	m_pdcBitmap = NULL;
	m_pmemDC = NULL;
	m_nTextureCount = 1;
	m_nSelectTextureIndex = -1;
	m_nSelectBlockIndex = -1;
	m_nDrawFlag = 0;
	m_ScrollPos = m_ScrollSize = CSize( 0, 0 );

	m_nSlotWidth = 165;
	m_nSlotHeight = 130;	
}

CLayerGrassView::~CLayerGrassView()
{
	DeleteMemDC();
}

void CLayerGrassView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLayerGrassView, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_TILEPANE_DROPITEM, OnDropItem )
	ON_MESSAGE( UM_TILEPANE_DROPMOVEITEM, OnDropMoveItem )
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_GRASS_1, &CLayerGrassView::OnGrass1)
	ON_COMMAND(ID_GRASS_2, &CLayerGrassView::OnGrass2)
	ON_COMMAND(ID_GRASS_3, &CLayerGrassView::OnGrass3)
	ON_COMMAND(ID_GRASS_4, &CLayerGrassView::OnGrass4)
	ON_UPDATE_COMMAND_UI(ID_GRASS_1, &CLayerGrassView::OnUpdateGrass1)
	ON_UPDATE_COMMAND_UI(ID_GRASS_2, &CLayerGrassView::OnUpdateGrass2)
	ON_UPDATE_COMMAND_UI(ID_GRASS_3, &CLayerGrassView::OnUpdateGrass3)
	ON_UPDATE_COMMAND_UI(ID_GRASS_4, &CLayerGrassView::OnUpdateGrass4)
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
	ON_COMMAND(ID_RELOAD, &CLayerGrassView::OnReload)
END_MESSAGE_MAP()


// CLayerGrassView diagnostics

#ifdef _DEBUG
void CLayerGrassView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CLayerGrassView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLayerGrassView message handlers

int CLayerGrassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_GRASSLIST);

	m_wndToolBar.GetControls()->SetControlType( 0, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 1, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 2, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 3, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 4, xtpControlButton );

	return 0;
}

void CLayerGrassView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	DeleteMemDC();
	CreateMemDC();

	ResetScroll();
	// TODO: Add your message handler code here
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, nTop, cx, ( cy - nTop ) / 2, SWP_FRAMECHANGED );
	}
}

BOOL CLayerGrassView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

//	return CFormView::OnEraseBkgnd(pDC);
	return TRUE;
}

void CLayerGrassView::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CLayerGrassView::DeleteMemDC()
{
	if( m_pdcBitmap ) {
		m_pdcBitmap->DeleteObject();
		SAFE_DELETE( m_pdcBitmap );
	}
	if( m_pmemDC ) {
		m_pmemDC->DeleteDC();
		SAFE_DELETE( m_pmemDC );
	}
}

LRESULT CLayerGrassView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return S_OK;
	if( (int)wParam == 1 ) {
		m_nSelectBlockIndex = -1;
		m_nDrawFlag = 0;
	}


	int nPrevBlockIndex = m_nSelectBlockIndex;
	int nPrevDrawFlag = m_nDrawFlag;

	m_nTextureCount = 1;
	m_nSelectBlockIndex = -1;

	if( CGlobalValue::GetInstance().m_bEditBlock == true ) {
		int nSize = (int)CGlobalValue::GetInstance().m_nVecEditBlockList.size();
		if( nSize == 0 ) m_nDrawFlag = 1;
		else if( nSize == 1 ) {
			m_nSelectBlockIndex = CGlobalValue::GetInstance().m_nVecEditBlockList[0];
			m_nDrawFlag = 4;
		}
		else {
			bool bCheckSame = true;
			if( bCheckSame == true ) {
				m_nSelectBlockIndex = CGlobalValue::GetInstance().m_nVecEditBlockList[0];
				m_nDrawFlag = 5;
			}
			else {
				m_nDrawFlag = 2;
			}
		}
	}
	else {
		int nIndex = pSector->GetCurrentBlock();
		if( nIndex == -1 ) m_nDrawFlag = 3;
		else {
			m_nSelectBlockIndex = nIndex;
			m_nDrawFlag = 4;
		}
	}
	if( m_nSelectBlockIndex == nPrevBlockIndex && nPrevDrawFlag == m_nDrawFlag ) return S_OK;
	ResetPropertyGrid();
	RefreshPropertyGrid( g_GrassInfoPropertyDefine );
	ResetScroll();
	Invalidate();

	return S_OK;
}

void CLayerGrassView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.


	CreateMemDC();

	CBitmap *pOldBitmap;
	CRect rcRect, rcToolbar, rcProp;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	m_PropertyGrid.GetClientRect( &rcProp );

	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );

	m_pmemDC->FillSolidRect( &rcRect, RGB( 0, 0, 0 ) );
	switch( m_nDrawFlag ) {
		case 0:
			break;
		case 1:
			DrawCannotView( m_pmemDC, "블럭을 선택하여 주세요." );
			break;
		case 2:
			DrawCannotView( m_pmemDC, "같지 않는 블럭들 선택" );
			break;
		case 3:
			DrawCannotView( m_pmemDC, "표시할 수 없습니다." );
			break;
		case 4:
			DrawSlot( m_pmemDC, m_nSelectBlockIndex );
			break;
		case 5:
			DrawSlot( m_pmemDC, m_nSelectBlockIndex, false );
			break;
	}

	// 스크롤 그린다.
	rcRect.top += rcToolbar.Height();
	rcRect.top += rcProp.Height();
	CRect rcScroll;
	rcScroll.top = 0;
	rcScroll.bottom = rcRect.bottom;
	rcScroll.left = rcRect.right - 7;
	rcScroll.right = rcRect.right;
	m_pmemDC->Draw3dRect( &rcScroll, RGB( 129, 129, 129 ), RGB( 255, 255, 255 ) );

	if( m_ScrollSize.cy != 0 ) {
		CRect rcPos;
		rcPos.left = rcScroll.left + 1;
		rcPos.right = rcScroll.right - 1;
		rcPos.top = rcScroll.top + (int)( ( ( rcScroll.Height() - 20 ) / (float)m_ScrollSize.cy ) * (float)m_ScrollPos.cy );
		rcPos.bottom = rcPos.top + 20;
		m_pmemDC->FillSolidRect( &rcPos, RGB( 250, 213, 14 ) );
	}


	dc.BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height() ,
		m_pmemDC, 0, 0, SRCCOPY );

	m_pmemDC->SelectObject( pOldBitmap );
}

CRect CLayerGrassView::GetSlotRect( int nSlotIndex )
{
	CRect rcSlot;

	CRect rcRect;
	GetClientRect( &rcRect );

	int nWidthCount = ( rcRect.Width() - 5 ) / ( m_nSlotWidth + 5 );
	if( nWidthCount < 1 ) nWidthCount = 1;

	rcSlot.left = 5 + ( ( m_nSlotWidth + 5 ) * ( nSlotIndex % nWidthCount ) );
	rcSlot.top = 5 + ( ( m_nSlotHeight + 5 ) * ( nSlotIndex / nWidthCount ) );
	rcSlot.right = rcSlot.left + m_nSlotWidth;
	rcSlot.bottom = rcSlot.top + m_nSlotHeight;

	return rcSlot;
}

void CLayerGrassView::DrawSlot( CDC *pDC, int nBlockIndex, bool bDrawImage )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	int nImageCount = m_nTextureCount;
	CWnd *pWnd = GetPaneWnd( TILEMNG_PANE );

	const char *szFileName;
	CRect rcSlot, rcTemp;
	TileStruct *pStruct = NULL;
	CString szTemp;

	pDC->SetBkMode( TRANSPARENT );
	pDC->SelectStockObject( DEFAULT_GUI_FONT );
	pDC->SetTextColor( RGB( 255, 255, 255 ) );
	pDC->SetStretchBltMode( COLORONCOLOR );

	for( int i=0; i<nImageCount; i++ ) {
		rcSlot = GetSlotRect(i);
		rcSlot.top -= m_ScrollPos.cy;
		rcSlot.bottom -= m_ScrollPos.cy;

		// 선택된것 그려준다.
		if( m_nSelectTextureIndex == i ) {
			pDC->FillSolidRect( &rcSlot, RGB( 255, 50, 20 ) );
		}

		pDC->Draw3dRect( &rcSlot, RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

		rcTemp = rcSlot;
		rcTemp.bottom = rcTemp.top + 15;
		pDC->Draw3dRect( &rcTemp, RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

		szFileName = pSector->GetGrassTextureName( nBlockIndex );
		pStruct = (TileStruct*)pWnd->SendMessage( UM_TILEPANE_GET_PREVIEW_IMAGE, (WPARAM)szFileName );
		if( pStruct ) {
			szTemp.Format( "%d x %d", pStruct->SurfaceDesc.Width, pStruct->SurfaceDesc.Height ); 
			pDC->IntersectClipRect( &rcTemp ); 
			pDC->TextOut( rcTemp.left + 1, rcTemp.top + 2, szTemp );
			pDC->SelectClipRgn( NULL );
		}

		rcTemp = rcSlot;
		rcTemp.top = rcTemp.bottom - 15;
		pDC->Draw3dRect( &rcTemp, RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

		pDC->IntersectClipRect( &rcTemp ); 
		pDC->TextOut( rcTemp.left + 1, rcTemp.top + 2, CString(szFileName) );
		pDC->SelectClipRgn( NULL );

		// 그림을 그려준당~
		if( bDrawImage == true ) {
			if( !pWnd ) continue;
			pStruct = (TileStruct*)pWnd->SendMessage( UM_TILEPANE_GET_PREVIEW_IMAGE, (WPARAM)szFileName );
			if( !pStruct ) continue;
			rcTemp = rcSlot;
			rcTemp.top += 15;
			rcTemp.bottom -= 15;
			rcTemp.left += 1;
			rcTemp.right -= 1;

			pDC->StretchBlt( rcTemp.left, rcTemp.top, rcTemp.Width(), rcTemp.Height(), pStruct->pmemDC, 0, 0, pStruct->SurfaceDesc.Width, pStruct->SurfaceDesc.Height, SRCPAINT );
		}
		else {
			rcTemp = rcSlot;
			rcTemp.top += 15;
			rcTemp.bottom -= 15;
			rcTemp.left += 1;
			rcTemp.right -= 1;
			pDC->IntersectClipRect( &rcTemp ); 
			pDC->TextOut( rcTemp.left + 1, rcTemp.top + 2, "Multi Select" );
			pDC->SelectClipRgn( NULL );
		}
	}
}

LRESULT CLayerGrassView::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)lParam >> 16;
	p.y = (long)( lParam << 16 ) >> 16;
	p.y += m_ScrollPos.cy;
	char *szFileName = (char *)wParam;

	CRect rcRect;
	m_wndToolBar.GetClientRect( &rcRect );
	p.y -= rcRect.Height();
	m_PropertyGrid.GetClientRect( &rcRect );
	p.y -= rcRect.Height();
	m_nSelectTextureIndex = -1;

	for( int i=0; i<m_nTextureCount; i++ ) {
		rcRect = GetSlotRect(i);
		if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {
			ChangeTexture( szFileName );
			break;
		}
	}

	SendMessage( UM_REFRESH_PANE_VIEW );
	return S_OK;
}

LRESULT CLayerGrassView::OnDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)wParam >> 16;
	p.y = (long)( wParam << 16 ) >> 16;
	p.y += m_ScrollPos.cy;

	CRect rcRect;
	m_wndToolBar.GetClientRect( &rcRect );
	p.y -= rcRect.Height();
	m_PropertyGrid.GetClientRect( &rcRect );
	p.y -= rcRect.Height();

	m_nSelectTextureIndex = -1;
	for( int i=0; i<m_nTextureCount; i++ ) {
		rcRect = GetSlotRect(i);
		if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {
			m_nSelectTextureIndex = i;
			break;
		}
	}

	Invalidate();
	return S_OK;
}

void CLayerGrassView::ChangeTexture( char *szFileName )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

//	if( CGlobalValue::GetInstance().m_bEditBlock != true ) return;
//	if( CGlobalValue::GetInstance().m_nVecEditBlockList.size() == 0 ) return;

	/*
	// Add Undo/Redo Infomation
	CString szDesc;
	std::vector<CString> szVecPrevNameList;
	for( DWORD i=0; i<CGlobalValue::GetInstance().m_nVecEditBlockList.size(); i++ ) {
		szVecPrevNameList.push_back( pSector->GetTextureName( CGlobalValue::GetInstance().m_nVecEditBlockList[i], m_nLayerIndex + ( nTextureIndex * 4 ) ) );
	}

	CActionElementChangeTexture *pAction = new CActionElementChangeTexture( pSector );
	szDesc.Format( "Change Block Texture : %d", CGlobalValue::GetInstance().m_nVecEditBlockList.size() );
	pAction->SetDesc( szDesc );
	pAction->ChangeTexture( m_nLayerIndex, nTextureIndex, CGlobalValue::GetInstance().m_nVecEditBlockList, szVecPrevNameList, szFileName );
	pAction->AddAction();

	*/
	// Change
	pSector->SetGrassTexture( -1, szFileName );

	CGlobalValue::GetInstance().RefreshRender();
	SendMessage( UM_REFRESH_PANE_VIEW, 1 );

	CGlobalValue::GetInstance().SetModify();
}

void CLayerGrassView::ResetScroll()
{
	if( m_nTextureCount == 0 ) {
		m_ScrollPos = CSize( 0, 0 );
		m_ScrollSize = CSize( 0, 0 );
		return;
	}
	CRect rcRect;
	GetClientRect( &rcRect );

	int nWidthCount = ( rcRect.Width() - 5 ) / ( m_nSlotWidth + 5 );
	int nHeightCount = ( rcRect.Height() - 5 ) / ( m_nSlotHeight + 5 );
	if( nWidthCount < 1 ) nWidthCount = 1;
	if( nHeightCount < 1 ) nHeightCount = 1;

	int nCount = m_nTextureCount / 4;
	int nNeedHeightCount = nCount / nWidthCount;
	if( nCount % nWidthCount != 0 ) nNeedHeightCount++;

	m_ScrollSize.cx = 0;
	m_ScrollSize.cy = 0;
	if( nNeedHeightCount > nHeightCount ) {
		m_ScrollSize.cx = 0;
		m_ScrollSize.cy = nNeedHeightCount * m_nSlotHeight;
		if( m_ScrollPos.cy > m_ScrollSize.cy )
			m_ScrollPos.cy = m_ScrollSize.cy;
	}
}
BOOL CLayerGrassView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( zDelta > 0 ) m_ScrollPos.cy -= ( m_nSlotHeight + 5 );
	else m_ScrollPos.cy += ( m_nSlotHeight + 5 );

	if( m_ScrollPos.cy < 0 ) m_ScrollPos.cy = 0;
	else if( m_ScrollPos.cy >= m_ScrollSize.cy ) m_ScrollPos.cy = m_ScrollSize.cy;

	Invalidate();

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}

void CLayerGrassView::DrawCannotView( CDC *pDC, char *szStr )
{
	CRect rcRect;
	GetClientRect( &rcRect );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SelectStockObject( DEFAULT_GUI_FONT );
	pDC->SetTextColor( RGB( 255, 255, 255 ) );

	pDC->TextOut( rcRect.left, rcRect.Height() / 2, szStr );
}

void CLayerGrassView::OnGrass1()
{
	CXTPControl *pCtrl = m_wndToolBar.GetControls()->FindControl( xtpControlError, ID_GRASS_1, TRUE, FALSE );
	CXTPControlButton *pButton = (CXTPControlButton *)pCtrl;

	CGlobalValue::GetInstance().m_bEnableGrassIndex[0] = !CGlobalValue::GetInstance().m_bEnableGrassIndex[0];
	pButton->SetChecked( CGlobalValue::GetInstance().m_bEnableGrassIndex[0] );
}

void CLayerGrassView::OnGrass2()
{
	CXTPControl *pCtrl = m_wndToolBar.GetControls()->FindControl( xtpControlError, ID_GRASS_2, TRUE, FALSE );
	CXTPControlButton *pButton = (CXTPControlButton *)pCtrl;

	CGlobalValue::GetInstance().m_bEnableGrassIndex[1] = !CGlobalValue::GetInstance().m_bEnableGrassIndex[1];
	pButton->SetChecked( CGlobalValue::GetInstance().m_bEnableGrassIndex[1] );
}

void CLayerGrassView::OnGrass3()
{
	CXTPControl *pCtrl = m_wndToolBar.GetControls()->FindControl( xtpControlError, ID_GRASS_3, TRUE, FALSE );
	CXTPControlButton *pButton = (CXTPControlButton *)pCtrl;

	CGlobalValue::GetInstance().m_bEnableGrassIndex[2] = !CGlobalValue::GetInstance().m_bEnableGrassIndex[2];
	pButton->SetChecked( CGlobalValue::GetInstance().m_bEnableGrassIndex[2] );
}

void CLayerGrassView::OnGrass4()
{
	CXTPControl *pCtrl = m_wndToolBar.GetControls()->FindControl( xtpControlError, ID_GRASS_4, TRUE, FALSE );
	CXTPControlButton *pButton = (CXTPControlButton *)pCtrl;

	CGlobalValue::GetInstance().m_bEnableGrassIndex[3] = !CGlobalValue::GetInstance().m_bEnableGrassIndex[3];
	pButton->SetChecked( CGlobalValue::GetInstance().m_bEnableGrassIndex[3] );
}

void CLayerGrassView::OnUpdateGrass1(CCmdUI *pCmdUI)
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) {
		pCmdUI->Enable( FALSE );
		return;
	}

	if( m_nSelectBlockIndex == -1 ) pCmdUI->Enable( FALSE );
	else pCmdUI->Enable( TRUE );
}

void CLayerGrassView::OnUpdateGrass2(CCmdUI *pCmdUI)
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) {
		pCmdUI->Enable( FALSE );
		return;
	}

	if( m_nSelectBlockIndex == -1 ) pCmdUI->Enable( FALSE );
	else pCmdUI->Enable( TRUE );
}

void CLayerGrassView::OnUpdateGrass3(CCmdUI *pCmdUI)
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) {
		pCmdUI->Enable( FALSE );
		return;
	}

	if( m_nSelectBlockIndex == -1 ) pCmdUI->Enable( FALSE );
	else pCmdUI->Enable( TRUE );
}

void CLayerGrassView::OnUpdateGrass4(CCmdUI *pCmdUI)
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) {
		pCmdUI->Enable( FALSE );
		return;
	}

	if( m_nSelectBlockIndex == -1 ) pCmdUI->Enable( FALSE );
	else pCmdUI->Enable( TRUE );
}

LRESULT CLayerGrassView::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CLayerGrassView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top = rcRect.bottom - ( ( rcRect.Height() - rcToolbar.Height() ) / 2 );
	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0 );

}

void CLayerGrassView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( pSector->GetGrassShakeMin() );
			break;
		case 1:
			pVariable->SetVariable( pSector->GetGrassShakeMax() );
			break;
	}
	if( dwIndex > 1 ) {
		int nIndex = (dwIndex-2) / 3;
		switch( (dwIndex-2)%3 ) {
			case 0:
				pVariable->SetVariable( pSector->GetGrassWidth( nIndex ) );
				break;
			case 1:
				pVariable->SetVariable( pSector->GetGrassHeightMin( nIndex ) );
				break;
			case 2:
				pVariable->SetVariable( pSector->GetGrassHeightMax( nIndex ) );
				break;
		}
	}
}

void CLayerGrassView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;
	switch( dwIndex ) {
		case 0:
			pSector->SetGrassShakeMin( pVariable->GetVariableFloat() );
			break;
		case 1:
			pSector->SetGrassShakeMax( pVariable->GetVariableFloat() );
			break;
	}
	if( dwIndex > 1 ) {
		int nIndex = (dwIndex-2) / 3;
		switch( (dwIndex-2)%3 ) {
			case 0:
				pSector->SetGrassWidth( nIndex, pVariable->GetVariableFloat() );
				break;
			case 1:
				pSector->SetGrassHeightMin( nIndex, pVariable->GetVariableFloat() );
				break;
			case 2:
				pSector->SetGrassHeightMax( nIndex, pVariable->GetVariableFloat() );
				break;
		}
	}
	pSector->ApplyGrassInfo();
}

void CLayerGrassView::OnReload()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	int nSize = (int)CGlobalValue::GetInstance().m_nVecEditBlockList.size();
	if( nSize > 1 ) return;

	EtTextureHandle hTexture;
#ifdef _TOOLCOMPILE
	hTexture = pSector->GetGrassTexture();
#endif
	if( hTexture )
	{
		// 현재 로딩된 텍스처를 쓰는 Terrain들의 텍스처를 모두 릴리즈 하고 다시 셋팅하는건 작업량이 많아서 이렇게 강제로 로딩하는거로 하겠다.
		CFileStream *pStream = new CFileStream( hTexture->GetFullName() );
		hTexture->LoadResource( pStream );
		pStream->Close();
		SAFE_DELETE( pStream );
	}
}