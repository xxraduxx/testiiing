// NaviRenderView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "NaviRenderView.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "EnviControl.h"
#include "SundriesFuncEx.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "GlobalValue.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "EnviLightBase.h"


// CNaviRenderView

IMPLEMENT_DYNCREATE(CNaviRenderView, CFormView)

CNaviRenderView::CNaviRenderView()
	: CFormView(CNaviRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_pCurAction = NULL;

}

CNaviRenderView::~CNaviRenderView()
{
	SAFE_DELETE( m_pCurAction );
}

void CNaviRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNaviRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
END_MESSAGE_MAP()


// CNaviRenderView diagnostics

#ifdef _DEBUG
void CNaviRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CNaviRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CNaviRenderView message handlers

void CNaviRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();
}

LRESULT CNaviRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );

	CEnviControl::GetInstance().ActivateElement( "Edit Set" );
	CWnd *pWnd = GetPaneWnd( ENVI_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) {
			pSector->ShowProp( true );
			pSector->ShowAttribute( true );
			pSector->ShowNavigation( true );
		}
	}

	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	CTEtWorld::GetInstance().LoadNavMesh();

	return S_OK;
}

LRESULT CNaviRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	if( CGlobalValue::GetInstance().IsModify( CGlobalValue::GetInstance().m_nActiveView ) )
		CGlobalValue::GetInstance().Save( CGlobalValue::GetInstance().m_nActiveView );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->Show( false );		
	}
	CTEtWorld::GetInstance().SetCurrentGrid( NULL );

	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;
	s_NaviActionCommander.Reset();

	return S_OK;
}

LRESULT CNaviRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	CString szStr;
	szStr = "Navigation Edit Mode : ";

//	szStr += "Edit";
	if( CGlobalValue::GetInstance().IsActiveRenderView( this ) ) {

		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		DWORD dwColor;
		if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
		else dwColor = 0xFFFFFF00;
		pSector->UpdateBrush( m_MousePos.x, m_MousePos.y, (float)CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetAttributeBlockSize(), dwColor );
	}
	/*
	// Area Draw
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->DrawArea( CGlobalValue::GetInstance().m_szSelectControlName );
	}

	if( CGlobalValue::GetInstance().IsCheckOutMe() )
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.05f ), szStr, 0xffffff00 );
	*/
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

//		pSector->DrawAttribute();
//		pSector->DrawNavigationMesh();
	}

	if( CEnviLightBase::s_pFocus ) {
		CEnviLightBase::s_pFocus->OnCustomDraw();
	}

	return S_OK;
}

LRESULT CNaviRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::NAVIGATION );
	if( wParam == 1 ) RefreshScene();

	m_Camera = *CRenderBase::GetInstance().GetLastUpdateCamera();
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

		if( IsPushKey( DIK_A ) && !IsPushKey( DIK_LCONTROL ) ) {
			m_Camera.m_vPosition -= m_Camera.m_vXAxis * ( ( 1000.f * GetPushKeyDelta(DIK_A,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		if( IsPushKey( DIK_D ) ) {
			m_Camera.m_vPosition += m_Camera.m_vXAxis * ( ( 1000.f * GetPushKeyDelta(DIK_D,LocalTime) ) * CGlobalValue::GetInstance().m_fCamSpeed );
			RefreshScene();
		}

		CRenderBase::GetInstance().UpdateCamera( m_Camera );

		/*
		if( GetAKState( VK_F5 ) < 0 ) {
			SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
			if( Sector != -1 ) {
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				pSector->GenerationNavigationMesh();
			}
		}
		*/
	}
	else if( CGlobalValue::GetInstance().m_bSimulation == true ) {
		m_bRefreshScene = true;
	}

	if( m_bRefreshScene == true ) {
		CRenderBase::GetInstance().UpdateCamera( m_Camera );
		Invalidate();
	}
	else Sleep(1);

	m_bRefreshScene = false;
	return S_OK;
}

void CNaviRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;

	// Light Control
	if( CEnviLightBase::s_pFocus ) {
		if( CEnviLightBase::s_pFocus->OnInputReceive( nReceiverState, LocalTime, this ) == false ) return;
	}
	m_Camera = *CRenderBase::GetInstance().GetLastUpdateCamera();

	if( nReceiverState & IR_KEY_DOWN ) {
	}

	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_nMouseFlag & RB_DOWN ) {
			m_Camera.RotateYAxis( GetMouseVariation().x / 4.f );
			m_Camera.RotatePitch( GetMouseVariation().y / 4.f );

			RefreshScene();
		}
		else if( m_nMouseFlag & WB_DOWN ) {
			m_Camera.m_vPosition += m_Camera.m_vXAxis * ( -GetMouseVariation().x ) * CGlobalValue::GetInstance().m_fCamSpeed;
			m_Camera.m_vPosition += m_Camera.m_vYAxis * ( GetMouseVariation().y ) * CGlobalValue::GetInstance().m_fCamSpeed;

			RefreshScene();
		}
		if( IsInMouseRect( this ) ) {
			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			if( m_nMouseFlag & LB_DOWN ) {
				if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
					ApplyTerrain( m_MousePos.x, m_MousePos.y );
				}
			}

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

			m_PrevMousePos = m_MousePos;

			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				ApplyTerrain( m_MousePos.x, m_MousePos.y );
			}

			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;

		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

		if( m_pCurAction ) {
			if( ((CActionElementBrushAttribute*)m_pCurAction)->FinishModify() == true ) {
				((CActionElementBrushAttribute*)m_pCurAction)->AddAction();
				m_pCurAction = NULL;
			}
			else SAFE_DELETE( m_pCurAction );

		}
//		else assert(0);

		pSector->ResetUpdateInfo();	
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
	if( nReceiverState & IR_KEY_DOWN ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			if( IsPushKey( DIK_DELETE ) && m_nMouseFlag == 0 ) {
				if( CGlobalValue::GetInstance().m_pSelectEvent ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					/*
					if( Sector != -1 ) {
						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
						pSector->RemoveArea( CGlobalValue::GetInstance().m_szSelectControlName, CGlobalValue::GetInstance().m_pSelectEvent->GetName() );
						RefreshScene();
						RefreshProerties();
						CGlobalValue::GetInstance().SetModify();
					}
					*/
				}
			}
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CNaviRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::NAVIGATION );
}

BOOL CNaviRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CNaviRenderView::ResetCamera()
{
	if( CGlobalValue::GetInstance().m_bMaintenanceCamera == true ) {
		m_Camera = *CRenderBase::GetInstance().GetLastUpdateCamera();
		return;
	}

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;

	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector ) {
		m_Camera.Identity();
		m_Camera.m_vPosition = *pSector->GetOffset();
		m_Camera.m_vPosition.y = pSector->GetHeightToWorld( m_Camera.m_vPosition.x, m_Camera.m_vPosition.z ) + 500.f;
	}

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
}


void CNaviRenderView::ApplyTerrain( int nX, int nY )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	CGlobalValue::GetInstance().SetModify();

	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementBrushAttribute( pSector );
		m_pCurAction->SetDesc( "Edit Attribute" );
		((CActionElementBrushAttribute*)m_pCurAction)->BeginModify();
	}

	char cAttr = CGlobalValue::GetInstance().m_cSelectAttribute;
	if( GetAKState( VK_LCONTROL ) < 0 ) cAttr = 0;;

	pSector->ApplyAttribute( nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * (float)pSector->GetAttributeBlockSize(), CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_fBrushStrong * 2500.f, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian, cAttr, true );

}


// Undo/ Redo Class
// Height Action
CActionElementBrushAttribute::CActionElementBrushAttribute( CTEtWorldSector *pSector )
: CActionElement( &s_NaviActionCommander )
{
	m_pSector = pSector;
	m_pAttribute[0] = m_pAttribute[1] = NULL;
}

CActionElementBrushAttribute::~CActionElementBrushAttribute()
{
	SAFE_DELETEA( m_pAttribute[0] );
	SAFE_DELETEA( m_pAttribute[1] );
}

bool CActionElementBrushAttribute::Redo()
{
	char *pTemp = m_pSector->GetAttributePtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;
	int nWidthCount = (int)( m_pSector->GetTileWidthCount() * m_pSector->GetTileSize() ) / m_pSector->GetAttributeBlockSize();

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * nWidthCount ) + m_rcRect.left ), m_pAttribute[1] + ( i * nWidth ), nWidth * sizeof(char) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateAttribute( m_rcRect, false );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CActionElementBrushAttribute::Undo()
{
	char *pTemp = m_pSector->GetAttributePtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;
	int nWidthCount = (int)( m_pSector->GetTileWidthCount() * m_pSector->GetTileSize() ) / m_pSector->GetAttributeBlockSize();

	bool bMakeRedo = false;
	if( m_pAttribute[1] == NULL ) bMakeRedo = true;

	if( bMakeRedo ) m_pAttribute[1] = new char[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		if( bMakeRedo )
			memcpy( m_pAttribute[1] + ( i * nWidth ), pTemp + ( ( ( i + m_rcRect.top ) * nWidthCount ) + m_rcRect.left ), nWidth * sizeof(char) );

		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * nWidthCount ) + m_rcRect.left ), m_pAttribute[0] + ( i * nWidth ), nWidth * sizeof(char) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateAttribute( m_rcRect, false );
	/*
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateHeight( m_rcRect );
	m_pSector->ResetUpdateInfo();
	*/

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CActionElementBrushAttribute::BeginModify()
{
	int nWidthCount = (int)( m_pSector->GetTileWidthCount() * m_pSector->GetTileSize() ) / m_pSector->GetAttributeBlockSize();
	int nHeightCount = (int)( m_pSector->GetTileHeightCount() * m_pSector->GetTileSize() ) / m_pSector->GetAttributeBlockSize();

	int nSize = nWidthCount * nHeightCount;
	m_pAttribute[0] = new char[nSize];
	memcpy( m_pAttribute[0], m_pSector->GetAttributePtr(), nSize * sizeof(char) );
}

bool CActionElementBrushAttribute::FinishModify()
{
	char *pTemp;
	CRect rcLastUpdate = m_pSector->GetLastUpdateRect();
	if( rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return false;
	int nWidth = rcLastUpdate.Width() + 1;
	int nHeight = rcLastUpdate.Height() + 1;
	int nWidthCount = (int)( m_pSector->GetTileWidthCount() * m_pSector->GetTileSize() ) / m_pSector->GetAttributeBlockSize();

	pTemp = new char[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( i * nWidth ), m_pAttribute[0] + ( ( ( i + rcLastUpdate.top ) * nWidthCount ) + rcLastUpdate.left ), nWidth * sizeof(char) );
	}
	SAFE_DELETEA( m_pAttribute[0] );
	m_pAttribute[0] = pTemp;
	m_rcRect = rcLastUpdate;
	return true;
}
