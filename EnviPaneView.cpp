// EnviPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EnviPaneView.h"
#include "UserMessage.h"
#include "EnviControl.h"
#include "CreateEnviDlg.h"
#include "FileServer.h"
#include "EnviBasePaneView.h"
#include "EnviLightPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "EnviLightBase.h"



// CEnviPaneView

IMPLEMENT_DYNCREATE(CEnviPaneView, CFormView)

CEnviPaneView::CEnviPaneView()
	: CFormView(CEnviPaneView::IDD)
{
	m_bActivate = false;
	m_pBaseView = NULL;
	m_pLightView = NULL;
}

CEnviPaneView::~CEnviPaneView()
{
}

void CEnviPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEnviPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_CREATE()
	ON_COMMAND(ID_ENVI_NEW, &CEnviPaneView::OnEnviNew)
	ON_COMMAND(ID_ENVI_SAVEAS, &CEnviPaneView::OnEnviSaveas)
	ON_COMMAND(ID_ENVI_RELOAD, &CEnviPaneView::OnEnviReload)
	ON_COMMAND(ID_ENVI_DELETE, &CEnviPaneView::OnEnviDelete)
	ON_COMMAND(ID_ENVI_LOADLIST, &CEnviPaneView::OnEnviLoadlist)
	ON_MESSAGE( UM_PROPERTY_PANE_SET_MODIFY, OnSetModify )
	ON_COMMAND(ID_ENVI_BAKELIGHTMAP, &CEnviPaneView::OnEnviBakelightmap)
	ON_UPDATE_COMMAND_UI(ID_ENVI_BAKELIGHTMAP, &CEnviPaneView::OnUpdateEnviBakelightmap)
	ON_UPDATE_COMMAND_UI(ID_ENVI_SAVEAS, &CEnviPaneView::OnUpdateEnviSaveas)
	ON_UPDATE_COMMAND_UI(ID_ENVI_RELOAD, &CEnviPaneView::OnUpdateEnviReload)
	ON_UPDATE_COMMAND_UI(ID_ENVI_DELETE, &CEnviPaneView::OnUpdateEnviDelete)
	ON_MESSAGE( UM_SOUNDPANE_DROPMOVEITEM, OnSoundPaneDropMoveItem )
	ON_MESSAGE( UM_SOUNDPANE_DROPITEM, OnSoundPaneDropItem )
END_MESSAGE_MAP()


// CEnviPaneView diagnostics

#ifdef _DEBUG
void CEnviPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEnviPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEnviPaneView message handlers

void CEnviPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->SetWidth( 160 );
	pCombo->SetCurSel(0);

}

int CEnviPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_ENVI_TOOLBAR);

	m_wndToolBar.GetControls()->SetControlType( 4, xtpControlComboBox );
	m_wndToolBar.GetControls()->SetControlType( 5, xtpControlButton );


	m_TabCtrl.Create(WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), this, 1001);
	m_TabCtrl.GetPaintManager()->m_bDisableLunaColors = TRUE;
	m_TabCtrl.SetAppearance(xtpTabAppearanceStateButtons);
	m_TabCtrl.GetPaintManager()->m_bHotTracking = TRUE;
	m_TabCtrl.GetPaintManager()->m_bRotateImageOnVerticalDraw = TRUE;
	m_TabCtrl.SetPosition( xtpTabPositionLeft );

	CCreateContext cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.m_pCurrentDoc = GetDocument();

	m_pBaseView = (CEnviBasePaneView*)RUNTIME_CLASS(CEnviBasePaneView)->CreateObject();	
	((CWnd *)m_pBaseView)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2005, &cc );

	m_pLightView = (CEnviLightPaneView*)RUNTIME_CLASS(CEnviLightPaneView)->CreateObject();	
	((CWnd *)m_pLightView)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2006, &cc );

	m_TabCtrl.InsertItem( 0, _T("Base"), m_pBaseView->m_hWnd );
	m_TabCtrl.InsertItem( 1, _T("Light"), m_pLightView->m_hWnd );

	return 0;
}

void CEnviPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}
	if( ::IsWindow(m_TabCtrl.m_hWnd) ) {
		m_TabCtrl.SetWindowPos( NULL, 0, nTop, cx, cy - nTop, SWP_FRAMECHANGED );
	}

}

BOOL CEnviPaneView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CEnviPaneView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	if( (UINT)wParam == 1001 ) {
		NMHDR* pNMHDR = (NMHDR*)lParam;

		switch(pNMHDR->code)
		{
		case TCN_SELCHANGING:
			break;

		case TCN_SELCHANGE:
			if( m_TabCtrl.GetCurSel() == 0 ) {
				m_pBaseView->SendMessage( UM_REFRESH_PANE_VIEW );
				CEnviLightBase::s_pFocus = NULL;
			}
			else {
				m_pLightView->SendMessage( UM_REFRESH_PANE_VIEW );
			}
			CGlobalValue::GetInstance().RefreshRender();
			break;
		}
	}

	return CFormView::OnNotify(wParam, lParam, pResult);
}

LRESULT CEnviPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->ResetContent();

	CEnviElement *pActive = CEnviControl::GetInstance().GetActiveElement();
	for( DWORD i=0; i<CEnviControl::GetInstance().GetCount(); i++ ) {
		CEnviElement *pElement = CEnviControl::GetInstance().GetElement(i);
//		pElement->Save();
		pCombo->InsertString( pCombo->GetCount(), pElement->GetInfo().GetName() );
		if( pActive ) {
			if( strcmp( pActive->GetInfo().GetName(), pElement->GetInfo().GetName() ) == NULL ) {
				pCombo->SetCurSel(i);
			}
		}
	}

	if( m_TabCtrl.GetCurSel() == 0 )
		m_pBaseView->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );
	else m_pLightView->SendMessage( UM_REFRESH_PANE_VIEW, wParam, lParam );

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector != -1 ) {
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_ENVI_BAKELIGHTMAP, TRUE, FALSE );
		CXTPControlCheckBox* pCheck = (CXTPControlCheckBox*)pControl;

		pCheck->SetChecked( FALSE );

		pSector->EnableLightmap( false );
	}


	return S_OK;
}
void CEnviPaneView::OnEnviNew()
{
	// TODO: Add your command handler code here
	CCreateEnviDlg Dlg(this);

	if( Dlg.DoModal() != IDOK ) return;

	CEnviControl::GetInstance().CreateElement( Dlg.m_szName, Dlg.m_szCopyTo );
	CEnviControl::GetInstance().ActivateElement( Dlg.m_szName );

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->SetCurSel( pCombo->GetCount() - 1 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CEnviPaneView::OnEnviSaveas()
{
	// TODO: Add your command handler code here
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	CString szStr;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );

	CEnviControl::GetInstance().SaveElement( szStr );
}

void CEnviPaneView::OnEnviDelete()
{
	// TODO: Add your command handler code here
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	CString szStr;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );
	pCombo->DeleteItem( pCombo->GetCurSel() );
	CEnviControl::GetInstance().ActivateElement( "Edit Set" );
	pCombo->SetCurSel(0);

	// 내부에서 사용하던 리소스 파일들 체크해서 지워줘야 하지만
	// 지금은 빼놓는다. 나중에 체크해서 넣차
	CString szFullName;
	szFullName.Format( "%s\\%s", GetCurrentDir(), szStr );
	DeleteFile( szFullName );

	CEnviControl::GetInstance().RemoveElement( szStr );
}

void CEnviPaneView::OnEnviReload()
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	CString szStr;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );

	CEnviControl::GetInstance().LoadElement( szStr );
	CEnviControl::GetInstance().ActivateElement( szStr );
	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CEnviPaneView::OnEnviLoadlist()
{
	// TODO: Add your command handler code here
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_ENVI_LOADLIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	CString szStr;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );

	CEnviControl::GetInstance().ActivateElement( szStr );
	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CEnviPaneView::CopyAccompanimentFile( const char *szSkinName )
{
	char szPathTemp[512] = {0,} ;
	_GetPath( szPathTemp, _countof(szPathTemp), szSkinName );
	szPathTemp[strlen(szPathTemp)-1] = 0;

	// 시스템 폴더에있는걸 선택할때는 걍 리턴
	if( _stricmp( szPathTemp, GetCurrentDir() ) == NULL ) return;

	BOOL bExist = FALSE;
	std::vector<std::string> szVecList;
	FindFileListInDirectory( GetCurrentDir(), "*.*", szVecList, false, true );

	char szTemp[2][512] = { 0, };
	_GetFullFileName( szTemp[0], _countof(szTemp[0]), szSkinName );

	for( DWORD i=0; i<szVecList.size(); i++ ) {
		_GetFullFileName( szTemp[1], _countof(szTemp[1]), szVecList[i].c_str() );
		if( _stricmp( szTemp[0], szTemp[1] ) == NULL ) {
			bExist = TRUE;
			break;
		}
	}
	if( bExist == TRUE ) {
		if( MessageBox( "같은 이름의 파일이 이미 등록되어 있습니다.\n선택한 파일로 덮어쓸래?", "질문", MB_YESNO ) == IDNO ) return;
	}

	std::vector<CString> szVecAccompanimentList;
	if( CalcAccompanimentFile( szSkinName, szVecAccompanimentList ) == false ) {
		MessageBox( "생성할 수 없는 파일입니다.", "Error", MB_OK );
		return;
	}

	char szFileName[512] = { 0, };
	CString szErrorFileName;
	CString szErrorStr;
	CString szFullName;
	char cResult = 1;

	for( DWORD m=0; m<szVecAccompanimentList.size(); m++ ) {
		_GetFullFileName( szFileName, _countof(szFileName), szVecAccompanimentList[m] );
		szFullName.Format( "%s\\%s", GetCurrentDir(), szFileName );

		BOOL bResult;
		if( m == 0 ) 
			bResult = CopyFile( szVecAccompanimentList[m], szFullName, !bExist );
		else bResult = CopyFile( szVecAccompanimentList[m], szFullName, TRUE );

		if( bResult == false ) {
			if( m == 0 ) {
				cResult = -1;
				MessageBox( "스킨을 복사할 수 없어요!!", "Error", MB_OK );
				break;
			}
			else {
				szErrorStr.Format( "스킨파일에 필요한 다음 파일이 이미 있어요.. \n\n    %s\n\n덮어씌울까요?", szVecAccompanimentList[m] );
				if( MessageBox( szErrorStr, "질문", MB_YESNO ) == IDYES ) {
					bResult = CopyFile( szVecAccompanimentList[m], szFullName, FALSE );
				}
				if( bResult == false ) {
					cResult = 0;
					szErrorFileName += "\n    ";
					szErrorFileName += szVecAccompanimentList[m];
				}
			}
		}
	}
	if( cResult == -1 ) return;
	else if( cResult == 0 ) {
		CString szErrorStr;
		szErrorStr.Format( "스킨파일에 필요한 다음 파일들을 복사를 못했어요\n%s", szErrorFileName );
		MessageBox( szErrorStr, "경고", MB_OK );
	}

	if( bExist == TRUE ) return;
	SetFileAttributes( szFullName, FILE_ATTRIBUTE_NORMAL );
}

bool CEnviPaneView::CalcAccompanimentFile( const char *szSkinName, std::vector<CString> &szVecList )
{
	// 자기자신도 리스트에 ADd
	FILE *fp = NULL;
	szVecList.push_back( szSkinName );

	char szBuf[512] = { 0, };
	char szExt[64] = { 0, };
	CString szPath;

	_GetPath( szBuf, _countof(szBuf), szSkinName );
	_GetExt( szExt, _countof(szExt), szSkinName );
	if( strcmp( szExt, "dds" ) == NULL ) return true;
	szPath = szBuf;

	// 스킨이름과 같은 이름의 ani 복사해준다. 
	CString szAniName;
	memset( szBuf, 0, sizeof(szBuf) );
	_GetFileName( szBuf, _countof(szBuf), szSkinName );
	szAniName.Format( "%s%s.ani", szPath, szBuf );
	fopen_s( &fp, szAniName, "rb" );
	if( fp ) {
		szVecList.push_back( szAniName );
		fclose(fp);
		fp = NULL;
	}
	

	// 스킨에서 사용하는 msh,texture 파일 복사해준다.

	EtObjectHandle Handle = EternityEngine::CreateStaticObject( szSkinName );
	if( !Handle ) {
		return false;
	}
	szVecList.push_back( szPath + Handle->GetMesh()->GetFileName() );
	std::string szTempFullName;
	for( int l=0; l<Handle->GetSubMeshCount(); l++ ) {
		for( int m=0; m<Handle->GetEffectCount(l); m++ ) {
			SCustomParam *pParam = Handle->GetCustomParam( l, m );
			if( pParam->Type != EPT_TEX ) continue;
			EtResourceHandle hResource = CEtResource::GetResource( pParam->nTextureIndex );
			szTempFullName = szPath + hResource->GetFileName();
			if( std::find( szVecList.begin(), szVecList.end(), szTempFullName.c_str() ) != szVecList.end() ) continue;

			szVecList.push_back( szTempFullName.c_str() );
		}
	}
	SAFE_RELEASE_SPTR( Handle );

	return true;
}

CString CEnviPaneView::GetCurrentDir()
{
	CString szFolder;
	szFolder.Format( "%s\\Resource\\Envi", CFileServer::GetInstance().GetLocalWorkingFolder() );

	return szFolder;
}



LRESULT CEnviPaneView::OnSetModify( WPARAM wParam, LPARAM lParam )
{
	return m_pLightView->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, wParam, lParam );
}

void CEnviPaneView::OnEnviBakelightmap()
{
	// TODO: Add your command handler code here
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_ENVI_BAKELIGHTMAP, TRUE, FALSE );
	CXTPControlCheckBox* pCheck = (CXTPControlCheckBox*)pControl;

	BOOL bCheck = pCheck->GetChecked();
	pCheck->SetChecked( !bCheck );
	
	pSector->EnableLightmap( ( bCheck == TRUE ) ? false : true );

	CGlobalValue::GetInstance().RefreshRender();
}

void CEnviPaneView::OnUpdateEnviBakelightmap(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	BOOL bEnable = TRUE;
	if( !CGlobalValue::GetInstance().IsCheckOutMe() ) bEnable = FALSE;
	if( CEnviControl::GetInstance().GetActiveElement() == NULL ) bEnable = FALSE;
	else {
		if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName() == "Edit Set" ) bEnable = FALSE;
		if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetLightCount() == 0 ) bEnable = FALSE;
	}
	
	pCmdUI->Enable( bEnable );
}


void CEnviPaneView::OnUpdateEnviReload(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName() == "Edit Set" ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CEnviPaneView::OnUpdateEnviSaveas(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName() == "Edit Set" ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CEnviPaneView::OnUpdateEnviDelete(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName() == "Edit Set" ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

LRESULT CEnviPaneView::OnSoundPaneDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	return m_pBaseView->SendMessage( UM_SOUNDPANE_DROPMOVEITEM, wParam, lParam );
}

LRESULT CEnviPaneView::OnSoundPaneDropItem( WPARAM wParam, LPARAM lParam )
{
	return m_pBaseView->SendMessage( UM_SOUNDPANE_DROPITEM, wParam, lParam );
}
