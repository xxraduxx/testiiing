// RenderView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "GridRenderView.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "MainFrm.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "UserMessage.h"
#include "GlobalValue.h"
#include "PaneDefine.h"
#include "TEtWorldSector.h"
#include "TEtWorldGrid.h"
#include "SundriesFuncEx.h"
#include "EnviControl.h"
#include "EtActionCoreMng.h"


// CGridRenderView

IMPLEMENT_DYNCREATE(CGridRenderView, CFormView)

CGridRenderView::CGridRenderView() : CInputReceiver( true )
	,CFormView(CGridRenderView::IDD)
{
	m_nMouseFlag = 0;
	m_Camera.RotatePitch( 90.f );

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
	m_bRefreshScene = true;

	m_bPrevCamPos = false;
}

CGridRenderView::~CGridRenderView()
{
	SAFE_DELETE( m_pContextMenu );
}

void CGridRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGridRenderView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GRIDCONTEXT_CHECKIN, &CGridRenderView::OnGridcontextCheckin)
	ON_COMMAND(ID_GRIDCONTEXT_CHECKOUT, &CGridRenderView::OnGridcontextCheckout)
	ON_COMMAND(ID_GRIDCONTEXT_CREATEEMPTYSECTOR, &CGridRenderView::OnGridcontextCreateEmptySector)
	ON_COMMAND(ID_GRIDCONTEXT_EMPTYSECTOR, &CGridRenderView::OnGridcontextEmptySector)
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_COMMAND_RANGE( 10000, 10100, OnGridcontextNationCheckOut )
	ON_MESSAGE( UM_GRIDPANE_CHANGECAMERA, OnChangeCamera )
END_MESSAGE_MAP()


// CGridRenderView 진단입니다.

#ifdef _DEBUG
void CGridRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CGridRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CGridRenderView 메시지 처리기입니다.


LRESULT CGridRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::GRID );
	if( wParam == 1 ) RefreshScene();

	if( m_bRefreshScene == true ) {
		CRenderBase::GetInstance().UpdateCamera( m_Camera );
		Invalidate();
	}
	else Sleep(1);

	m_bRefreshScene = false;
	return S_OK;
}

BOOL CGridRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CGridRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::GRID );

}


LRESULT CGridRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	return S_OK;
}

void CGridRenderView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
}

void CGridRenderView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
//	CRenderBase::GetInstance().Reconstruction( cx, cy );
}

void CGridRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;
	float fDelta = ( LocalTime - m_PrevLocalTime ) / 1000.f;

	OnGridViewControl( nReceiverState, fDelta );

	m_PrevLocalTime = LocalTime;
}

void CGridRenderView::OnGridViewControl( int nReceiverState, float fDelta )
{
	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_nMouseFlag & WB_DOWN ) {
			float fSpeed = ( m_Camera.m_vPosition.y / 500.f );
			m_Camera.m_vPosition -= ( m_Camera.m_vXAxis * GetMouseVariation().x * fSpeed );
			m_Camera.m_vPosition += ( m_Camera.m_vYAxis * GetMouseVariation().y * fSpeed );
			RefreshScene();
		}
		if( m_nMouseFlag & LB_DOWN ) {
			CPoint p;
			CRect rcRect;

			GetClientRect( &rcRect );
			GetCursorPos( &p );
			ScreenToClient( &p );

			m_CurPos.x = (float)p.x;
			m_CurPos.y = (float)p.y;
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_WHEEL ) {
		if( IsInMouseRect( this ) == true ) {
			m_Camera.m_vPosition += ( m_Camera.m_vZAxis * GetMouseVariation().z * 6 );
			if( m_Camera.m_vPosition.y < 500.f )
				m_Camera.m_vPosition.y = 500.f;
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_DOWN ) {
		m_nMouseFlag |= LB_DOWN;
		if( IsInMouseRect( this ) == true ) {
			CPoint p;
			CRect rcRect;

			GetClientRect( &rcRect );
			GetCursorPos( &p );
			ScreenToClient( &p );

			m_StartPos.x = (float)p.x;
			m_StartPos.y = (float)p.y;
			m_CurPos = m_StartPos;
			
			CheckAndSelectGrid();
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
	}
	if( nReceiverState & IR_MOUSE_RB_DOWN ) {
		m_nMouseFlag |= RB_DOWN;
		if( IsInMouseRect( this ) == true ) {
			CPoint p;
			CRect rcRect;

			GetClientRect( &rcRect );
			GetCursorPos( &p );
			ScreenToClient( &p );

			m_StartPos.x = (float)p.x;
			m_StartPos.y = (float)p.y;
			m_CurPos = m_StartPos;

			CheckAndSelectGrid();
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_RB_UP ) {
		m_nMouseFlag &= ~RB_DOWN;
	}
	if( nReceiverState & IR_MOUSE_WB_DOWN ) {
		m_nMouseFlag |= WB_DOWN;
	}
	if( nReceiverState & IR_MOUSE_WB_UP ) {
		m_nMouseFlag &= ~WB_DOWN;
	}
	/*
	if( CRenderBase::GetInstance().GetCameraHandle() )
		CRenderBase::GetInstance().GetCameraHandle()->Update( m_Camera );
	*/
	CRenderBase::GetInstance().UpdateCamera( m_Camera );
}

void CGridRenderView::CheckAndSelectGrid( CEtWorldGrid *pGrid, EtVector3 &vOrig, EtVector3 &vDir, CString &szResultName, SectorIndex &ResultIndex )
{
	EtVector3 vTemp[2], vPoly[3];
	for( DWORD i=0; i<pGrid->GetGridX(); i++ ) {
		for( DWORD j=0; j<pGrid->GetGridY(); j++ ) {
			float ft, fu, fv;
			CTEtWorld::GetInstance().GetGridRect( pGrid, i, j, &vTemp[0], &vTemp[1] );

			vPoly[0] = vTemp[0];
			vPoly[1] = vTemp[1];
			vPoly[2].x = vTemp[0].x;
			vPoly[2].z = vTemp[1].z;
			vPoly[2].y = vTemp[0].y;

			if( IntersectTriangle( vOrig, vDir, vPoly[0], vPoly[1], vPoly[2], &ft, &fu, &fv ) == true ) {
				ResultIndex = SectorIndex( i, j );
				szResultName = pGrid->GetName();
				return;
			}

			vPoly[0] = vTemp[0];
			vPoly[1].x = vTemp[1].x;
			vPoly[1].z = vTemp[0].z;
			vPoly[1].y = vTemp[0].y;
			vPoly[2] = vTemp[1];

			if( IntersectTriangle( vOrig, vDir, vPoly[0], vPoly[1], vPoly[2], &ft, &fu, &fv ) == true ) {
				ResultIndex = SectorIndex( i, j );
				szResultName = pGrid->GetName();
				return;
			}

		}
	}
	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		CheckAndSelectGrid( pGrid->GetChildFromIndex(i), vOrig, vDir, szResultName, ResultIndex );
	}
}

void CGridRenderView::CheckAndSelectGrid()
{
	SectorIndex SelectGridIndex;
	CString szSelectGridName;

	// Lay 구해노쿠
	EtMatrix matView, matProj, matTemp;
	EtVector3 vOrig, vDir;
	if( CRenderBase::GetInstance().GetCameraHandle() ) {
		matView = *CRenderBase::GetInstance().GetCameraHandle()->GetViewMat();
		matProj = *CRenderBase::GetInstance().GetCameraHandle()->GetProjMat();
	}

	CRect rcRect;
	D3DXVECTOR3 vOrtho;
	CPoint ClientMousePos;

	GetCursorPos( &ClientMousePos );
	ScreenToClient( &ClientMousePos );

	GetClientRect( &rcRect );
	vOrtho.x = ( ( ( 2.f / rcRect.Width() ) * (float)ClientMousePos.x ) - 1.f ) / matProj._11;
	vOrtho.y = -( ( ( 2.f / rcRect.Height() ) * (float)ClientMousePos.y ) - 1.f ) / matProj._22;
	vOrtho.z = 1.f;

	EtMatrixInverse( &matView, NULL, &matView );

	vDir.x = vOrtho.x*matView._11 + vOrtho.y*matView._21 + vOrtho.z*matView._31;
	vDir.y = vOrtho.x*matView._12 + vOrtho.y*matView._22 + vOrtho.z*matView._32;
	vDir.z = vOrtho.x*matView._13 + vOrtho.y*matView._23 + vOrtho.z*matView._33;
	vOrig.x = matView._41;
	vOrig.y = matView._42;
	vOrig.z = matView._43;

	CheckAndSelectGrid( CTEtWorld::GetInstance().GetGrid(), vOrig, vDir, szSelectGridName, SelectGridIndex );

	bool bResult = false;
	if( SelectGridIndex != -1 && CGlobalValue::GetInstance().CheckAndSelectGrid( szSelectGridName, SelectGridIndex ) == true ) bResult = true;
	else if( SelectGridIndex == -1 ) {
		CGlobalValue::GetInstance().CheckAndSelectGrid( "", SelectGridIndex );
	}

	if( bResult == true ) {
		CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
		if( pView != NULL ) {
			bool bEnableTab = true;
			if( SelectGridIndex != -1 ) {
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( SelectGridIndex, szSelectGridName );
				if( pSector && pSector->IsEmpty() ) bEnableTab = false;
			}
			if( bEnableTab == true )
				pView->EnableTabView( -1, true );
			else {
				pView->EnableTabView( -1, false );
				pView->EnableTabView( CGlobalValue::GRID, true );
			}
		}
		CWnd *pWnd = GetPaneWnd( GRID_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

		pWnd = GetPaneWnd( GRIDLIST_PANE );
		if( pWnd ) pWnd->SendMessage( UM_GRIDLISTPANE_SELECT, (WPARAM)szSelectGridName.GetBuffer() );

		pWnd = GetPaneWnd( GRIDDESC_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)szSelectGridName.GetBuffer() );
	}
	else {
		if( CGlobalValue::GetInstance().m_CheckOutGrid == SectorIndex( -1, -1 ) ) {
			CGlobalValue::GetInstance().m_SelectGrid = SectorIndex( -1, -1 );

			CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
			if( pView != NULL ) {
				pView->EnableTabView( -1, false );
				pView->EnableTabView( CGlobalValue::GRID, true );
			}
			CWnd *pWnd = GetPaneWnd( GRID_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

			pWnd = GetPaneWnd( GRIDDESC_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		}
	}
}

// http://www.acm.org/jgt/
bool CGridRenderView::IntersectTriangle( const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, 
					   D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2, 
					   FLOAT* t, FLOAT* u, FLOAT* v )
{
	// Find vectors for two edges sharing vert0
	D3DXVECTOR3 edge1 = v1 - v0;
	D3DXVECTOR3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	D3DXVECTOR3 pvec;
	D3DXVec3Cross( &pvec, &dir, &edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	FLOAT det = D3DXVec3Dot( &edge1, &pvec );

	D3DXVECTOR3 tvec;
	if( det > 0 )
	{
		tvec = orig - v0;
	}
	else
	{
		tvec = v0 - orig;
		det = -det;
	}

	if( det < 0.0001f )
		return FALSE;

	// Calculate U parameter and test bounds
	*u = D3DXVec3Dot( &tvec, &pvec );
	if( *u < 0.0f || *u > det )
		return FALSE;

	// Prepare to test V parameter
	D3DXVECTOR3 qvec;
	D3DXVec3Cross( &qvec, &tvec, &edge1 );

	// Calculate V parameter and test bounds
	*v = D3DXVec3Dot( &dir, &qvec );
	if( *v < 0.0f || *u + *v > det )
		return FALSE;

	// Calculate t, scale parameters, ray intersects triangle
	*t = D3DXVec3Dot( &edge2, &qvec );
	FLOAT fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return TRUE;
}

void CGridRenderView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( CFileServer::GetInstance().IsConnect() == false ) return;
	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(0);

	pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CHECKOUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CHECKIN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CREATEEMPTYSECTOR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_EMPTYSECTOR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	SectorIndex SelectGrid = CGlobalValue::GetInstance().m_SelectGrid;
	SectorIndex CheckOutGrid = CGlobalValue::GetInstance().m_CheckOutGrid;
	CString szSelectGrid = CGlobalValue::GetInstance().m_szSelectGrid;
	CString szCheckOutGrid = CGlobalValue::GetInstance().m_szCheckOutGrid;
	if( !szSelectGrid.IsEmpty() && szCheckOutGrid.IsEmpty() && SelectGrid != -1 && CheckOutGrid == -1 ) {
		pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CHECKOUT, MF_BYCOMMAND | MF_ENABLED );
	}
	if( !szCheckOutGrid.IsEmpty() && CheckOutGrid != -1 ) {
		pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CHECKIN, MF_BYCOMMAND | MF_ENABLED );

		bool bEmpty = CTEtWorld::GetInstance().IsEmptySector( szCheckOutGrid, CheckOutGrid );
		if( bEmpty == true ) 
		{
			pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CREATEEMPTYSECTOR, MF_BYCOMMAND | MF_ENABLED );
		}
		else 
		{
			pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_EMPTYSECTOR, MF_BYCOMMAND | MF_ENABLED );
		}
	}
	/*
	CMenu NationMenu;
	bool bNation = CFileServer::GetInstance().IsExistNationSector( szSelectGrid.GetBuffer(), SelectGrid );

	if( bNation ) {
		pSubMenu->EnableMenuItem( ID_GRIDCONTEXT_CHECKOUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

		std::vector<CString >szVecList;
		szVecList.push_back( "Local" );
		CFileServer::GetInstance().GetNationSectorNationName( szSelectGrid.GetBuffer(), SelectGrid, szVecList );
		NationMenu.CreatePopupMenu();
		for( DWORD i=0; i<szVecList.size(); i++ ) {
			NationMenu.AppendMenu( MF_STRING, 10000 + i, szVecList[i].GetBuffer() );
		}
		if( !( !szSelectGrid.IsEmpty() && szCheckOutGrid.IsEmpty() && SelectGrid != -1 && CheckOutGrid == -1 ) ) {
			NationMenu.EnableMenuItem( 10000, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
		}

		pSubMenu->AppendMenu( MF_POPUP, (UINT)NationMenu.Detach(), "Check Out For" );
		if( !( !szSelectGrid.IsEmpty() && szCheckOutGrid.IsEmpty() && SelectGrid != -1 && CheckOutGrid == -1 ) ) {
			pSubMenu->EnableMenuItem( (UINT)NationMenu.Detach(), MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
		}
	}
	*/

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
	/*
	if( bNation ) {
		pSubMenu->DeleteMenu( 5, MF_BYPOSITION );
	}
	*/
}

void CGridRenderView::OnGridcontextCheckin()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( CFileServer::GetInstance().CheckIn() == true ) {
		if( CGlobalValue::GetInstance().Save() == false ) {
			MessageBox( "세이브 에러났삼!!", "Error", MB_OK );
		}

		CGlobalValue::GetInstance().SetModify( -1, false );
		RefreshScene();
	}
	else {
		MessageBox( "체크인 실패!!", "Error", MB_OK );
	}
	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( GRIDDESC_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGridRenderView::OnGridcontextCheckout()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( CFileServer::GetInstance().CheckOut() == true ) {
		CGlobalValue::GetInstance().SetModify( -1, true );
		RefreshScene();
	}
	else {
		MessageBox( "체크아웃 실패!!", "Error", MB_OK );
	}
	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( GRIDDESC_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGridRenderView::OnGridcontextCreateEmptySector()
{
	SectorIndex CheckOutGrid = CGlobalValue::GetInstance().m_CheckOutGrid;
	CString szCheckOutGrid = CGlobalValue::GetInstance().m_szCheckOutGrid;
	CTEtWorldGrid *pGrid = (CTEtWorldGrid *)CTEtWorld::GetInstance().FindGrid( szCheckOutGrid );
	if( pGrid == NULL ) return;
	pGrid->CreateSector( CheckOutGrid );

	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	pView->EnableTabView( -1, true );

	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGridRenderView::OnGridcontextEmptySector()
{
	if( MessageBox( "정말 비우시겠습니까?\n기존의 데이타들이 모두 날라갑니다~\n나중에 탓하지 마세요~", "경고!!!!!!", MB_YESNO ) == IDNO ) {
		return;
	}

	SectorIndex CheckOutGrid = CGlobalValue::GetInstance().m_CheckOutGrid;
	CString szCheckOutGrid = CGlobalValue::GetInstance().m_szCheckOutGrid;
	CTEtWorldGrid *pGrid = (CTEtWorldGrid *)CTEtWorld::GetInstance().FindGrid( szCheckOutGrid );
	if( pGrid == NULL ) return;
	pGrid->EmptySector( CheckOutGrid );

	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	pView->EnableTabView( -1, false );
	pView->EnableTabView( CGlobalValue::GRID, true );

	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}


LRESULT CGridRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	CGlobalValue::GetInstance().m_bMaintenanceCamera = false;
	ResetCamera();

	CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
	CEnviControl::GetInstance().ActivateElement( "Edit Set" );


	CWnd *pWnd = GetPaneWnd( GRID_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return S_OK;
}

LRESULT CGridRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	CGlobalValue::GetInstance().m_bMaintenanceCamera = false;
	m_bPrevCamPos = true;
	m_PrevCameraPos = m_Camera;
	return S_OK;
}


void CGridRenderView::ResetCamera()
{
	if( CFileServer::GetInstance().IsConnect() == false ) return;

	if( m_bPrevCamPos == true ) {
		m_Camera = m_PrevCameraPos;
	}
	else {
		m_Camera.Identity();
		m_Camera.RotatePitch( 90 );

		float fDistance = 0.f;
		m_Camera.m_vPosition.y = 10000.f;
	}

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	RefreshScene();
}

void CGridRenderView::OnGridcontextNationCheckOut( UINT nID )
{
	SectorIndex SelectGrid = CGlobalValue::GetInstance().m_SelectGrid;
	CString szSelectGrid = CGlobalValue::GetInstance().m_szSelectGrid;

	int nIndex = nID - 10001;
	if( nIndex == -1 ) {
		CFileServer::GetInstance().LockNation( "" );
	}
	else {
		std::vector<CString> szVecList;
		CFileServer::GetInstance().GetNationSectorNationName( szSelectGrid.GetBuffer(), SelectGrid, szVecList );
		CFileServer::GetInstance().LockNation( szVecList[nIndex] );
	}

	OnGridcontextCheckout();
}


LRESULT CGridRenderView::OnChangeCamera( WPARAM wParam, LPARAM lParam )
{
	EtVector3 vPos = *(EtVector3*)wParam;
	m_Camera.m_vPosition.x = vPos.x;
	m_Camera.m_vPosition.z = vPos.z;

	RefreshScene();
	return S_OK;
}