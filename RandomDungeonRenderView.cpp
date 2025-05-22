// RandomDungeonRenderView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MainFrm.h"

#include "EtWorldPainter.h"
#include "RandomDungeonRenderView.h"
#include "TEtRandomDungeon.h"
#include "EtMaze.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include "PaneDefine.h"
#include "EtMatrixEx.h"
#include "TdDummyActor.h"


// CRandomDungeonRenderView

IMPLEMENT_DYNCREATE(CRandomDungeonRenderView, CFormView)

CRandomDungeonRenderView::CRandomDungeonRenderView()
	: CFormView(CRandomDungeonRenderView::IDD)
	, CInputReceiver( true )
{
	m_bActivate = false;
	m_bDraw2DMap = true;
	m_n2DMapType = 2;
	m_nDraw2DMapLevel = 0;
}

CRandomDungeonRenderView::~CRandomDungeonRenderView()
{
}

void CRandomDungeonRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRandomDungeonRenderView, CFormView)
	ON_WM_PAINT()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CRandomDungeonRenderView 진단입니다.

#ifdef _DEBUG
void CRandomDungeonRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRandomDungeonRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CRandomDungeonRenderView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

}


// CRandomDungeonRenderView 메시지 처리기입니다.

void CRandomDungeonRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::RDUNGEON );
	
	if( m_bDraw2DMap )  {
		CTEtRandomDungeon::GetInstance().DrawMazeToDC( 10, 10, m_nDraw2DMapLevel, m_n2DMapType, &dc );
		if( CGlobalValue::GetInstance().m_bSimulation == true ) {
			CRect rcRect;
			MatrixEx *pCross = CTdDummyActor::GetInstance().GetDummyCross();
			EtVector3 *pOffset = CTEtRandomDungeon::GetInstance().GetOffset();
			IntVec3 *pSize = CTEtRandomDungeon::GetInstance().GetDungeonSize();
			float fBlockSize = CTEtRandomDungeon::GetInstance().GetBlockSize();
			
			rcRect.left = 17 + (int)( ( ( pCross->m_vPosition.x - pOffset->x ) + ( pSize->nX * fBlockSize / 2.f ) ) * ( 15.f / fBlockSize ) );
			rcRect.top = 17 + (int)( ( ( -pCross->m_vPosition.z - pOffset->z ) + ( pSize->nY * fBlockSize / 2.f ) ) * ( 15.f / fBlockSize ) );
			rcRect.right = rcRect.left + 3;
			rcRect.bottom = rcRect.top + 3;
			dc.FillSolidRect( &rcRect, RGB( 255, 0, 0 ) );
		}
	}
}

LRESULT CRandomDungeonRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	CTEtRandomDungeon::GetInstance().RenderDungeon();
	return S_OK;
}

LRESULT CRandomDungeonRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::RDUNGEON );
	if( wParam == 1 ) RefreshScene();

	if( CGlobalValue::GetInstance().IsActiveRenderView( this ) ) {
		LOCAL_TIME LocalTime = CRenderBase::GetInstance().GetLocalTime();
		if( IsPushKey( DIK_W ) ) {
			m_Camera.m_vPosition += m_Camera.m_vZAxis * ( ( 1000.f * GetPushKeyDelta(DIK_W,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		if( IsPushKey( DIK_S ) ) {
			m_Camera.m_vPosition -= m_Camera.m_vZAxis * ( ( 1000.f * GetPushKeyDelta(DIK_S,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		if( IsPushKey( DIK_A ) ) {
			m_Camera.m_vPosition -= m_Camera.m_vXAxis * ( ( 1000.f * GetPushKeyDelta(DIK_A,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		if( IsPushKey( DIK_D ) ) {
			m_Camera.m_vPosition += m_Camera.m_vXAxis * ( ( 1000.f * GetPushKeyDelta(DIK_D,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		CRenderBase::GetInstance().UpdateCamera( m_Camera );
	}
	else if( CGlobalValue::GetInstance().m_bSimulation == true ) {
		m_bRefreshScene = true;
	}

	if( m_bRefreshScene == true )
		Invalidate();
	else Sleep(1);

	m_bRefreshScene = false;

	return S_OK;
}

void CRandomDungeonRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;
	if( nReceiverState & IR_KEY_DOWN ) {
		if( IsPushKey( DIK_F5 ) ) {
			CTEtRandomDungeon::GetInstance().Destroy();
			CTEtRandomDungeon::GetInstance().Initialize( CGlobalValue::GetInstance().m_nDungeonSeed );
			CTEtRandomDungeon::GetInstance().GenerateMaze( CGlobalValue::GetInstance().m_DungeonSize.nX, CGlobalValue::GetInstance().m_DungeonSize.nY, CGlobalValue::GetInstance().m_DungeonSize.nZ,
					CGlobalValue::GetInstance().m_nDungeonSparseness, CGlobalValue::GetInstance().m_nDungeonRandomness, CGlobalValue::GetInstance().m_nDungeonDeadendRemove );
			CWnd *pWnd = GetPaneWnd( RANDOM_DEFAULT_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
			RefreshScene();
		}
		if( IsPushKey( DIK_F2 ) ) {
			m_bDraw2DMap = !m_bDraw2DMap;
			RefreshScene();
		}
		if( IsPushKey( DIK_F3 ) ) {
			m_n2DMapType++;
			if( m_n2DMapType > 2 ) m_n2DMapType = 0;
			RefreshScene();
		}
		if( IsPushKey( DIK_F4 ) ) {
			m_nDraw2DMapLevel++;
			if( m_nDraw2DMapLevel >= CTEtRandomDungeon::GetInstance().GetMaze()->GetLevel() ) m_nDraw2DMapLevel = 0;
			RefreshScene();
		}
	}

	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_nMouseFlag & RB_DOWN ) {
			m_Camera.RotateYAxis( GetMouseVariation().x / 4.f );
			m_Camera.RotatePitch( GetMouseVariation().y / 4.f );

			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_WHEEL ) {
		if( IsInMouseRect( this ) == true ) {
			m_Camera.m_vPosition += ( m_Camera.m_vZAxis * 0.4f *( GetMouseVariation().z * CGlobalValue::GetInstance().m_fCamSpeed )  );
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= LB_DOWN;

			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
	}
	if( nReceiverState & IR_MOUSE_RB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= RB_DOWN;
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

	m_PrevLocalTime = LocalTime;

	/*
	if( CRenderBase::GetInstance().GetCameraHandle() )
		CRenderBase::GetInstance().GetCameraHandle()->Update( m_Camera );
	*/
	CRenderBase::GetInstance().UpdateCamera( m_Camera );
}

BOOL CRandomDungeonRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}


LRESULT CRandomDungeonRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	return S_OK;
}

LRESULT CRandomDungeonRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	CTEtRandomDungeon::GetInstance().Destroy();
	return S_OK;
}
