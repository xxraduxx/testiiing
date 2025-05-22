// PropPoolView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropPoolView.h"
#include "UserMessage.h"
#include "PropPoolControl.h"
#include "FileServer.h"
#include "SkinManagerPaneView.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"


// CPropPoolView

IMPLEMENT_DYNCREATE(CPropPoolView, CFormView)

CPropPoolView::CPropPoolView()
	: CFormView(CPropPoolView::IDD)
{
	m_bActivate = false;
	m_pdcBitmap = NULL;
	m_pmemDC = NULL;

	m_ScrollSize = CSize( 0, 0 );
	m_ScrollPos = CSize( 0, 0 );
	m_fScale = 1.f;
	m_nCircleRadius = 180;
	m_nIconSize = 5;

	m_bLBDown = false;
	m_bInScroll = false;

	m_nDetailViewIndex = -1;
	m_nSelectItemIndex = -1;
	m_bRemoveArea = false;
}

CPropPoolView::~CPropPoolView()
{
	DeleteMemDC();
}

void CPropPoolView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropPoolView, CFormView)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE( UM_SKINPANE_DROPITEM, OnDropItem )
	ON_MESSAGE( UM_SKINPANE_DROPMOVEITEM, OnDropMoveItem )
	ON_MESSAGE( UM_SKINPANE_DBLCLICKITEM, OnDblClickItem )
	ON_COMMAND(ID_POOL_EMPTY, &CPropPoolView::OnPoolEmpty)
	ON_COMMAND(ID_POOL_SAVE, &CPropPoolView::OnPoolSave)
	ON_COMMAND(ID_POOL_LOADLIST, &CPropPoolView::OnPoolLoadlist)
	ON_UPDATE_COMMAND_UI(ID_POOL_EMPTY, &CPropPoolView::OnUpdatePoolEmpty)
	ON_UPDATE_COMMAND_UI(ID_POOL_SAVE, &CPropPoolView::OnUpdatePoolSave)
	ON_UPDATE_COMMAND_UI(ID_POOL_LOADLIST, &CPropPoolView::OnUpdatePoolLoadlist)
END_MESSAGE_MAP()


// CPropPoolView diagnostics

#ifdef _DEBUG
void CPropPoolView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropPoolView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropPoolView message handlers

void CPropPoolView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;
}

void CPropPoolView::OnDestroy()
{
	CFormView::OnDestroy();

	// TODO: Add your message handler code here
}

BOOL CPropPoolView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CPropPoolView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}

	// TODO: Add your message handler code here
	DeleteMemDC();
	CreateMemDC();
	ResetScroll();
}

void CPropPoolView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CreateMemDC();

	CBitmap *pOldBitmap;

	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );

	CRect rcTemp;
	GetClientRect( &rcTemp );
	rcTemp.bottom -= rcToolbar.Height();
	m_pmemDC->FillSolidRect( &rcTemp, RGB( 0, 0, 0 ) );

	CRect rcCircle = CRect( 7, 7, 7 + m_nCircleRadius, 7 + m_nCircleRadius );
	rcCircle -= m_ScrollPos;

	// 원을 그린다.
	CBrush Brush( RGB(255,255,255) );
	m_pmemDC->SelectObject( &Brush );
	m_pmemDC->Ellipse( rcCircle );

	CBrush Brush2( RGB(0,0,0) );
	m_pmemDC->SelectObject( &Brush2 );
	rcCircle.DeflateRect( 2, 2, 2, 2 );
	m_pmemDC->Ellipse( rcCircle );

	// 내용물을 그린다.
	DrawPropList( m_pmemDC );


	// 스크롤 그린다.
	CRect rcScroll;
	CalcScrollRect( true, rcScroll );
	m_pmemDC->Draw3dRect( &rcScroll, RGB( 129, 129, 129 ), RGB( 255, 255, 255 ) );

	if( m_ScrollSize.cy != 0 ) {
		CRect rcPos;
		rcPos.left = rcScroll.left + 1;
		rcPos.right = rcScroll.right - 1;
		rcPos.top = rcScroll.top + (int)( ( ( rcScroll.Height() - 20 ) / (float)m_ScrollSize.cy ) * (float)m_ScrollPos.cy );
		rcPos.bottom = rcPos.top + 20;
		m_pmemDC->FillSolidRect( &rcPos, RGB( 250, 213, 14 ) );
	}

	CalcScrollRect( false, rcScroll );
	m_pmemDC->Draw3dRect( &rcScroll, RGB( 129, 129, 129 ), RGB( 255, 255, 255 ) );

	if( m_ScrollSize.cx != 0 ) {
		CRect rcPos;
		rcPos.left = rcScroll.left + (int)( ( ( rcScroll.Width() - 20 ) / (float)m_ScrollSize.cx ) * (float)m_ScrollPos.cx );
		rcPos.right = rcPos.left + 20;
		rcPos.top = rcScroll.top + 1;
		rcPos.bottom = rcScroll.bottom - 1;
		m_pmemDC->FillSolidRect( &rcPos, RGB( 250, 213, 14 ) );
	}

	dc.BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height() ,
		m_pmemDC, 0, 0, SRCCOPY );

	m_pmemDC->SelectObject( pOldBitmap );
}

void CPropPoolView::DrawPropList( CDC *pDC )
{
	CPropPoolStage *pStage = g_PropPoolControl.GetCurrentStage();

	CRect rcRect, rcDetailRect, rcIconRect;
	EtVector2 vOffset;
	int nCenter = ( 7 + 2 )/*앞쪽여유공간+두께*/ + ( m_nCircleRadius / 2 );

	for( DWORD i=0; i<pStage->GetPropCount(); i++ ) {
		vOffset = pStage->GetPropOffset(i);

		vOffset *= (float)( m_nCircleRadius / 2.f );
		vOffset.x = nCenter - vOffset.x;
		vOffset.y = nCenter - vOffset.y;

		rcRect.left = (int)( vOffset.x - m_nIconSize );
		rcRect.right = (int)( vOffset.x + m_nIconSize );
		rcRect.top = (int)( vOffset.y - m_nIconSize );
		rcRect.bottom = (int)( vOffset.y + m_nIconSize );
		rcRect -= m_ScrollPos;
		pDC->Draw3dRect( &rcRect, RGB( 255,255,255 ), RGB( 120, 120, 120 ) );
		rcRect.DeflateRect( 1, 1, 1, 1 );
		if( m_nSelectItemIndex == (int)i ) {
			pDC->FillSolidRect( &rcRect, RGB( 255, 0, 0 ) );

			rcRect.InflateRect( 2, 2, 2, 2 );
			pDC->Draw3dRect( &rcRect, RGB( 255,255,255 ), RGB( 120, 120, 120 ) );
		}
		else {
			pDC->FillSolidRect( &rcRect, RGB( 220, 20, 40 ) );
		}

		if( m_nDetailViewIndex == (int)i ) {
			rcIconRect = rcRect;
			rcDetailRect.left = rcRect.right + 1;
			rcDetailRect.top = rcRect.bottom + 1;
			rcDetailRect.right = rcDetailRect.left + 100;
			rcDetailRect.bottom = rcRect.top + 115;

			CRect rcClientRect, rcToolbar;
			GetClientRect( &rcClientRect );
			m_wndToolBar.GetClientRect( &rcToolbar );

			if( rcDetailRect.bottom > ( rcClientRect.bottom - rcToolbar.Height() ) ) {
				int nOffset = rcDetailRect.bottom - rcClientRect.bottom + rcToolbar.Height();
				rcDetailRect.top -= nOffset;
				rcDetailRect.bottom -= nOffset;
			}
			if( rcDetailRect.right > rcClientRect.right ) {
				int nOffset = rcDetailRect.right - rcClientRect.right;
				rcDetailRect.left -= nOffset;
				rcDetailRect.right -= nOffset;
			}

		}
	}
	if( m_nDetailViewIndex != -1 ) {
		CRect rcTemp = rcDetailRect;

		pDC->Draw3dRect( &rcTemp, RGB( 255,255,255 ), RGB( 120, 120, 120 ) );
		rcTemp.top = rcTemp.bottom - 15;
		pDC->Draw3dRect( &rcTemp, RGB( 255,255,255 ), RGB( 120, 120, 120 ) );

		pDC->SetBkMode( TRANSPARENT );
		pDC->SelectStockObject( DEFAULT_GUI_FONT );
		rcTemp.DeflateRect( 1, 1, 1, 1 );
		pDC->FillSolidRect( &rcTemp, RGB(0,0,0) );

		pDC->IntersectClipRect( &rcTemp ); 
		pDC->SetTextColor( RGB( 255, 255, 255 ) );
		
		CString szPropName = pStage->GetPropName( (DWORD)m_nDetailViewIndex );
		char szBuf[256] = { 0, };
		char szExt[6] = { 0, };

		pDC->TextOut( rcTemp.left + 1, rcTemp.top + 1, szPropName );
		pDC->SelectClipRgn( NULL );

		// 거리 퍼센트 써준다.
		rcTemp = rcIconRect;
		rcTemp.top -= 13;
		rcTemp.bottom -= 13;
		rcTemp.left -= 10;
		rcTemp.right += 10;
		float fLength = EtVec2Length( &pStage->GetPropOffset( (DWORD)m_nDetailViewIndex ) );
		if( fLength > 1.f ) {
			sprintf_s( szBuf, "Out" );
			pDC->SetTextColor( RGB( 250, 0, 0 ) );
		}
		else {
			sprintf_s( szBuf, "%d%%", 100 - (int)(fLength * 100.f) );
			pDC->SetTextColor( RGB( 250, 213, 14 ) );
		}
		pDC->TextOut( rcTemp.left + 1, rcTemp.top + 1, szBuf );

		// 그림 그려준다.
		CWnd *pWnd = GetPaneWnd( SKINMNG_PANE );
		if( pWnd ) {
			rcTemp = rcDetailRect;
			rcTemp.bottom -= 15;
			rcTemp.DeflateRect( 1, 1, 1, 1 );

			SkinStruct *pStruct = (SkinStruct*)pWnd->SendMessage( UM_SKINPANE_GET_PREVIEW_IMAGE, (WPARAM)szPropName.GetBuffer() );
			if( pStruct ) {
				pDC->SetStretchBltMode( COLORONCOLOR );
				pDC->StretchBlt( rcTemp.left, rcTemp.top, rcTemp.Width(), rcTemp.Height(), pStruct->pmemDC, 0, 0, pStruct->SurfaceDesc.Width, pStruct->SurfaceDesc.Height, SRCPAINT );
			}
		}
	}
}

void CPropPoolView::CreateMemDC()
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

void CPropPoolView::DeleteMemDC()
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

int CPropPoolView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	g_PropPoolControl.LoadStage();

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_PROPPOOL);

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_POOL_EMPTY, TRUE, FALSE );
	pControl->SetStyle(xtpButtonIconAndCaption);

	m_wndToolBar.GetControls()->SetControlType( 2, xtpControlComboBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_POOL_LOADLIST, TRUE, FALSE );

	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	pCombo->InsertString( pCombo->GetCount(), "None" );

	for( DWORD i=0; i<g_PropPoolControl.GetStageCount(); i++ ) {
		pCombo->InsertString( pCombo->GetCount(), g_PropPoolControl.GetStageName(i) );
	}	
	pCombo->SetCurSel(0);

	return 0;
}

BOOL CPropPoolView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if( GetAKState( VK_LCONTROL ) < 0 ) {
		if( zDelta > 0 ) {
			m_nCircleRadius += 10;
		}
		else {
			m_nCircleRadius -= 10;
			if( m_nCircleRadius < 100 ) m_nCircleRadius = 100;
		}
		ResetScroll();
		Invalidate();
	}

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}


void CPropPoolView::ResetScroll()
{
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();


	m_ScrollSize.cx = 0;
	m_ScrollSize.cy = 0;

	if( rcRect.Height() - 8 < m_nCircleRadius ) {
		m_ScrollSize.cy = m_nCircleRadius - rcRect.Height() + 8;
		if( m_ScrollPos.cy > m_ScrollSize.cy )
			m_ScrollPos.cy = m_ScrollSize.cy;
	}

	if( rcRect.Width() - 8 < m_nCircleRadius ) {
		m_ScrollSize.cx = m_nCircleRadius - rcRect.Width() + 8;
		if( m_ScrollPos.cx > m_ScrollSize.cx )
			m_ScrollPos.cx = m_ScrollSize.cx;
	}
}

void CPropPoolView::CalcScrollRect( bool bVert, CRect &rcResult )
{
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	if( bVert ) {
		rcResult.top = 0;
		rcResult.bottom = rcRect.bottom - rcToolbar.Height() - 7;
		rcResult.left = rcRect.right - 7;
		rcResult.right = rcRect.right;
	}
	else {
		rcResult.top = rcRect.bottom - rcToolbar.Height() - 7;
		rcResult.bottom = rcRect.bottom- rcToolbar.Height();
		rcResult.left = 0;
		rcResult.right = rcRect.right - 7;
	}
}


void CPropPoolView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bLBDown = true;

	CRect rcScroll, rcToolbar;
	m_wndToolBar.GetClientRect( &rcToolbar );

	CalcScrollRect( false, rcScroll );
	rcScroll.top += rcToolbar.Height();
	rcScroll.bottom += rcToolbar.Height();

	if( point.x >= rcScroll.left && point.x <= rcScroll.right && point.y >= rcScroll.top && point.y <= rcScroll.bottom ) {
		m_ScrollPos.cx = (int)( ( m_ScrollSize.cx / 100.f ) * ( ( 100.f / ( rcScroll.Width() - 20 ) ) * ( point.x - rcScroll.left ) ) );
		if( m_ScrollPos.cx > m_ScrollSize.cx ) m_ScrollPos.cx = m_ScrollSize.cx;
		m_bInScroll = true;
		ClientToScreen( &rcScroll );
		ClipCursor( &rcScroll );
		Invalidate();
	}

	CalcScrollRect( true, rcScroll );
	rcScroll.top += rcToolbar.Height();
	rcScroll.bottom += rcToolbar.Height();

	if( point.x >= rcScroll.left && point.x <= rcScroll.right && point.y >= rcScroll.top && point.y <= rcScroll.bottom ) {
		m_ScrollPos.cy = (int)( ( m_ScrollSize.cy / 100.f ) * ( ( 100.f / ( rcScroll.Height() - 20 ) ) * ( point.y - rcScroll.top ) ) );
		if( m_ScrollPos.cy > m_ScrollSize.cy ) m_ScrollPos.cy = m_ScrollSize.cy;
		m_bInScroll = true;
		ClientToScreen( &rcScroll );
		ClipCursor( &rcScroll );
		Invalidate();
	}

	// 아이템셀렉트
	if( !m_bInScroll ) {
		CPropPoolStage *pStage = g_PropPoolControl.GetCurrentStage();

		CRect rcRect;
		EtVector2 vOffset;
		int nCenter = ( 7 + 2 )/*앞쪽여유공간+두께*/ + ( m_nCircleRadius / 2 );

		CRect rcToolbar;
		m_wndToolBar.GetClientRect( &rcToolbar );

		bool bRefresh = false;
		for( DWORD i=0; i<pStage->GetPropCount(); i++ ) {
			vOffset = pStage->GetPropOffset(i);

			vOffset *= (float)( m_nCircleRadius / 2.f );
			vOffset.x = nCenter - vOffset.x;
			vOffset.y = nCenter - vOffset.y;

			rcRect.left = (int)( vOffset.x - m_nIconSize );
			rcRect.right = (int)( vOffset.x + m_nIconSize );
			rcRect.top = (int)( vOffset.y - m_nIconSize );
			rcRect.bottom = (int)( vOffset.y + m_nIconSize );
			rcRect -= m_ScrollPos;

			if( point.x >= rcRect.left && point.x <= rcRect.right && point.y - rcToolbar.Height() >= rcRect.top && point.y - rcToolbar.Height() <= rcRect.bottom ) {
				m_nSelectItemIndex = (int)i;
				m_bRemoveArea = false;
				bRefresh = true;
				break;
			}
		}
		if( bRefresh == false && m_nSelectItemIndex != -1 ) {
			m_nSelectItemIndex = -1;
			bRefresh = true;
		}
		if( bRefresh == true ) Invalidate();
	}

	CFormView::OnLButtonDown(nFlags, point);
}

void CPropPoolView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bLBDown = false;
	m_bInScroll = false;
	ClipCursor( NULL );
	if( m_nSelectItemIndex != -1 ) {
		if( m_bRemoveArea == false ) {
			EtVector2 vOffset = MousePosToOffset( point );
			if( EtVec2Length( &vOffset ) > 1.f ) m_bRemoveArea = true;
		}
		if( m_bRemoveArea == true  ) {
			g_PropPoolControl.GetCurrentStage()->RemoveProp( m_nSelectItemIndex );
			m_nSelectItemIndex = -1;
			m_nDetailViewIndex = -1;
			m_bRemoveArea = false;
			Invalidate();
		}
	}

	CFormView::OnLButtonUp(nFlags, point);
}

void CPropPoolView::OnMouseMove(UINT nFlags, CPoint point)
{
	CFormView::OnMouseMove(nFlags, point);
	// TODO: Add your message handler code here and/or call default
	if( m_bLBDown ) {
		if( m_bInScroll == true ) {
			CRect rcScroll, rcToolbar;
			m_wndToolBar.GetClientRect( &rcToolbar );

			CalcScrollRect( false, rcScroll );
			rcScroll.top += rcToolbar.Height();
			rcScroll.bottom += rcToolbar.Height();

			if( point.x >= rcScroll.left && point.x <= rcScroll.right && point.y >= rcScroll.top && point.y <= rcScroll.bottom ) {
				m_ScrollPos.cx = (int)( ( m_ScrollSize.cx / 100.f ) * ( ( 100.f / ( rcScroll.Width() - 20 ) ) * ( point.x - rcScroll.left ) ) );
				if( m_ScrollPos.cx > m_ScrollSize.cx ) m_ScrollPos.cx = m_ScrollSize.cx;
				Invalidate();
			}

			CalcScrollRect( true, rcScroll );
			rcScroll.top += rcToolbar.Height();
			rcScroll.bottom += rcToolbar.Height();

			if( point.x >= rcScroll.left && point.x <= rcScroll.right && point.y >= rcScroll.top && point.y <= rcScroll.bottom ) {
				m_ScrollPos.cy = (int)( ( m_ScrollSize.cy / 100.f ) * ( ( 100.f / ( rcScroll.Height() - 20 ) ) * ( point.y - rcScroll.top ) ) );
				if( m_ScrollPos.cy > m_ScrollSize.cy ) m_ScrollPos.cy = m_ScrollSize.cy;
				Invalidate();
			}
		}
		else if( m_nSelectItemIndex != -1 ) {
			CPropPoolStage *pStage = g_PropPoolControl.GetCurrentStage();
			
			CRect rcToolbar;
			m_wndToolBar.GetClientRect( &rcToolbar );

			CPoint p = point;
			p.x += m_ScrollPos.cx;
			p.y += m_ScrollPos.cy - rcToolbar.Height();

			EtVector2 vOffset = MousePosToOffset( p );
			if( EtVec2Length( &vOffset ) > 1.f ) m_bRemoveArea = true;
			else m_bRemoveArea = false;

			pStage->SetPropOffset( (DWORD)m_nSelectItemIndex, vOffset );
			Invalidate();
			return;
		}
	}

	// 췌크
	if( m_bInScroll == false ) {
		CPropPoolStage *pStage = g_PropPoolControl.GetCurrentStage();

		CRect rcRect;
		EtVector2 vOffset;
		int nCenter = ( 7 + 2 )/*앞쪽여유공간+두께*/ + ( m_nCircleRadius / 2 );

		CRect rcToolbar;
		m_wndToolBar.GetClientRect( &rcToolbar );

		bool bRefresh = false;
		for( DWORD i=0; i<pStage->GetPropCount(); i++ ) {
			vOffset = pStage->GetPropOffset(i);

			vOffset *= (float)( m_nCircleRadius / 2.f );
			vOffset.x = nCenter - vOffset.x;
			vOffset.y = nCenter - vOffset.y;

			rcRect.left = (int)( vOffset.x - m_nIconSize );
			rcRect.right = (int)( vOffset.x + m_nIconSize );
			rcRect.top = (int)( vOffset.y - m_nIconSize );
			rcRect.bottom = (int)( vOffset.y + m_nIconSize );
			rcRect -= m_ScrollPos;

			if( point.x >= rcRect.left && point.x <= rcRect.right && point.y - rcToolbar.Height() >= rcRect.top && point.y - rcToolbar.Height() <= rcRect.bottom ) {
				m_nDetailViewIndex = (int)i;
				bRefresh = true;
				break;
			}
		}
		if( bRefresh == false && m_nDetailViewIndex != -1 ) {
			m_nDetailViewIndex = -1;
			bRefresh = true;
		}
		if( bRefresh == true ) Invalidate();
	}

}

EtVector2 CPropPoolView::MousePosToOffset( CPoint vPos )
{
	int nCenter = ( 7 + 2 )/*앞쪽여유공간+두께*/ + ( m_nCircleRadius / 2 );
	EtVector2 vOffset( (float)( nCenter - vPos.x ) , (float)( nCenter - vPos.y ) );

	vOffset.x = ( 1.f / (float)( m_nCircleRadius / 2.f ) ) * vOffset.x;
	vOffset.y = ( 1.f / (float)( m_nCircleRadius / 2.f ) ) * vOffset.y;
	return vOffset;
}

LRESULT CPropPoolView::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	CRect rcToolbar;
	m_wndToolBar.GetClientRect( &rcToolbar );

	CPoint p;
	p.x = (long)lParam >> 16;
	p.y = (long)( lParam << 16 ) >> 16;
	p.x += m_ScrollPos.cx;
	p.y += m_ScrollPos.cy - rcToolbar.Height();
	char *szFileName = (char *)wParam;

	EtVector2 vOffset = MousePosToOffset( p );
	if( EtVec2Length( &vOffset ) > 1.f ) return S_OK;

	char szName[256] = { 0, };
	_GetFullFileName( szName, _countof(szName), szFileName );
	g_PropPoolControl.GetCurrentStage()->AddProp( szName, vOffset );
	Invalidate();

	return S_OK;
}

LRESULT CPropPoolView::OnDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	return S_OK;
}

LRESULT CPropPoolView::OnDblClickItem( WPARAM wParam, LPARAM lParam )
{
	g_PropPoolControl.GetCurrentStage()->Empty();

	char *szFileName = (char *)wParam;

	char szName[256] = { 0, };
	_GetFullFileName( szName, _countof(szName), szFileName );
	g_PropPoolControl.GetCurrentStage()->AddProp( szName, EtVector2( 0.f, 0.f ) );
	Invalidate();

	return S_OK;
}

void CPropPoolView::OnPoolEmpty()
{
	// TODO: Add your command handler code here
	g_PropPoolControl.GetCurrentStage()->Empty();
	Invalidate();
}

void CPropPoolView::OnPoolSave()
{
	// TODO: Add your command handler code here
}

void CPropPoolView::OnPoolLoadlist()
{
	// TODO: Add your command handler code here
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_POOL_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	CString szStr;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );

	g_PropPoolControl.SetCurrentStage( szStr );
}

void CPropPoolView::OnUpdatePoolEmpty(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CPropPoolView::OnUpdatePoolSave(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	bool bEnable = CFileServer::GetInstance().IsConnect();

	if( bEnable == true ) {
		CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_POOL_LOADLIST, TRUE, FALSE );
		CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
		if( pCombo->GetCurSel() == 0 ) bEnable = true;
		else bEnable = false;
	}
	pCmdUI->Enable( bEnable );
}

void CPropPoolView::OnUpdatePoolLoadlist(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}
