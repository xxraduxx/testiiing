// PropRenderView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropRenderView.h"
#include "UserMessage.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "PropPoolControl.h"
#include "GlobalValue.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "TEtWorldProp.h"
#include "EnviControl.h"
#include "EnviLightBase.h"
#include "AxisRenderObject.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "DummyView.h"
#include "PropPropertiesPaneView.h"
#include "TEtWorldActProp.h"


// CPropRenderView

IMPLEMENT_DYNCREATE(CPropRenderView, CFormView)

CPropRenderView::CPropRenderView()
	: CFormView(CPropRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_bLockSelect = false;
	m_bEnableCopy = false;

	m_EditMode = EM_BRUSH;
	m_pCurAction = NULL;
	m_pOutlineFilter = NULL;
}

CPropRenderView::~CPropRenderView()
{
	SAFE_DELETE( m_pCurAction );
	SAFE_DELETE( m_pOutlineFilter );
}

void CPropRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
END_MESSAGE_MAP()


// CPropRenderView 진단입니다.

#ifdef _DEBUG
void CPropRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropRenderView 메시지 처리기입니다.

void CPropRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();
	if( !m_pOutlineFilter ) {
		m_pOutlineFilter = (CEtOutlineFilter*)EternityEngine::CreateFilter( SF_OUTLINE );;
	}
}

LRESULT CPropRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	CString szStr;
	szStr = "Prop Edit Mode : ";
	switch( m_EditMode ) {
		case EM_BRUSH:
			{
				szStr += "Add Prop";
				if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) break;
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				DWORD dwColor;
				if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
				else dwColor = 0xFFFFFF00;
				pSector->UpdateBrush( m_MousePos.x, m_MousePos.y, (float)CGlobalValue::GetInstance().m_nBrushDiameter * 100.f, dwColor );

			}
			break;
		case EM_MODIFY:
			{
				szStr += "Modify Prop";
				if( m_nMouseFlag & LB_DOWN ) {
					if( !CGlobalValue::GetInstance().IsActiveRenderView( this ) ) break;

					if( GetAKState( VK_CONTROL ) < 0 )
						CalcPropIndex( m_MousePos.x, m_MousePos.y, 1 );
					else if( GetAKState( VK_MENU ) < 0 )
						CalcPropIndex( m_MousePos.x, m_MousePos.y, 2 );
					else CalcPropIndex( m_MousePos.x, m_MousePos.y, 0 );
				}
				if( m_bLockSelect ) szStr += " - Lock";

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

LRESULT CPropRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_MousePos = CPoint( 0, 0 );

	m_EditMode = EM_BRUSH;

	CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
	CGlobalValue::GetInstance().m_bControlAxis = false;

	CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	/*
	CEnviControl::GetInstance().ActivateElement( "Edit Set" );
	pWnd = GetPaneWnd( ENVI_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	*/

	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) pSector->ShowProp( true );
	}

	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	return S_OK;
}

LRESULT CPropRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
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
	}
	CTEtWorld::GetInstance().SetCurrentGrid( NULL );

	CWnd *pWnd = GetPaneWnd( SKINMNG_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 0, -1 );

	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );

	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;

	s_PropActionCommander.Reset();

	SAFE_RELEASE_SPTR( m_hOutline );
//	m_pOutlineFilter->ClearOutlineObject();
	return S_OK;
}

LRESULT CPropRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::PROP );
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
	else {
		if( IsPushKey( DIK_LCONTROL ) && IsPushKey( DIK_C ) ) {
			int nPaneID = ((CMainFrame*)AfxGetMainWnd())->GetFocusPaneID();
			if( nPaneID == PROPPROP_PANE ) {
				if( CGlobalValue::GetInstance().m_pVecSelectPropList.size() == 1 ) {
					CWnd *pPropPropWnd = GetPaneWnd( PROPPROP_PANE );
					if( ((CPropPropertiesPaneView*)pPropPropWnd)->GetCurrentSelectIndex() == 7 ) {
						CEtWorldProp *pProp = CGlobalValue::GetInstance().m_pVecSelectPropList[0];
						if( pProp ) {
							CTEtWorldActProp *pActProp = dynamic_cast<CTEtWorldActProp*>(pProp);
							if( pActProp && pActProp->GetActionIndex() != -1 ) {
								CEtActionBase::ActionElementStruct *pStruct = pActProp->GetElement( pActProp->GetActionIndex() );
								if( pStruct ) {

									std::string szName = pStruct->szName;
									if( !szName.empty() ) {
										////////////////////////////////////////////////////////////////
										if( ::OpenClipboard( NULL ) ) {
											HGLOBAL hBlock = NULL;
											int nLength = (int)strlen( szName.c_str() );
											::EmptyClipboard();
											hBlock = ::GlobalAlloc( GMEM_SHARE, sizeof( char ) * ( nLength + 1 ) );
											if( hBlock ) {
												char *szText = ( char * )::GlobalLock( hBlock );
												if( szText ) {
													memcpy( szText, szName.c_str(), nLength*sizeof(char) );
													szText[nLength] = '\0';
													::GlobalUnlock( hBlock );
												}
												::SetClipboardData( CF_TEXT, hBlock );
											}
											::CloseClipboard();
											if( hBlock ) ::GlobalFree( hBlock );
										}
										////////////////////////////////////////////////////////////////
									}
								}
							}
						}
					}
				}
			}
		}
	}


	if( m_bRefreshScene == true ) {
		CRenderBase::GetInstance().UpdateCamera( m_Camera );
		Invalidate();
	}
	else Sleep(1);

	m_bRefreshScene = false;
	return S_OK;
}

void CPropRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
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
			CPoint pointTemp = m_MousePos;
			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				switch( m_EditMode ) {
					case EM_BRUSH:
						if( CGlobalValue::GetInstance().m_cDrawType == 1 ) {
							if( m_nMouseFlag & LB_DOWN ) {
								ApplyProp( m_MousePos.x, m_MousePos.y );
							}
						}
						RefreshScene();
						break;
					case EM_MODIFY:
						{
							if( !( m_nMouseFlag & LB_DOWN ) ) {
								ControlProp( m_MousePos.x, m_MousePos.y );
							}
							else if( m_nMouseFlag & LB_DOWN && pointTemp == m_PrevMousePos && GetAKState( VK_LSHIFT ) < 0 && m_bEnableCopy ) {
								if( m_pCurAction == NULL ) {
									m_pCurAction = new CActionElementBrushProp( true );
									m_pCurAction->SetDesc( "Clone Prop" );
								}

								SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
								if( Sector == -1 ) break;
								CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

								DNVector(CEtWorldProp *) pVecList;
								for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
									((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->SetSelect( false );

									pVecList.push_back( pSector->CloneProp( CGlobalValue::GetInstance().m_pVecSelectPropList[i], m_pCurAction ) );
									((CTEtWorldProp*)pVecList[i])->SetSelect( true );
								}
								CAxisRenderObject::m_nSelectAxis = CGlobalValue::GetInstance().m_nSelectAxis;

								CGlobalValue::GetInstance().m_pVecSelectPropList = pVecList;
								m_bEnableCopy = false;
								ReleaseAKState( VK_LSHIFT );

								m_pCurAction->AddAction();
								m_pCurAction = NULL;
							}
							RefreshScene();
						}
						break;
				}
			}
			else {
				if( m_EditMode == EM_BRUSH )
					RefreshScene();
			}
		}
		if( m_EditMode == EM_MODIFY ) {
			if( m_nMouseFlag & LB_DOWN ) {
				GetCursorPos( &m_MousePos );
				ScreenToClient( &m_MousePos );

				if( CGlobalValue::GetInstance().m_bControlAxis == true ) {
					ControlProp( (int)GetMouseVariation().x, (int)GetMouseVariation().y );
				}

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

			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			m_PrevMousePos = m_MousePos;

			switch( m_EditMode ) {
				case EM_BRUSH:
					if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
						ApplyProp( m_MousePos.x, m_MousePos.y );
						RefreshScene();
					}
					break;
				case EM_MODIFY:
					{
						SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
						if( Sector == -1 ) break;
						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
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
					}
					break;
			}
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
		switch( m_EditMode ) {
			case EM_BRUSH:
				if( CGlobalValue::GetInstance().m_cDrawType == 1 ) {
					if( m_pCurAction ) {
						m_pCurAction->AddAction();
						m_pCurAction = NULL;
					}
				}
				break;
			case EM_MODIFY:
				if( m_pCurAction ) {
					m_pCurAction->AddAction();
					m_pCurAction = NULL;
				}
				m_bEnableCopy = false;
//				CGlobalValue::GetInstance().m_bControlAxis = false;
				RefreshScene();
				break;
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
			if( m_EditMode == EM_MODIFY && IsPushKey( DIK_LCONTROL ) && IsPushKey( DIK_A ) ) {
				CalcPropIndex( -1, -1, 3 );
				RefreshScene();
			}
			if( IsPushKey( DIK_DELETE ) ) {
				DeleteProp();
				RefreshScene();
			}
			if( IsPushKey( DIK_ESCAPE ) ) {
				CalcPropIndex( -1, -1, 4 );
				RefreshScene();
			}
			if( IsPushKey( DIK_SCROLL ) ) {
				if( GetKeyState( VK_SCROLL ) == 0xffffff81 ) {
					if( CGlobalValue::GetInstance().m_pVecSelectPropList.size() > 0 ) m_bLockSelect = true;
					else {
//						keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
						m_bLockSelect = false;
					}
				}
				else m_bLockSelect = false;
				RefreshScene();
			}
		}
	}


	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CPropRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::PROP );
}

BOOL CPropRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CPropRenderView::ResetCamera()
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


void CPropRenderView::ApplyProp( int nX, int nY )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;

	CPropPoolStage *pStage = g_PropPoolControl.GetCurrentStage();
	if( !pStage ) return;
	if( pStage->IsEmpty() ) return;

	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementBrushProp( true );
		m_pCurAction->SetDesc( "Add Prop" );
	}

	pSector->AddProp( nX, nY, pStage, m_pCurAction, CGlobalValue::GetInstance().m_nBrushDiameter * 100.f, CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian,
		CGlobalValue::GetInstance().m_bRandomPosition, CGlobalValue::GetInstance().m_bRandomRotationX, CGlobalValue::GetInstance().m_bRandomRotationY, CGlobalValue::GetInstance().m_bRandomRotationZ, CGlobalValue::GetInstance().m_bLockHeightNormal,
		CGlobalValue::GetInstance().m_bLockScaleAxis, CGlobalValue::GetInstance().m_rScaleXRange, CGlobalValue::GetInstance().m_rScaleYRange, CGlobalValue::GetInstance().m_rScaleZRange, CGlobalValue::GetInstance().m_fHeightDigPer, CGlobalValue::GetInstance().m_fMinPropDistance, CGlobalValue::GetInstance().m_bIgnoreDistanceSize );

	if( CGlobalValue::GetInstance().m_cDrawType == 0 ) {
		m_pCurAction->AddAction();
		m_pCurAction = NULL;
	}

	RefreshProperties();

	CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().SetModify();
}

void CPropRenderView::DeleteProp()
{
	SAFE_RELEASE_SPTR( m_hOutline );
//	m_pOutlineFilter->ClearOutlineObject();

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;

	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementBrushProp( false );
		m_pCurAction->SetDesc( "Delete Prop" );
	}
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
		CEtWorldProp *pProp = CGlobalValue::GetInstance().m_pVecSelectPropList[i];
		if( pSector != pProp->GetSector() ) {
			assert(0);
			continue;	// 이럴 일은 없겠지?
		}
		pSector->DeleteProp( pProp );

		// 지우지 말고 Undo/Redo 리스트에 넣는다.
		((CActionElementBrushProp*)m_pCurAction)->AddProp( pProp );
//		SAFE_DELETE( pProp );
	}
	m_pCurAction->AddAction();
	m_pCurAction = NULL;

	CGlobalValue::GetInstance().m_bControlAxis = false;
	CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
	RefreshScene();
	RefreshProperties();

	CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().SetModify();
}

void CPropRenderView::ToggleEditMode()
{
	EDIT_MODE PrevMode = m_EditMode;
	m_EditMode = ( m_EditMode == EM_BRUSH ) ? EM_MODIFY : EM_BRUSH;
	
	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
		((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->SetSelect( false );
	}
	CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
	CGlobalValue::GetInstance().m_bControlAxis = false;
	m_bEnableCopy = false;
}

void CPropRenderView::RefreshProperties()
{
	CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
	if( pWnd ) {
		pWnd->SendMessage( UM_REFRESH_PANE_VIEW, ( CGlobalValue::GetInstance().m_pVecSelectPropList.size() > 0 ) ? 1 : 0 );
	}
}

void CPropRenderView::ControlProp( int nX, int nY )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

	if( CGlobalValue::GetInstance().m_bControlAxis == true && m_nMouseFlag & LB_DOWN ) {
		if( !CGlobalValue::GetInstance().m_pVecSelectPropList.empty() ) {
			EtVector3 vTempPos = ((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[0])->GetMatEx()->m_vPosition;
			float fScale = EtVec3Length( &( vTempPos - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) ) / 1000.f;

			for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
				((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->MoveAxis( nX, nY, fScale );
			}
		}
		if( nX != 0 || nY != 0 ) {
			RefreshProperties();
		}
		CGlobalValue::GetInstance().SetModify();
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
	}
	else {
		EtVector3 vOrig, vDir;
		CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, vOrig, vDir );
		int nAxisTemp = -1;
		bool bControlBody = false;
		for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
			if( ((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->CheckAxis( vOrig, vDir ) == true ) {
				if( CGlobalValue::GetInstance().m_nSelectAxis != 3 ) {
					nAxisTemp = CGlobalValue::GetInstance().m_nSelectAxis;
					break;
				}
				else bControlBody = true;
			}
		}
		if( nAxisTemp != -1 ) {
			CGlobalValue::GetInstance().m_nSelectAxis = nAxisTemp;
			CAxisRenderObject::m_nSelectAxis = nAxisTemp;
			CGlobalValue::GetInstance().m_bControlAxis = true;
		}
		else if( bControlBody ) {
			CGlobalValue::GetInstance().m_nSelectAxis = 3;
			CAxisRenderObject::m_nSelectAxis = 3;
			CGlobalValue::GetInstance().m_bControlAxis = true;
		}

		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) {
			DNVector(CEtWorldProp *) pVecList;
			pSector->PickProp( nX, nY, pVecList );
			SAFE_RELEASE_SPTR( m_hOutline );
			if( !pVecList.empty() ) {
				m_hOutline = (new CEtOutlineObject( pVecList[0]->GetObjectHandle() ) )->GetMySmartPtr();
				m_hOutline->SetColor( EtColor(0.2f, 0.5f, 1.0f, 1.0f) );
				m_hOutline->SetWidth( 1.f );
//				m_hOutline->SetSolid( true );

				float fFastSpeed = 1000.0f;
				m_hOutline->Show( true , fFastSpeed );
			} 
		}
	}
}

void CPropRenderView::CalcPropIndex( int nX, int nY, char cFlag, bool bClick )
{
	if( m_bLockSelect ) return;
	if( cFlag != 4 && CGlobalValue::GetInstance().m_bControlAxis == true ) {
		if( bClick == false ) return;
		if( CGlobalValue::GetInstance().m_nSelectAxis != 3 ) return;
	}
	DWORD dwColor;
	if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
	else dwColor = 0xFFFFFF00;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	DNVector(CEtWorldProp *) pVecList;
	if( cFlag < 3 )
		pSector->UpdateRectBrush( nX, nY, dwColor, pVecList );

	// 범위로 선택시엔 컨트롤 락되있는거 강제로 리스트에서 뺀다.
	DNVector(CEtWorldProp *) pVecListTemp;
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		if( pVecList[i]->IsControlLock() ) continue;
		pVecListTemp.push_back( pVecList[i] );
	}
	pVecList.clear();
	for( DWORD i=0; i<pVecListTemp.size(); i++ )
		pVecList.push_back( pVecListTemp[i] );
	///////////////////////////////////////////////////////////

	for( DWORD i=0; i<pSector->GetPropCount(); i++ )
		((CTEtWorldProp*)pSector->GetPropFromIndex(i))->SetSelect( false );

	switch( cFlag ) {
		case 0:
			{
				if( GetAKState( VK_SPACE ) < 0 && CGlobalValue::GetInstance().m_bControlAxis == true ) {
  					bool bValid = true;
					for( DWORD i=0; i<pVecList.size(); i++ ) {
						if( std::find( CGlobalValue::GetInstance().m_pVecSelectPropList.begin(), CGlobalValue::GetInstance().m_pVecSelectPropList.end(), pVecList[i] ) == CGlobalValue::GetInstance().m_pVecSelectPropList.end() ) {
							bValid = false;
							break;
						}
					}
					if( bValid == true ) break;
				}

				if( bClick && pVecList.empty() ) {
					CGlobalValue::GetInstance().m_bControlAxis = false;
				}
				CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
				CGlobalValue::GetInstance().m_pVecSelectPropList = pVecList;
			}
			break;
		case 1:
			for( DWORD i=0; i<pVecList.size(); i++ ) {
				if( std::find( CGlobalValue::GetInstance().m_pVecSelectPropList.begin(), CGlobalValue::GetInstance().m_pVecSelectPropList.end(), pVecList[i] ) == CGlobalValue::GetInstance().m_pVecSelectPropList.end() )
					CGlobalValue::GetInstance().m_pVecSelectPropList.push_back( pVecList[i] );
			}
			break;
		case 2:
			if( GetAKState( VK_SPACE ) < 0 && CGlobalValue::GetInstance().m_bControlAxis == true ) {
				bool bValid = true;
				for( DWORD i=0; i<pVecList.size(); i++ ) {
					if( std::find( CGlobalValue::GetInstance().m_pVecSelectPropList.begin(), CGlobalValue::GetInstance().m_pVecSelectPropList.end(), pVecList[i] ) == CGlobalValue::GetInstance().m_pVecSelectPropList.end() ) {
						bValid = false;
						break;
					}
				}
				if( bValid == true ) break;
			}

			for( DWORD j=0; j<pVecList.size(); j++ ) {
				for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
					if( pVecList[j] == CGlobalValue::GetInstance().m_pVecSelectPropList[i] ) {
						CGlobalValue::GetInstance().m_pVecSelectPropList.erase( CGlobalValue::GetInstance().m_pVecSelectPropList.begin() + i );
						break;
					}
				}
			}
			break;
		case 3:
			CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
			for( DWORD i=0; i<pSector->GetPropCount(); i++ ) {
				CGlobalValue::GetInstance().m_pVecSelectPropList.push_back( pSector->GetPropFromIndex(i) );
			}
			break;
		case 4:
			CGlobalValue::GetInstance().m_bControlAxis = false;
			CGlobalValue::GetInstance().m_pVecSelectPropList.clear();
			break;
	}

	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ )
		((CTEtWorldProp*)CGlobalValue::GetInstance().m_pVecSelectPropList[i])->SetSelect( true );

	RefreshProperties();

	CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
}

void CPropRenderView::ClearOutline() 
{ 
	/*
	if( m_pOutlineFilter ) 
		m_pOutlineFilter->ClearOutlineObject(); 
	*/
	SAFE_RELEASE_SPTR( m_hOutline );
}


CActionElementBrushProp::CActionElementBrushProp( bool bAddRemove )
: CActionElement( &s_PropActionCommander )
{
	m_bAddRemove = bAddRemove;
	if( m_bAddRemove == true )
		m_bLastActionUndo = false;
	else m_bLastActionUndo = true;
}

CActionElementBrushProp::~CActionElementBrushProp()
{
	if( m_bLastActionUndo == true ) {
		SAFE_DELETE_PVEC( m_pVecProp );
	}
	else {
		SAFE_DELETE_VEC( m_pVecProp );
	}
}

bool CActionElementBrushProp::Redo()
{
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	if( pView ) {
		CPropRenderView *pPropRenderView = dynamic_cast<CPropRenderView *>(pView->GetDummyView()->GetTabViewStruct(CGlobalValue::PROP)->pView);
		if( pPropRenderView ) pPropRenderView->ClearOutline();
	}

	if( m_bAddRemove == true ) {
		for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
			CEtWorldSector *pSector = m_pVecProp[i]->GetSector();
			((CTEtWorldProp*)m_pVecProp[i])->LoadObject();
			pSector->InsertProp( m_pVecProp[i] );
		}
		m_bLastActionUndo = false;
	}
	else {
		for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
			CEtWorldSector *pSector = m_pVecProp[i]->GetSector();
			pSector->DeleteProp( m_pVecProp[i] );
			((CTEtWorldProp*)m_pVecProp[i])->FreeObject();
		}
		m_bLastActionUndo = true;

	}
	CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

bool CActionElementBrushProp::Undo()
{
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	if( pView ) {
		CPropRenderView *pPropRenderView = dynamic_cast<CPropRenderView *>(pView->GetDummyView()->GetTabViewStruct(CGlobalValue::PROP)->pView);
		if( pPropRenderView ) pPropRenderView->ClearOutline();
	}
	if( m_bAddRemove == true ) {
		for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
			CEtWorldSector *pSector = m_pVecProp[i]->GetSector();
			pSector->DeleteProp( m_pVecProp[i] );
			((CTEtWorldProp*)m_pVecProp[i])->FreeObject();
		}
		m_bLastActionUndo = true;
	}
	else {
		for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
			CEtWorldSector *pSector = m_pVecProp[i]->GetSector();
			((CTEtWorldProp*)m_pVecProp[i])->LoadObject();
			pSector->InsertProp( m_pVecProp[i] );
		}
		m_bLastActionUndo = false;

		CGlobalValue::GetInstance().m_pVecSelectPropList.clear();

		CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
		if( pWnd ) {
			pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		}

		pWnd = GetPaneWnd( PROPLIST_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

void CActionElementBrushProp::AddProp( CEtWorldProp *pProp )
{
	if( m_bAddRemove == true ) {
	}
	else {
		((CTEtWorldProp*)pProp)->FreeObject();
	}
	m_pVecProp.push_back( pProp );
}

CActionElementModifyProp::CActionElementModifyProp( char cType )
: CActionElement( &s_PropActionCommander )
{
	m_cType = cType;
}

CActionElementModifyProp::~CActionElementModifyProp()
{
}

bool CActionElementModifyProp::Redo()
{
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	if( pView ) {
		CPropRenderView *pPropRenderView = dynamic_cast<CPropRenderView *>(pView->GetDummyView()->GetTabViewStruct(CGlobalValue::PROP)->pView);
		if( pPropRenderView ) pPropRenderView->ClearOutline();
	}
	for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
		CEtWorldProp *pProp = m_pVecProp[i];
		switch( m_cType ) {
			case 0:	
				pProp->SetPosition( m_vVecRedo[i] );	
				break;
			case 1:	
				pProp->SetRotation( m_vVecRedo[i] );	
				break;
			case 2:	
				pProp->SetScale( m_vVecRedo[i] );	
				break;
		}
		((CTEtWorldProp*)pProp)->UpdateMatrixEx();
	}
	CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, ( CGlobalValue::GetInstance().m_pVecSelectPropList.size() > 0 ) ? 1 : 0 );

	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CActionElementModifyProp::Undo()
{
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	if( pView ) {
		CPropRenderView *pPropRenderView = dynamic_cast<CPropRenderView *>(pView->GetDummyView()->GetTabViewStruct(CGlobalValue::PROP)->pView);
		if( pPropRenderView ) pPropRenderView->ClearOutline();
	}
	for( DWORD i=0; i<m_pVecProp.size(); i++ ) {
		CEtWorldProp *pProp = m_pVecProp[i];
		switch( m_cType ) {
			case 0:	
				m_vVecRedo.push_back( *pProp->GetPosition() );
				pProp->SetPosition( m_vVecModify[i] );	
				break;
			case 1:	
				m_vVecRedo.push_back( *pProp->GetRotation() );
				pProp->SetRotation( m_vVecModify[i] );	
				break;
			case 2:	
				m_vVecRedo.push_back( *pProp->GetScale() );
				pProp->SetScale( m_vVecModify[i] );	
				break;
		}
		((CTEtWorldProp*)pProp)->UpdateMatrixEx();
	}
	CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, ( CGlobalValue::GetInstance().m_pVecSelectPropList.size() > 0 ) ? 1 : 0 );

	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CActionElementModifyProp::AddProp( CEtWorldProp *pProp )
{
	m_pVecProp.push_back( pProp );
	switch( m_cType ) {
		case 0:	// Position
			m_vVecModify.push_back( *pProp->GetPrePosition() );	// 수정
			break;
		case 1:	// Rotate
			m_vVecModify.push_back( *pProp->GetPreRotation() ); // 수정
			break;
		case 2:	// Scale
			m_vVecModify.push_back( *pProp->GetScale() );
			break;
	}
}
