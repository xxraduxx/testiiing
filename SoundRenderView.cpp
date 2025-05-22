// SoundRenderView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundRenderView.h"
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
#include "EnviControl.h"
#include "EnviLightBase.h"
#include "TEtWorldSound.h"
#include "TEtWorldSoundEnvi.h"


// CSoundRenderView

IMPLEMENT_DYNCREATE(CSoundRenderView, CFormView)

CSoundRenderView::CSoundRenderView()
	: CFormView(CSoundRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bActivate = false;
	m_bCanceled = false;

	m_pCurAction = NULL;
}

CSoundRenderView::~CSoundRenderView()
{
	SAFE_DELETE( m_pCurAction );
}

void CSoundRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
END_MESSAGE_MAP()


// CSoundRenderView diagnostics

#ifdef _DEBUG
void CSoundRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSoundRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSoundRenderView message handlers
void CSoundRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	if( m_bActivate == true ) return;
	m_bActivate = true;

}

LRESULT CSoundRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );

	/*
	CEnviControl::GetInstance().ActivateElement( "Edit Set" );
	CWnd *pWnd = GetPaneWnd( SOUNDLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	*/

	CGlobalValue::GetInstance().m_pSelectSound = NULL;
	CGlobalValue::GetInstance().m_bControlAxis = false;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) pSector->ShowProp( true );
	}

	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	return S_OK;
}

LRESULT CSoundRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
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

	s_SoundActionCommander.Reset();

	return S_OK;
}

LRESULT CSoundRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	CString szStr;
	szStr = "Sound Edit Mode : ";

	szStr += "Edit";
	if( CGlobalValue::GetInstance().IsActiveRenderView( this ) ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

			if( m_nMouseFlag & LB_DOWN ) {
				if( CGlobalValue::GetInstance().m_pSelectSound == NULL ) {
					DWORD dwColor;
					if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
					else dwColor = 0xFFFFFF00;

					pSector->UpdateSoundRectBrush( m_MousePos.x, m_MousePos.y, dwColor );
				}
				else {
				}
			}
		}
	}

	// Sound Draw
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->DrawSound();
	}

	if( CGlobalValue::GetInstance().IsCheckOutMe() )
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.05f ), szStr, 0xffffff00 );

	if( CEnviLightBase::s_pFocus ) {
		CEnviLightBase::s_pFocus->OnCustomDraw();
	}

	return S_OK;
}

LRESULT CSoundRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::SOUND );
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

void CSoundRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
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
			if( !( m_nMouseFlag & LB_DOWN ) ) {
				GetCursorPos( &m_MousePos );
				ScreenToClient( &m_MousePos );

				ControlSound( m_MousePos.x, m_MousePos.y );
			}
			RefreshScene();
		}
		if( m_nMouseFlag & LB_DOWN ) {
			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			if( CGlobalValue::GetInstance().m_bControlAxis == true ) {
				ControlSound( (int)GetMouseVariation().x, (int)GetMouseVariation().y );

				RefreshScene();
			}
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
			m_bCanceled = false;

			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			m_PrevMousePos = m_MousePos;

			if( CGlobalValue::GetInstance().IsCheckOutMe() && CGlobalValue::GetInstance().m_bControlAxis == false ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector->BeginRectBrush( m_MousePos.x, m_MousePos.y ) == false ) {
						m_bCanceled = true;
					}

					CTEtWorldSoundEnvi *pSound = (CTEtWorldSoundEnvi*)pSector->UpdateSoundRectBrush( m_MousePos.x, m_MousePos.y, 0xFFFFFF00 );
					CGlobalValue::GetInstance().m_pSelectSound = pSound;
					if( pSound ) {
						CWnd *pWnd = GetPaneWnd( SOUNDLIST_PANE );
						if( pWnd ) pWnd->SendMessage( UM_SOUNDLIST_PANE_SELCHANGED, (WPARAM)pSound->GetName() );
					}
					else {
						CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
						if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
					}
				}
			}

			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
		if( IsInMouseRect( this ) == true ) {
			if( CGlobalValue::GetInstance().IsCheckOutMe() && m_bCanceled == false ) {
				if( CGlobalValue::GetInstance().m_bControlAxis == true ) {
					if( m_pCurAction ) {
						m_pCurAction->AddAction();
						m_pCurAction = NULL;
					}
				}
				else {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector != -1 ) {
						RefreshScene();
						if( CGlobalValue::GetInstance().m_pSelectSound == NULL ) {
							CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
							CString szName = GetUniqueSoundName();
							if( pSector->AddSound( szName ) == true ) {
								RefreshScene();
								CWnd *pWnd = GetPaneWnd( SOUNDLIST_PANE );
								if( pWnd ) pWnd->SendMessage( UM_SOUNDLIST_PANE_ADDSOUNDENVI, (WPARAM)szName.GetBuffer() );
								CGlobalValue::GetInstance().SetModify();
							}
						}
					}
				}
			}
		}
	}
	if( nReceiverState & IR_MOUSE_RB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= RB_DOWN;
			// 사운드 박을땐 켄슬이 먹게해주자.
			m_bCanceled = true;
			m_nMouseFlag &= ~LB_DOWN;
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
						pSector->RemoveSound( CGlobalValue::GetInstance().m_pSelectSound->GetName() );

						CGlobalValue::GetInstance().m_pSelectSound = NULL;
						CGlobalValue::GetInstance().m_bControlAxis = false;

						RefreshScene();
						RefreshProerties();
						CGlobalValue::GetInstance().SetModify();
					}
				}
			}
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CSoundRenderView::ControlSound( int nX, int nY )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	if( CGlobalValue::GetInstance().m_bControlAxis == true && m_nMouseFlag & LB_DOWN ) {
		if( CGlobalValue::GetInstance().m_pSelectSound ) {
			if( m_pCurAction == NULL ) {
				m_pCurAction = new CActionElementModifySound( 0, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
				m_pCurAction->SetDesc( "Modify SoundEnvi - Move" );
			}

			((CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound)->MoveAxis( nX, nY );
			CGlobalValue::GetInstance().SetModify();
			CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 5 );
		}
	}
	else if( CGlobalValue::GetInstance().m_pSelectSound ) {
		EtVector3 vOrig, vDir;
		CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, vOrig, vDir );
		((CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound)->CheckAxis( vOrig, vDir );
	}
}

void CSoundRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::SOUND );
}

BOOL CSoundRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CSoundRenderView::ResetCamera()
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

void CSoundRenderView::RefreshProerties()
{
	CWnd *pWnd = GetPaneWnd( SOUNDLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}



CString CSoundRenderView::GetUniqueSoundName()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return CString("");
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return CString("");

	CString szTemp;

	int nCount = 0;
	while(1) {
		szTemp.Format( "SoundObject - %03d", nCount );
		if( !pSector->GetSoundInfo()->GetEnviFromName( szTemp ) ) break;
		nCount++;
	}
	return szTemp;
}


CActionElementModifySound::CActionElementModifySound( char cType, CTEtWorldSoundEnvi *pSoundEnvi )
: CActionElement( &s_SoundActionCommander )
{
	m_pSoundEnvi = pSoundEnvi;
	m_cType = cType;

	switch( m_cType ) {
		case 0:	
			m_vModifyPos = *m_pSoundEnvi->GetPosition();
			break;
		case 1:	
			m_fModifyRange = m_pSoundEnvi->GetRange();
			break;
		case 2:	
			m_fModifyRollOff = m_pSoundEnvi->GetRollOff();
			break;
		case 3:	
			m_fModifyVolume = m_pSoundEnvi->GetVolume();
			break;
		case 4:	
			m_bModifyStream = m_pSoundEnvi->IsStream();
			break;
	}
}

CActionElementModifySound::~CActionElementModifySound()
{
}

bool CActionElementModifySound::Redo()
{
	CGlobalValue::GetInstance().m_pSelectSound = m_pSoundEnvi;
	switch( m_cType ) {
		case 0:	
			m_pSoundEnvi->SetPosition( m_vRedoPos );	
			break;
		case 1:	
			m_pSoundEnvi->SetRange( m_fRedoRange );
			break;
		case 2:	
			m_pSoundEnvi->SetRollOff( m_fRedoRollOff );	
			break;
		case 3:	
			m_pSoundEnvi->SetVolume( m_fRedoVolume );	
			break;
		case 4:
			m_pSoundEnvi->SetStream( m_bRedoStream );	
			break;
	}

	CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 5 );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CActionElementModifySound::Undo()
{
	CGlobalValue::GetInstance().m_pSelectSound = m_pSoundEnvi;
	switch( m_cType ) {
		case 0:	
			m_vRedoPos = *m_pSoundEnvi->GetPosition();
			m_pSoundEnvi->SetPosition( m_vModifyPos );
			break;
		case 1:	
			m_fRedoRange = m_pSoundEnvi->GetRange();
			m_pSoundEnvi->SetRange( m_fModifyRange );	
			break;
		case 2:	
			m_fRedoRollOff = m_pSoundEnvi->GetRollOff();
			m_pSoundEnvi->SetRollOff( m_fModifyRollOff );	
			break;
		case 3:	
			m_fRedoVolume = m_pSoundEnvi->GetVolume();
			m_pSoundEnvi->SetVolume( m_fModifyVolume );	
			break;
		case 4:
			m_bRedoStream = m_pSoundEnvi->IsStream();
			m_pSoundEnvi->SetStream( m_bModifyStream );	
			break;
	}
	CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 5 );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}
