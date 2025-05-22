#include "StdAfx.h"
#include "GlobalValue.h"
#include "FileServer.h"

#include "resource.h"
#include "MainFrm.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "DummyView.h"
#include "UserMessage.h"

#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "PaneDefine.h"

#include "PreLogoDlg.h"

CGlobalValue g_GlobalValue;

CGlobalValue::CGlobalValue()
{
	m_ViewTypeLoadLevel[GRID] = CTEtWorldSector::SRL_EMPTY;
	m_ViewTypeLoadLevel[BRUSH] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP;
	m_ViewTypeLoadLevel[PROP] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP;
	m_ViewTypeLoadLevel[WATER] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP | CTEtWorldSector::SRL_WATER;
	m_ViewTypeLoadLevel[NAVIGATION] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP | CTEtWorldSector::SRL_ATTRIBUTE | CTEtWorldSector::SRL_NAVIGATION;
	m_ViewTypeLoadLevel[EVENT] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP | CTEtWorldSector::SRL_EVENT;
	m_ViewTypeLoadLevel[SOUND] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP | CTEtWorldSector::SRL_SOUND;
	m_ViewTypeLoadLevel[TRIGGER] = CTEtWorldSector::SRL_TERRAIN | CTEtWorldSector::SRL_PROP | CTEtWorldSector::SRL_EVENT | CTEtWorldSector::SRL_TRIGGER;
	m_ViewTypeLoadLevel[RDUNGEON] = CTEtWorldSector::SRL_EMPTY;
	m_nActiveView = 0;
	m_nSelectLayer = 0;
	m_nBrushType = 0;
	m_nBrushDiameter = 5;
	m_fBrushHardness = 1.f;
	m_fBrushStrong = 0.1f;
	m_rRadian = frange( 0.f, 90.f );
	m_rHeight = irange( -32767, 32767 );
	m_SelectGrid = -1;
	m_CheckOutGrid = -1;
	m_bModify = false;
	memset( m_bEnableGrassIndex, 0, sizeof(m_bEnableGrassIndex) );

	m_fFps = 60.f;

	m_fCamSpeed = 5.f;
	m_fCamNear = 10.f;
	m_fCamFar = 200000.f;
	m_fCamFOV = ET_PI / 4;

	m_bCanDragTile = false;
	m_bEditBlock = false;
	m_bAutoCheckIn = false;
	m_bLockSideHeight = false;

	m_DungeonSize = IntVec3( 20, 20, 1 );
	m_nDungeonSparseness = 15;
	m_nDungeonRandomness = 40;
	m_nDungeonDeadendRemove = 40;
	m_nDungeonSeed = 0;

	m_bSimulation = false;
	m_bCanSimulation = false;

	m_cDrawType = 0;
	m_bRandomPosition = false;
	m_bRandomRotationX = false;
	m_bRandomRotationY = true;
	m_bRandomRotationZ = false;
	m_bLockHeightNormal = false;
	m_fMinPropDistance = 1100.f;
	m_bLockScaleAxis = true;
	m_bControlAxis = false;
	m_nSelectAxis = -1;
	m_fHeightDigPer = 2.f;
	m_rScaleXRange = m_rScaleYRange = m_rScaleZRange = frange( 1.f, 1.f );
	m_bIgnoreDistanceSize = false;

	m_pSelectEvent = NULL;
	m_bMaintenanceCamera = false;

	m_bPlaySound = true;
	m_fMasterVolume = 1.f;
	m_pSelectSound = NULL;

	m_cSelectAttribute = 1;

	m_bShowProp = false;
	m_bShowPropCollision = false;
	m_bLoadedProp = false;
	m_fPropAlpha = 1.f;

	m_bShowWater = false;
	m_bLoadedWater = false;
	m_bDrawRevisionEmptyMeshProp = false;

	m_bLoadedAttribute = false;
	m_bLoadedNavigation = false;
	m_bShowNavigation = false;
	m_bShowAttribute = false;

	m_bShowEventArea = false;
	m_bLoadedEventArea = false;

	char szStr[2048] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ViewerLocation", szStr, 2048 );
	m_szViewerLocation = szStr;

	memset( szStr, 0, sizeof(szStr) );
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ActionLocation", szStr, 2048 );
	m_szActionLocation = szStr;

	m_nPickType = -1;
	m_pPickStandbyElement = NULL;
	m_nPickParamIndex = 0;

	m_bAutoDiagonal = true;

	memset( m_bKeyFlags, 0, sizeof(m_bKeyFlags) );

	m_pSelectDecal = NULL;

	int nVersion = 7;
	DWORD nTemp = 0;

	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "Version", nTemp );
	if( (int)nTemp < nVersion ) {
		sprintf_s( szStr, "%s\\DockingPaneLayouts", REG_SUBKEY );
		DeleteRegistry( HKEY_CURRENT_USER, szStr );
	}

	SetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "Version", nVersion );

#ifdef _CHECK_MISSING_INDEX
	m_szFolderName.clear();
	m_strPreviousGridName.Empty();
	m_bIsFirst = false;
#endif

}

CGlobalValue::~CGlobalValue()
{
}

bool CGlobalValue::CheckAndSelectGrid( const char *szGridName, SectorIndex GridIndex )
{
	if( m_CheckOutGrid != -1 ) return false;
	if( CFileServer::GetInstance().IsCanCheckOut( szGridName, GridIndex ) == false ) return false;
	m_SelectGrid = GridIndex;
	m_szSelectGrid = szGridName;
	return true;
}

bool CGlobalValue::IsCheckOutMe()
{
	if( !m_szCheckOutGrid.IsEmpty() && m_CheckOutGrid != -1 && m_CheckOutGrid == m_SelectGrid && m_szCheckOutGrid == m_szSelectGrid ) return true;
	return false;
}

bool CGlobalValue::IsActiveRenderView( CView *pView )
{
	CDummyView::TabViewStruct *pStruct = ((CEtWorldPainterView*)m_pParentView)->GetDummyView()->GetTabViewStruct( m_nActiveView );
	if( !((CMainFrame*)AfxGetMainWnd())->GetFocus() ) return false;
	CWnd *pWnd = ((CMainFrame*)AfxGetMainWnd())->GetActiveWindow();
	if( pWnd == NULL ) return false;
	if( pWnd->GetFocus() != pView ) return false;
	if( pStruct->pView->m_hWnd != pView->m_hWnd ) return false;
	if( m_bSimulation == true ) return false;
	return true;
}

void CGlobalValue::RefreshRender()
{
	if( m_pParentView )
		m_pParentView->SendMessage( UM_REFRESH_VIEW, 1 );
}

bool Compare1( int &nValue1, int &nValue2 )
{
	if( nValue1 > nValue2 ) return true;
	return false;
}


bool CGlobalValue::IsModify( int nTabIndex )
{
	if( nTabIndex == -1 ) nTabIndex = m_nActiveView;

	for( DWORD i=0; i<m_nVecModifyTabList.size(); i++ ) {
		if( m_nVecModifyTabList[i] == nTabIndex ) return true;
	}
	return false;
}

void CGlobalValue::SetModify( int nTabIndex, bool bModify )
{
	if( bModify == true ) {
		int nIndex = -1;
		if( nTabIndex == -1 ) nIndex = m_nActiveView;

		m_bModify = true;
		bool bFlag = true;
		for( DWORD i=0; i<m_nVecModifyTabList.size(); i++ ) {
			if( m_nVecModifyTabList[i] == nIndex ) {
				bFlag = false;
				break;
			}
		}
		if( bFlag == true ) {
			((CEtWorldPainterView*)m_pParentView)->ChangeModify( nIndex, true );
			m_nVecModifyTabList.push_back( nIndex );
			std::sort( m_nVecModifyTabList.begin(), m_nVecModifyTabList.end(), Compare1 );
		}
	}
	else {
		if( nTabIndex == -1 ) {
			for( DWORD i=0; i<m_nVecModifyTabList.size(); i++ ) {
				((CEtWorldPainterView*)m_pParentView)->ChangeModify( m_nVecModifyTabList[i], false );
			}
			m_nVecModifyTabList.clear();
			m_bModify = false;
		}
		else {
			for( DWORD i=0; i<m_nVecModifyTabList.size(); i++ ) {
				if( m_nVecModifyTabList[i] == nTabIndex ) {
					((CEtWorldPainterView*)m_pParentView)->ChangeModify( m_nVecModifyTabList[i], false );
					m_nVecModifyTabList.erase( m_nVecModifyTabList.begin() + i );
					break;
				}
			}
		}
	}
}

bool CGlobalValue::Save( int nIndex )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
	if( Sector == -1 ) return false;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector, szGridName );
	if( pSector == NULL ) return false;
	if( pSector->IsEmpty() ) return true;

	bool bResult = true;
	if( nIndex == -1 ) {
		std::vector<int> nVecList = m_nVecModifyTabList;
		for( DWORD i=0; i<nVecList.size(); i++ ) {
			if( Save( nVecList[i] ) == false ) bResult = false;
		}
	}
	else {
		switch( nIndex ) {
			case GRID:	// Grid Select.
				if( pSector->SaveSector( CTEtWorldSector::SRL_PREVIEW ) == false ) bResult = false;
				if( m_bAutoCheckIn == true ) {
					CFileServer::GetInstance().CheckIn();
					SetModify( nIndex, false );
				}
				break;
			case BRUSH:	// Brush
				if( pSector->SaveSector( CTEtWorldSector::SRL_TERRAIN, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case PROP:	// Prop
				if( pSector->SaveSector( CTEtWorldSector::SRL_PROP, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case NAVIGATION: // Navigation
				if( pSector->SaveSector( CTEtWorldSector::SRL_ATTRIBUTE | CTEtWorldSector::SRL_NAVIGATION, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case EVENT:	// Event
				if( pSector->SaveSector( CTEtWorldSector::SRL_EVENT, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case SOUND:	// Sound
				if( pSector->SaveSector( CTEtWorldSector::SRL_SOUND, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case TRIGGER: // Trigger
				if( pSector->SaveSector( CTEtWorldSector::SRL_TRIGGER, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
			case WATER: // Water
				if( pSector->SaveSector( CTEtWorldSector::SRL_WATER, true ) == false ) bResult = false;
				else SetModify( nIndex, false );
				break;
		}
	}

	return bResult;
}

CString CGlobalValue::GetSaveDesc( int nIndex )
{
	switch( nIndex ) {
		case GRID:	// Grid.
			return CString( " - 섹터 체크아웃" );
		case BRUSH:	// Brush
			return CString( " - 지형 수정" );
		case PROP:	// Prop
			return CString( " - 프랍 수정" );
		case NAVIGATION:	// Navigation
			return CString( " - 속성 및 네비게이션 수정" );
		case EVENT:	// Event
			return CString( " - 이벤트 영역 수정" );
		case SOUND:	// Sound
			return CString( " - 사운드 수정" );
		case TRIGGER:
			return CString( " - 트리거 수정" );
		case WATER:
			return CString( " - 물 수정" );
	}
	return CString("");
}

DWORD CGlobalValue::GetSaveCount()
{
	return (DWORD)m_nVecModifyTabList.size();
}


bool CGlobalValue::CheckSaveMessage( HWND hWnd, bool bExit )
{
	if( CGlobalValue::GetInstance().m_bModify == true ) {
		CGlobalValue::GetInstance().m_bAutoCheckIn = true;
		CString szMsg;
		std::vector<int> nVecList = CGlobalValue::GetInstance().m_nVecModifyTabList;
		for( DWORD i=0; i<nVecList.size(); i++ ) {
			szMsg.Format( "다음 작업이 저장 또는 완료되지 않았습니다.\n\n%s\n\n저장 또는 완료하시겠습니까?", CGlobalValue::GetInstance().GetSaveDesc(nVecList[i]) );
			int nResult = MessageBox( hWnd, szMsg, "질문", ( bExit ) ? MB_YESNOCANCEL : MB_YESNO );
			if( nResult == IDCANCEL ) return false;
			if( nResult == IDYES ) {
				if( CGlobalValue::GetInstance().Save( nVecList[i] ) == false ) {
					MessageBox( hWnd, "세이브 실패!!", "Error", MB_OK );
				}
			}
		}
	}
	return true;
}

void CGlobalValue::ShowProp( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		if( !( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_PROP ) && !m_bLoadedProp ) {
			m_bLoadedProp = true;
			pSector->LoadSector( CTEtWorldSector::SRL_PROP );
		}
	}
	m_bShowProp = bShow;
	pSector->ShowProp( bShow );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::ShowPropCollision( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		if( !( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_PROP ) && !m_bLoadedProp ) {
			m_bLoadedProp = true;
			pSector->LoadSector( CTEtWorldSector::SRL_PROP );
		}
	}
	m_bShowPropCollision = bShow;
	pSector->ShowPropCollision( bShow );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::ShowWater( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		m_bLoadedWater = true;
		pSector->LoadSector( CTEtWorldSector::SRL_WATER );
	}
	m_bShowWater = bShow;
	pSector->ShowWater( bShow );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::ShowAttribute( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		if( !( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_ATTRIBUTE ) && !m_bLoadedAttribute ) {
			m_bLoadedAttribute = true;
			pSector->LoadSector( CTEtWorldSector::SRL_ATTRIBUTE );
		}
	}
	m_bShowAttribute = bShow;
	pSector->ShowAttribute( m_bShowAttribute );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::ShowNavigation( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		if( !( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_NAVIGATION ) && !m_bLoadedNavigation ) {
			m_bLoadedNavigation = true;
			pSector->LoadSector( CTEtWorldSector::SRL_NAVIGATION );
		}
	}
	m_bShowNavigation = bShow;
	pSector->ShowNavigation( m_bShowNavigation );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::ShowEventArea( bool bShow )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	if( bShow ) {
		if( !( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_EVENT ) && !m_bLoadedEventArea ) {
			m_bLoadedEventArea = true;
			CGlobalValue::GetInstance().m_szSelectControlName = "";
			pSector->LoadSector( CTEtWorldSector::SRL_EVENT );
		}
	}
	m_bShowEventArea = bShow;
	pSector->ShowEventArea( m_bShowEventArea );

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::DrawRevisionEmptyMeshProp( bool bShow ) 
{ 
	m_bDrawRevisionEmptyMeshProp = bShow; 
}

void CGlobalValue::ApplyRenderOption()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	//m_bShowWater설정값 유지 되도록..
	ShowWater(m_bShowWater);

	if( m_bShowAttribute && !(m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_ATTRIBUTE) ) {
		m_bLoadedAttribute = false;
		ShowAttribute( false );
	}
	if( m_bShowNavigation && !(m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_NAVIGATION) ) {
		m_bLoadedNavigation = false;
		ShowNavigation( false );
	}
	if( m_bShowEventArea && !(m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_EVENT) ) {
		m_bLoadedEventArea = false;
		ShowEventArea( false );
	}

	if( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_PROP ) {
		m_bShowProp = true;
	}

	if( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_WATER ) {
		m_bShowWater = true;
	}

	if( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_ATTRIBUTE ) {
		m_bShowAttribute = true;
	}
	if( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_NAVIGATION ) {
		m_bShowNavigation = true;
	}
	if( m_ViewTypeLoadLevel[m_nActiveView] & CTEtWorldSector::SRL_EVENT ) {
		m_bShowEventArea = true;
	}

	CWnd *pWnd = GetPaneWnd( RENDEROPTION_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CGlobalValue::SetPropAlpha( float fValue )
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	m_fPropAlpha = fValue;
	for( DWORD i=0; i<pSector->GetPropCount(); i++ ) {
		EtAniObjectHandle hHandle = pSector->GetPropFromIndex(i)->GetObjectHandle();
		if( hHandle ) hHandle->SetObjectAlpha( m_fPropAlpha );
	}
}

void CGlobalValue::CheckAccelerationKey( CInputReceiver *pReceiver )
{
	if( m_nActiveView == BRUSH || m_nActiveView == PROP || m_nActiveView == NAVIGATION || m_nActiveView == EVENT || m_nActiveView == SOUND ) {
		if( pReceiver->IsPushKey( DIK_F2 ) ) {
			ShowProp( !m_bShowProp );
			RefreshRender();
		}
	}
}

short CGlobalValue::GetAsyncKeyState( int vKey )
{
	if( m_bKeyFlags[vKey] == true ) return 0;
	return ::GetAsyncKeyState( vKey );

}

void CGlobalValue::ReleaseAsyncKeyState( int vKey )
{
	m_bKeyFlags[vKey] = true;
}

void CGlobalValue::ProcessAsyncKey()
{
	for( int i=0; i<256; i++ ) {
		if( !m_bKeyFlags[i] ) continue;
		if( ::GetAsyncKeyState(i) >= 0 ) m_bKeyFlags[i] = false;
	}
}

void CGlobalValue::UpdateInitDesc( const char *szStr, ... )
{
	va_list list;
	char szTemp[65535];

	va_start( list, szStr );
	vsprintf_s( szTemp, szStr, list );
	va_end( list );

	if( CPreLogoDlg::IsActive() && CPreLogoDlg::GetInstance().IsWindowVisible() ) CPreLogoDlg::GetInstance().UpdateDescription( szTemp );
	else {
		CMainFrame *pMainFrame = ((CMainFrame*)AfxGetMainWnd());
		if( pMainFrame->GetStatusBar() ) {
			pMainFrame->GetStatusBar()->SetWindowText( szTemp );
			pMainFrame->GetStatusBar()->Invalidate( FALSE );
		}

	}
}

void CGlobalValue::UpdateProgress( int nMax, int nCur )
{
	CMainFrame *pMainFrame = ((CMainFrame*)AfxGetMainWnd());
	if( pMainFrame->GetProgressBar() ) {
		pMainFrame->GetProgressBar()->SetRange32( 0, nMax );
		pMainFrame->GetProgressBar()->SetPos( nCur );
		pMainFrame->GetProgressBar()->Invalidate( FALSE );
	}
}