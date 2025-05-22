// TriggerRenderView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerRenderView.h"
#include "PropRenderView.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "GlobalValue.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "TEtWorldProp.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"
#include "TEtTrigger.h"


// CTriggerRenderView

IMPLEMENT_DYNCREATE(CTriggerRenderView, CFormView)

CTriggerRenderView::CTriggerRenderView()
	: CFormView(CTriggerRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = m_LastClickTime = 0;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bRefreshScene = true;
	m_bActivate = false;
}

CTriggerRenderView::~CTriggerRenderView()
{
}

void CTriggerRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTriggerRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CTriggerRenderView diagnostics

#ifdef _DEBUG
void CTriggerRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTriggerRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTriggerRenderView message handlers

// CTriggerRenderView message handlers
void CTriggerRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	if( m_bActivate == true ) return;
	m_bActivate = true;

}

LRESULT CTriggerRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bRefreshScene = true;
	CGlobalValue::GetInstance().m_nPickType = -1;
	CGlobalValue::GetInstance().m_pPickStandbyElement = NULL;
	CGlobalValue::GetInstance().m_nPickParamIndex = 0;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) pSector->ShowProp( true );
	}

	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	CWnd *pWnd = GetPaneWnd( TRIGGER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( TRIGGER_VALUE_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -2 );

	return S_OK;
}

LRESULT CTriggerRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	if( CGlobalValue::GetInstance().IsModify( CGlobalValue::GetInstance().m_nActiveView ) )
		CGlobalValue::GetInstance().Save( CGlobalValue::GetInstance().m_nActiveView );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->Show( false );

		for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
			((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->SetSelect( false );
		}
		CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
		CGlobalValue::GetInstance().m_pSelectEvent = NULL;
		CGlobalValue::GetInstance().m_szSelectControlName = "";
	}
	CTEtWorld::GetInstance().SetCurrentGrid( NULL );


	CWnd *pWnd = GetPaneWnd( TRIGGER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	if( pWnd ) {
		pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
		pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -2 );
	}
	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( TRIGGER_VALUE_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );


	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;

	return S_OK;
}

LRESULT CTriggerRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

		// Area Control
		for( DWORD i=0; i<pSector->GetControlCount(); i++ ) {
			pSector->DrawArea( ((CTEtWorldEventControl*)pSector->GetControlFromIndex(i))->GetName() );
		}
	}



	return S_OK;
}

LRESULT CTriggerRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::TRIGGER );
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
		if( IsPushKey( DIK_ESCAPE ) ) {
			if( CGlobalValue::GetInstance().m_nPickType != -1 ) {
				CGlobalValue::GetInstance().m_nPickType = -1;
				CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
				if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, NULL );
			}
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

void CTriggerRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;

	/*
	// Light Control
	if( CEnviLightBase::s_pFocus ) {
		if( CEnviLightBase::s_pFocus->OnInputReceive( nReceiverState, LocalTime, this ) == false ) return;
	}
	*/

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
			if( !( m_nMouseFlag & LB_DOWN ) ) {
				GetCursorPos( &m_MousePos );
				ScreenToClient( &m_MousePos );
			}
			RefreshScene();
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

			if( CGlobalValue::GetInstance().IsCheckOutMe() && CGlobalValue::GetInstance().m_bControlAxis == false ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					bool bCheckEventArea = false;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					pSector->BeginRectBrush( m_MousePos.x, m_MousePos.y );

					// Event Area Pick
					CEtWorldEventArea *pResultArea = NULL;
					float fMinSize = FLT_MAX;
					int nControlIndex = -1;
					for( DWORD j=0; j<pSector->GetControlCount(); j++ ) {
						CTEtWorldEventControl *pControl = (CTEtWorldEventControl*)pSector->GetControlFromIndex(j);
						pSector->UpdateAreaControlType( m_MousePos.x, m_MousePos.y, pControl->GetName() );
						CEtWorldEventArea *pArea = pSector->UpdateAreaRectBrush( m_MousePos.x, m_MousePos.y, 0xFFFFFFFF, pControl->GetName() );
						if( pArea ) {
							float fSize = ( pArea->GetMax()->x - pArea->GetMin()->x ) * ( pArea->GetMax()->z - pArea->GetMin()->z );
							if( fSize < fMinSize ) {
								pResultArea = pArea;
								fMinSize = fSize;
								nControlIndex = j;
							}
						}
					}
					CGlobalValue::GetInstance().m_pSelectEvent = pResultArea;
					if( pResultArea ) {
						CWnd *pWnd = GetPaneWnd( EVENT_PANE );
						if( pWnd ) pWnd->SendMessage( UM_EVENT_PANE_CONTROL_CHANGED, (WPARAM)nControlIndex );
						bool bValid = false;
						/*
						for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
							if( pControl->GetAreaFromIndex(i) == pArea ) {
								((CMainFrame*)AfxGetMainWnd())->SetFocus( EVENT_PANE );
								SetFocus();

								bCheckEventArea = true;
								bValid = true;

								if( pWnd ) pWnd->SendMessage( UM_EVENT_PANE_SELCHANGED, (WPARAM)i );
								if( CGlobalValue::GetInstance().m_nPickType == CEtTrigger::EventArea && LocalTime - GetEventMouseTime( IR_MOUSE_LB_UP ) < 100 ) {
									CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
									if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)pArea );
								}
								break;
							}
						}
						*/
						((CMainFrame*)AfxGetMainWnd())->SetFocus( EVENT_PANE );
						SetFocus();

						bCheckEventArea = true;
						bValid = true;

						if( pWnd ) pWnd->SendMessage( UM_EVENT_PANE_SELCHANGED, (WPARAM)pResultArea->GetCreateUniqueID() );
						if( CGlobalValue::GetInstance().m_nPickType == CEtTrigger::EventArea && LocalTime - GetEventMouseTime( IR_MOUSE_LB_UP ) < 100 && GetEventMouseTime( IR_MOUSE_LB_DOWN ) > GetEventMouseTime( IR_MOUSE_LB_UP ) ) {
							CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
							if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)pResultArea );
						}

//						if( bValid ) break;
					}
					bool bCheckProp = false;
					if( bCheckEventArea == false ) {
						// Prop Pick
						CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
						CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
						pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

						DNVector(CEtWorldProp *) pVecList;
						pSector->UpdateRectBrush( m_MousePos.x, m_MousePos.y, 0x00000000, pVecList );
						for( DWORD i=0; i<pSector->GetPropCount(); i++ )
							((CTEtWorldProp*)pSector->GetPropFromIndex(i))->SetSelect( false );
						if( pVecList.size() > 0 ) {
							((CTEtWorldProp*)pVecList[0])->SetSelect( true );
							CGlobalValue::GetInstance().m_pVecSelectPropList.push_back( pVecList[0] );

							((CMainFrame*)AfxGetMainWnd())->SetFocus( PROPLIST_PANE );
							if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
							SetFocus();

							if( CGlobalValue::GetInstance().m_nPickType == CEtTrigger::Prop && LocalTime - GetEventMouseTime( IR_MOUSE_LB_UP ) < 100 && GetEventMouseTime( IR_MOUSE_LB_DOWN ) > GetEventMouseTime( IR_MOUSE_LB_UP ) ) {
								CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
								if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)pVecList[0] );
							}
							bCheckProp = true;
						}
					}
					if( CGlobalValue::GetInstance().m_nPickType == CEtTrigger::Position && LocalTime - GetEventMouseTime( IR_MOUSE_LB_UP ) < 100 && GetEventMouseTime( IR_MOUSE_LB_DOWN ) > GetEventMouseTime( IR_MOUSE_LB_UP ) ) {
						EtVector3 vPos = pSector->GetPickPos();
						CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
						if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)&vPos );
					}
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
			if( IsPushKey( DIK_DELETE ) && m_nMouseFlag == 0 ) {
				if( CGlobalValue::GetInstance().m_pSelectSound ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector != -1 ) {
						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					}
				}
			}
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CTriggerRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::TRIGGER );
}

BOOL CTriggerRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CTriggerRenderView::ResetCamera()
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

void CTriggerRenderView::RefreshProerties()
{
}

void CTriggerRenderView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}