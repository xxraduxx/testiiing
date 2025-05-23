 // SkinManagerPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SkinManagerPaneView.h"
#include "UserMessage.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include <Process.h>
#include "EtMatrixEx.h"
#include "SkinPrevDlg.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "RenderBase.h"
#include "EtEngine.h"
#include <map>
using namespace std;

// CSkinManagerPaneView

IMPLEMENT_DYNCREATE(CSkinManagerPaneView, CFormView)

HANDLE CSkinManagerPaneView::s_hThreadHandle = NULL;
bool CSkinManagerPaneView::s_bExitThread = false;
HANDLE CSkinManagerPaneView::s_hThreadEndRequest = NULL;
std::vector<CSkinIconItem> CSkinIconItem::s_vecList;

CSkinIconItem::CSkinIconItem()
{
	m_pStruct = NULL;
}

CSkinIconItem::~CSkinIconItem()
{
}

int CSkinIconItem::AddItem( SkinStruct *pStruct, int nIndex )
{
	static int iIndex = 0;
	CSkinIconItem Item;
	Item.m_pStruct = pStruct;
	Item.m_nIndex = nIndex;

	s_vecList.push_back( Item );

	iIndex++;
	return Item.m_nIndex;
}

void CSkinIconItem::RemoveItem( int nIndex )
{
	for( DWORD i=0; i<s_vecList.size(); i++ ) {
		if( s_vecList[i].m_nIndex == nIndex ) {
			s_vecList.erase( s_vecList.begin() + i );
			break;
		}
	}
}

void CSkinIconItem::DeleteAllItems()
{
	SAFE_DELETE_VEC( s_vecList );
}

void CSkinIconItem::DrawItem( CRect *rcRect, CDC *pDC, bool bSelect )
{
	CRect rcTemp, rcTemp2;
	int nInfoHeight = 15;
	if( bSelect == true ) {
		rcTemp = rcRect;
		rcTemp.top = rcTemp.bottom - nInfoHeight;
		pDC->FillSolidRect( rcTemp, RGB(255, 50, 50) );
		pDC->Draw3dRect( rcRect, RGB( 255, 255, 255 ), RGB( 128, 128, 128 ) );
	}
	else  pDC->Draw3dRect( rcRect, RGB( 128, 128, 128 ), RGB( 255, 255, 255 ) );

	// 그림기린다.
	if( m_pStruct->pImage && m_pStruct->cPrevFlag == 1 ) {
		rcTemp = *rcRect;
		rcTemp.DeflateRect( 1, 1, 1, 1 );
		rcTemp.bottom -= nInfoHeight;

		pDC->SetStretchBltMode( COLORONCOLOR );
		pDC->StretchBlt( rcTemp.left, rcTemp.top, rcTemp.Width(), rcTemp.Height(), m_pStruct->pmemDC, 0, 0, m_pStruct->SurfaceDesc.Width, m_pStruct->SurfaceDesc.Height, SRCPAINT );


	}

	// 글씨를 쓴다.
	rcTemp = *rcRect;
	rcTemp.top = rcTemp.bottom - nInfoHeight;
	if( rcTemp.top < rcRect->top ) rcTemp.top = rcRect->top;

	if( bSelect == true )
		pDC->Draw3dRect( &rcTemp, RGB( 255, 255, 255 ), RGB( 128, 128, 128 ) );
	else pDC->Draw3dRect( &rcTemp, RGB( 128, 128, 128 ), RGB( 255, 255, 255 )  );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SelectStockObject( DEFAULT_GUI_FONT );
	rcTemp.DeflateRect( 1, 1, 1, 1 );
	pDC->IntersectClipRect( &rcTemp ); 
	pDC->SetTextColor( RGB( 255, 255, 255 ) );
	pDC->TextOut( rcTemp.left + 1, rcTemp.top + 1, m_pStruct->szFileName );
	pDC->SelectClipRgn( NULL );
	//	pDC->ExcludeClipRect( &rcTemp );

}


CSkinManagerPaneView::CSkinManagerPaneView()
	: CFormView(CSkinManagerPaneView::IDD)
	, CInputReceiver( true )
{
	m_bActivate = false;
	m_dwThreadIndex = 0;
	m_nThreadStatus = -1;

	m_pdcBitmap = NULL;
	m_pmemDC = NULL;

	m_IconSize = CSize( 90, 105 );
	m_IconSpace = CSize( 10, 10 );
	m_nSelectIndex = -1;
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	m_ScrollSize = CSize( 0, 0 );
	m_ScrollPos = CSize( 0, 0 );

	m_bShowPreview = false;
	m_bLBDown = false;
	m_pDragImage = NULL;
	m_bDragBegin = false;
	m_bDragClickFlag = false;

	m_nCurPrevImageSize = 1;

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
}

CSkinManagerPaneView::~CSkinManagerPaneView()
{
	SAFE_DELETE( m_pContextMenu );
	DeleteMemDC();
}

void CSkinManagerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSkinManagerPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_SKINPANE_GET_PREVIEW_IMAGE, OnGetPreviewImage )
	ON_MESSAGE( UM_EXPLORER_MOVE_FILE, OnMoveFileFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER, OnChangeFolderFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER_NAME, OnChangeFolderNameFromExplorer )
	ON_WM_DESTROY()
	ON_COMMAND(ID_SKINADD, &CSkinManagerPaneView::OnSkinadd)
	ON_COMMAND(ID_SKINREFRESH, &CSkinManagerPaneView::OnSkinRefresh)
	ON_COMMAND(ID_SKINREMOVE, &CSkinManagerPaneView::OnSkinremove)
	ON_COMMAND(ID_SKINPREVIEW, &CSkinManagerPaneView::OnSkinpreview)
	ON_COMMAND(ID_SKINMODE, &CSkinManagerPaneView::OnSkinMode)
	ON_COMMAND(ID_SKINSIZE, &CSkinManagerPaneView::OnSkinSize)
	ON_UPDATE_COMMAND_UI(ID_SKINMODE, &CSkinManagerPaneView::OnUpdateSkinMode)
	ON_UPDATE_COMMAND_UI(ID_SKINSIZE, &CSkinManagerPaneView::OnUpdateSkinSize)
	ON_UPDATE_COMMAND_UI(ID_SKINADD, &CSkinManagerPaneView::OnUpdateSkinAdd)
	ON_UPDATE_COMMAND_UI(ID_SKINREMOVE, &CSkinManagerPaneView::OnUpdateSkinRemove)
	ON_UPDATE_COMMAND_UI(ID_SKINREFRESH, &CSkinManagerPaneView::OnUpdateSkinRefresh)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SKINMANAGER_OPENTOETVIEWER, &CSkinManagerPaneView::OnSkinmanagerOpentoetviewer)
	ON_COMMAND(ID_SKINMANAGER_OPENTOETACTIONTOOL, &CSkinManagerPaneView::OnSkinmanagerOpentoetactiontool)
END_MESSAGE_MAP()


// CSkinManagerPaneView 진단입니다.

#ifdef _DEBUG
void CSkinManagerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSkinManagerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSkinManagerPaneView 메시지 처리기입니다.

void CSkinManagerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	SendMessage( UM_REFRESH_PANE_VIEW );
}

LRESULT CSkinManagerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( CFileServer::GetInstance().IsConnect() == false ) return S_OK;

	if( s_hThreadHandle ) {
		s_bExitThread = true;
		SetEvent( s_hThreadEndRequest );
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		CloseHandle( s_hThreadEndRequest );
		s_hThreadHandle = NULL;
		s_hThreadEndRequest = NULL;
		m_dwThreadIndex = 0;
		/*
		while(1) {
			Sleep(10);
			if( m_nThreadStatus == -1 ) break;
		}
		*/
	}
	if( lParam == -1 ) return S_OK;

	if( wParam ) {
		m_szRootFolder = (char*)wParam;
	}

	ResetInfoList();
	m_nThreadStatus = 0;
	s_bExitThread = false;
	s_hThreadEndRequest = CreateEvent( NULL, FALSE, FALSE, NULL );
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
//	GetExitCodeThread( s_hThreadHandle, &m_dwThreadExitCode );

	return S_OK;
}

UINT __stdcall CSkinManagerPaneView::BeginThread( void *pParam )
{
	while(1) {
		Sleep(1);
		if( WaitForSingleObject( CSkinManagerPaneView::s_hThreadEndRequest, 0 ) == WAIT_OBJECT_0 ) {
			OutputDebug( "정상으로\n" );
			break;
		}

		if( CSkinManagerPaneView::s_hThreadHandle == NULL ) {
			OutputDebug( "이상하게 죽었으\n" );
			break;
		}
//		if( CSkinManagerPaneView::s_bExitThread ) break;
		CSkinManagerPaneView *pView = (CSkinManagerPaneView *)pParam;

		if( CFileServer::GetInstance().IsConnect() == false && pView->m_nThreadStatus != -1 ) {
			pView->ResetInfoList();
			pView->Invalidate();
			pView->m_nThreadStatus = -1;
			continue;
		}
		if( CFileServer::GetInstance().IsConnect() == true && pView->m_nThreadStatus == -1 && CGlobalValue::GetInstance().m_nActiveView == 2 ) {
			pView->m_nThreadStatus = 0;
		}


		switch( pView->m_nThreadStatus ) {
			case 0:
				pView->SearchDefaultInfo();
				pView->m_nThreadStatus = 1;
				pView->Invalidate();
				break;
			case 1:
				if( pView->m_bShowPreview == false ) break;
				if( pView->SearchPrevInfo() == true ) {
					pView->m_nThreadStatus = 2;
				}
				break;
			case 2:
				break;
		}

	}
	CSkinManagerPaneView *pView = (CSkinManagerPaneView *)pParam;
	if( pView ) pView->m_nThreadStatus = -1;

	_endthreadex(0);
//	_endthreadex( 0 );
	return 0;
}


BOOL CSkinManagerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CSkinManagerPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}
	if( m_wndToolBar2.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar2.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar2.MoveWindow(0, nTop, cx, sz.cy);

		CXTPControl *pControl = m_wndToolBar2.GetControls()->FindControl( xtpControlEdit, ID_SKINNAME, TRUE, FALSE );
		pControl->SetRect( CRect( 2, 2, cx-2, sz.cy-2 ) );

		m_wndToolBar2.Invalidate(FALSE);
		nTop += sz.cy;
	}
	DeleteMemDC();
	CreateMemDC();
	ResetScroll();
}

void CSkinManagerPaneView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CreateMemDC();

	CBitmap *pOldBitmap;

	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();
	m_wndToolBar2.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );


	int nWidthCount = ( rcRect.Width() - m_IconSpace.cx ) / ( m_IconSize.cx + m_IconSpace.cx );
	int nHeightCount = ( rcRect.Height() - rcToolbar.Height() - m_IconSpace.cy ) / ( m_IconSize.cy + m_IconSpace.cy );
	if( nWidthCount == 0 ) nWidthCount = 1;
	if( nHeightCount == 0 ) nHeightCount = 1;

	CRect rcTemp;
	GetClientRect( &rcTemp );
	rcTemp.bottom -= rcToolbar.Height();
	m_pmemDC->FillSolidRect( &rcTemp, RGB( 0, 0, 0 ) );

	std::vector<CSkinIconItem> *pvecList = &CSkinIconItem::s_vecList;
	CRect rcIconRect;
	bool bSelect;
	for( DWORD i=0; i<pvecList->size(); i++ ) {
		rcIconRect.left = m_IconSpace.cx + ( ( i % nWidthCount ) * ( m_IconSize.cx + m_IconSpace.cx ) );
		rcIconRect.top = m_IconSpace.cy + ( ( i / nWidthCount ) * ( m_IconSize.cy + m_IconSpace.cy ) );
		rcIconRect.right = rcIconRect.left + m_IconSize.cx;
		rcIconRect.bottom = rcIconRect.top + m_IconSize.cy;

		rcIconRect.left += rcRect.left;
		rcIconRect.top += m_IconSpace.cy;//rcRect.top;
		rcIconRect.right += rcRect.left;
		rcIconRect.bottom += m_IconSpace.cy;//rcRect.top;
		rcIconRect.top -= m_ScrollPos.cy;
		rcIconRect.bottom -= m_ScrollPos.cy;
		if( rcIconRect.left < 0 && rcIconRect.top < 0 && rcIconRect.right < 0 && rcIconRect.bottom < 0 ) continue;
		if( rcIconRect.left < 0 && rcIconRect.top > rcRect.Height() && rcIconRect.right < 0 && rcIconRect.bottom > rcRect.Height() ) continue;
		if( rcIconRect.left > rcRect.Width() && rcIconRect.top < 0 && rcIconRect.right > rcRect.Width() && rcIconRect.bottom < 0 ) continue;
		if( rcIconRect.left > rcRect.Width() && rcIconRect.top > rcRect.Height() && rcIconRect.right > rcRect.Width() && rcIconRect.bottom > rcRect.Height() ) continue;
		if( m_nSelectIndex == (int)(*pvecList)[i].m_nIndex ) bSelect = true;
		else bSelect = false;
		(*pvecList)[i].DrawItem( &rcIconRect, m_pmemDC, bSelect );
	}

	// 스크롤 그린다.
	CRect rcScroll;
	rcScroll.top = 0;
	rcScroll.bottom = rcRect.bottom - rcToolbar.Height();
	rcScroll.left = rcRect.right - 7;
	rcScroll.right = rcRect.right;
	m_pmemDC->Draw3dRect( &rcScroll, RGB( 129, 129, 129 ), RGB( 255, 255, 255 ) );

	if( m_ScrollSize.cy != 0 ) {
		CRect rcPos;
		rcPos.left = rcScroll.left + 1;
		rcPos.right = rcScroll.right - 1;
		rcPos.top = rcScroll.top + (int)( ( ( rcScroll.Height() - 20 ) / (float)m_ScrollSize.cy ) * (float)m_ScrollPos.cy );
		rcPos.bottom = rcPos.top + 20;
		m_pmemDC->FillSolidRect( &rcPos, RGB( 250, 213, 14 ) );
	}


	dc.BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height() ,
		m_pmemDC, 0, 0, SRCCOPY );
	m_pmemDC->SelectObject( pOldBitmap );
}


void CSkinManagerPaneView::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();
	m_wndToolBar2.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CSkinManagerPaneView::DeleteMemDC()
{
	if( m_pdcBitmap ) {
		m_pdcBitmap->DeleteObject();
		SAFE_DELETE( m_pdcBitmap );
	}
	if( m_pmemDC ) {
		m_pmemDC->DeleteDC();
		SAFE_DELETE( m_pmemDC );
	}
}

void CSkinManagerPaneView::ResetScroll()
{
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();
	m_wndToolBar2.GetClientRect( &rcToolbar );
	rcRect.top += rcToolbar.Height();

	int nWidthCount = ( rcRect.Width() - m_IconSpace.cx ) / ( m_IconSize.cx + m_IconSpace.cx );
	int nHeightCount = ( rcRect.Height() - rcToolbar.Height() - m_IconSpace.cy ) / ( m_IconSize.cy + m_IconSpace.cy );
	if( nWidthCount == 0 ) nWidthCount = 1;
	if( nHeightCount == 0 ) nHeightCount = 1;

	std::vector<CSkinIconItem> *pvecList = &CSkinIconItem::s_vecList;

	int nNeedHeightCount = (int)pvecList->size() / nWidthCount;
	if( (int)pvecList->size() % nWidthCount != 0 ) nNeedHeightCount++;

	m_ScrollSize.cx = 0;
	m_ScrollSize.cy = 0;
	if( nNeedHeightCount > nHeightCount ) {
		m_ScrollSize.cx = 0;
		m_ScrollSize.cy = ( nNeedHeightCount * (m_IconSize.cy + m_IconSpace.cy) );
		if( m_ScrollPos.cy > m_ScrollSize.cy )
			m_ScrollPos.cy = m_ScrollSize.cy;
	}
}

int CSkinManagerPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_SKINLIST);

	m_wndToolBar2.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar2.LoadToolBar(IDR_SKINLIST2);

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_SKINREFRESH, TRUE, FALSE );
	pControl->SetStyle(xtpButtonIconAndCaption);

	m_wndToolBar.GetControls()->SetControlType( 3, xtpControlCheckBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlCheckBox, ID_SKINPREVIEW, TRUE, FALSE );
	pControl->SetChecked( FALSE );

	m_wndToolBar.GetControls()->SetControlType( 4, xtpControlComboBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SKINMODE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->InsertString( pCombo->GetCount(), "Wireframe" );
	pCombo->InsertString( pCombo->GetCount(), "Mesh" );
	pCombo->InsertString( pCombo->GetCount(), "Mapping" );
	pCombo->SetCurSel(1);

	m_wndToolBar.GetControls()->SetControlType( 5, xtpControlComboBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SKINSIZE, TRUE, FALSE );
	pCombo = (CXTPControlComboBox*)pControl;
	pCombo->InsertString( pCombo->GetCount(), "32x32" );
	pCombo->InsertString( pCombo->GetCount(), "64x64" );
	pCombo->InsertString( pCombo->GetCount(), "128x128" );
	pCombo->InsertString( pCombo->GetCount(), "256x256" );
	pCombo->InsertString( pCombo->GetCount(), "512x512" );
	pCombo->SetCurSel(2);


	m_wndToolBar2.GetControls()->SetControlType( 0, xtpControlEdit );
	return 0;
}

void CSkinManagerPaneView::SearchDefaultInfo( char *szFolder )
{
	std::vector<CString> szVecFolderName;
	CString szFolderName;
	if( szFolder == NULL ) {
		ResetInfoList();
		if( CFileServer::GetInstance().IsLockNation() ) {
			szFolderName.Format( "%s\\Resource\\%s", CFileServer::GetInstance().GetWorkingFolder(), m_szRootFolder );
			szVecFolderName.push_back( szFolderName );
		}
		szFolderName.Format( "%s\\Resource\\%s", CFileServer::GetInstance().GetLocalWorkingFolder(), m_szRootFolder );
		szVecFolderName.push_back( szFolderName );
	}
	else szVecFolderName.push_back( szFolder );

	std::vector<std::string> szVecList;
	for( DWORD j=0; j<szVecFolderName.size(); j++ ) {
		szVecList.clear();
		FindFileListInDirectory( szVecFolderName[j], "*.skn", szVecList, false );

		SkinStruct *pStruct;

		for( DWORD i=0; i<szVecList.size(); i++ ) {
			std::map<CString, CString>::iterator it = m_MapSkinList.find( szVecList[i].c_str() );
			if( it != m_MapSkinList.end() ) continue;

			pStruct = new SkinStruct;
			pStruct->szFileName = szVecList[i].c_str();
			pStruct->szFolder = szVecFolderName[j];
			pStruct->cPrevFlag = 0;

			m_pVecSkinList.push_back( pStruct );
			m_MapSkinList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
			if( _stricmp( GetCurrentDir(), szVecFolderName[j] ) == NULL )
				CSkinIconItem::AddItem( pStruct, (int)m_pVecSkinList.size() - 1 );
		}

		CString szTemp;
		std::vector<CFileNameString> szVecFolder;
		_FindFolder( szVecFolderName[j].GetBuffer(), szVecFolder );

		for( DWORD i=0; i<szVecFolder.size(); i++ ) {
			szTemp.Format( "%s\\%s", szVecFolderName[j], szVecFolder[i].c_str() );
			SearchDefaultInfo( szTemp.GetBuffer() );
		}
	}
	if( szFolder == NULL ) ResetScroll();
}

bool CSkinManagerPaneView::SearchPrevInfo()
{
	CString szFullName;
	LPDIRECT3DTEXTURE9 pTexture;
	HRESULT hResult;
	D3DLOCKED_RECT rc;
	D3DSURFACE_DESC sd;
	int nImageSize = 64;
	switch( m_nCurPrevImageSize ) {
		case 0:	nImageSize = 32;	break;
		case 1:	nImageSize = 64;	break;
		case 2:	nImageSize = 128;	break;
		case 3:	nImageSize = 256;	break;
		case 4:	nImageSize = 512;	break;
	}

	int nStartOffset = 0;
	int nEndOffset = (int)m_pVecSkinList.size();

	if( m_nSelectFolderOffsetStart >= 0 ) nStartOffset = m_nSelectFolderOffsetStart;
	if( m_nSelectFolderOffsetEnd >= 0 ) nEndOffset = m_nSelectFolderOffsetEnd;

	bool bSearchFirst = false;
	bool bCheckFlag = false;
	for( int i=nStartOffset; i<nEndOffset; i++ ) {
		if( !bSearchFirst && m_nSelectFolderOffsetStart >= 0 && m_nSelectFolderOffsetEnd >= 0 && i >= m_nSelectFolderOffsetEnd - 1 ) {
			bSearchFirst = true;
		}
		if( !bSearchFirst || bCheckFlag ) {
			if( m_pVecSkinList[i]->pImage && m_pVecSkinList[i]->cPrevFlag == 1 ) continue;
			if( m_pVecSkinList[i]->cPrevFlag == -1 ) continue;
		}
		szFullName.Format( "%s\\%s", m_pVecSkinList[i]->szFolder, m_pVecSkinList[i]->szFileName );

		if( CRenderBase::s_bRenderMainScene == true ) return false;

		// 렌더 타겟 카메라 생성
		CRenderBase::GetInstance().Lock();
		EtCameraHandle CamHandle;
		SCameraInfo CamInfo;
		EtMatrix matWorld;
		CamInfo.fWidth = (float)nImageSize;
		CamInfo.fHeight = (float)nImageSize;
		CamInfo.Target = CT_RENDERTARGET;
		CamHandle = EternityEngine::CreateCamera( &CamInfo, false );
		MatrixEx Cross;
		SAABox Box;

		EtMatrixIdentity( &matWorld );
//		CRenderBase::GetInstance().Lock();
//		if( g_pEtRenderLock ) g_pEtRenderLock->Lock();
		EtSkinHandle hSkinHandle = CamHandle->AddRenderSkin( szFullName, &matWorld );
//		if( g_pEtRenderLock ) g_pEtRenderLock->UnLock();
//		CRenderBase::GetInstance().Unlock();
		if( !hSkinHandle ) {
			m_pVecSkinList[i]->cPrevFlag = -1;
			SAFE_RELEASE_SPTR( CamHandle );
			CRenderBase::GetInstance().Unlock();
			return false;
		}
		hSkinHandle->GetMeshHandle()->GetBoundingBox( Box );

		EtVector3 vCross;
		float fSizeX = Box.Max.x - Box.Min.x;
		float fSizeY = Box.Max.y - Box.Min.y;
		Cross.m_vPosition.z = -( max( fSizeX, fSizeY ) * 1.6f );

		Cross.m_vPosition.x = Box.Max.x - ( ( Box.Max.x - Box.Min.x ) / 2.f );
		Cross.m_vPosition.y = Box.Max.y - ( ( Box.Max.y - Box.Min.y ) / 2.f );

		if( CRenderBase::s_bRenderMainScene == true ) {
			SAFE_RELEASE_SPTR( CamHandle );
			CRenderBase::GetInstance().Unlock();
			return false;
		}
//		CRenderBase::GetInstance().Lock();
		CRenderBase::s_bThreadRenderTarget = true;

		CamHandle->Update( Cross );

		CamHandle->RenderSkinList();

		/*
		if( g_pEtRenderLock ) g_pEtRenderLock->Lock();
		CEtResource::SetWaitDeleteMode( false );
		CamHandle->DeleteRenderSkin( szFullName );
		CEtResource::SetWaitDeleteMode( true );
		if( g_pEtRenderLock ) g_pEtRenderLock->UnLock();
		*/

		CRenderBase::s_bThreadRenderTarget = false;
//		CRenderBase::GetInstance().Unlock();


		EtTextureHandle TextureHandle = CamHandle->GetRenderTargetTexture();
		LPD3DXBUFFER pBuffer = NULL;
		EtBaseTexture *pEtTexture = TextureHandle->GetTexturePtr();

		hResult = D3DXSaveTextureToFileInMemory( &pBuffer, D3DXIFF_BMP, pEtTexture, NULL );

		SAFE_RELEASE_SPTR( CamHandle );

		CRenderBase::GetInstance().Unlock();
		if( FAILED( hResult ) ) {
			m_pVecSkinList[i]->cPrevFlag = -1;
			continue;
		}

		hResult = D3DXCreateTextureFromFileInMemoryEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), nImageSize, nImageSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );

		SAFE_RELEASE( pBuffer );

		if( FAILED( hResult ) || !pTexture ) {
			m_pVecSkinList[i]->cPrevFlag = -1;
			continue;
		}

		pTexture->GetLevelDesc( 0, &sd );
		m_pVecSkinList[i]->SurfaceDesc = sd;

		pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

		CDC *pDC = GetDC();
		m_pVecSkinList[i]->pmemDC = new CDC;
		m_pVecSkinList[i]->pImage = new CBitmap;

		m_pVecSkinList[i]->pmemDC->CreateCompatibleDC( pDC );
		if( pDC ) m_pVecSkinList[i]->pImage->CreateCompatibleBitmap( pDC, nImageSize, nImageSize );	// DC 못구해와서 덤프나는거 방지.
		if( pDC ) ReleaseDC( pDC );

		m_pVecSkinList[i]->pmemDC->SelectObject( m_pVecSkinList[i]->pImage );

		DWORD *pSource;
		DWORD *pRowSource = (DWORD*)rc.pBits;
		DWORD dwColor;
		BYTE r,g,b;

		for( int j=0; j<(int)sd.Height; j++ ) {
			pSource = pRowSource;
			for( int k=0; k<(int)sd.Width; k++ ) {

				dwColor = *pSource;
				r = (BYTE)( ( dwColor << 8 ) >> 24 );
				g = (BYTE)( ( dwColor << 16) >> 24 );
				b = (BYTE)( ( dwColor << 24 ) >> 24 );

				m_pVecSkinList[i]->pmemDC->SetPixel( k, j, RGB(r,g,b) );
				pSource++;
			}
			pRowSource += rc.Pitch / 4;
		}
		pTexture->UnlockRect(0);
		pTexture->Release();
		m_pVecSkinList[i]->cPrevFlag = 1;

		Invalidate();

		if( !bCheckFlag && bSearchFirst ) {
			bCheckFlag = true;
			nStartOffset = 0;
			nEndOffset = (int)m_pVecSkinList.size();
			i = -1;
			continue;
		}

		return false;
	}
//	CRenderBase::s_bThreadRenderTarget = false;
	return true;
}

void CSkinManagerPaneView::ResetInfoList()
{
	CSkinIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( m_pVecSkinList[i]->pImage )
			m_pVecSkinList[i]->pImage->DeleteObject();
		if( m_pVecSkinList[i]->pmemDC )
			m_pVecSkinList[i]->pmemDC->DeleteDC();

		SAFE_DELETE( m_pVecSkinList[i]->pImage );
		SAFE_DELETE( m_pVecSkinList[i]->pmemDC );
	}

	m_MapSkinList.clear();
	SAFE_DELETE_PVEC( m_pVecSkinList );

	m_nThreadStatus = 0;
	m_nSelectIndex = -1;
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	m_ScrollPos = m_ScrollSize = CSize( 0, 0 );
	RefreshFullName();
}

CString CSkinManagerPaneView::GetCurrentDir()
{
	CString szFolder;
	if( m_szCurrentFolder.IsEmpty() ) {
		szFolder.Format( "%s\\Resource\\%s", CFileServer::GetInstance().GetWorkingFolder(), m_szRootFolder );
	}
	else szFolder = m_szCurrentFolder;

	return szFolder;
}

LRESULT CSkinManagerPaneView::OnChangeFolderFromExplorer( WPARAM wParam, LPARAM lParam )
{
	m_szCurrentFolder = (char*)wParam;

	static bool bFirst = false;
	if( bFirst == false ) {
		bFirst = true;
		SendMessage( UM_REFRESH_PANE_VIEW );
		return S_OK;
	}
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	CSkinIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( _stricmp( GetCurrentDir(), m_pVecSkinList[i]->szFolder ) == NULL ) {
			if( m_nSelectFolderOffsetStart == -1 ) m_nSelectFolderOffsetStart = i;
			m_nSelectFolderOffsetEnd = i;
			CSkinIconItem::AddItem( m_pVecSkinList[i], i );
		}
	}
	if( m_nSelectFolderOffsetEnd != -1 ) m_nSelectFolderOffsetEnd++;
	m_ScrollPos = CSize( 0, 0 );
	ResetScroll();
	Invalidate();

	return S_OK;
}

LRESULT CSkinManagerPaneView::OnChangeFolderNameFromExplorer( WPARAM wParam, LPARAM lParam )
{
	m_szCurrentFolder = (char*)wParam;
	CString szSourceName = (char*)lParam;
	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( m_pVecSkinList[i]->szFolder == szSourceName ) {
			m_pVecSkinList[i]->szFolder = m_szCurrentFolder;
		}
	}
	return S_OK;
}

LRESULT CSkinManagerPaneView::OnMoveFileFromExplorer( WPARAM wParam, LPARAM lParam )
{
	char szFullPath[512] = { 0, };
	char szName[512] = { 0, };
	CString szBeforName = (char*)wParam;
	CString szAfterName = (char*)lParam;
	_GetPath( szFullPath, _countof(szFullPath), szBeforName );
	szFullPath[ strlen(szFullPath) - 1 ] = 0;
	_GetFullFileName( szName, _countof(szName), szBeforName );
	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( _stricmp( szFullPath, m_pVecSkinList[i]->szFolder ) == NULL &&
			_stricmp( szName, m_pVecSkinList[i]->szFileName ) == NULL ) {
				memset( szFullPath, 0, sizeof(szFullPath) );
				_GetPath( szFullPath, _countof(szFullPath), szAfterName );
				szFullPath[ strlen(szFullPath) - 1 ] = 0;
				m_pVecSkinList[i]->szFolder = szFullPath;
		}
	}
	return S_OK;
}

void CSkinManagerPaneView::OnDestroy()
{
	__super::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		SetEvent( s_hThreadEndRequest );
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		CloseHandle( s_hThreadEndRequest );
		s_hThreadHandle = NULL;
		s_hThreadEndRequest = NULL;
		m_dwThreadIndex = 0;

	}

	ResetInfoList();
}

bool CSkinManagerPaneView::CalcAccompanimentFile( const char *szSkinName, std::vector<CString> &szVecList )
{
	FILE *fp = NULL;
	// 자기자신도 리스트에 ADd
	szVecList.push_back( szSkinName );

	char szBuf[512] = { 0, };
	CString szPath;
	
	_GetPath( szBuf, _countof(szBuf), szSkinName );
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
	// 스킨이름과 같은 이름의 act 복사해준다. 
	CString szActName;
	memset( szBuf, 0, sizeof(szBuf) );
	_GetFileName( szBuf, _countof(szBuf), szSkinName );
	szActName.Format( "%s%s.act", szPath, szBuf );
	fopen_s( &fp, szActName, "rb" );
	if( fp ) {
		szVecList.push_back( szActName );
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
			if( pParam->nTextureIndex == -1 ) continue;
			EtResourceHandle hResource = CEtResource::GetResource( pParam->nTextureIndex );
			szTempFullName = szPath + hResource->GetFileName();
			if( std::find( szVecList.begin(), szVecList.end(), szTempFullName.c_str() ) != szVecList.end() ) continue;

			szVecList.push_back( szTempFullName.c_str() );
		}
	}
	SAFE_RELEASE_SPTR( Handle );

	return true;
}

void CSkinManagerPaneView::OnSkinadd()
{
	CString szFileList;
	TCHAR szFilter[] = _T( "Eternity Skin File (*.skn)|*.skn|All Files (*.*)|*.*||" );

	CSkinPrevDlg dlg(TRUE, _T("skn"), _T("*.skn"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_ENABLESIZING, szFilter);

	dlg.GetOFN().lpstrFile = szFileList.GetBuffer(100000);
	dlg.GetOFN().nMaxFile = 100000;

	if ( dlg.DoModal( ) == IDOK ) {
		std::vector<CString> szVecFileList;
		ParseFileList( szFileList, "skn", szVecFileList );
		szFileList.ReleaseBuffer();

		CString szFullName;
		std::vector<std::string> szVecList;
		std::vector<CString> szVecResultList;
		szFullName = GetCurrentDir();
		FindFileListInDirectory( szFullName, "*.*", szVecList, false, true );



		for( DWORD k=0; k<szVecFileList.size(); k++ ) {
			BOOL bExist = FALSE;
			char szTemp[2][512] = { 0, };
			_GetFullFileName( szTemp[0], _countof(szTemp[0]), szVecFileList[k] );
			if( strlen( szTemp[0] ) >= 63 ) {
				int test = (int)strlen( szTemp[0] );
				assert(0);
				continue;
			}
			for( DWORD i=0; i<szVecList.size(); i++ ) {
				_GetFullFileName( szTemp[1], _countof(szTemp[1]), szVecList[i].c_str() );
				if( _stricmp( szTemp[0], szTemp[1] ) == NULL ) {
					bExist = TRUE;
					break;
				}
			}

			if( bExist == TRUE ) {
				if( MessageBox( "같은 이름의 파일이 이미 등록되어 있습니다.\n선택한 파일로 덮어쓸래?", "질문", MB_YESNO ) == IDNO ) continue;
				CString szResult = "스킨이 다른 섹터 어딘가에서 사용중입니다.\n그래두 덮어쓸래?\n\n";
				if( CheckAnyoneUseSkin( szTemp[0], &szResult ) == true ) {
					if( MessageBox( szResult, "질문", MB_YESNO ) == IDNO ) continue;
				}

				for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
					if( _stricmp( szTemp[0], m_pVecSkinList[i]->szFileName ) == NULL ) {
						// 같은 폴더에 있는 파일일 경우랑 폴더가 틀릴 경우랑 따로 처리해줘야한다.
						CString szExistPath = m_pVecSkinList[i]->szFolder;
						if( _stricmp( szExistPath, GetCurrentDir() ) == NULL ) {
							SAFE_DELETE( m_pVecSkinList[i]->pImage );
							SAFE_DELETE( m_pVecSkinList[i]->pmemDC );
							m_pVecSkinList[i]->cPrevFlag = 0;
						}
						else {
							bExist = false;
							CString szExistFullName;
							szExistFullName.Format( "%s\\%s", szExistPath, szTemp[0] );
							DeleteFile( szExistFullName );

							DeleteInSkinStruct( i );
						}
						break;
					}
				}
			}
			std::vector<CString> szVecAccompanimentList;
			if( CalcAccompanimentFile( szVecFileList[k], szVecAccompanimentList ) == false ) {
				MessageBox( "생성할 수 없는 파일입니다.", "Error", MB_OK );
				continue;
			}
			
			char szFileName[512] = { 0, };
			CString szErrorFileName;
			CString szErrorStr;
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
			if( cResult == -1 ) continue;
			else if( cResult == 0 ) {
				CString szErrorStr;
				szErrorStr.Format( "스킨파일에 필요한 다음 파일들을 복사를 못했어요\n%s", szErrorFileName );
				MessageBox( szErrorStr, "경고", MB_OK );
			}

			if( bExist == TRUE ) continue;	
			SetFileAttributes( szFullName, FILE_ATTRIBUTE_NORMAL );

			_GetFullFileName( szFileName, _countof(szFileName), szVecFileList[k] );

			SkinStruct *pStruct;
			pStruct = new SkinStruct;
			pStruct->szFileName = szFileName;
			pStruct->szFolder = GetCurrentDir();
			pStruct->cPrevFlag = 0;

			m_pVecSkinList.push_back( pStruct );
			m_MapSkinList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
			CSkinIconItem::AddItem( pStruct, (int)m_pVecSkinList.size() - 1 );
		}

		m_nThreadStatus = 1;
		ResetScroll();
		Invalidate();
	}
	else szFileList.ReleaseBuffer();
}

void CSkinManagerPaneView::OnSkinremove()
{
	if( m_nSelectIndex == -1 ) return;

	if( _stricmp( m_pVecSkinList[m_nSelectIndex]->szFileName, "Light.skn" ) == NULL ||
		_stricmp( m_pVecSkinList[m_nSelectIndex]->szFileName, "Camera.skn" ) == NULL ) {
			MessageBox( "삭제할 수 없는 파일입니다.\n", "에러", MB_OK );
			return;
	}

	CFileServer::GetInstance().UpdateUseSkinList();
	CString szResult = "삭제하려고 한 스킨이 다음 섹터에서 사용중입니다.\n\n";
	if( CheckAnyoneUseSkin( m_pVecSkinList[m_nSelectIndex]->szFileName, &szResult ) == true ) {
		MessageBox( szResult, "에러", MB_OK );
		return;
	}

	bool bDeleteOnlySkin = false;

	CString szFullName;
	szFullName.Format( "%s\\%s", GetCurrentDir(), m_pVecSkinList[m_nSelectIndex]->szFileName );

	std::vector<CString> szVecAccompanimentList;
	CString szErrorFileName;

	if( CalcAccompanimentFile( szFullName, szVecAccompanimentList ) == false ) {
		if( MessageBox( "내부에서 어떤 파일들을 사용하는지 알 수 없습니다.\n스킨이랑 에니파일만이래도 지울래요?", "질문", MB_YESNO ) == IDNO ) return;

		bDeleteOnlySkin = true;
	}
	else {
		szErrorFileName += "내부에 다음 파일들을 사용하고 있습니다.\n\n";
		for( DWORD i=0; i<szVecAccompanimentList.size(); i++ ) {
			szErrorFileName += "\n    ";
			szErrorFileName += szVecAccompanimentList[i];
		}
		szErrorFileName += "\n\n전부 지워버릴까요? 아님 스킨만 지울까요? 예를 누르면 전부, 아니요는 스킨만입죠.";
		int nResult = MessageBox( szErrorFileName, "질문", MB_YESNOCANCEL );
		switch( nResult ) {
			case IDYES:
				break;
			case IDNO:
				szVecAccompanimentList.clear();
				szVecAccompanimentList.push_back( szFullName );
				break;
			case IDCANCEL:
				return;
				break;
		}
	}

	szErrorFileName.Empty();
	char cResult = 1;
	for( DWORD i=0; i<szVecAccompanimentList.size(); i++ ) {
		if( DeleteFile( szVecAccompanimentList[i] ) == false ) {
			if( i == 0 ) cResult = -1;
			else cResult = 0;
			szErrorFileName += "\n    ";
			szErrorFileName += szVecAccompanimentList[i];
		}
	}
	if( cResult == -1 ) {
		MessageBox( "삭제 실패", "에러", MB_OK );
		return;
	}
	else if( cResult == 0 ) {
		CString szErrorStr;


		szErrorStr.Format( "다음 파일들을 삭제를 복사를 못했어요\n %s", szErrorFileName );
		MessageBox( szErrorStr, "경고", MB_OK );
	}

	// 지울때는 쓰레드 잠시 종료시키구.. 기존 쓰레드 Process 인덱스 기억해노쿠.. 지운 후 다시 쓰레드 가동
	DeleteInSkinStruct( m_nSelectIndex );
	m_nSelectIndex = -1;

	CSkinIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( _stricmp( GetCurrentDir(), m_pVecSkinList[i]->szFolder ) == NULL )
			CSkinIconItem::AddItem( m_pVecSkinList[i], i );
	}

	ResetScroll();
	Invalidate();
}

void CSkinManagerPaneView::OnSkinRefresh()
{
	CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CSkinManagerPaneView::OnSkinpreview()
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlCheckBox, ID_SKINPREVIEW, TRUE, FALSE );
	pControl->SetChecked( !pControl->GetChecked() );
	if( pControl->GetChecked() == TRUE ) m_bShowPreview = true;
	else m_bShowPreview = false;
}

void CSkinManagerPaneView::OnSkinMode()
{
	/*
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SKINMODE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	if( pCombo->GetCurSel() == m_nCurPrevImageSize ) return;
	*/

}

void CSkinManagerPaneView::OnSkinSize()
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SKINSIZE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	if( pCombo->GetCurSel() == m_nCurPrevImageSize ) return;

	m_nCurPrevImageSize = pCombo->GetCurSel();

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CSkinManagerPaneView::OnUpdateSkinMode(CCmdUI *pCmdUI)
{
	// 일단 막아놓는다.. 쓸지 안쓸지 몰라서.
//	pCmdUI->Enable( (BOOL)m_bShowPreview );
	pCmdUI->Enable( FALSE );
}

void CSkinManagerPaneView::OnUpdateSkinAdd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CSkinManagerPaneView::OnUpdateSkinRemove(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
	if( m_nSelectIndex == -1 ) pCmdUI->Enable( false );
}

void CSkinManagerPaneView::OnUpdateSkinRefresh(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CSkinManagerPaneView::OnUpdateSkinSize(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( (BOOL)m_bShowPreview );
}

BOOL CSkinManagerPaneView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( GetAKState( VK_LCONTROL ) < 0 ) {
		if( zDelta > 0 ) {
			m_IconSize.cx += 5;
			m_IconSize.cy += 5;
		}
		else {
			if( m_IconSize.cx > 10 && m_IconSize.cy > 10 ) {
				m_IconSize.cx -= 5;
				m_IconSize.cy -= 5;
			}
		}
		ResetScroll();
		Invalidate();
	}
	else {
		if( zDelta > 0 ) m_ScrollPos.cy -= ( m_IconSize.cy + m_IconSpace.cy );
		else m_ScrollPos.cy += ( m_IconSize.cy + m_IconSpace.cy );

		if( m_ScrollPos.cy < 0 ) m_ScrollPos.cy = 0;
		else if( m_ScrollPos.cy >= m_ScrollSize.cy ) m_ScrollPos.cy = m_ScrollSize.cy;

		Invalidate();
	}

	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSkinManagerPaneView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_MOUSE_LB_DOWN ) {
		m_bLBDown = true;
		m_bDragBegin = false;
	}
	if( nReceiverState & IR_MOUSE_LB_UP ) {
		m_bLBDown = false; 
		m_bDragBegin = false;
		if( m_pDragImage ) {
			ReleaseCapture();
			m_pDragImage->DragLeave( GetDesktopWindow() );
			m_pDragImage->EndDrag();
			SAFE_DELETE( m_pDragImage );

			if( m_nSelectIndex != -1 ) {
				CPoint p;
				GetCursorPos( &p );
				CWnd* pDropWnd = WindowFromPoint(p);
				if( pDropWnd ) {
					long lPos;
					pDropWnd->ScreenToClient( &p );
					lPos = p.x << 16 | p.y;
					CString szTemp;
					szTemp = m_pVecSkinList[m_nSelectIndex]->szFolder + "\\" + m_pVecSkinList[m_nSelectIndex]->szFileName;
					pDropWnd->SendMessage( UM_SKINPANE_DROPITEM, (WPARAM)szTemp.GetBuffer(), lPos );
				}
			}
		}
	}
	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_bDragClickFlag == true && m_bDragBegin == false && m_bLBDown == true && IsInMouseRect( this ) && m_nSelectIndex != -1 && m_pDragImage == NULL && m_pVecSkinList[m_nSelectIndex]->cPrevFlag != -1 ) {
			CPoint p;
			GetCursorPos( &p );
			m_bDragBegin = true;

			SetCapture();
			m_pDragImage = new CImageList;
			m_pDragImage->Create( 64, 64, ILC_COLOR32|ILC_MASK, 1, 1 );
			if( m_pVecSkinList[m_nSelectIndex]->pImage ) 
				m_pDragImage->Add( m_pVecSkinList[m_nSelectIndex]->pImage, RGB( 0, 0, 0 ) );

			m_pDragImage->BeginDrag( 0, CPoint( 0, 0 ) );
			m_pDragImage->DragEnter( GetDesktopWindow(), p ) ;
		}
		if( m_pDragImage ) {
			CPoint p;
			GetCursorPos( &p );

			m_pDragImage->DragMove( p );
			m_pDragImage->DragShowNolock( FALSE );
			m_pDragImage->DragShowNolock( TRUE );

			CWnd* pDropWnd = WindowFromPoint(p);
			if( pDropWnd ) {
				long lPos;
				pDropWnd->ScreenToClient( &p );
				lPos = p.x << 16 | p.y;
				pDropWnd->SendMessage( UM_SKINPANE_DROPMOVEITEM, lPos );
			}
		}
		m_bDragClickFlag = false;
	}
}

void CSkinManagerPaneView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	point.y -= rcToolbar.Height();
	m_wndToolBar2.GetClientRect( &rcToolbar );
	point.y -= rcToolbar.Height();
	rcRect.top += m_IconSpace.cy;

	int nWidthCount = ( rcRect.Width() - m_IconSpace.cx ) / ( m_IconSize.cx + m_IconSpace.cx );
	int nHeightCount = ( rcRect.Height() - rcToolbar.Height() - m_IconSpace.cy ) / ( m_IconSize.cy + m_IconSpace.cy );
	if( nWidthCount == 0 ) nWidthCount = 1;
	if( nHeightCount == 0 ) nHeightCount = 1;

	m_pmemDC->FillSolidRect( &rcRect, RGB( 0, 0, 0 ) );
	std::vector<CSkinIconItem> *pvecList = &CSkinIconItem::s_vecList;
	CRect rcIconRect;
	m_nSelectIndex = -1;
	CPoint p = point;
	p.x += m_ScrollPos.cx;
	p.y += m_ScrollPos.cy;
	m_bDragClickFlag = false;
	for( DWORD i=0; i<pvecList->size(); i++ ) {
		rcIconRect.left = m_IconSpace.cx + ( ( i % nWidthCount ) * ( m_IconSize.cx + m_IconSpace.cx ) );
		rcIconRect.top = m_IconSpace.cy + ( ( i / nWidthCount ) * ( m_IconSize.cy + m_IconSpace.cy ) );
		rcIconRect.right = rcIconRect.left + m_IconSize.cx;
		rcIconRect.bottom = rcIconRect.top + m_IconSize.cy;

		rcIconRect.left += rcRect.left;
		rcIconRect.top += rcRect.top;
		rcIconRect.right += rcRect.left;
		rcIconRect.bottom += rcRect.top;

		if( p.x >= rcIconRect.left && p.x <= rcIconRect.right && p.y >= rcIconRect.top && p.y <= rcIconRect.bottom ) {
			m_nSelectIndex = (*pvecList)[i].m_nIndex;
			m_bDragClickFlag = true;
			break;
		}
	}
	Invalidate();
	RefreshFullName();

	__super::OnLButtonDown(nFlags, point);
}

LRESULT CSkinManagerPaneView::OnGetPreviewImage( WPARAM wParam, LPARAM lParam )
{
	char *szFileName = (char *)wParam;
	if( szFileName == NULL ) return 0;

	for( DWORD i=0; i<m_pVecSkinList.size(); i++ ) {
		if( _stricmp( m_pVecSkinList[i]->szFileName, szFileName ) == NULL && m_pVecSkinList[i]->cPrevFlag == 1 ) {
			return (LRESULT)m_pVecSkinList[i];
		}
	}
	return 0;
}

bool CSkinManagerPaneView::CheckAnyoneUseSkin( CString szSkinName, CString *szResult )
{
	return CFileServer::GetInstance().IsUseSkin( szSkinName, szResult );
}

void CSkinManagerPaneView::DeleteInSkinStruct( DWORD dwIndex )
{
	bool bThread = false;
	DWORD dwPrevThreadIndex = -1;
	if( s_hThreadHandle ) {
		s_bExitThread = true;

		SetEvent( s_hThreadEndRequest );
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		CloseHandle( s_hThreadEndRequest );
		s_hThreadHandle = NULL;
		s_hThreadEndRequest = NULL;
		dwPrevThreadIndex = m_dwThreadIndex;
		m_dwThreadIndex = 0;
		bThread = true;
	}

	m_MapSkinList.erase( m_pVecSkinList[dwIndex]->szFileName );
	SAFE_DELETE( m_pVecSkinList[dwIndex] );
	m_pVecSkinList.erase( m_pVecSkinList.begin() + dwIndex );

	if( bThread ) {
		m_nThreadStatus = dwPrevThreadIndex;
		s_bExitThread = false;
		s_hThreadEndRequest = CreateEvent( NULL, FALSE, FALSE, NULL );
		s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
//		GetExitCodeThread( s_hThreadHandle, &m_dwThreadExitCode );
	}
}

void CSkinManagerPaneView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if( strcmp( m_szRootFolder, "Prop" ) == NULL ) {
		if( m_nSelectIndex != -1 ) {
			CWnd *pWnd = GetPaneWnd( PROPPOOL_PANE );
			if( pWnd ) {
				CString szTemp;
				szTemp = m_pVecSkinList[m_nSelectIndex]->szFolder + "\\" + m_pVecSkinList[m_nSelectIndex]->szFileName;
				pWnd->SendMessage( UM_SKINPANE_DBLCLICKITEM, (WPARAM)szTemp.GetBuffer() );
			}
		}
	}

	__super::OnLButtonDblClk(nFlags, point);
}

void CSkinManagerPaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CPoint temp = point;
	ScreenToClient( &temp );
	OnLButtonDown( 0, temp );
	// TODO: Add your message handler code here
	CMenu *pMenu = m_pContextMenu->GetSubMenu(4);

	if( m_nSelectIndex == -1 ) return;

	if( CGlobalValue::GetInstance().m_szViewerLocation.IsEmpty() )
		pMenu->EnableMenuItem( ID_SKINMANAGER_OPENTOETVIEWER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	if( CGlobalValue::GetInstance().m_szViewerLocation.IsEmpty() )
		pMenu->EnableMenuItem( ID_SKINMANAGER_OPENTOETACTIONTOOL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	//pMenu->EnableMenuItem( ID_SKINMANAGER_OPENTOETACTIONTOOL, MF_BYCOMMAND | MF_ENABLED );

	pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void CSkinManagerPaneView::OnSkinmanagerOpentoetviewer()
{
	if( m_nSelectIndex == -1 ) return;

	CString szTemp;
	szTemp = "\"" + m_pVecSkinList[m_nSelectIndex]->szFolder + "\\" + m_pVecSkinList[m_nSelectIndex]->szFileName + "\"";

	HINSTANCE hInst = ShellExecute( CGlobalValue::GetInstance().m_pParentView->m_hWnd, "open", CGlobalValue::GetInstance().m_szViewerLocation, szTemp, NULL, SW_SHOWNORMAL );
}

void CSkinManagerPaneView::OnSkinmanagerOpentoetactiontool()
{
	if( m_nSelectIndex == -1 ) return;

	CString szTemp;
	szTemp = "\"" + m_pVecSkinList[m_nSelectIndex]->szFolder + "\\" + m_pVecSkinList[m_nSelectIndex]->szFileName + "\"";

	HINSTANCE hInst = ShellExecute( CGlobalValue::GetInstance().m_pParentView->m_hWnd, "open", CGlobalValue::GetInstance().m_szActionLocation, szTemp, NULL, SW_SHOWNORMAL );
	
}

void CSkinManagerPaneView::RefreshFullName()
{
	CXTPControlEdit *pEdit = (CXTPControlEdit *)m_wndToolBar2.GetControls()->FindControl( xtpControlEdit, ID_SKINNAME, TRUE, FALSE );

	CString szFullName;

	if( m_nSelectIndex != -1 ) {
		CString szTemp = m_pVecSkinList[m_nSelectIndex]->szFolder + "\\" + m_pVecSkinList[m_nSelectIndex]->szFileName;
		char *pPtr = strstr( szTemp.GetBuffer(), CFileServer::GetInstance().GetLocalWorkingFolder() );
		char *pPtrNation = strstr( szTemp.GetBuffer(), CFileServer::GetInstance().GetWorkingFolder() );

		szFullName = "WorkingFolder";

		if( pPtrNation ) {
			szFullName += pPtrNation + strlen(CFileServer::GetInstance().GetWorkingFolder());
		}
		else if( pPtr ) {
			szFullName += pPtr + strlen(CFileServer::GetInstance().GetLocalWorkingFolder());
		}

		else szFullName = szTemp;
	}
	pEdit->SetEditText( szFullName );
}

void CSkinManagerPaneView::KillThread()
{
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		SetEvent( s_hThreadEndRequest );
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		CloseHandle( s_hThreadEndRequest );
		s_hThreadHandle = NULL;
		s_hThreadEndRequest = NULL;
		m_dwThreadIndex = 0;
	}
	s_bExitThread = false;
}