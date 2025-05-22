// WaterRenderView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "WaterRenderView.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "GlobalValue.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "EnviControl.h"
#include "EnviLightBase.h"
#include "SundriesFuncEx.h"
#include "EtUIType.h"
#include "EtSprite.h"
#include "PointAxisRenderObject.h"
#include "TEtWorldWater.h"
#include "TEtWorldWaterRiver.h"



// CWaterRenderView

IMPLEMENT_DYNCREATE(CWaterRenderView, CFormView)

CWaterRenderView::CWaterRenderView()
	: CFormView(CWaterRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = m_LastClickTime = 0;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bRefreshScene = true;
	m_bActivate = false;
	m_bDrawSelectLayer = false;
	m_bReadyAddRiver = false;

	m_GlobalWaterShowBackupValue = false;
}

CWaterRenderView::~CWaterRenderView()
{
}

void CWaterRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWaterRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CWaterRenderView diagnostics

#ifdef _DEBUG
void CWaterRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CWaterRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CWaterRenderView message handlers

void CWaterRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	if( m_bActivate == true ) return;
	m_bActivate = true;
}

void CWaterRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::WATER );
}

BOOL CWaterRenderView::OnEraseBkgnd(CDC* pDC)
{
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

LRESULT CWaterRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::WATER );
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

LRESULT CWaterRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	if( CEnviLightBase::s_pFocus ) {
		CEnviLightBase::s_pFocus->OnCustomDraw();
	}
	return S_OK;
}

void CWaterRenderView::RenderCustom( float fElapsedTime )
{
	if( m_bDrawSelectLayer ) {
		EtMatrix *pmat = CRenderBase::GetInstance().GetCameraHandle()->GetViewProjMat();
		SAABox Box;
		SUICoord CoordRect;
		int nWidth = CEtDevice::GetInstance().Width();
		int nHeight = CEtDevice::GetInstance().Height();
		float fWidth = 1.f / nWidth;
		float fHeight = 1.f / nHeight;
		float fX1 = fWidth * (float)m_PrevMousePos.x;
		float fY1 = fHeight * (float)m_PrevMousePos.y;
		float fX2 = fWidth * (float)m_MousePos.x;
		float fY2 = fHeight * (float)m_MousePos.y;
		CoordRect.fX = min( fX1, fX2 );
		CoordRect.fY = min( fY1, fY2 );
		CoordRect.fWidth = fabs( fX1 - fX2 );
		CoordRect.fHeight = fabs( fY1 - fY2 );
		CEtSprite::GetInstance().DrawRect( CoordRect, 0x441010DD );
	}
}

LRESULT CWaterRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_GlobalWaterShowBackupValue = CGlobalValue::GetInstance().m_bShowWater;

	CGlobalValue::GetInstance().ShowWater(true);

	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bRefreshScene = true;
	CGlobalValue::GetInstance().m_bCanDragTile = true;

	CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) {
			pSector->ShowProp( true );
			pSector->ShowWater( true );
		}
	}
	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	CWnd *pWnd = GetPaneWnd( WATER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( WATERPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	return S_OK;
}

LRESULT CWaterRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	if( CGlobalValue::GetInstance().IsModify( CGlobalValue::GetInstance().m_nActiveView ) )
		CGlobalValue::GetInstance().Save( CGlobalValue::GetInstance().m_nActiveView );
	CGlobalValue::GetInstance().m_bCanDragTile = false;
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->Show( false );
		pSector->ShowWater(false);

		//백업되어 있는 값으로 변경..
		CGlobalValue::GetInstance().ShowWater(m_GlobalWaterShowBackupValue);
	}
	CTEtWorld::GetInstance().SetCurrentGrid( NULL );

	CWnd *pWnd = GetPaneWnd( WATERPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;
	CGlobalValue::GetInstance().m_pVecSelectPointList.clear();
	CGlobalValue::GetInstance().m_pVecSelectRiverList.clear();

	return S_OK;
}

void CWaterRenderView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
}

void CWaterRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;

	// Light Control
	if( CEnviLightBase::s_pFocus ) {
	if( CEnviLightBase::s_pFocus->OnInputReceive( nReceiverState, LocalTime, this ) == false ) return;
	}

	m_Camera = *CRenderBase::GetInstance().GetLastUpdateCamera();

	if( nReceiverState & IR_KEY_DOWN ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			if( IsPushKey( DIK_DELETE ) ) {
				DeletePoint();
				RefreshScene();
			}
		}
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
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			if( IsInMouseRect( this ) ) {
				if( !( m_nMouseFlag & LB_DOWN ) ) {
					GetCursorPos( &m_MousePos );
					ScreenToClient( &m_MousePos );

					ControlPoint( m_MousePos.x, m_MousePos.y );
				}
				else {
					GetCursorPos( &m_MousePos );
					ScreenToClient( &m_MousePos );
					if( m_bDrawSelectLayer ) {
						if( IsPushKey( DIK_LCONTROL ) )
							CalcPointIndex( m_MousePos.x, m_MousePos.y, 1 );
						else if( IsPushKey( DIK_LMENU ) )
							CalcPointIndex( m_MousePos.x, m_MousePos.y, 2 );
						else CalcPointIndex( m_MousePos.x, m_MousePos.y, 0 );
					}
					else {
						if( CGlobalValue::GetInstance().m_bControlAxis == true ) { 
							if( m_bReadyAddRiver == false && CGlobalValue::GetInstance().m_pVecSelectRiverList.size() == 1 && CGlobalValue::GetInstance().m_pVecSelectPointList.size() == 1 && GetAsyncKeyState( VK_LSHIFT ) & 0x8000 ) {
								SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
								CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
								CTEtWorldWaterRiver::RiverPoint *pPoint = pSector->AddRiverPoint( CGlobalValue::GetInstance().m_pVecSelectRiverList[0], CGlobalValue::GetInstance().m_pVecSelectPointList[0] );

								CGlobalValue::GetInstance().m_pVecSelectPointList[0]->bSelect = false;
								CGlobalValue::GetInstance().m_pVecSelectPointList[0]->pAxis->Show( false );
								CGlobalValue::GetInstance().m_pVecSelectPointList.clear();
								CGlobalValue::GetInstance().m_pVecSelectPointList.push_back( pPoint );
								CPointAxisRenderObject::m_nSelectAxis = CGlobalValue::GetInstance().m_nSelectAxis;
								m_bReadyAddRiver = true;
							}
						}


						ControlPoint( (int)GetMouseVariation().x, (int)GetMouseVariation().y );
					}
				}
				RefreshScene();
			}
		}
		if( m_nMouseFlag & LB_DOWN ) {
			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );
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

			SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
			/*
			pSector->BeginRectBrush( m_MousePos.x, m_MousePos.y );
			if( GetAKState( VK_LSHIFT ) < 0 ) {
				int nCount = (int)CGlobalValue::GetInstance().m_pVecSelectPropList.size();

				if( (int)CGlobalValue::GetInstance().m_pVecSelectPropList.size() == nCount ) {
					m_bEnableCopy = true;
				}
			}
			if( GetAKState( VK_LCONTROL ) < 0 )
				CalcPropIndex( m_MousePos.x, m_MousePos.y, 1, true );
			else if( GetAKState( VK_MENU ) < 0 )
				CalcPropIndex( m_MousePos.x, m_MousePos.y, 2, true );
			else CalcPropIndex( m_MousePos.x, m_MousePos.y, 0, true );
			RefreshScene();
			*/

			if( !CGlobalValue::GetInstance().m_bControlAxis ) {
				if( IsPushKey( DIK_LSHIFT ) ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					CTEtWorldWaterRiver *pRiver = pSector->ApplyRiver( m_PrevMousePos.x, m_PrevMousePos.y );
					if( pRiver ) {
						CWnd *pWnd = GetPaneWnd( WATER_PANE );
						if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
						/*
						if( pWnd ) pWnd->SendMessage( UM_WATER_PANE_ADDRIVER, (WPARAM)pRiver );
						*/
						pWnd->SendMessage( UM_WATER_PANE_SELECTRIVER );

						m_bReadyAddRiver = true;
					}
				}
				else {
					m_bReadyAddRiver = false;

					if( IsPushKey( DIK_LCONTROL ) )
						CalcPointIndex( m_MousePos.x, m_MousePos.y, 1, true );
					else if( IsPushKey( DIK_LMENU ) )
						CalcPointIndex( m_MousePos.x, m_MousePos.y, 2, true );
					else CalcPointIndex( m_MousePos.x, m_MousePos.y, 0, true );
				}

				if( !m_bReadyAddRiver && !CGlobalValue::GetInstance().m_bControlAxis ) {
					m_bDrawSelectLayer = true;
				}
			}

			RefreshScene();
		}
	}

	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
		if( IsInMouseRect( this ) == true ) {
			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					RefreshScene();
				}
			}
		}
		m_bDrawSelectLayer = false;
		m_bReadyAddRiver = false;
	}
	if( nReceiverState & IR_MOUSE_RB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= RB_DOWN;
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
	if( nReceiverState & IR_KEY_DOWN ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CWaterRenderView::ResetCamera()
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

void CWaterRenderView::ControlPoint( int nX, int nY )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	if( CGlobalValue::GetInstance().m_bControlAxis == true && m_nMouseFlag & LB_DOWN ) {
		if( !CGlobalValue::GetInstance().m_pVecSelectPointList.empty() ) {
			EtVector3 vTempPos = CGlobalValue::GetInstance().m_pVecSelectPointList[0]->vPos;
			float fScale = EtVec3Length( &( vTempPos - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) ) / 1000.f;

			for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPointList.size(); i++ ) {
				CGlobalValue::GetInstance().m_pVecSelectPointList[i]->pAxis->MoveAxis( nX, nY, fScale );
			}
			for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectRiverList.size(); i++ ) {
				CGlobalValue::GetInstance().m_pVecSelectRiverList[i]->UpdatePoint();
			}
		}
		if( nX != 0 || nY != 0 ) {
			RefreshProperties();
		}
		CGlobalValue::GetInstance().SetModify();
		/*
		if( !m_pCurAction ) {
			char cModifyType = 0;
			if( GetAKState( VK_LCONTROL ) < 0 ) cModifyType = 1;
			if( GetAKState( VK_LMENU ) < 0 ) cModifyType = 2;
			m_pCurAction = new CActionElementModifyProp( cModifyType );
			for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
				((CActionElementModifyProp*)m_pCurAction)->AddProp( CGlobalValue::GetInstance().m_pVecSelectPropList[i] );
			}
			if( cModifyType == 0 )
				m_pCurAction->SetDesc( "Modify Prop - Move" );
			else if( cModifyType == 1 )
				m_pCurAction->SetDesc( "Modify Prop - Rotate" );
			else if( cModifyType == 2 )
				m_pCurAction->SetDesc( "Modify Prop - Scale" );
		}
		*/
	}
	else {
		EtVector3 vOrig, vDir;
		CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, vOrig, vDir );
		for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPointList.size(); i++ ) {
			if( CGlobalValue::GetInstance().m_pVecSelectPointList[i]->pAxis->CheckAxis( vOrig, vDir ) != -1 ) break;
		}
	}

}

void CWaterRenderView::CalcPointIndex( int nX, int nY, char cFlag, bool bClick )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	EtVector2 vMin, vMax;
	float fX[2]; 
	float fY[2]; 

	fX[0] = ( 2.f / (float)CEtDevice::GetInstance().Width() ) * m_PrevMousePos.x;
	fY[0] = ( 2.f / (float)CEtDevice::GetInstance().Height() ) * -m_PrevMousePos.y;

	fX[1] = ( 2.f / (float)CEtDevice::GetInstance().Width() ) * m_MousePos.x;
	fY[1] = ( 2.f / (float)CEtDevice::GetInstance().Height() ) * -m_MousePos.y;

	if( fX[0] < fX[1] ) {
		vMin.x = fX[0];
		vMax.x = fX[1];
	}
	else {
		vMin.x = fX[1];
		vMax.x = fX[0];
	}
	if( fY[0] < fY[1] ) {
		vMin.y = fY[0];
		vMax.y = fY[1];
	}
	else {
		vMin.y = fY[1];
		vMax.y = fY[0];
	}
	vMin.x -= 1.f;
	vMax.x -= 1.f;
	vMin.y += 1.f;
	vMax.y += 1.f;
	if( bClick == true && vMin == vMax ) {
		vMin.x -= 0.01f;
		vMax.x += 0.01f;
		vMin.y -= 0.01f;
		vMax.y += 0.01f;
		if( cFlag == 0 ) {
			CGlobalValue::GetInstance().m_pVecSelectRiverList.clear();
			for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPointList.size(); i++ ) {
				CGlobalValue::GetInstance().m_pVecSelectPointList[i]->bSelect = false;
				CGlobalValue::GetInstance().m_pVecSelectPointList[i]->pAxis->Show( false );
			}
			CGlobalValue::GetInstance().m_pVecSelectPointList.clear();
			cFlag = -1;
		}
	}

	CTEtWorldWater *pWater = (CTEtWorldWater*)pSector->GetWater();
	for( DWORD i=0; i<pWater->GetRiverCount(); i++ ) {
		CTEtWorldWaterRiver *pRiver = (CTEtWorldWaterRiver *)pWater->GetRiverFromIndex(i);
		if( pRiver->CheckSelect( vMin, vMax, cFlag ) == true ) {
			if( cFlag == -1 ) break;
		}
	}
	CWnd *pWnd = GetPaneWnd( WATER_PANE );
	pWnd->SendMessage( UM_WATER_PANE_SELECTRIVER );
}

void CWaterRenderView::RefreshProperties()
{
}

void CWaterRenderView::DeletePoint()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPointList.size(); i++ ) {
		pSector->DeleteRiverPoint( CGlobalValue::GetInstance().m_pVecSelectPointList[i] );
	}

	CGlobalValue::GetInstance().m_pVecSelectRiverList.clear();
	CGlobalValue::GetInstance().m_pVecSelectPointList.clear();

	CWnd *pWnd = GetPaneWnd( WATER_PANE );
	pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}