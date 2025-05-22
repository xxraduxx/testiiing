// TileManagerPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TileManagerPaneView.h"
#include "FileServer.h"
#include "UserMessage.h"
#include <process.h>
#include "SundriesFuncEx.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "TexturePrevDlg.h"
#include <map>
using namespace std;

// CTileManagerPaneView
std::vector<CTileIconItem> CTileIconItem::s_vecList;
HANDLE CTileManagerPaneView::s_hThreadHandle = NULL;
bool CTileManagerPaneView::s_bExitThread = false;

CTileIconItem::CTileIconItem()
{
	m_pStruct = NULL;
}

CTileIconItem::~CTileIconItem()
{
}

int CTileIconItem::AddItem( TileStruct *pStruct, int nIndex )
{
	static int iIndex = 0;
	CTileIconItem Item;
	Item.m_pStruct = pStruct;
	Item.m_nIndex = nIndex;

	s_vecList.push_back( Item );

	iIndex++;
	return Item.m_nIndex;
}

void CTileIconItem::RemoveItem( int nIndex )
{
	for( DWORD i=0; i<s_vecList.size(); i++ ) {
		if( s_vecList[i].m_nIndex == nIndex ) {
			s_vecList.erase( s_vecList.begin() + i );
			break;
		}
	}
}

void CTileIconItem::DeleteAllItems()
{
	SAFE_DELETE_VEC( s_vecList );
}

void CTileIconItem::DrawItem( CRect *rcRect, CDC *pDC, bool bSelect )
{
	CRect rcTemp, rcTemp2;
	int nInfoHeight = 15;
	if( bSelect == true ) {
		pDC->FillSolidRect( rcRect, RGB(255, 50, 50) );
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


IMPLEMENT_DYNCREATE(CTileManagerPaneView, CFormView)

CTileManagerPaneView::CTileManagerPaneView()
	: CFormView(CTileManagerPaneView::IDD)
	,CInputReceiver( true )
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

	m_bShowPreview = true;
	m_nCurPrevImageSize = 1;
	m_bLBDown = false;
	m_pDragImage = NULL;
	m_bDragBegin = false;
	m_bDragClickFlag = false;
}

CTileManagerPaneView::~CTileManagerPaneView()
{
	DeleteMemDC();
}

void CTileManagerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTileManagerPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_TILEPANE_GET_PREVIEW_IMAGE, OnGetPreviewImage )
	ON_MESSAGE( UM_EXPLORER_MOVE_FILE, OnMoveFileFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER, OnChangeFolderFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER_NAME, OnChangeFolderNameFromExplorer )
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_TEXTUREADD, &CTileManagerPaneView::OnTextureadd)
	ON_COMMAND(ID_TEXTUREREFRESH, &CTileManagerPaneView::OnTexturerefresh)
	ON_COMMAND(ID_TEXTUREREMOVE, &CTileManagerPaneView::OnTextureremove)
	ON_COMMAND(ID_TEXTUREPREVIEW, &CTileManagerPaneView::OnTexturepreview)
	ON_COMMAND(ID_TEXTURESIZE, &CTileManagerPaneView::OnTexturesize)
	ON_UPDATE_COMMAND_UI(ID_TEXTURESIZE, &CTileManagerPaneView::OnUpdateTexturesize)
	ON_UPDATE_COMMAND_UI(ID_TEXTUREADD, &CTileManagerPaneView::OnUpdateTextureAdd)
	ON_UPDATE_COMMAND_UI(ID_TEXTUREREMOVE, &CTileManagerPaneView::OnUpdateTextureRemove)
	ON_UPDATE_COMMAND_UI(ID_TEXTUREREFRESH, &CTileManagerPaneView::OnUpdateTextureRefresh)
END_MESSAGE_MAP()


// CTileManagerPaneView 진단입니다.

#ifdef _DEBUG
void CTileManagerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTileManagerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTileManagerPaneView 메시지 처리기입니다.

void CTileManagerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();


	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CTileManagerPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

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

BOOL CTileManagerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

int CTileManagerPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_TEXTURELIST);
	m_wndToolBar2.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar2.LoadToolBar(IDR_SKINLIST2);

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_TEXTUREREFRESH, TRUE, FALSE );
	pControl->SetStyle(xtpButtonIconAndCaption);

	m_wndToolBar.GetControls()->SetControlType( 3, xtpControlCheckBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlCheckBox, ID_TEXTUREPREVIEW, TRUE, FALSE );
	pControl->SetChecked( TRUE );

	m_wndToolBar.GetControls()->SetControlType( 4, xtpControlComboBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_TEXTURESIZE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->InsertString( pCombo->GetCount(), "32x32" );
	pCombo->InsertString( pCombo->GetCount(), "64x64" );
	pCombo->InsertString( pCombo->GetCount(), "128x128" );
	pCombo->InsertString( pCombo->GetCount(), "256x256" );
	pCombo->InsertString( pCombo->GetCount(), "512x512" );
	pCombo->InsertString( pCombo->GetCount(), "1024x1024" );
	pCombo->InsertString( pCombo->GetCount(), "2048x2048" );
	pCombo->SetCurSel(1);

	m_wndToolBar2.GetControls()->SetControlType( 0, xtpControlEdit );

	return 0;
}

void CTileManagerPaneView::SearchDefaultInfo( char *szFolder )
{
	std::vector<CString> szVecFolderName;
	CString szFolderName;
	if( szFolder == NULL ) {
		ResetInfoList();		 
		if( CFileServer::GetInstance().IsLockNation() ) {
			szFolderName.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetWorkingFolder() );
			szVecFolderName.push_back( szFolderName );
		}
		szFolderName.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetLocalWorkingFolder() );
		szVecFolderName.push_back( szFolderName );
	}
	else szVecFolderName.push_back( szFolder );

	std::vector<std::string> szVecList;
	for( DWORD j=0; j<szVecFolderName.size(); j++ ) {
		szVecList.clear();
		FindFileListInDirectory( szVecFolderName[j], "*.dds", szVecList, false );

		TileStruct *pStruct;

		for( DWORD i=0; i<szVecList.size(); i++ ) {
			std::map<CString, CString>::iterator it = m_MapTileList.find( szVecList[i].c_str() );
			if( it != m_MapTileList.end() ) continue;

			pStruct = new TileStruct;
			pStruct->szFileName = szVecList[i].c_str();
			pStruct->szFolder = szVecFolderName[j];
			pStruct->cPrevFlag = 0;

			m_pVecTileList.push_back( pStruct );
			m_MapTileList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
			if( _stricmp( GetCurrentDir(), szVecFolderName[j] ) == NULL )
				CTileIconItem::AddItem( pStruct, (int)m_pVecTileList.size() - 1 );
		}

		CString szTemp;
		std::vector<CFileNameString> szVecFolder;
		_FindFolder( szVecFolderName[j], szVecFolder );

		for( DWORD i=0; i<szVecFolder.size(); i++ ) {
			szTemp.Format( "%s\\%s", szVecFolderName[j], szVecFolder[i].c_str() );
			SearchDefaultInfo( szTemp.GetBuffer() );
		}
	}

	if( szFolder == NULL ) ResetScroll();
}


bool CTileManagerPaneView::SearchPrevInfo()
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
		case 5:	nImageSize = 1024;	break;
		case 6:	nImageSize = 2048;	break;
	}

	int nStartOffset = 0;
	int nEndOffset = (int)m_pVecTileList.size();

	if( m_nSelectFolderOffsetStart >= 0 ) nStartOffset = m_nSelectFolderOffsetStart;
	if( m_nSelectFolderOffsetEnd >= 0 ) nEndOffset = m_nSelectFolderOffsetEnd;

	bool bSearchFirst = false;
	bool bCheckFlag = false;
	for( int i=nStartOffset; i<nEndOffset; i++ ) {
		if( !bSearchFirst && m_nSelectFolderOffsetStart >= 0 && m_nSelectFolderOffsetEnd >= 0 && i >= m_nSelectFolderOffsetEnd - 1 ) {
			bSearchFirst = true;
		}
		if( !bSearchFirst || bCheckFlag ) {
			if( m_pVecTileList[i]->pImage && m_pVecTileList[i]->cPrevFlag == 1 ) continue;
			if( m_pVecTileList[i]->cPrevFlag == -1 ) continue;
		}

		szFullName.Format( "%s\\%s", m_pVecTileList[i]->szFolder, m_pVecTileList[i]->szFileName );

		hResult = D3DXCreateTextureFromFileEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), szFullName, nImageSize, nImageSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );
		if( FAILED( hResult ) || !pTexture ) {
			m_pVecTileList[i]->cPrevFlag = -1;
			continue;
		}
		pTexture->GetLevelDesc( 0, &sd );
		m_pVecTileList[i]->SurfaceDesc = sd;

		pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

		CDC *pDC = GetDC();
		m_pVecTileList[i]->pmemDC = new CDC;
		m_pVecTileList[i]->pImage = new CBitmap;

		m_pVecTileList[i]->pmemDC->CreateCompatibleDC( pDC );
		if( pDC ) m_pVecTileList[i]->pImage->CreateCompatibleBitmap( pDC, nImageSize, nImageSize );
		if( pDC ) ReleaseDC( pDC );

		m_pVecTileList[i]->pmemDC->SelectObject( m_pVecTileList[i]->pImage );

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

				m_pVecTileList[i]->pmemDC->SetPixel( k, j, RGB(r,g,b) );
				pSource++;
			}
			pRowSource += rc.Pitch / 4;
		}
		pTexture->UnlockRect(0);
		pTexture->Release();
		m_pVecTileList[i]->cPrevFlag = 1;
		Invalidate();
		if( !bCheckFlag && bSearchFirst ) {
			bCheckFlag = true;
			nStartOffset = 0;
			nEndOffset = (int)m_pVecTileList.size();
			i = -1;
			continue;
		}

		return false;
	}
	return true;
}

void CTileManagerPaneView::ResetInfoList()
{
	CTileIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
		if( m_pVecTileList[i]->pImage )
			m_pVecTileList[i]->pImage->DeleteObject();
		if( m_pVecTileList[i]->pmemDC )
			m_pVecTileList[i]->pmemDC->DeleteDC();

		SAFE_DELETE( m_pVecTileList[i]->pImage );
		SAFE_DELETE( m_pVecTileList[i]->pmemDC );
	}

	m_MapTileList.clear();
	SAFE_DELETE_PVEC( m_pVecTileList );

	m_nThreadStatus = 0;
	m_nSelectIndex = -1;
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	m_ScrollPos = m_ScrollSize = CSize( 0, 0 );

	RefreshFullName();
}

LRESULT CTileManagerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( CFileServer::GetInstance().IsConnect() == false ) return S_OK;

	if( s_hThreadHandle ) {
		s_bExitThread = true;
		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
	}
	if( lParam == -1 ) return S_OK;


	ResetInfoList();
	m_nThreadStatus = 0;
	s_bExitThread = false;
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );

	return S_OK;
}

UINT __stdcall CTileManagerPaneView::BeginThread( void *pParam )
{
	while(1) {
		Sleep(1);
		if( CTileManagerPaneView::s_hThreadHandle == NULL ) break;
		if( CTileManagerPaneView::s_bExitThread ) break;
		CTileManagerPaneView *pView = (CTileManagerPaneView *)pParam;

		if( CFileServer::GetInstance().IsConnect() == false && pView->m_nThreadStatus != -1 ) {
			pView->ResetInfoList();
			pView->Invalidate();
			pView->m_nThreadStatus = -1;
			continue;
		}
		if( CFileServer::GetInstance().IsConnect() == true && pView->m_nThreadStatus == -1 && ( CGlobalValue::GetInstance().m_nActiveView == 1 || CGlobalValue::GetInstance().m_nActiveView == 3 )) {
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
	CTileManagerPaneView *pView = (CTileManagerPaneView *)pParam;
	if( pView ) pView->m_nThreadStatus = -1;

	_endthreadex( 0 );
	return 0;
}

void CTileManagerPaneView::OnDestroy()
{
	CFormView::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
	}
	ResetInfoList();
}

void CTileManagerPaneView::OnPaint()
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

	std::vector<CTileIconItem> *pvecList = &CTileIconItem::s_vecList;
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

void CTileManagerPaneView::CreateMemDC()
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

void CTileManagerPaneView::DeleteMemDC()
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

void CTileManagerPaneView::ResetScroll()
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

	std::vector<CTileIconItem> *pvecList = &CTileIconItem::s_vecList;

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

void CTileManagerPaneView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	point.y -= rcToolbar.Height();
	m_wndToolBar2.GetClientRect( &rcToolbar );
	point.y -= rcToolbar.Height();
	rcRect.top += m_IconSpace.cy;//rcToolbar.Height();

	int nWidthCount = ( rcRect.Width() - m_IconSpace.cx ) / ( m_IconSize.cx + m_IconSpace.cx );
	int nHeightCount = ( rcRect.Height() - rcToolbar.Height() - m_IconSpace.cy ) / ( m_IconSize.cy + m_IconSpace.cy );
	if( nWidthCount == 0 ) nWidthCount = 1;
	if( nHeightCount == 0 ) nHeightCount = 1;

	m_pmemDC->FillSolidRect( &rcRect, RGB( 0, 0, 0 ) );
	std::vector<CTileIconItem> *pvecList = &CTileIconItem::s_vecList;
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
	CFormView::OnLButtonDown(nFlags, point);
}

/*
bool CTileManagerPaneView::CheckSelectIcon( CPoint p )
{
	CRect rcRect, rcToolbar;
	GetClientRect( &rcRect );
	m_wndToolBar.GetClientRect( &rcToolbar );
	p.y -= rcToolbar.Height();
	rcRect.top += m_IconSpace.cy;

	int nWidthCount = ( rcRect.Width() - m_IconSpace.cx ) / ( m_IconSize.cx + m_IconSpace.cx );
	if( nWidthCount == 0 ) nWidthCount = 1;

	p.x += m_ScrollPos.cx;
	p.y += m_ScrollPos.cy;

	CRect rcIconRect;
	rcIconRect.left = m_IconSpace.cx + ( ( dwIndex % nWidthCount ) * ( m_IconSize.cx + m_IconSpace.cx ) );
	rcIconRect.top = m_IconSpace.cy + ( ( dwIndex / nWidthCount ) * ( m_IconSize.cy + m_IconSpace.cy ) );
	rcIconRect.right = rcIconRect.left + m_IconSize.cx;
	rcIconRect.bottom = rcIconRect.top + m_IconSize.cy;

	rcIconRect.left += rcRect.left;
	rcIconRect.top += rcRect.top;
	rcIconRect.right += rcRect.left;
	rcIconRect.bottom += rcRect.top;

	if( p.x >= rcIconRect.left && p.x <= rcIconRect.right && p.y >= rcIconRect.top && p.y <= rcIconRect.bottom )
		return true;
	return false;
}
*/

void CTileManagerPaneView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( CGlobalValue::GetInstance().m_bEditBlock == true || CGlobalValue::GetInstance().m_bCanDragTile ) {
		if( nReceiverState & IR_MOUSE_LB_DOWN ) {
			m_bLBDown = true;
			m_bDragBegin = false;
		}
		if( nReceiverState & IR_MOUSE_LB_UP ) {
			m_bLBDown = false; 
			m_bDragBegin = false;
			if( m_pDragImage ) {
				ReleaseCapture();
				m_pDragImage->DragLeave(GetDesktopWindow());
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
						szTemp = m_pVecTileList[m_nSelectIndex]->szFolder + "\\" + m_pVecTileList[m_nSelectIndex]->szFileName;
						pDropWnd->SendMessage( UM_TILEPANE_DROPITEM, (WPARAM)szTemp.GetBuffer(), lPos );
						if( pDropWnd->GetParent() ) {
							pDropWnd->GetParent()->SendMessage( UM_TILEPANE_DROPITEM, (WPARAM)szTemp.GetBuffer(), lPos );
							if( pDropWnd->GetParent()->GetParent() ) {
								pDropWnd->GetParent()->GetParent()->SendMessage( UM_TILEPANE_DROPITEM, (WPARAM)szTemp.GetBuffer(), lPos );
							}
						}
					}
				}
			}
		}
		if( nReceiverState & IR_MOUSE_MOVE ) {
			if( m_bDragClickFlag == true && m_bDragBegin == false && m_bLBDown == true && IsInMouseRect( this ) && m_nSelectIndex != -1 && m_pDragImage == NULL && m_pVecTileList[m_nSelectIndex]->cPrevFlag != -1 ) {
				CPoint p;
				GetCursorPos( &p );

				m_bDragBegin = true;
				m_pDragImage = new CImageList;
				m_pDragImage->Create( 64, 64, ILC_COLOR32|ILC_MASK, 1, 1 );
				if( m_pVecTileList[m_nSelectIndex]->pImage ) {
					m_pDragImage->Add( m_pVecTileList[m_nSelectIndex]->pImage, RGB( 0, 0, 0 ) );
				}

				m_pDragImage->BeginDrag( 0, CPoint( 32, 32 ) );
				m_pDragImage->DragEnter( GetDesktopWindow(), p ) ;
				SetCapture();
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
					pDropWnd->SendMessage( UM_TILEPANE_DROPMOVEITEM, lPos );
				}
			}
			m_bDragClickFlag = false;
		}
	}
}

void CTileManagerPaneView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFormView::OnLButtonUp(nFlags, point);
}

void CTileManagerPaneView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CFormView::OnLButtonDblClk(nFlags, point);
}

void CTileManagerPaneView::OnMouseMove(UINT nFlags, CPoint point)
{
	CFormView::OnMouseMove(nFlags, point);
}

BOOL CTileManagerPaneView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void CTileManagerPaneView::OnTextureadd()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CString szFileList;
	TCHAR szFilter[] = _T( "DirectX Texture File (*.dds)|*.dds|All Files (*.*)|*.*||" );

	CTexturePrevDlg dlg(TRUE, _T("dds"), _T("*.dds"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_ENABLESIZING, szFilter);

	dlg.GetOFN().lpstrFile = szFileList.GetBuffer(100000);
	dlg.GetOFN().nMaxFile = 100000;

	if ( dlg.DoModal( ) == IDOK ) {
		std::vector<CString> szVecFileList;
		ParseFileList( szFileList, "dds", szVecFileList );
		szFileList.ReleaseBuffer();

		CString szFullName;
		std::vector<std::string> szVecList;
		std::vector<CString> szVecResultList;
		szFullName.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetWorkingFolder() );
		FindFileListInDirectory( szFullName, "*.*", szVecList, false, true );



		for( DWORD k=0; k<szVecFileList.size(); k++ ) {
			BOOL bExist = FALSE;
			char szTemp[2][512] = { 0, };
			_GetFullFileName( szTemp[0], _countof(szTemp[0]), szVecFileList[k] );
			for( DWORD i=0; i<szVecList.size(); i++ ) {
				_GetFullFileName( szTemp[1], _countof(szTemp[1]), szVecList[i].c_str() );
				if( _stricmp( szTemp[0], szTemp[1] ) == NULL ) {
					bExist = TRUE;
					break;
				}
			}

			if( bExist == TRUE ) {
				if( MessageBox( "같은 이름의 파일이 이미 등록되어 있습니다.\n선택한 파일로 덮어쓸래?", "질문", MB_YESNO ) == IDNO ) continue;
				CString szResult = "덮어 쓰려고 한 텍스쳐가 다른 섹터 어딘가에서 사용중입니다.\n그래두 덮어쓸래?\n\n";
				if( CheckAnyoneUseTexture( szTemp[0], &szResult ) == true ) {
					if( MessageBox( szResult, "질문", MB_YESNO ) == IDNO ) continue;
				}

				for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
					if( _stricmp( szTemp[0], m_pVecTileList[i]->szFileName ) == NULL ) {
						// 같은 폴더에 있는 파일일 경우랑 폴더가 틀릴 경우랑 따로 처리해줘야한다.
						CString szExistPath = m_pVecTileList[i]->szFolder;
						if( _stricmp( szExistPath, GetCurrentDir() ) == NULL ) {
							SAFE_DELETE( m_pVecTileList[i]->pImage );
							SAFE_DELETE( m_pVecTileList[i]->pmemDC );
							m_pVecTileList[i]->cPrevFlag = 0;
						}
						else {
							bExist = false;
							CString szExistFullName;
							szExistFullName.Format( "%s\\%s", szExistPath, szTemp[0] );
							DeleteFile( szExistFullName );

							DeleteInTileStruct( i );
						}
						break;
					}
				}
			}

			char szFileName[512] = { 0, };
			_GetFullFileName( szFileName, _countof(szFileName), szVecFileList[k].GetBuffer() );
			szFullName.Format( "%s\\%s", GetCurrentDir(), szFileName );
			if( CopyFile( szVecFileList[k], szFullName, !bExist ) == false ) {
				MessageBox( "타일 복사 실패!!", "에러", MB_OK );
			}
			else {
				if( bExist == TRUE ) continue;	
				SetFileAttributes( szFullName, FILE_ATTRIBUTE_NORMAL );
				TileStruct *pStruct;
				pStruct = new TileStruct;
				pStruct->szFileName = szFileName;
				pStruct->szFolder = GetCurrentDir();
				pStruct->cPrevFlag = 0;

				m_pVecTileList.push_back( pStruct );
				m_MapTileList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
				CTileIconItem::AddItem( pStruct, (int)m_pVecTileList.size() - 1 );
			}
		}

		m_nThreadStatus = 1;
		ResetScroll();
		Invalidate();
	}
	else szFileList.ReleaseBuffer();

}

void CTileManagerPaneView::OnTextureremove()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_nSelectIndex == -1 ) return;
	if( _stricmp( m_pVecTileList[m_nSelectIndex]->szFileName, "DefaultLayerTexture.dds" ) == NULL ||
		_stricmp( m_pVecTileList[m_nSelectIndex]->szFileName, "GrassTexture.dds" ) == NULL ) {
		MessageBox( "삭제할 수 없는 파일입니다.\n", "에러", MB_OK );
		return;
	}

	CFileServer::GetInstance().UpdateUseTextureList();
	CString szResult = "삭제하려고 한 텍스쳐가 다른 섹터에서 사용중입니다.\n\n";
	if( CheckAnyoneUseTexture( m_pVecTileList[m_nSelectIndex]->szFileName, &szResult ) == true ) {
		MessageBox( szResult, "에러", MB_OK );
		return;
	}

	CString szFullName;
	szFullName.Format( "%s\\%s", GetCurrentDir(), m_pVecTileList[m_nSelectIndex]->szFileName );
	if( DeleteFile( szFullName ) == false ) {
		MessageBox( "삭제 실패", "에러", MB_OK );
		return;
	}
	else {
		// 지울때는 쓰레드 잠시 종료시키구.. 기존 쓰레드 Process 인덱스 기억해노쿠.. 지운 후 다시 쓰레드 가동
		DeleteInTileStruct( m_nSelectIndex );
		m_nSelectIndex = -1;

		CTileIconItem::DeleteAllItems();
		for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
			if( _stricmp( GetCurrentDir(), m_pVecTileList[i]->szFolder ) == NULL )
				CTileIconItem::AddItem( m_pVecTileList[i], i );
		}
		
		ResetScroll();
		Invalidate();
	}
}


void CTileManagerPaneView::OnTexturerefresh()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );

	SendMessage( UM_REFRESH_PANE_VIEW );

}

void CTileManagerPaneView::OnTexturepreview()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlCheckBox, ID_TEXTUREPREVIEW, TRUE, FALSE );
	pControl->SetChecked( !pControl->GetChecked() );
	if( pControl->GetChecked() == TRUE ) m_bShowPreview = true;
	else m_bShowPreview = false;

}

void CTileManagerPaneView::OnTexturesize()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_TEXTURESIZE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	if( pCombo->GetCurSel() == m_nCurPrevImageSize ) return;

	m_nCurPrevImageSize = pCombo->GetCurSel();

	SendMessage( UM_REFRESH_PANE_VIEW );
}

LRESULT CTileManagerPaneView::OnGetPreviewImage( WPARAM wParam, LPARAM lParam )
{
	char *szFileName = (char *)wParam;
	if( szFileName == NULL ) return 0;

	for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
		if( _stricmp( m_pVecTileList[i]->szFileName, szFileName ) == NULL && m_pVecTileList[i]->cPrevFlag == 1 ) {
			return (LRESULT)m_pVecTileList[i];
		}
	}
	return 0;
}

void CTileManagerPaneView::OnUpdateTexturesize(CCmdUI *pCmdUI)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
	pCmdUI->Enable( (BOOL)m_bShowPreview );
}

void CTileManagerPaneView::OnUpdateTextureAdd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CTileManagerPaneView::OnUpdateTextureRemove(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
	if( m_nSelectIndex == -1 ) pCmdUI->Enable( false );
}

void CTileManagerPaneView::OnUpdateTextureRefresh(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

bool CTileManagerPaneView::CheckAnyoneUseTexture( CString szTextureName, CString *szResult )
{
	return CFileServer::GetInstance().IsUseTexture( szTextureName, szResult );
}

void CTileManagerPaneView::DeleteInTileStruct( DWORD dwIndex )
{
	bool bThread = false;
	DWORD dwPrevThreadIndex = -1;
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		dwPrevThreadIndex = m_dwThreadIndex;
		m_dwThreadIndex = 0;
		bThread = true;
	}

	m_MapTileList.erase( m_pVecTileList[dwIndex]->szFileName );
	SAFE_DELETE( m_pVecTileList[dwIndex] );
	m_pVecTileList.erase( m_pVecTileList.begin() + dwIndex );

	if( bThread ) {
		m_nThreadStatus = dwPrevThreadIndex;
		s_bExitThread = false;
		s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
	}
}

LRESULT CTileManagerPaneView::OnChangeFolderFromExplorer( WPARAM wParam, LPARAM lParam )
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
	CTileIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
		if( _stricmp( GetCurrentDir(), m_pVecTileList[i]->szFolder ) == NULL ) {
			if( m_nSelectFolderOffsetStart == -1 ) m_nSelectFolderOffsetStart = i;
			m_nSelectFolderOffsetEnd = i;
			CTileIconItem::AddItem( m_pVecTileList[i], i );
		}
	}
	if( m_nSelectFolderOffsetEnd != -1 ) m_nSelectFolderOffsetEnd++;
	m_ScrollPos = CSize( 0, 0 );
	ResetScroll();
	Invalidate();

	return S_OK;
}

LRESULT CTileManagerPaneView::OnChangeFolderNameFromExplorer( WPARAM wParam, LPARAM lParam )
{
	m_szCurrentFolder = (char*)wParam;
	CString szSourceName = (char*)lParam;
	for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
		if( m_pVecTileList[i]->szFolder == szSourceName ) {
			m_pVecTileList[i]->szFolder = m_szCurrentFolder;
		}
	}
	return S_OK;
}

LRESULT CTileManagerPaneView::OnMoveFileFromExplorer( WPARAM wParam, LPARAM lParam )
{
	char szFullPath[512] = { 0, };
	char szName[512] = { 0, };
	CString szBeforName = (char*)wParam;
	CString szAfterName = (char*)lParam;
	_GetPath( szFullPath, _countof(szFullPath), szBeforName );
	szFullPath[ strlen(szFullPath) - 1 ] = 0;
	_GetFullFileName( szName, _countof(szName), szBeforName );
	for( DWORD i=0; i<m_pVecTileList.size(); i++ ) {
		if( _stricmp( szFullPath, m_pVecTileList[i]->szFolder ) == NULL &&
			_stricmp( szName, m_pVecTileList[i]->szFileName ) == NULL ) {
				memset( szFullPath, 0, sizeof(szFullPath) );
				_GetPath( szFullPath, _countof(szFullPath), szAfterName );
				szFullPath[ strlen(szFullPath) - 1 ] = 0;
				m_pVecTileList[i]->szFolder = szFullPath;
		}
	}
	return S_OK;
}

CString CTileManagerPaneView::GetCurrentDir()
{
	CString szFolder;
	if( m_szCurrentFolder.IsEmpty() ) {
		szFolder.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetWorkingFolder() );
	}
	else szFolder = m_szCurrentFolder;

	return szFolder;
}
void CTileManagerPaneView::RefreshFullName()
{
	CXTPControlEdit *pEdit = (CXTPControlEdit *)m_wndToolBar2.GetControls()->FindControl( xtpControlEdit, ID_SKINNAME, TRUE, FALSE );

	CString szFullName;

	if( m_nSelectIndex != -1 ) {
		CString szTemp = m_pVecTileList[m_nSelectIndex]->szFolder + "\\" + m_pVecTileList[m_nSelectIndex]->szFileName;
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

void CTileManagerPaneView::KillThread()
{
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
	}
	s_bExitThread = false;
}