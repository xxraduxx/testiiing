// EventRenderView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventRenderView.h"
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
#include "EtWorldEventControl.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"
#include "PluginManager.h"
#include "EtEngine.h"


// CEventRenderView

IMPLEMENT_DYNCREATE(CEventRenderView, CFormView)

CEventRenderView::CEventRenderView()
	: CFormView(CEventRenderView::IDD)
	, CInputReceiver( true )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos2 = m_PrevMousePos = m_MousePos = CPoint( 0, 0 );
	m_pCursor = IDC_ARROW;
	m_nControlType = -1;
	m_pCurAction = NULL;
}

CEventRenderView::~CEventRenderView()
{
	SAFE_DELETE( m_pCurAction );
}

void CEventRenderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEventRenderView, CFormView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_VIEW, OnRefresh )
	ON_MESSAGE( UM_CUSTOM_RENDER, OnCustomRender )
	ON_MESSAGE( UM_SELECT_CHANGE_OPEN_VIEW, OnOpenView )
	ON_MESSAGE( UM_SELECT_CHANGE_CLOSE_VIEW, OnCloseView )
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CEventRenderView diagnostics

#ifdef _DEBUG
void CEventRenderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventRenderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventRenderView message handlers

void CEventRenderView::OnInitialUpdate()
{
	__super::OnInitialUpdate();
}

LRESULT CEventRenderView::OnOpenView( WPARAM wParam, LPARAM lParam )
{
	m_nMouseFlag = 0;
	m_LocalTime = m_PrevLocalTime = 0;
	m_bRefreshScene = true;
	m_PrevMousePos2 = m_PrevMousePos = m_MousePos = CPoint( 0, 0 );

	CGlobalValue::GetInstance().m_pSelectEvent = NULL;
	CGlobalValue::GetInstance().m_szSelectControlName = "";

	/*
	CEnviControl::GetInstance().ActivateElement( "Edit Set" );
	CWnd *pWnd = GetPaneWnd( ENVI_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	*/

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector ) {
			pSector->ShowProp( true );
			pSector->ShowEventArea( true );
		}
	}

	ResetCamera();
	CGlobalValue::GetInstance().ApplyRenderOption();

	CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, -1, "OnOpenView", CGlobalValue::GetInstance().m_pParentView, GetEtEngine() );

	return S_OK;
}

LRESULT CEventRenderView::OnCloseView( WPARAM wParam, LPARAM lParam )
{
	CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, -1, "OnCloseView", CGlobalValue::GetInstance().m_pParentView, GetEtEngine() );

	if( CGlobalValue::GetInstance().IsModify( CGlobalValue::GetInstance().m_nActiveView ) )
		CGlobalValue::GetInstance().Save( CGlobalValue::GetInstance().m_nActiveView );
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->Show( false );		
	}
	CTEtWorld::GetInstance().SetCurrentGrid( NULL );

	CGlobalValue::GetInstance().m_bMaintenanceCamera = true;
	CGlobalValue::GetInstance().m_pSelectEvent = NULL;
	CGlobalValue::GetInstance().m_szSelectControlName = "";

	s_EventActionCommander.Reset();

	return S_OK;
}

LRESULT CEventRenderView::OnCustomRender( WPARAM wParam, LPARAM lParam )
{
	CString szStr;
	szStr = "Event Edit Mode : ";

	szStr += "Edit";
	if( CGlobalValue::GetInstance().IsActiveRenderView( this ) ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() && !CGlobalValue::GetInstance().m_szSelectControlName.IsEmpty() ) {
			SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
			int nControlType = pSector->UpdateAreaControlType( m_MousePos.x, m_MousePos.y, CGlobalValue::GetInstance().m_szSelectControlName );

			if( m_nMouseFlag & LB_DOWN ) {
				char cFlag = 0;
				if( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) cFlag |= 0x01;
				if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) cFlag |= 0x02;

				if( cFlag == 0 ) {
					if( CGlobalValue::GetInstance().m_pSelectEvent == NULL ) {
						DWORD dwColor;
						if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) dwColor = 0xFFFF0000;
						else dwColor = 0xFFFFFF00;
						pSector->UpdateAreaRectBrush( m_MousePos.x, m_MousePos.y, dwColor, CGlobalValue::GetInstance().m_szSelectControlName );
					}
					else {
						if( m_pCurAction == NULL ) {
							m_pCurAction = new CActionElementModifyEventArea(0);
							m_pCurAction->SetDesc( "Modify EventArea - Move" );
							((CActionElementModifyEventArea*)m_pCurAction)->SetEventArea( CGlobalValue::GetInstance().m_pSelectEvent );
						}

						pSector->ControlEventArea( m_MousePos.x, m_MousePos.y, m_nControlType );

						CWnd *pWnd = GetPaneWnd( EVENTPROP_PANE );
						if( pWnd ) 
						{
							CEtWorldEventArea *pSelectedEvent = CGlobalValue::GetInstance().m_pSelectEvent;
							CEtWorldEventControl *pControl = NULL;
							if (pSelectedEvent)
								pControl = pSelectedEvent->GetControl();

							pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pControl, (LPARAM)pSelectedEvent );
						}

					}
				}
				else {
					CEtWorldEventArea *pArea = CGlobalValue::GetInstance().m_pSelectEvent;
					if( pArea ) {
						if( m_pCurAction == NULL ) {
							m_pCurAction = new CActionElementModifyEventArea(0);
							m_pCurAction->SetDesc( "Modify EventArea - Move" );
							((CActionElementModifyEventArea*)m_pCurAction)->SetEventArea( CGlobalValue::GetInstance().m_pSelectEvent );
						}

						float fValue = (float)( m_PrevMousePos2.y - m_MousePos.y );
						EtVector3 *vTemp, *vTemp2; 
						if( cFlag == 0x01 ) {
							vTemp = pArea->GetMin();
							vTemp2 = pArea->GetMax();
						}
						else {
							vTemp = pArea->GetMax();
							vTemp2 = pArea->GetMin();
						}

						vTemp->y += fValue;
						if( cFlag == 0x03 ) {
							vTemp2->y = vTemp->y;
						}
						else if( cFlag == 0x01 ) {
							if( vTemp->y > vTemp2->y ) vTemp->y = vTemp2->y;
						}
						else { 
							if( vTemp->y < vTemp2->y ) vTemp->y = vTemp2->y;
						}
						((CTEtWorldEventArea*)pArea)->ModifyCustomRender();
					}
					m_nControlType = 0;
					CWnd *pWnd = GetPaneWnd( EVENT_PANE );
					if( pWnd ) pWnd->SendMessage( UM_EVENT_PANE_REFRESH_INFO, (WPARAM)CGlobalValue::GetInstance().m_szSelectControlName.GetBuffer(), (LPARAM)CGlobalValue::GetInstance().m_pSelectEvent );
				}
				nControlType = m_nControlType;
				CGlobalValue::GetInstance().SetModify();
			}

			if( nControlType == -1 ) m_pCursor = IDC_ARROW;
			else {
				if( nControlType == 0x0F ) m_pCursor = IDC_CROSS;
				else if( ( nControlType & 0x01 || nControlType & 0x02 ) && !( nControlType & 0x04 || nControlType & 0x08 ) ) m_pCursor = IDC_SIZEWE;
				else if( ( nControlType & 0x04 || nControlType & 0x08 ) && !( nControlType & 0x01 || nControlType & 0x02 ) ) m_pCursor = IDC_SIZENS;
				else m_pCursor = IDC_SIZEALL;
			}
		}
	}
	/*
	// Area Draw
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		pSector->DrawArea( CGlobalValue::GetInstance().m_szSelectControlName );
	}
	*/

	if( CGlobalValue::GetInstance().IsCheckOutMe() )
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.05f ), szStr, 0xffffff00 );

	if( CEnviLightBase::s_pFocus ) {
		CEnviLightBase::s_pFocus->OnCustomDraw();
	}

	return S_OK;
}

LRESULT CEventRenderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CRenderBase::GetInstance().Process( CGlobalValue::EVENT );
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

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 && !CGlobalValue::GetInstance().m_szSelectControlName.IsEmpty() ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, -1, "OnRefresh", CGlobalValue::GetInstance().m_pParentView, GetEtEngine(), pSector );
	}

	m_bRefreshScene = false;
	return S_OK;
}

void CEventRenderView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
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
		m_PrevMousePos2 = m_MousePos;
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

			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_WHEEL ) {
		if( IsInMouseRect( this ) == true ) {
			if( CGlobalValue::GetInstance().m_pSelectEvent && GetAsyncKeyState( VK_LCONTROL ) ) {
				if( m_pCurAction == NULL ) {
					m_pCurAction = new CActionElementModifyEventArea(1);
					m_pCurAction->SetDesc( "Modify EventArea - Rotate" );
					((CActionElementModifyEventArea*)m_pCurAction)->SetEventArea( CGlobalValue::GetInstance().m_pSelectEvent );
				}

				float fValue = CGlobalValue::GetInstance().m_pSelectEvent->GetRotate();
				fValue -= GetMouseVariation().z * 0.02f;
				CGlobalValue::GetInstance().m_pSelectEvent->SetRotate( fValue );
				((CTEtWorldEventArea*)CGlobalValue::GetInstance().m_pSelectEvent)->UpdatePropertyRotationInfo();
				CGlobalValue::GetInstance().SetModify();

				if( m_pCurAction ) {
					m_pCurAction->AddAction();
					m_pCurAction = NULL;
				}

				CWnd *pWnd = GetPaneWnd( EVENTPROP_PANE );
				if( pWnd ) 
				{
					CEtWorldEventArea *pSelectedEvent = CGlobalValue::GetInstance().m_pSelectEvent;
					CEtWorldEventControl *pControl = NULL;
					if (pSelectedEvent)
						pControl = pSelectedEvent->GetControl();

					pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pControl, (LPARAM)pSelectedEvent );
				}
			}
			else {
				m_Camera.m_vPosition += ( m_Camera.m_vZAxis * 0.4f *( GetMouseVariation().z * CGlobalValue::GetInstance().m_fCamSpeed )  );
			}
			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( this ) == true ) {
			m_nMouseFlag |= LB_DOWN;

			GetCursorPos( &m_MousePos );
			ScreenToClient( &m_MousePos );

			m_PrevMousePos = m_MousePos;

			/*
			HMODULE hModule = LoadLibrary( "C:\\WorkSpace\\DragonNest\\Client\\WorldPainterPlugins\\EventUnitAreaInfo\\Debug\\EventUnitAreaInfo.dll" );
			*( FARPROC * )&g_pRegRes = GetProcAddress( hModule, "SetPluginsResource" );
			HINSTANCE hInstance = g_pRegRes();
			*( FARPROC * )&g_pCallFunc = GetProcAddress( hModule, "CallProperties" );
			g_pCallFunc( CGlobalValue::GetInstance().m_pParentView );
			AfxSetResourceHandle( AfxGetInstanceHandle() );
			FreeLibrary( hModule );
			*/
			
			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 && !CGlobalValue::GetInstance().m_szSelectControlName.IsEmpty() ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					pSector->BeginRectBrush( m_MousePos.x, m_MousePos.y );
					m_nControlType = pSector->UpdateAreaControlType( m_MousePos.x, m_MousePos.y, CGlobalValue::GetInstance().m_szSelectControlName );
					CEtWorldEventArea *pArea = pSector->UpdateAreaRectBrush( m_MousePos.x, m_MousePos.y, 0xFFFFFFFF, CGlobalValue::GetInstance().m_szSelectControlName );
					CGlobalValue::GetInstance().m_pSelectEvent = pArea;
					if( !( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) && !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) {
						if( pArea ) {
							CEtWorldEventControl *pControl = pSector->GetControlFromName( CGlobalValue::GetInstance().m_szSelectControlName );
							CWnd *pWnd = GetPaneWnd( EVENT_PANE );
							if( pWnd ) pWnd->SendMessage( UM_EVENT_PANE_SELCHANGED, (WPARAM)pArea->GetCreateUniqueID() );

							if( LocalTime - GetEventMouseTime( IR_MOUSE_LB_UP ) < 200 ) {
								if( CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, pArea->GetControl()->GetUniqueID(), "OnLButtonDoubleClick",
									CGlobalValue::GetInstance().m_pParentView, GetEtEngine(), pArea ) == true ) {
									m_nMouseFlag &= ~LB_DOWN;
									m_pCursor = IDC_ARROW;
								}
							}
							else {
								if( ( GetAsyncKeyState( VK_LMENU ) & 0x8000 ) ) {

									if( m_pCurAction ) {
										m_pCurAction->AddAction();
										m_pCurAction = NULL;
									}

									m_pCurAction = new CActionElementAddRemoveEventArea( true );
									m_pCurAction->SetDesc( "Add EventArea" );

									CEtWorldEventArea *pCopyArea = pSector->CopyArea( pArea, m_pCurAction );
									RefreshScene();
									RefreshProerties();
									CGlobalValue::GetInstance().SetModify();

									m_pCurAction->AddAction();
									m_pCurAction = NULL;

									if( pCopyArea )
										CGlobalValue::GetInstance().m_pSelectEvent = pCopyArea;
								}

								if( CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, pArea->GetControl()->GetUniqueID(), "OnLButtonClick",
									CGlobalValue::GetInstance().m_pParentView, GetEtEngine(), pArea ) == true ) {
									m_nMouseFlag &= ~LB_DOWN;
									m_pCursor = IDC_ARROW;
								}
							}
						}
					}
					CGlobalValue::GetInstance().SetModify();
				}
			}

			RefreshScene();
		}
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		if( m_pCurAction ) {
			m_pCurAction->AddAction();
			m_pCurAction = NULL;
		}

		m_nMouseFlag &= ~LB_DOWN;
		m_pCursor = IDC_ARROW;
		if( IsInMouseRect( this ) == true ) {
			if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 && !CGlobalValue::GetInstance().m_szSelectControlName.IsEmpty() ) {
					if( CGlobalValue::GetInstance().m_pSelectEvent == NULL ) {
						if( m_pCurAction == NULL ) {
							m_pCurAction = new CActionElementAddRemoveEventArea( true );
							m_pCurAction->SetDesc( "Add EventArea" );
						}

						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
						pSector->AddArea( CGlobalValue::GetInstance().m_szSelectControlName, GetUniqueAreaName(), m_pCurAction );
						RefreshScene();
						RefreshProerties();
						CGlobalValue::GetInstance().SetModify();

						m_pCurAction->AddAction();
						m_pCurAction = NULL;
					}
				}
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
		if( CGlobalValue::GetInstance().m_pSelectEvent && GetAsyncKeyState( VK_LCONTROL ) ) {
			CGlobalValue::GetInstance().m_pSelectEvent->SetRotate( 0.f );
			CGlobalValue::GetInstance().SetModify();
		}
	}
	if( nReceiverState & IR_MOUSE_WB_UP ) {
		m_nMouseFlag &= ~WB_DOWN;
	}
	if( nReceiverState & IR_KEY_DOWN ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			if( IsPushKey( DIK_DELETE ) && m_nMouseFlag == 0 ) {
				if( CGlobalValue::GetInstance().m_pSelectEvent ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector != -1 ) {
						if( m_pCurAction == NULL ) {
							m_pCurAction = new CActionElementAddRemoveEventArea( false );
							m_pCurAction->SetDesc( "Remove EventArea" );
						}

						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
						pSector->RemoveArea( CGlobalValue::GetInstance().m_szSelectControlName, CGlobalValue::GetInstance().m_pSelectEvent->GetCreateUniqueID(), m_pCurAction );
						CGlobalValue::GetInstance().m_pSelectEvent = NULL;
						RefreshScene();
						RefreshProerties();
						CGlobalValue::GetInstance().SetModify();

						m_pCurAction->AddAction();
						m_pCurAction = NULL;
					}
				}
			}
		}
	}

	m_PrevLocalTime = LocalTime;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
	if( nReceiverState & IR_KEY_DOWN ) CGlobalValue::GetInstance().CheckAccelerationKey( this );
}

void CEventRenderView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRenderBase::GetInstance().Render( CGlobalValue::EVENT );
}

BOOL CEventRenderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( CFileServer::GetInstance().IsConnect() == false )
		return CFormView::OnEraseBkgnd(pDC);
	return FALSE;
}

void CEventRenderView::ResetCamera()
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

BOOL CEventRenderView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	SetCursor( LoadCursor( NULL, m_pCursor ) );

	return TRUE;
//	return __super::OnSetCursor(pWnd, nHitTest, message);
}


void CEventRenderView::RefreshProerties()
{
	CWnd *pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}


CString CEventRenderView::GetUniqueAreaName()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return CString("");
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return CString("");

	CEtWorldEventControl *pControl = pSector->GetControlFromName( CGlobalValue::GetInstance().m_szSelectControlName );
	CString szTemp;

	int nCount = 0;
	while(1) {
		szTemp.Format( "%s - %03d", CGlobalValue::GetInstance().m_szSelectControlName, nCount );
		if( !pControl->GetAreaFromName( szTemp ) ) break;
		nCount++;
	}
	return szTemp;
}

CActionElementAddRemoveEventArea::CActionElementAddRemoveEventArea( bool bAddRemove )
: CActionElement( &s_EventActionCommander )
{
	m_bAddRemove = bAddRemove;
}

CActionElementAddRemoveEventArea::~CActionElementAddRemoveEventArea()
{
}

bool CActionElementAddRemoveEventArea::Redo()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return false;
	CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( m_nControlUniqueID );
	if( !pControl ) return false;

	if( m_bAddRemove ) {
		pSector->SetEventAreaCreateUniqueCount( m_nCreateAreaUniqueCount );
		pControl->InsertArea( m_vMin, m_vMax, m_szAreaName );
	}
	else {
		pControl->RemoveArea( m_szAreaName );
	}

	CGlobalValue::GetInstance().RefreshRender();
	CWnd *pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	return true;
}

bool CActionElementAddRemoveEventArea::Undo()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return false;
	CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( m_nControlUniqueID );
	if( !pControl ) return false;

	if( m_bAddRemove ) {
		pSector->SetEventAreaCreateUniqueCount( m_nCreateAreaUniqueCount );
		pControl->RemoveArea( m_szAreaName );
	}
	else {
		((CTEtWorldEventControl*)pControl)->InsertArea( m_vMin, m_vMax, m_szAreaName, m_nCreateAreaUniqueCount );
	}

	CGlobalValue::GetInstance().RefreshRender();
	CWnd *pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	return true;
}

void CActionElementAddRemoveEventArea::SetEventArea( CEtWorldEventArea *pArea )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	m_nControlUniqueID = pArea->GetControl()->GetUniqueID();
	m_szAreaName = pArea->GetName();
	m_vMin = *pArea->GetMin();
	m_vMax = *pArea->GetMax();
	if( m_bAddRemove )
		m_nCreateAreaUniqueCount = pSector->GetEventAreaCreateUniqueCount() - 1;
	else m_nCreateAreaUniqueCount = pArea->GetCreateUniqueID();
}

CActionElementModifyEventArea::CActionElementModifyEventArea( char cType )
: CActionElement( &s_EventActionCommander )
{
	m_cType = cType;
}

CActionElementModifyEventArea::~CActionElementModifyEventArea()
{
}


void CActionElementModifyEventArea::SetEventArea( CEtWorldEventArea *pArea )
{
	m_pEventArea = pArea;
	switch( m_cType ) {
		case 0:
			{
				m_vCurMin = *pArea->GetMin();
				m_vCurMax = *pArea->GetMax();
			}
			break;
		case 1:
			{
				m_fCurRotate = pArea->GetRotate();
			}
			break;
		case 2:
			{
				m_szCurName = m_pEventArea->GetName();
			}
			break;
	}
}

bool CActionElementModifyEventArea::Redo()
{
	switch( m_cType ) {
		case 0:
			{
				EtVector3 vMin = *m_pEventArea->GetMin();
				EtVector3 vMax = *m_pEventArea->GetMax();
				m_pEventArea->SetMin( m_vCurMin );
				m_pEventArea->SetMax( m_vCurMax );
				m_vCurMin = vMin;
				m_vCurMax = vMax;
			}
			break;
		case 1:
			{
				float fTemp = m_pEventArea->GetRotate();
				m_pEventArea->SetRotate( m_fCurRotate );
				m_fCurRotate = fTemp;
			}
			break;
		case 2:
			{
				CString szTemp = m_pEventArea->GetName();
				m_pEventArea->SetName( m_szCurName );
				m_szCurName = szTemp;
			}
			break;
	}
	CGlobalValue::GetInstance().RefreshRender();
	CWnd *pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return true;
}

bool CActionElementModifyEventArea::Undo()
{
	switch( m_cType ) {
		case 0:
			{
				EtVector3 vMin = *m_pEventArea->GetMin();
				EtVector3 vMax = *m_pEventArea->GetMax();
				m_pEventArea->SetMin( m_vCurMin );
				m_pEventArea->SetMax( m_vCurMax );
				m_vCurMin = vMin;
				m_vCurMax = vMax;
			}
			break;
		case 1:
			{
				float fTemp = m_pEventArea->GetRotate();
				m_pEventArea->SetRotate( m_fCurRotate );
				m_fCurRotate = fTemp;
			}
			break;
		case 2:
			{
				CString szTemp = m_pEventArea->GetName();
				m_pEventArea->SetName( m_szCurName );
				m_szCurName = szTemp;
			}
			break;
	}
	CGlobalValue::GetInstance().RefreshRender();
	CWnd *pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return true;
}