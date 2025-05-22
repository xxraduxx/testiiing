// BrushRenderView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "BrushRenderView.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "GlobalValue.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "SundriesFuncEx.h"

#include "MainFrm.h"
#include "PaneDefine.h"
#include "PerfCheck.h"
#include "SundriesFunc.h"
#include "EnviControl.h"
#include "EnviLightBase.h"
#include "TEtWorldDecal.h"
#include "EngineUtil.h"

// CBrushRenderView

IMPLEMENT_DYNCREATE(CBrushRenderView, CFormView)

CBrushRenderView::CBrushRenderView() : CInputReceiver( true )
	, CFormView(CBrushRenderView::IDD)
{
	m_nMouseFlag = 0;
	m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_MousePos = CPoint( 0, 0 );
	m_nVecSelectBlockList.clear();

	m_pCurAction = NULL;
	m_EditMode = EM_TERRAIN;
	m_bBrushOneBlock = false;
	m_nBrushOneBlockIndex = -1;
	m_bShowProp = false;
}

CBrushRenderView::~CBrushRenderView()
{
	SAFE_DELETE( m_pCurAction );
}

void CBrushRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBrushRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_COMMAND(ID_DK_BRUSHVIEW_1, &CBrushRenderView::OnDkBrushview1)
	ON_COMMAND(ID_DK_BRUSHVIEW_2, &CBrushRenderView::OnDkBrushview2)
	ON_COMMAND(ID_DK_BRUSHVIEW_3, &CBrushRenderView::OnDkBrushview3)
	ON_COMMAND(ID_DK_BRUSHVIEW_4, &CBrushRenderView::OnDkBrushview4)
	ON_COMMAND(ID_DK_BRUSHVIEW_5, &CBrushRenderView::OnDkBrushview5)
	ON_COMMAND(ID_DK_BRUSHVIEW_H, &CBrushRenderView::OnDkBrushviewH)
	ON_COMMAND(ID_DK_BRUSHVIEW_D, &CBrushRenderView::OnDkBrushviewD)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CBrushRenderView 진단입니다.

#ifdef _DEBUG
void CBrushRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBrushRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBrushRenderView 메시지 처리기입니다.

void CBrushRenderView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
}

void CBrushRenderView::OnPaint()
{
	CPaintDC dc(this);

	CRenderBase::GetInstance().Render( CGlobalValue::BRUSH );
}

BOOL CBrushRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

LRESULT CBrushRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::BRUSH );
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

LRESULT CBrushRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	// 브러쉬 그리깅~
	CString szStr;
	szStr = "Brush Edit Mode : ";

	switch( m_EditMode ) {
		case EM_TERRAIN:
			{
				if( CGlobalValue::GetInstance().m_nSelectLayer == 6 ) {
					szStr += "Add Decal";
				}
				else {
					szStr += "Terrain";
					if( m_bBrushOneBlock )
						szStr += " + Only Current Block";
				}

				if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) break;
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				DWORD dwColor;
				if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
				else dwColor = 0xFFFFFF00;
				pSector->UpdateBrush( m_MousePos.x, m_MousePos.y, (float)CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), dwColor );
			}
			break;
		case EM_BLOCK:
			{
				if( CGlobalValue::GetInstance().m_nSelectLayer == 6 ) {
					szStr += "Modify Decal";
				}
				else {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

					for( DWORD i=0; i<m_nVecSelectBlockList.size(); i++ ) {
						DrawBlockArea(m_nVecSelectBlockList[i]);
					}

					szStr += "Block";
				}
			}
			break;
	}
	if( CGlobalValue::GetInstance().IsCheckOutMe() )
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.05f ), szStr, 0xffffff00 );

	if( CEnviLightBase::s_pFocus ) {
		CEnviLightBase::s_pFocus->OnCustomDraw();
	}

	return S_OK;
}

void CBrushRenderView::ResetCamera()
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

LRESULT CBrushRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_MousePos = CPoint( 0, 0 );
	m_nVecSelectBlockList.clear();
	CGlobalValue::GetInstance().m_pSelectDecal = NULL;

	m_pCurAction = NULL;
	m_EditMode = EM_TERRAIN;
	m_bBrushOneBlock = false;
	m_nBrushOneBlockIndex = -1;

	CGlobalValue::GetInstance().m_nVecEditBlockList.clear();
	CGlobalValue::GetInstance().m_bEditBlock = false;
	CGlobalValue::GetInstance().m_bLockSideHeight = false;

	/*
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) pSector->ShowProp( m_bShowProp );
	}
	*/

	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );


	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();
	CGlobalValue::GetInstance().ShowProp( false );
	return S_OK;
}

LRESULT CBrushRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	if( CGlobalValue::GetInstance().IsModify( CGlobalValue::GetInstance().m_nActiveView ) )
		CGlobalValue::GetInstance().Save( CGlobalValue::GetInstance().m_nActiveView );

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->Show( false );

		// Show Side Sector 가 True 일 경우 그것들두 Release 해준다.
		SectorIndex SideSector[8];
		SideSector[0] = SectorIndex( Sector.nX - 1, Sector.nY - 1 );
		SideSector[1] = SectorIndex( Sector.nX, Sector.nY - 1 );
		SideSector[2] = SectorIndex( Sector.nX + 1, Sector.nY - 1 );
		SideSector[3] = SectorIndex( Sector.nX - 1, Sector.nY );
		SideSector[4] = SectorIndex( Sector.nX + 1, Sector.nY );
		SideSector[5] = SectorIndex( Sector.nX - 1, Sector.nY + 1 );
		SideSector[6] = SectorIndex( Sector.nX, Sector.nY + 1 );
		SideSector[7] = SectorIndex( Sector.nX + 1, Sector.nY + 1 );

		for( int i=0; i<8; i++ ) {
			if( SideSector[i].nX < 0 || SideSector[i].nX >= (int)CTEtWorld::GetInstance().GetGridX() ) continue;
			if( SideSector[i].nY < 0 || SideSector[i].nY >= (int)CTEtWorld::GetInstance().GetGridY() ) continue;

			CTEtWorldSector *pSideSector = CTEtWorld::GetInstance().GetSector( SideSector[i] );
			if( pSideSector->IsEmpty() ) continue;
			pSideSector->Show( false );
			pSideSector->FreeSector( CTEtWorldSector::SRL_TERRAIN );
		}
	}
	m_EditMode = EM_TERRAIN;
	m_nVecSelectBlockList.clear();
	if( CGlobalValue::GetInstance().m_pSelectDecal ) {
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
		CGlobalValue::GetInstance().m_pSelectDecal = NULL;
	}
	CGlobalValue::GetInstance().m_nVecEditBlockList.clear();
	CGlobalValue::GetInstance().m_nVecEditBlockList = m_nVecSelectBlockList;


	CTEtWorld::GetInstance().SetCurrentGrid( NULL );

	CWnd *pWnd = GetPaneWnd( TILEMNG_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 0, -1 );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );

	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;

	s_BrushActionCommander.Reset();

	return S_OK;
}


void CBrushRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) return;

	// Light Control
	if( CEnviLightBase::s_pFocus ) {
		if( CEnviLightBase::s_pFocus->OnInputReceive( nReceiverState, LocalTime, this ) == false ) return;
	}

	m_Camera = *CRenderBase::GetInstance().GetLastUpdateCamera();
	float fDelta = ( LocalTime - m_PrevLocalTime ) / 1000.f;
	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_nMouseFlag & RB_DOWN ) {
			m_Camera.RotateYAxis( GetMouseVariation().x / 4.f );
			m_Camera.RotatePitch( GetMouseVariation().y / 4.f );

			RefreshScene();
		}
		else if( m_nMouseFlag & WB_DOWN ) {
			m_Camera.m_vPosition += m_Camera.m_vXAxis * ( -GetMouseVariation().x ) * CGlobalValue::GetInstance().m_fCamSpeed;
			m_Camera.m_vPosition += m_Camera.m_vYAxis * ( +GetMouseVariation().y ) * CGlobalValue::GetInstance().m_fCamSpeed;

			RefreshScene();
		}

		if( IsInMouseRect( this ) ) {
			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			if( CGlobalValue::GetInstance().IsCheckOutMe() == false ) {
				if( CGlobalValue::GetInstance().m_nSelectLayer != 0 &&
					CGlobalValue::GetInstance().m_nSelectLayer != 6 ) {
					CWnd *pWnd = GetPaneWnd( LAYER_PANE );
					if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
				}
				RefreshScene();
			}
			else {
				switch( m_EditMode ) {
					case EM_TERRAIN:
						{
							if( m_nMouseFlag & LB_DOWN ) {
								ApplyTerrain( m_MousePos.x, m_MousePos.y, false );
							}
							if( CGlobalValue::GetInstance().m_nSelectLayer != 0 && 
								CGlobalValue::GetInstance().m_nSelectLayer != 6 ) {
								CWnd *pWnd = GetPaneWnd( LAYER_PANE );
								if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
							}
							RefreshScene();
						}
						break;
					case EM_BLOCK:
						if( CGlobalValue::GetInstance().m_nSelectLayer == 6 ) {
							if( m_nMouseFlag & LB_DOWN ) {
								CalcSelectDecal( m_MousePos.x, m_MousePos.y, true );
								RefreshScene();
							}
						}
						else {
							if( m_nMouseFlag & LB_DOWN ) {
								if( GetAKState( VK_LCONTROL ) < 0 ) 
									CalcBlockIndex( m_MousePos.x, m_MousePos.y, 1 );
								else if( GetAKState( VK_MENU  ) < 0 ) 
									CalcBlockIndex( m_MousePos.x, m_MousePos.y, 2 );
								RefreshScene();
							}
						}
						break;
				}
			}
		}
	}
	if( nReceiverState & IR_MOUSE_WHEEL ) {
		if( IsInMouseRect( this ) == true ) {
			if( IsPushKey( DIK_LCONTROL ) && m_EditMode == EM_BLOCK && CGlobalValue::GetInstance().m_nSelectLayer == 6 && CGlobalValue::GetInstance().m_pSelectDecal ) {
				float fRotate = CGlobalValue::GetInstance().m_pSelectDecal->GetRotate();
				fRotate -= GetMouseVariation().z * 0.02f;
				if( fRotate > 360.f ) fRotate -= 360.f;
				if( fRotate < 0.f ) fRotate += 360.f;

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector ) {
					CActionElementModifyDecal *pAction = new CActionElementModifyDecal( pSector, CGlobalValue::GetInstance().m_pSelectDecal, 2 );
					pAction->SetDesc( "Modify Decal - Rotate" );
					pAction->AddAction();
				}

				CGlobalValue::GetInstance().m_pSelectDecal->SetRotate( fRotate );

				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
			}
			else {
				m_Camera.m_vPosition += ( m_Camera.m_vZAxis * 0.4f * ( GetMouseVariation().z * CGlobalValue::GetInstance().m_fCamSpeed )  );
			}
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= LB_DOWN;

			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				switch( m_EditMode ) {
					case EM_TERRAIN:
						ApplyTerrain( m_MousePos.x, m_MousePos.y, true );
						RefreshScene();
						break;
					case EM_BLOCK:
						if( CGlobalValue::GetInstance().m_nSelectLayer == 6 ) {
							CalcSelectDecal( m_MousePos.x, m_MousePos.y, false );
						}
						else {
							if( GetAKState( VK_LCONTROL ) < 0 ) 
								CalcBlockIndex( m_MousePos.x, m_MousePos.y, 1 );
							else if( GetAKState( VK_MENU  ) < 0 ) 
								CalcBlockIndex( m_MousePos.x, m_MousePos.y, 2 );
							else CalcBlockIndex( m_MousePos.x, m_MousePos.y, 0 );
						}
						RefreshScene();
						break;
				}
			}
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;

		if( IsInMouseRect( this ) == true ) {
			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				// 마우스 업됬을때 Normal 과 Texture Coord 계산을 해준다.
				switch( m_EditMode ) {
					case EM_TERRAIN:
						FinishApplyTerrian();
						break;
					case EM_BLOCK:
						{
							if( m_pCurAction ) {
								m_pCurAction->AddAction();
								m_pCurAction = NULL;
							}
						}
						break;
				}
				RefreshScene();
			}
		}

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
			if( IsPushKey( DIK_TAB ) ) {
				ToggleEditMode();
				RefreshScene();
			}
			if( m_EditMode == EM_BLOCK && IsPushKey( DIK_LCONTROL ) && IsPushKey( DIK_A ) ) {
				CalcBlockIndex( -1, -1, 3 );
				RefreshScene();
			}
			if( IsPushKey( DIK_SPACE ) ) {
				m_bBrushOneBlock = true;
			}
			if( IsPushKey( DIK_F5 ) ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				pSector->ReloadTexture();
				RefreshScene();
			}
			if( m_EditMode == EM_BLOCK && CGlobalValue::GetInstance().m_nSelectLayer == 6 ) {
				if( CGlobalValue::GetInstance().m_pSelectDecal && IsPushKey( DIK_DELETE ) ) {
					DeleteDecal();
				}
			}
			/*
			if( IsPushKey( DIK_F2 ) ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				m_bShowProp = !m_bShowProp;
				pSector->ShowProp( m_bShowProp );
				RefreshScene();
			}
			*/
		}
	}
	if( nReceiverState & IR_KEY_UP ) {
		if( !IsPushKey( DIK_SPACE ) ) {
			m_bBrushOneBlock = false;
			m_nBrushOneBlockIndex = -1;
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CBrushRenderView::DeleteDecal()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	pSector->DeleteDecal( CGlobalValue::GetInstance().m_pSelectDecal );
	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementBrushDecal( pSector, CGlobalValue::GetInstance().m_pSelectDecal, false );
		m_pCurAction->SetDesc( "Delete Decal" );
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
		CGlobalValue::GetInstance().m_pSelectDecal->Finalize();
		CGlobalValue::GetInstance().m_pSelectDecal = NULL;
		m_pCurAction->AddAction();
		m_pCurAction = NULL;
	}
	else {
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
		SAFE_DELETE( CGlobalValue::GetInstance().m_pSelectDecal );
	}

	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	RefreshScene();
}

void CBrushRenderView::ApplyTerrain( int nX, int nY, bool bClick )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	CGlobalValue::GetInstance().SetModify();
	switch( CGlobalValue::GetInstance().m_nSelectLayer ) {
		case 0: // Height
			{
				if( m_pCurAction == NULL ) {
					m_pCurAction = new CActionElementBrushHeight( pSector );
					m_pCurAction->SetDesc( "Edit Height" );
					((CActionElementBrushHeight*)m_pCurAction)->BeginModify();
				}


				int nModifyType = CGlobalValue::GetInstance().m_nBrushType;
				if( nModifyType == 0 && GetAKState( VK_LCONTROL ) < 0 ) nModifyType = -1;
				else if( nModifyType == 2 && GetAKState( VK_LCONTROL ) < 0 ) nModifyType = -2;

				if( m_bBrushOneBlock == true && m_nBrushOneBlockIndex == -1 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector ) {
						m_nBrushOneBlockIndex = pSector->GetSelectBlock( -1, -1, false );
					}
				}

				pSector->ApplyHeight( nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_fBrushStrong * 200.f, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian, nModifyType, true, m_nBrushOneBlockIndex );
			}
			break;
		case 1:	// Layer
		case 2:
		case 3:
		case 4:
			{
				if( m_pCurAction == NULL ) {
					m_pCurAction = new CActionElementBrushAlpha( pSector );
					CString szStr;
					szStr.Format( "Edit Layer %d", CGlobalValue::GetInstance().m_nSelectLayer );
					m_pCurAction->SetDesc( szStr );
					((CActionElementBrushAlpha*)m_pCurAction)->BeginModify();
				}

				if( m_bBrushOneBlock == true && m_nBrushOneBlockIndex == -1 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector ) {
 						m_nBrushOneBlockIndex = pSector->GetSelectBlock( -1, -1, false );
					}
				}
				int nModifyType = CGlobalValue::GetInstance().m_nBrushType;
				pSector->ApplyAlpha( CGlobalValue::GetInstance().m_nSelectLayer - 1, nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_fBrushStrong / 10.f, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian, nModifyType, m_nBrushOneBlockIndex );
			}
			break;
		case 5:	// Grass
			{
				if( m_pCurAction == NULL ) {
					m_pCurAction = new CActionElementBrushGrass( pSector );
					m_pCurAction->SetDesc( "Edit Grass" );
					((CActionElementBrushGrass*)m_pCurAction)->BeginModify();
				}

				if( m_bBrushOneBlock == true && m_nBrushOneBlockIndex == -1 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector ) {
						m_nBrushOneBlockIndex = pSector->GetSelectBlock( -1, -1, false );
					}
				}

				int nModifyType = CGlobalValue::GetInstance().m_nBrushType;
				if( nModifyType == 0 && GetAKState( VK_LCONTROL ) < 0 ) nModifyType = -1;

				pSector->ApplyGrass( nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_fBrushStrong * 2500.f, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian, nModifyType, true, m_nBrushOneBlockIndex );
			}
			break;
		case 6: // Decal
			if( !bClick ) break;
			{
				CEtWorldDecal *pDecal = pSector->AddDecal( nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), pSector->GetDecalRotate(), pSector->GetDecalTextureName(), pSector->GetDecalColor(), pSector->GetDecalAlpha() );
				if( pDecal ) {
					if( m_pCurAction == NULL ) {
						m_pCurAction = new CActionElementBrushDecal( pSector, pDecal, true );
						m_pCurAction->SetDesc( "Add Decal" );
					}
					m_pCurAction->AddAction();
					m_pCurAction = NULL;
				}
				CWnd *pWnd = GetPaneWnd( DECALLIST_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

			}
			break;
	}
}

void CBrushRenderView::FinishApplyTerrian()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	switch( CGlobalValue::GetInstance().m_nSelectLayer ) {
		case 0: // Height
			{
				if( m_pCurAction ) {
					if( ((CActionElementBrushHeight*)m_pCurAction)->FinishModify() == true ) {
						((CActionElementBrushHeight*)m_pCurAction)->AddAction();
						m_pCurAction = NULL;
					}
					else SAFE_DELETE( m_pCurAction );

				}
				else assert(0);
				pSector->ResetUpdateInfo();	

				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
			}
			break;
		case 1:	// Layer
		case 2:
		case 3:
		case 4:
			if( m_pCurAction ) {
				if( ((CActionElementBrushAlpha*)m_pCurAction)->FinishModify() == true ) {
					((CActionElementBrushAlpha*)m_pCurAction)->AddAction();
					m_pCurAction = NULL;
				}
				else SAFE_DELETE( m_pCurAction );
			}
			else assert(0);

			pSector->SetLastUpdateRect( CRect( -1, -1, -1, -1 ) );
			break;
		case 5:	// Grass
			if( m_pCurAction ) {
				if( ((CActionElementBrushGrass*)m_pCurAction)->FinishModify() == true ) {
					((CActionElementBrushGrass*)m_pCurAction)->AddAction();
					m_pCurAction = NULL;
				}
				else SAFE_DELETE( m_pCurAction );
			}
			else assert(0);
			pSector->ResetUpdateGrassInfo();

			break;
	}
}

void CBrushRenderView::ToggleEditMode()
{
	EDIT_MODE PrevMode = m_EditMode;
	m_EditMode = ( m_EditMode == EM_TERRAIN ) ? EM_BLOCK : EM_TERRAIN;

	m_nVecSelectBlockList.clear();
	CGlobalValue::GetInstance().m_nVecEditBlockList.clear();
	CGlobalValue::GetInstance().m_nVecEditBlockList = m_nVecSelectBlockList;
	if( CGlobalValue::GetInstance().m_pSelectDecal ) {
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
		CGlobalValue::GetInstance().m_pSelectDecal = NULL;
	}

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	switch( PrevMode ) {
		case EM_TERRAIN:
			break;
		case EM_BLOCK:
			break;
	}

	switch( m_EditMode ) {
		case EM_TERRAIN:
			CGlobalValue::GetInstance().m_bEditBlock = false;
			break;
		case EM_BLOCK:
			CGlobalValue::GetInstance().m_bEditBlock = true;
			break;
	}
	if( CGlobalValue::GetInstance().m_nSelectLayer != 0 ) {
		CWnd *pWnd = GetPaneWnd( LAYER_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
}

void CBrushRenderView::CalcBlockIndex( int nX, int nY, char cFlag )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	int nIndex = ( cFlag == 3 ) ? -1 : pSector->GetSelectBlock( nX, nY );

	switch( cFlag ) {
		case 0:	// 단일 Select
			m_nVecSelectBlockList.clear();
			if( nIndex != -1 )
				m_nVecSelectBlockList.push_back( nIndex );
			break;
		case 1:	// Add
			if( nIndex != -1 ) {
				bool bFlag = true;
				for( DWORD i=0; i<m_nVecSelectBlockList.size(); i++ ) {
					if( m_nVecSelectBlockList[i] == nIndex ) {
						bFlag = false;
						break;
					}
				}
				if( bFlag == true ) 
					m_nVecSelectBlockList.push_back( nIndex );
			}
			break;
		case 2:	// Remove
			if( nIndex != -1 ) {
				for( DWORD i=0; i<m_nVecSelectBlockList.size(); i++ ) {
					if( m_nVecSelectBlockList[i] == nIndex ) {
						m_nVecSelectBlockList.erase( m_nVecSelectBlockList.begin() + i );
						break;
					}
				}
			}
			break;
		case 3:	// Select All
			{
				m_nVecSelectBlockList.clear();
				int nCntX, nCntY;
				pSector->GetBlockCount( nCntX, nCntY );
				for( int i=0; i<nCntY; i++ ) {
					for( int j=0; j<nCntX; j++ ) {
						m_nVecSelectBlockList.push_back( i * nCntX + j );
					}
				}
			}
			break;
	}
	CGlobalValue::GetInstance().m_nVecEditBlockList.clear();
	CGlobalValue::GetInstance().m_nVecEditBlockList = m_nVecSelectBlockList;

	if( CGlobalValue::GetInstance().m_nSelectLayer != 0 &&
		CGlobalValue::GetInstance().m_nSelectLayer != 6 ) {
		CWnd *pWnd = GetPaneWnd( LAYER_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CBrushRenderView::DrawBlockArea( int nBlockIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	if( nBlockIndex == -1 ) {
		int nBlockX, nBlockY;
		pSector->GetBlockCount( nBlockX, nBlockY );
		int nCount = 0;
		for( int i=0; i<nBlockY; i++ ) {
			for( int j=0; j<nBlockX; j++ ) {
				DrawBlockArea( nCount );
				nCount++;
			}
		}
		return;
	}

	EtVector3 vPos[8], vTemp;

	/*
	int nBlockX, nBlockY;
	int nSizeX, nSizeY;

	EtVector3 vOffset = *pSector->GetOffset();
	vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;

	pSector->GetBlockCount( nBlockX, nBlockY );
	pSector->GetBlockSize( nSizeX, nSizeY );

	int nX = nBlockIndex % nBlockX;
	int nY = nBlockIndex / nBlockX;

	// Left.Bottom 점 찾는다.
	vTemp.x = vOffset.x + ( nX * ( nSizeX * pSector->GetTileSize() ) );
	vTemp.z = vOffset.z + ( nY * ( nSizeY * pSector->GetTileSize() ) );
	*/

	// 여덜개 점을 구해노쿠
	/*
	3-------2
	|       |
	|       |
	0-------1
	*/

	/*
	vPos[0] = vTemp;

	vPos[1] = vTemp;
	vPos[1].x += nSizeX * pSector->GetTileSize();

	vPos[2] = vPos[1];
	vPos[2].z += nSizeY * pSector->GetTileSize();

	vPos[3] = vTemp;
	vPos[3].z += nSizeY * pSector->GetTileSize();

	memcpy( &vPos[4], &vPos[0], sizeof(EtVector3)*4 );
	for( int i=0; i<4; i++ )
		vPos[i].y = -10000.f;
	for( int i=4; i<8; i++ )
		vPos[i].y = 10000.f;

	*/

	SAABox Box = *pSector->GetBoundingBox( nBlockIndex );
	vPos[0].x = Box.Min.x;
	vPos[0].z = Box.Min.z;

	vPos[1].x = Box.Max.x;
	vPos[1].z = Box.Min.z;

	vPos[2].x = Box.Max.x;
	vPos[2].z = Box.Max.z;

	vPos[3].x = Box.Min.x;
	vPos[3].z = Box.Max.z;

	memcpy( &vPos[4], &vPos[0], sizeof(EtVector3)*4 );

	DWORD dwColor = D3DCOLOR_ARGB( 100, 200, 200, 200 );
	for( int i=0; i<4; i++ )
		vPos[i].y = Box.Min.y - 1000.f;
	for( int i=4; i<8; i++ )
		vPos[i].y = Box.Max.y + 1000.f;


	// 찍는다.
	// 아래바닥
	EternityEngine::DrawTriangle3D( vPos[0], vPos[1], vPos[2], dwColor );
	EternityEngine::DrawTriangle3D( vPos[0], vPos[2], vPos[3], dwColor );
	// 위천장
	EternityEngine::DrawTriangle3D( vPos[4], vPos[5], vPos[6], dwColor );
	EternityEngine::DrawTriangle3D( vPos[4], vPos[6], vPos[7], dwColor );
	// 왼쪽벽
	EternityEngine::DrawTriangle3D( vPos[0], vPos[3], vPos[7], dwColor );
	EternityEngine::DrawTriangle3D( vPos[0], vPos[7], vPos[4], dwColor );
	// 오른쪽벽
	EternityEngine::DrawTriangle3D( vPos[1], vPos[2], vPos[6], dwColor );
	EternityEngine::DrawTriangle3D( vPos[1], vPos[6], vPos[5], dwColor );
	// 앞쪽벽
	EternityEngine::DrawTriangle3D( vPos[3], vPos[2], vPos[6], dwColor );
	EternityEngine::DrawTriangle3D( vPos[3], vPos[6], vPos[7], dwColor );
	// 뒷쪽벽
	EternityEngine::DrawTriangle3D( vPos[0], vPos[1], vPos[5], dwColor );
	EternityEngine::DrawTriangle3D( vPos[0], vPos[5], vPos[4], dwColor );

}


void CBrushRenderView::OnDkBrushview1()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 1 );
}

void CBrushRenderView::OnDkBrushview2()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 2 );
}

void CBrushRenderView::OnDkBrushview3()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 3 );
}

void CBrushRenderView::OnDkBrushview4()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 4 );
}

void CBrushRenderView::OnDkBrushview5()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 5 );
}

void CBrushRenderView::OnDkBrushviewH()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 0 );
}

void CBrushRenderView::OnDkBrushviewD()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_LAYERPANE_CHANGE_LAYER, 6 );
}

void CBrushRenderView::CalcSelectDecal( int nX, int nY, bool bDrag )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	if( !bDrag ) {
		CGlobalValue::GetInstance().m_pSelectDecal = NULL;
		std::vector<CEtWorldDecal *> pVecResult;
		pSector->ScanDecal( nX, nY, 1.f, pVecResult );

		for( DWORD i=0; i<pSector->GetDecalCount(); i++ ) {
			((CTEtWorldDecal*)pSector->GetDecalFromIndex(i))->SetSelect( false );
		}

		if( !pVecResult.empty() ) {
			float fMinDist = FLT_MAX;
			CTEtWorldDecal *pDecal = NULL;
			for( DWORD i=0; i<pVecResult.size(); i++ ) {
				float fTemp = EtVec2Length( &EtVector2( pVecResult[i]->GetPosition() - EtVec3toVec2( pSector->GetPickPos() ) ) );
				if( fTemp < fMinDist ) {
					fMinDist = fTemp;
					pDecal = (CTEtWorldDecal *)pVecResult[i];
				}
			}
			CGlobalValue::GetInstance().m_pSelectDecal = pDecal;
			CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );
			m_vDecalOffset = pDecal->GetPosition() - EtVec3toVec2( pSector->GetPickPos() );
		}

		CWnd *pWnd = GetPaneWnd( LAYER_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		pWnd = GetPaneWnd( DECALLIST_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );

	}
	else {
		CTEtWorldDecal *pDecal = CGlobalValue::GetInstance().m_pSelectDecal;
		if( !pDecal ) return;

		std::vector<CEtWorldDecal *> pVecResult;
		pSector->ScanDecal( nX, nY, 1.f, pVecResult );

		if( m_pCurAction == NULL ) {
			m_pCurAction = new CActionElementModifyDecal( pSector, pDecal, 0 );
			m_pCurAction->SetDesc( "Modify Decal - Move" );
		}

		pDecal->SetPosition( EtVec3toVec2( pSector->GetPickPos() ) + m_vDecalOffset );
		pSector->UpdateDecal( pDecal );

		CWnd *pWnd = GetPaneWnd( LAYER_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
}

/*
bool CBrushRenderView::Save()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	bool bResult = pSector->SaveSector( CTEtWorldSector::SRL_PREVIEW | CTEtWorldSector::SRL_TERRAIN );

	return bResult;
}
*/


// Undo/ Redo Class
// Height Action
CActionElementBrushHeight::CActionElementBrushHeight( CTEtWorldSector *pSector )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_pHeight[0] = m_pHeight[1] = NULL;
}

CActionElementBrushHeight::~CActionElementBrushHeight()
{
	SAFE_DELETEA( m_pHeight[0] );
	SAFE_DELETEA( m_pHeight[1] );
}

bool CActionElementBrushHeight::Redo()
{
	short *pTemp = m_pSector->GetHeightPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), m_pHeight[1] + ( i * nWidth ), nWidth * sizeof(short) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateHeight( m_rcRect );
	m_pSector->ResetUpdateInfo();

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CActionElementBrushHeight::Undo()
{
	short *pTemp = m_pSector->GetHeightPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;
	bool bMakeRedo = false;
	if( m_pHeight[1] == NULL ) bMakeRedo = true;

	if( bMakeRedo ) m_pHeight[1] = new short[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		if( bMakeRedo )
			memcpy( m_pHeight[1] + ( i * nWidth ), pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), nWidth * sizeof(short) );

		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), m_pHeight[0] + ( i * nWidth ), nWidth * sizeof(short) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateHeight( m_rcRect );
	m_pSector->ResetUpdateInfo();

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CActionElementBrushHeight::BeginModify()
{
	int nSize = m_pSector->GetTileWidthCount() * m_pSector->GetTileHeightCount();
	m_pHeight[0] = new short[nSize];
	memcpy( m_pHeight[0], m_pSector->GetHeightPtr(), nSize * sizeof(short) );
}

bool CActionElementBrushHeight::FinishModify()
{
	short *pTemp;
	CRect rcLastUpdate = m_pSector->GetLastUpdateRect();
	if( rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return false;
	int nWidth = rcLastUpdate.Width() + 1;
	int nHeight = rcLastUpdate.Height() + 1;

	pTemp = new short[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( i * nWidth ), m_pHeight[0] + ( ( ( i + rcLastUpdate.top ) * m_pSector->GetTileWidthCount() ) + rcLastUpdate.left ), nWidth * sizeof(short) );
	}
	SAFE_DELETEA( m_pHeight[0] );
	m_pHeight[0] = pTemp;
	m_rcRect = rcLastUpdate;
	return true;
}


// Alpha Action
CActionElementBrushAlpha::CActionElementBrushAlpha( CTEtWorldSector *pSector )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_pAlpha[0] = m_pAlpha[1] = NULL;
}

CActionElementBrushAlpha::~CActionElementBrushAlpha()
{
	SAFE_DELETEA( m_pAlpha[0] );
	SAFE_DELETEA( m_pAlpha[1] );
}

bool CActionElementBrushAlpha::Redo()
{
	DWORD *pTemp = m_pSector->GetAlphaPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), m_pAlpha[1] + ( i * nWidth ), nWidth * sizeof(DWORD) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateAlpha( m_rcRect );
	m_pSector->SetLastUpdateRect( CRect( -1, -1, -1, -1 ) );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CActionElementBrushAlpha::Undo()
{
	DWORD *pTemp = m_pSector->GetAlphaPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;
	bool bMakeRedo = false;
	if( m_pAlpha[1] == NULL ) bMakeRedo = true;

	if( bMakeRedo ) m_pAlpha[1] = new DWORD[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		if( bMakeRedo )
			memcpy( m_pAlpha[1] + ( i * nWidth ), pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), nWidth * sizeof(DWORD) );

		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * m_pSector->GetTileWidthCount() ) + m_rcRect.left ), m_pAlpha[0] + ( i * nWidth ), nWidth * sizeof(DWORD) );
	}
	m_pSector->UpdateAlpha( m_rcRect );
	m_pSector->SetLastUpdateRect( CRect( -1, -1, -1, -1 ) );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CActionElementBrushAlpha::BeginModify()
{
	int nSize = m_pSector->GetTileWidthCount() * m_pSector->GetTileHeightCount();
	m_pAlpha[0] = new DWORD[nSize];
	memcpy( m_pAlpha[0], m_pSector->GetAlphaPtr(), nSize * sizeof(DWORD) );
}

bool CActionElementBrushAlpha::FinishModify()
{
	DWORD *pTemp;
	CRect rcLastUpdate = m_pSector->GetLastUpdateRect();
	if( rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return false;
	int nWidth = rcLastUpdate.Width() + 1;
	int nHeight = rcLastUpdate.Height() + 1;

	pTemp = new DWORD[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( i * nWidth ), m_pAlpha[0] + ( ( ( i + rcLastUpdate.top ) * m_pSector->GetTileWidthCount() ) + rcLastUpdate.left ), nWidth * sizeof(DWORD) );
	}
	SAFE_DELETEA( m_pAlpha[0] );
	m_pAlpha[0] = pTemp;
	m_rcRect = rcLastUpdate;
	return true;
}

void CBrushRenderView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}


CActionElementBrushGrass::CActionElementBrushGrass( CTEtWorldSector *pSector )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_pGrass[0] = m_pGrass[1] = NULL;
}

CActionElementBrushGrass::~CActionElementBrushGrass()
{
	SAFE_DELETEA( m_pGrass[0] );
	SAFE_DELETEA( m_pGrass[1] );
}

bool CActionElementBrushGrass::Redo()
{
	char *pTemp = m_pSector->GetGrassPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * ( m_pSector->GetTileWidthCount() - 1 ) ) + m_rcRect.left ), m_pGrass[1] + ( i * nWidth ), nWidth * sizeof(char) );
	}
	m_pSector->SetLastUpdateRect( m_rcRect );
	m_pSector->UpdateGrass( m_rcRect );
	m_pSector->SetLastUpdateRect( CRect( -1, -1, -1, -1 ) );

	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

bool CActionElementBrushGrass::Undo()
{
	char *pTemp = m_pSector->GetGrassPtr();
	int nWidth = m_rcRect.Width() + 1;
	int nHeight = m_rcRect.Height() + 1;
	bool bMakeRedo = false;
	if( m_pGrass[1] == NULL ) bMakeRedo = true;

	if( bMakeRedo ) m_pGrass[1] = new char[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		if( bMakeRedo )
			memcpy( m_pGrass[1] + ( i * nWidth ), pTemp + ( ( ( i + m_rcRect.top ) * ( m_pSector->GetTileWidthCount()-1 ) ) + m_rcRect.left ), nWidth * sizeof(char) );

		memcpy( pTemp + ( ( ( i + m_rcRect.top ) * ( m_pSector->GetTileWidthCount() - 1 ) ) + m_rcRect.left ), m_pGrass[0] + ( i * nWidth ), nWidth * sizeof(char) );
	}
	m_pSector->UpdateGrass( m_rcRect );
	m_pSector->SetLastUpdateRect( CRect( -1, -1, -1, -1 ) );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CActionElementBrushGrass::BeginModify()
{
	int nSize = ( m_pSector->GetTileWidthCount() - 1 ) * ( m_pSector->GetTileHeightCount() - 1 );
	m_pGrass[0] = new char[nSize];
	memcpy( m_pGrass[0], m_pSector->GetGrassPtr(), nSize * sizeof(char) );
}

bool CActionElementBrushGrass::FinishModify()
{
	char *pTemp;
	CRect rcLastUpdate = m_pSector->GetLastUpdateRect();
	if( rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return false;
	int nWidth = rcLastUpdate.Width() + 1;
	int nHeight = rcLastUpdate.Height() + 1;

	pTemp = new char[nWidth*nHeight];

	for( int i=0; i<nHeight; i++ ) {
		memcpy( pTemp + ( i * nWidth ), m_pGrass[0] + ( ( ( i + rcLastUpdate.top ) * ( m_pSector->GetTileWidthCount() - 1 ) ) + rcLastUpdate.left ), nWidth * sizeof(char) );
	}
	SAFE_DELETEA( m_pGrass[0] );
	m_pGrass[0] = pTemp;
	m_rcRect = rcLastUpdate;
	return true;
}

CActionElementBrushDecal::CActionElementBrushDecal( CTEtWorldSector *pSector, CEtWorldDecal *pDecal, bool bAddRemove )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_bAddRemove = bAddRemove;
	if( m_bAddRemove == true )
		m_bLastActionUndo = false;
	else m_bLastActionUndo = true;
	m_pDecal = pDecal;
}

CActionElementBrushDecal::~CActionElementBrushDecal()
{
//	SAFE_DELETE( m_pDecal );
}

bool CActionElementBrushDecal::Redo()
{
	if( m_bAddRemove == true ) {
		m_pSector->InsertDecal( m_pDecal );
		((CTEtWorldDecal*)m_pDecal)->Reinitialize( true );
		CGlobalValue::GetInstance().m_pSelectDecal = (CTEtWorldDecal*)m_pDecal;
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );

		m_bLastActionUndo = false;
	}
	else {
		m_pSector->DeleteDecal( m_pDecal );
		((CTEtWorldDecal*)m_pDecal)->Finalize();
		if( m_pDecal == CGlobalValue::GetInstance().m_pSelectDecal ) {
			CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
			CGlobalValue::GetInstance().m_pSelectDecal = NULL;
		}

		m_bLastActionUndo = true;

	}
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW  );

	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

bool CActionElementBrushDecal::Undo()
{
	if( m_bAddRemove == true ) {
		m_pSector->DeleteDecal( m_pDecal );
		((CTEtWorldDecal*)m_pDecal)->Finalize();
		if( m_pDecal == CGlobalValue::GetInstance().m_pSelectDecal ) {
			CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( false );
			CGlobalValue::GetInstance().m_pSelectDecal = NULL;
		}
		m_bLastActionUndo = true;
	}
	else {
		m_pSector->InsertDecal( m_pDecal );
		((CTEtWorldDecal*)m_pDecal)->Reinitialize( true );
		CGlobalValue::GetInstance().m_pSelectDecal = (CTEtWorldDecal*)m_pDecal;
		CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );

		m_bLastActionUndo = false;

	}
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW  );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}


CActionElementModifyDecal::CActionElementModifyDecal( CTEtWorldSector *pSector, CEtWorldDecal *pDecal, char cType )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_pDecal = pDecal;
	m_cType = cType;

	switch( cType ) {
		case 0:
			m_vModifyPos = pDecal->GetPosition();
			break;
		case 1:
			m_fModifyRadius = pDecal->GetRadius();
			break;
		case 2:
			m_fModifyRotate = pDecal->GetRotate();
			break;
		case 3:
			m_ModifyColor = pDecal->GetColor();
			break;
		case 4:
			m_fModifyAlpha = pDecal->GetAlpha();
			break;
	}
}

CActionElementModifyDecal::~CActionElementModifyDecal()
{
}

bool CActionElementModifyDecal::Redo()
{
	CGlobalValue::GetInstance().m_pSelectDecal = (CTEtWorldDecal*)m_pDecal;
	CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );
	switch( m_cType ) {
		case 0:	
			((CTEtWorldDecal*)m_pDecal)->SetPosition( m_vRedoPos );
			break;
		case 1:	
			((CTEtWorldDecal*)m_pDecal)->SetRadius( m_fRedoRadius );
			break;
		case 2:	
			((CTEtWorldDecal*)m_pDecal)->SetRotate( m_fRedoRotate );
			break;
		case 3:	
			((CTEtWorldDecal*)m_pDecal)->SetColor( m_RedoColor );
			break;
		case 4:
			((CTEtWorldDecal*)m_pDecal)->SetAlpha( m_fRedoAlpha );
			break;
	}
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW  );

	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

bool CActionElementModifyDecal::Undo()
{
	CGlobalValue::GetInstance().m_pSelectDecal = (CTEtWorldDecal*)m_pDecal;
	CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );
	switch( m_cType ) {
		case 0:	
			m_vRedoPos = m_pDecal->GetPosition();
			((CTEtWorldDecal*)m_pDecal)->SetPosition( m_vModifyPos );
			break;
		case 1:	
			m_fRedoRadius = m_pDecal->GetRadius();
			((CTEtWorldDecal*)m_pDecal)->SetRadius( m_fModifyRadius );
			break;
		case 2:	
			m_fRedoRotate = m_pDecal->GetRotate();
			((CTEtWorldDecal*)m_pDecal)->SetRotate( m_fModifyRotate );
			break;
		case 3:	
			m_RedoColor = m_pDecal->GetColor();
			((CTEtWorldDecal*)m_pDecal)->SetColor( m_ModifyColor );
			break;
		case 4:
			m_fRedoAlpha = m_pDecal->GetAlpha();
			((CTEtWorldDecal*)m_pDecal)->SetAlpha( m_fModifyAlpha );
			break;
	}
	CWnd *pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( DECALLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW  );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}
