// LayerDecalView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "LayerDecalView.h"
#include "GlobalValue.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "PaneDefine.h"
#include "resource.h"
#include "MainFrm.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "UserMessage.h"
#include "TEtWorldDecal.h"
#include "BrushRenderView.h"


// CLayerDecalView
PropertyGridBaseDefine g_DecalInfoPropertyDefine[] = {
	{ "Common", "Position", CUnionValueProperty::Vector2, "위치", TRUE },
	{ "Common", "Radius", CUnionValueProperty::Float, "반지름|1.f|10000.f|1.f", TRUE },
	{ "Common", "Rotate", CUnionValueProperty::Float, "회전|0.f|360.f|0.1f", TRUE },
	{ "Common", "Color", CUnionValueProperty::Vector4Color, "색깔", TRUE },
	{ "Common", "Alpha", CUnionValueProperty::Float, "알파|0.f|1.f|0.01f", TRUE },
	{ NULL },
};

IMPLEMENT_DYNCREATE(CLayerDecalView, CFormView)

CLayerDecalView::CLayerDecalView()
	: CFormView(CLayerDecalView::IDD)
{
	m_bActivate = false;

	m_pdcBitmap = NULL;
	m_pmemDC = NULL;
	m_nSelectTextureIndex = -1;
	m_nTextureCount = 1;
	m_nDrawFlag = 0;
	m_ScrollPos = m_ScrollSize = CSize( 0, 0 );

	m_nSlotWidth = 100;
	m_nSlotHeight = 130;	
}

CLayerDecalView::~CLayerDecalView()
{
	DeleteMemDC();
}

void CLayerDecalView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLayerDecalView, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_TILEPANE_DROPITEM, OnDropItem )
	ON_MESSAGE( UM_TILEPANE_DROPMOVEITEM, OnDropMoveItem )
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CLayerDecalView diagnostics

#ifdef _DEBUG
void CLayerDecalView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CLayerDecalView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLayerDecalView message handlers

int CLayerDecalView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CLayerDecalView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	DeleteMemDC();
	CreateMemDC();

	ResetScroll();
	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy / 2, SWP_FRAMECHANGED );
	}
}

BOOL CLayerDecalView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CLayerDecalView::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect;
	GetClientRect( &rcRect );

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CLayerDecalView::DeleteMemDC()
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

LRESULT CLayerDecalView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return S_OK;

	ResetPropertyGrid();
	RefreshPropertyGrid( g_DecalInfoPropertyDefine );
	ResetScroll();
	Invalidate();

	return S_OK;
}

void CLayerDecalView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.


	CreateMemDC();

	CBitmap *pOldBitmap;
	CRect rcRect, rcProp;
	GetClientRect( &rcRect );
	m_PropertyGrid.GetClientRect( &rcProp );

	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );

	m_pmemDC->FillSolidRect( &rcRect, RGB( 0, 0, 0 ) );
	/*
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
	*/
	DrawSlot( m_pmemDC, -1 );

	// 스크롤 그린다.
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

CRect CLayerDecalView::GetSlotRect( int nSlotIndex )
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

void CLayerDecalView::DrawSlot( CDC *pDC, int nSelectDecal, bool bDrawImage )
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

		if( !CGlobalValue::GetInstance().m_pSelectDecal ) szFileName = pSector->GetDecalTextureName();
		else szFileName = CGlobalValue::GetInstance().m_pSelectDecal->GetTextureName();

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

LRESULT CLayerDecalView::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)lParam >> 16;
	p.y = (long)( lParam << 16 ) >> 16;
	p.y += m_ScrollPos.cy;
	char *szFileName = (char *)wParam;

	CRect rcRect;
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

LRESULT CLayerDecalView::OnDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)wParam >> 16;
	p.y = (long)( wParam << 16 ) >> 16;
	p.y += m_ScrollPos.cy;

	CRect rcRect;
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

void CLayerDecalView::ChangeTexture( char *szFileName )
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
	if( !CGlobalValue::GetInstance().m_pSelectDecal ) {
		pSector->SetDecalTexture( szFileName );
	}
	else {
		CGlobalValue::GetInstance().m_pSelectDecal->SetTextureName( szFileName );
	}


	CGlobalValue::GetInstance().RefreshRender();
	SendMessage( UM_REFRESH_PANE_VIEW, 1 );

	CGlobalValue::GetInstance().SetModify();
}

void CLayerDecalView::ResetScroll()
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
BOOL CLayerDecalView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( zDelta > 0 ) m_ScrollPos.cy -= ( m_nSlotHeight + 5 );
	else m_ScrollPos.cy += ( m_nSlotHeight + 5 );

	if( m_ScrollPos.cy < 0 ) m_ScrollPos.cy = 0;
	else if( m_ScrollPos.cy >= m_ScrollSize.cy ) m_ScrollPos.cy = m_ScrollSize.cy;

	Invalidate();

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}

LRESULT CLayerDecalView::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CLayerDecalView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );
	rcRect.top = rcRect.bottom - ( rcRect.Height() / 2 );
	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0 );

}

void CLayerDecalView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( !CGlobalValue::GetInstance().m_pSelectDecal ) {
		switch( dwIndex ) {
			case 0:
				pVariable->SetVariable( EtVector2( 0.f, 0.f ) );
				SetReadOnly( pVariable, true );
				break;
			case 1:
				pVariable->SetVariable( (float)CGlobalValue::GetInstance().m_nBrushDiameter * 100.f / 2.f );
				SetReadOnly( pVariable, true );
				break;
			case 2:
				pVariable->SetVariable( pSector->GetDecalRotate() );
				break;
			case 3:
				{
					EtVector4 vColor = pSector->GetDecalColor();
					pVariable->SetVariable( vColor );
				}
				break;
			case 4:
				pVariable->SetVariable( pSector->GetDecalAlpha() );
				break;
		}
	}
	else {
		CTEtWorldDecal *pDecal = CGlobalValue::GetInstance().m_pSelectDecal;
		switch( dwIndex ) {
			case 0:
				pVariable->SetVariable( pDecal->GetPosition() );
				break;
			case 1:
				pVariable->SetVariable( pDecal->GetRadius() );
				break;
			case 2:
				pVariable->SetVariable( pDecal->GetRotate() );
				break;
			case 3:
				{
					EtVector4 vColor = pDecal->GetColor();
					pVariable->SetVariable( vColor );
				}
				break;
			case 4:
				pVariable->SetVariable( pDecal->GetAlpha() );
				break;
		}
	}
}

void CLayerDecalView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( !CGlobalValue::GetInstance().m_pSelectDecal ) {
		switch( dwIndex ) {
			case 2:
				pSector->SetDecalRotate( pVariable->GetVariableFloat() );
				break;
			case 3:
				{
					EtVector4 vColor = pVariable->GetVariableVector4();
					EtColor Color( vColor.x, vColor.y, vColor.z, vColor.w );
					pSector->SetDecalColor( Color );
				}
				break;
			case 4:
				pSector->SetDecalAlpha( pVariable->GetVariableFloat() );
				break;
		}
	}
	else {
		CTEtWorldDecal *pDecal = CGlobalValue::GetInstance().m_pSelectDecal;

		CActionElementModifyDecal *pAction = NULL;

		switch( dwIndex ) {
			case 0: 
				pAction = new CActionElementModifyDecal( pSector, pDecal, 0 );
				pAction->SetDesc( "Modify Decal - Move" );
				break;
			case 1:
				pAction = new CActionElementModifyDecal( pSector, pDecal, 1 );
				pAction->SetDesc( "Modify Decal - Radius" );
				break;
			case 2:
				pAction = new CActionElementModifyDecal( pSector, pDecal, 2 );
				pAction->SetDesc( "Modify Decal - Rotate" );
				break;
			case 3:
				pAction = new CActionElementModifyDecal( pSector, pDecal, 3 );
				pAction->SetDesc( "Modify Decal - Color" );
				break;
			case 4:
				pAction = new CActionElementModifyDecal( pSector, pDecal, 4 );
				pAction->SetDesc( "Modify Decal - Alpha" );
				break;
		}

		switch( dwIndex ) {
			case 0:
				pDecal->SetPosition( pVariable->GetVariableVector2() );
				break;
			case 1:
				pDecal->SetRadius( pVariable->GetVariableFloat() );
				break;
			case 2:
				pDecal->SetRotate( pVariable->GetVariableFloat() );
				break;
			case 3:
				{
					EtVector4 vColor = pVariable->GetVariableVector4();
					EtColor Color( vColor.x, vColor.y, vColor.z, vColor.w );
					pDecal->SetColor( Color );
				}
				break;
			case 4:
				pDecal->SetAlpha( pVariable->GetVariableFloat() );
				break;
		}
		CGlobalValue::GetInstance().RefreshRender();
		if( pAction ) pAction->AddAction();
	}
}
