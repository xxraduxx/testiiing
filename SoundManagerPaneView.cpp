#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundManagerPaneView.h"
#include "UserMessage.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include <Process.h>
#include "EtMatrixEx.h"
#include "SoundPrevDlg.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "RenderBase.h"
#include "EtSoundEngine.h"
#include "fmod.hpp"
#include "fmod.h"
#include "EtSoundChannel.h"
#include "EtSoundChannelGroup.h"
#include <map>
using namespace std;

// CSoundManagerPaneView

IMPLEMENT_DYNCREATE(CSoundManagerPaneView, CFormView)

HANDLE CSoundManagerPaneView::s_hThreadHandle = NULL;
std::vector<CSoundIconItem> CSoundIconItem::s_vecList;
bool CSoundManagerPaneView::s_bExitThread = false;

CSoundIconItem::CSoundIconItem()
{
	m_pStruct = NULL;
}

CSoundIconItem::~CSoundIconItem()
{
}

int CSoundIconItem::AddItem( SoundStruct *pStruct, int nIndex )
{
	static int iIndex = 0;
	CSoundIconItem Item;
	Item.m_pStruct = pStruct;
	Item.m_nIndex = nIndex;

	s_vecList.push_back( Item );

	iIndex++;
	return Item.m_nIndex;
}

void CSoundIconItem::RemoveItem( int nIndex )
{
	for( DWORD i=0; i<s_vecList.size(); i++ ) {
		if( s_vecList[i].m_nIndex == nIndex ) {
			s_vecList.erase( s_vecList.begin() + i );
			break;
		}
	}
}

void CSoundIconItem::DeleteAllItems()
{
	SAFE_DELETE_VEC( s_vecList );
}

CRect CSoundIconItem::GetSoundButtonRect( CRect *rcRect )
{
	CRect rcTemp = *rcRect;
	rcTemp.left += 5;
	rcTemp.top = rcTemp.bottom - 40;
	rcTemp.bottom = rcTemp.bottom - 20;
	rcTemp.right = rcTemp.left + 20;

	return rcTemp;
}

void CSoundIconItem::DrawItem( CRect *rcRect, CDC *pDC, bool bSelect )
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
	rcTemp = *rcRect;
	rcTemp.DeflateRect( 1, 1, 1, 1 );
	rcTemp.bottom -= nInfoHeight;

	CWnd *pWnd = GetPaneWnd( SOUNDMNG_PANE );
	if( pWnd ) {
		CDC *pBitmap = ((CSoundManagerPaneView*)pWnd)->GetBackImage();
		pDC->SetStretchBltMode( COLORONCOLOR );
		pDC->StretchBlt( rcTemp.left, rcTemp.top, rcTemp.Width(), rcTemp.Height(), pBitmap, 0, 0, 32, 31, SRCPAINT );
	}
	// 버튼 그린다
	bool bDrawPlayButton = true;
	if( m_pStruct->pChannel ) {
		bDrawPlayButton = false;
		if( m_pStruct->pChannel->IsPause() )
			bDrawPlayButton = true;
	}
	rcTemp = GetSoundButtonRect( rcRect );
	pDC->FillSolidRect( rcTemp, RGB( 250, 213, 14 ) );
	pDC->Draw3dRect( rcTemp, RGB( 255, 255, 255 ), RGB( 128, 128, 128 ) );

	if( bDrawPlayButton ) {
		CPoint p[3];
		p[0].x = rcTemp.left + 4;
		p[0].y = rcTemp.top + 4;

		p[1].x = rcTemp.left + 4;
		p[1].y = rcTemp.bottom - 4;

		p[2].x = rcTemp.right - 4;
		p[2].y = p[0].y + ( (p[1].y - p[0].y) / 2 );

		pDC->Polygon( p, 3 );
	}
	else {
		rcTemp.top += 5;
		rcTemp.bottom -= 5;
		rcTemp.left += 6;
		int nTemp = rcTemp.right;
		rcTemp.right = rcTemp.left + 3;
		pDC->FillSolidRect( rcTemp, RGB( 0, 0, 0 ) );

		rcTemp.right = nTemp - 6;
		rcTemp.left = rcTemp.right - 3;
		pDC->FillSolidRect( rcTemp, RGB( 0, 0, 0 ) );
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

	// 파일 정보 글씨를 쓴다.
	rcTemp = *rcRect;
	rcTemp.bottom -= nInfoHeight;
	rcTemp.DeflateRect( 2, 2, 2, 2 );
	pDC->IntersectClipRect( &rcTemp ); 
	pDC->SelectStockObject( ANSI_VAR_FONT );

//////////////////////////////////////////////////////
	pDC->SetTextColor( RGB( 0, 0, 0 ) );
	rcTemp.left += 1; rcTemp.top += 1;
	pDC->DrawText( m_pStruct->szInfo, rcTemp, 0 );
	pDC->SetTextColor( RGB( 255, 255, 255 ) );
	rcTemp.left -= 1; rcTemp.top -= 1;
	pDC->DrawText( m_pStruct->szInfo, rcTemp, 0 );

	pDC->SelectClipRgn( NULL );

	//	pDC->ExcludeClipRect( &rcTemp );

}


CSoundManagerPaneView::CSoundManagerPaneView()
: CFormView(CSoundManagerPaneView::IDD)
, CInputReceiver( true )
{
	m_bActivate = false;
	m_dwThreadIndex = 0;
	m_nThreadStatus = -1;

	m_pdcBitmap = NULL;
	m_pmemDC = NULL;
	m_pBackImagememDC = NULL;
	m_pBackImagedcBitmap = NULL;

	m_IconSize = CSize( 90, 105 );
	m_IconSpace = CSize( 10, 10 );
	m_nSelectIndex = -1;
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	m_ScrollSize = CSize( 0, 0 );
	m_ScrollPos = CSize( 0, 0 );

	m_bLBDown = false;
	m_pDragImage = NULL;
	m_bDragBegin = false;
	m_bDragClickFlag = false;

	m_nViewFileType = 0;
}

CSoundManagerPaneView::~CSoundManagerPaneView()
{
	SAFE_DELETE( m_pBackImagememDC );
	SAFE_DELETE( m_pBackImagedcBitmap );

	DeleteMemDC();
}

void CSoundManagerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundManagerPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_EXPLORER_MOVE_FILE, OnMoveFileFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER, OnChangeFolderFromExplorer )
	ON_MESSAGE( UM_EXPLORER_CHANGE_FOLDER_NAME, OnChangeFolderNameFromExplorer )
	ON_WM_DESTROY()
	ON_COMMAND(ID_SOUNDADD, &CSoundManagerPaneView::OnSoundadd)
	ON_COMMAND(ID_SOUNDREMOVE, &CSoundManagerPaneView::OnSoundremove)
	ON_COMMAND(ID_SOUNDMODE, &CSoundManagerPaneView::OnSoundMode)
	ON_COMMAND(ID_SOUNDREFRESH, &CSoundManagerPaneView::OnSoundRefresh)
	ON_UPDATE_COMMAND_UI(ID_SOUNDMODE, &CSoundManagerPaneView::OnUpdateSoundMode)
	ON_UPDATE_COMMAND_UI(ID_SOUNDADD, &CSoundManagerPaneView::OnUpdateSoundAdd)
	ON_UPDATE_COMMAND_UI(ID_SOUNDREMOVE, &CSoundManagerPaneView::OnUpdateSoundRemove)
	ON_UPDATE_COMMAND_UI(ID_SOUNDREFRESH, &CSoundManagerPaneView::OnUpdateSoundRefresh)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CSoundManagerPaneView 진단입니다.

#ifdef _DEBUG
void CSoundManagerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSoundManagerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSoundManagerPaneView 메시지 처리기입니다.

void CSoundManagerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_pBackImagememDC = new CDC;
	m_pBackImagedcBitmap = new CBitmap;

	CDC *pDC = GetDC();
	m_pBackImagedcBitmap->LoadBitmap( IDB_SOUNDICON );
	m_pBackImagememDC->CreateCompatibleDC( pDC );
	ReleaseDC( pDC );

	m_pBackImagememDC->SelectObject( m_pBackImagedcBitmap );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

LRESULT CSoundManagerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( CFileServer::GetInstance().IsConnect() == false ) return S_OK;

	if( s_hThreadHandle ) {
		s_bExitThread = true;
		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
	}

	ResetInfoList();
	m_nThreadStatus = 0;
	s_bExitThread = false;
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );

	return S_OK;
}

UINT __stdcall CSoundManagerPaneView::BeginThread( void *pParam )
{
	while(1) {
		Sleep(1);
		if( CSoundManagerPaneView::s_hThreadHandle == NULL ) break;
		if( CSoundManagerPaneView::s_bExitThread ) break;
		CSoundManagerPaneView *pView = (CSoundManagerPaneView *)pParam;

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
				if( pView->SearchPrevInfo() == true ) {
					pView->m_nThreadStatus = 2;
				}
				break;
			case 2:
				break;
		}

	}
	CSoundManagerPaneView *pView = (CSoundManagerPaneView *)pParam;
	if( pView ) pView->m_nThreadStatus = -1;

	_endthreadex( 0 );
	return 0;
}


BOOL CSoundManagerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return TRUE;
	//	return CFormView::OnEraseBkgnd(pDC);
}

void CSoundManagerPaneView::OnSize(UINT nType, int cx, int cy)
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

void CSoundManagerPaneView::OnPaint()
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

	std::vector<CSoundIconItem> *pvecList = &CSoundIconItem::s_vecList;
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


void CSoundManagerPaneView::CreateMemDC()
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

void CSoundManagerPaneView::DeleteMemDC()
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

void CSoundManagerPaneView::ResetScroll()
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

	std::vector<CSoundIconItem> *pvecList = &CSoundIconItem::s_vecList;

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

int CSoundManagerPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_SOUNDLIST);
	m_wndToolBar2.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar2.LoadToolBar(IDR_SKINLIST2);

	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlButton, ID_SOUNDREFRESH, TRUE, FALSE );
	pControl->SetStyle(xtpButtonIconAndCaption);

	m_wndToolBar.GetControls()->SetControlType( 3, xtpControlComboBox );
	pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SOUNDMODE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	pCombo->InsertString( pCombo->GetCount(), "all" );
	pCombo->InsertString( pCombo->GetCount(), "wav" );
	pCombo->InsertString( pCombo->GetCount(), "ogg" );
	pCombo->InsertString( pCombo->GetCount(), "mp3" );
	pCombo->InsertString( pCombo->GetCount(), "wmv" );
	pCombo->InsertString( pCombo->GetCount(), "wma" );
	pCombo->SetCurSel(0);

	m_wndToolBar2.GetControls()->SetControlType( 0, xtpControlEdit );

	return 0;
}

void CSoundManagerPaneView::SearchDefaultInfo( char *szFolder )
{
	std::vector<CString> szVecFolderName;
	CString szFolderName;
	if( szFolder == NULL ) {
		ResetInfoList();		 
		if( CFileServer::GetInstance().IsLockNation() ) {
			szFolderName.Format( "%s\\Resource\\Sound", CFileServer::GetInstance().GetWorkingFolder() );
			szVecFolderName.push_back( szFolderName );
		}
		szFolderName.Format( "%s\\Resource\\Sound", CFileServer::GetInstance().GetLocalWorkingFolder() );
		szVecFolderName.push_back( szFolderName );
	}
	else szVecFolderName.push_back( szFolder );

	std::vector<std::string> szVecList;
	std::vector<std::string> szExtList;
	switch( m_nViewFileType ) {
		case 0:
			szExtList.push_back( "*.wav" );
			szExtList.push_back( "*.mp3" );
			szExtList.push_back( "*.ogg" );
			szExtList.push_back( "*.wmv" );
			szExtList.push_back( "*.wma" );
			break;
		case 1:
			szExtList.push_back( "*.wav" );
			break;
		case 2:
			szExtList.push_back( "*.ogg" );
			break;
		case 3:
			szExtList.push_back( "*.mp3" );
			break;
		case 4:
			szExtList.push_back( "*.wmv" );
			break;
		case 5:
			szExtList.push_back( "*.wma" );
			break;
	}

	for( DWORD j=0; j<szVecFolderName.size(); j++ ) {
		szVecList.clear();
		for( DWORD i=0; i<szExtList.size(); i++ )
			FindFileListInDirectory( szVecFolderName[j], szExtList[i].c_str(), szVecList, false );

		SoundStruct *pStruct;

		for( DWORD i=0; i<szVecList.size(); i++ ) {
			std::map<CString, CString>::iterator it = m_MapSoundList.find( szVecList[i].c_str() );
			if( it != m_MapSoundList.end() ) continue;

			pStruct = new SoundStruct;
			pStruct->szFileName = szVecList[i].c_str();
			pStruct->szFolder = szVecFolderName[j];
			pStruct->cPrevFlag = 0;

			m_pVecSoundList.push_back( pStruct );
			m_MapSoundList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
			if( _stricmp( GetCurrentDir(), szVecFolderName[j] ) == NULL )
				CSoundIconItem::AddItem( pStruct, (int)m_pVecSoundList.size() - 1 );
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

bool CSoundManagerPaneView::SearchPrevInfo()
{
	CString szFullName;
	int nStartOffset = 0;
	int nEndOffset = (int)m_pVecSoundList.size();

	if( m_nSelectFolderOffsetStart >= 0 ) nStartOffset = m_nSelectFolderOffsetStart;
	if( m_nSelectFolderOffsetEnd >= 0 ) nEndOffset = m_nSelectFolderOffsetEnd;

	bool bSearchFirst = false;
	bool bCheckFlag = false;
	for( int i=nStartOffset; i<nEndOffset; i++ ) {
		if( !bSearchFirst && m_nSelectFolderOffsetStart >= 0 && m_nSelectFolderOffsetEnd >= 0 && i >= m_nSelectFolderOffsetEnd - 1 ) {
			bSearchFirst = true;
		}
		if( !bSearchFirst || bCheckFlag ) {
			if( m_pVecSoundList[i]->cPrevFlag == 1 ) continue;
			if( m_pVecSoundList[i]->cPrevFlag == -1 ) continue;
		}

		szFullName.Format( "%s\\%s", m_pVecSoundList[i]->szFolder, m_pVecSoundList[i]->szFileName );


		/*
		CDC *pDC = GetDC();
		m_pVecSoundList[i]->pmemDC = new CDC;
		m_pVecSoundList[i]->pImage = new CBitmap;

		m_pVecSoundList[i]->pImage->LoadBitmap( IDB_SOUNDICON );
		m_pVecSoundList[i]->pmemDC->CreateCompatibleDC( pDC );

		m_pVecSoundList[i]->pmemDC->SelectObject( m_pVecSoundList[i]->pImage );
		ReleaseDC( pDC );
		*/

		FMOD::System *pSystem = CRenderBase::GetInstance().GetSoundEngine()->GetFMODSystem();
		FMOD::Sound *pSound;
		if( pSystem->createStream( szFullName.GetBuffer(), FMOD_DEFAULT, NULL, &pSound ) == FMOD_OK ) {
			FMOD_SOUND_FORMAT Format;
			FMOD_SOUND_TYPE Type;
			int nChannel;
			int nBits;
			unsigned int nLength;

			pSound->getFormat( &Type, &Format, &nChannel, &nBits );
			pSound->getLength( &nLength, FMOD_TIMEUNIT_MS );
			pSound->release();

			CString szInfo;
			szInfo = "Type : ";
			switch( Format ) {
				case FMOD_SOUND_FORMAT_NONE:	szInfo += "Unknown";	break;
				case FMOD_SOUND_FORMAT_PCM8:	szInfo += "PCM8";	break;
				case FMOD_SOUND_FORMAT_PCM16:	szInfo += "PCM16";	break;
				case FMOD_SOUND_FORMAT_PCM24:	szInfo += "PCM24";	break;
				case FMOD_SOUND_FORMAT_PCM32:	szInfo += "PCM32";	break;
				case FMOD_SOUND_FORMAT_PCMFLOAT:szInfo += "PCMFLOAT";	break;
				case FMOD_SOUND_FORMAT_GCADPCM:	szInfo += "GameCube DSP";	break;
				case FMOD_SOUND_FORMAT_IMAADPCM:szInfo += "ADPCM";	break;
				case FMOD_SOUND_FORMAT_VAG:		szInfo += "PS2";	break;
				case FMOD_SOUND_FORMAT_XMA:		szInfo += "XBox360";	break;
				case FMOD_SOUND_FORMAT_MPEG:	szInfo += "MPEG2";	break;
			}
			szInfo += "\nFormat : ";
			switch( Type ) {
				case FMOD_SOUND_TYPE_UNKNOWN:	szInfo += "Unknown";	break;
				case FMOD_SOUND_TYPE_AAC:		szInfo += "AAC";	break;
				case FMOD_SOUND_TYPE_AIFF:      szInfo += "AIFF";	break;
				case FMOD_SOUND_TYPE_ASF:       szInfo += "Microsoft Advanced Systems Format";break;
				case FMOD_SOUND_TYPE_AT3:       szInfo += "Sony ATRAC 3 format";	break;
				case FMOD_SOUND_TYPE_CDDA:      szInfo += "Digital CD audio";	break;
				case FMOD_SOUND_TYPE_DLS:       szInfo += "Sound font";	break;
				case FMOD_SOUND_TYPE_FLAC:      szInfo += "FLAC lossless codec";	break;
				case FMOD_SOUND_TYPE_FSB:       szInfo += "FMOD Sample Bank";	break;
				case FMOD_SOUND_TYPE_GCADPCM:   szInfo += "GameCube ADPCM";	break;
				case FMOD_SOUND_TYPE_IT:        szInfo += "Impulse Tracker";	break;
				case FMOD_SOUND_TYPE_MIDI:      szInfo += "MIDI";	break;
				case FMOD_SOUND_TYPE_MOD:       szInfo += "Protracker / Fasttracker MOD";	break;
				case FMOD_SOUND_TYPE_MPEG:      szInfo += "MP2/MP3 MPEG";	break;
				case FMOD_SOUND_TYPE_OGGVORBIS: szInfo += "Ogg vorbis";	break;
				case FMOD_SOUND_TYPE_PLAYLIST:  szInfo += "Information only from ASX/PLS/M3U/WAX playlists";	break;
				case FMOD_SOUND_TYPE_RAW:       szInfo += "Raw PCM data";	break;
				case FMOD_SOUND_TYPE_S3M:       szInfo += "ScreamTracker 3";	break;
				case FMOD_SOUND_TYPE_SF2:       szInfo += "Sound font 2 format";	break;
				case FMOD_SOUND_TYPE_USER:      szInfo += "User created sound";	break;
				case FMOD_SOUND_TYPE_WAV:       szInfo += "Microsoft WAV";	break;
				case FMOD_SOUND_TYPE_XM:        szInfo += "FastTracker 2 XM";	break;
				case FMOD_SOUND_TYPE_XMA:       szInfo += "Xbox360 XMA";	break;
				case FMOD_SOUND_TYPE_VAG:       szInfo += "PlayStation 2 / PlayStation Portable adpcm VAG format";	break;
			}
			char szTemp[64];
			sprintf_s( szTemp, "\nChannel : %d", nChannel );
			szInfo += szTemp;

			sprintf_s( szTemp, "\nBitrate : %d bit", nBits );
			szInfo += szTemp;

			int nMinute = nLength / 60000;
			nLength %= 60000;
			int nSec = nLength / 1000;
			nLength %= 1000;
			int nMil = (int)( nLength * 0.1f );
			sprintf_s( szTemp, "\nLength : %02d:%02d:%02d", nMinute, nSec, nMil );
			szInfo += szTemp;
			m_pVecSoundList[i]->szInfo = szInfo;
		}

		m_pVecSoundList[i]->cPrevFlag = 1;

		Invalidate();

		if( !bCheckFlag && bSearchFirst ) {
			bCheckFlag = true;
			nStartOffset = 0;
			nEndOffset = (int)m_pVecSoundList.size();
			i = -1;
			continue;
		}

		return false;
	}
	return true;
}

void CSoundManagerPaneView::ResetInfoList()
{
	CSoundIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
	}

	m_MapSoundList.clear();
	SAFE_DELETE_PVEC( m_pVecSoundList );

	m_nThreadStatus = 0;
	m_nSelectIndex = -1;
	m_nSelectFolderOffsetStart = -1;
	m_nSelectFolderOffsetEnd = -1;
	m_ScrollPos = m_ScrollSize = CSize( 0, 0 );

	RefreshFullName();
}

CString CSoundManagerPaneView::GetCurrentDir()
{
	CString szFolder;
	if( m_szCurrentFolder.IsEmpty() ) {
		szFolder.Format( "%s\\Resource\\Sound", CFileServer::GetInstance().GetWorkingFolder() );
	}
	else szFolder = m_szCurrentFolder;

	return szFolder;
}

LRESULT CSoundManagerPaneView::OnChangeFolderFromExplorer( WPARAM wParam, LPARAM lParam )
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
	CSoundIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
		if( _stricmp( GetCurrentDir(), m_pVecSoundList[i]->szFolder ) == NULL ) {
			if( m_nSelectFolderOffsetStart == -1 ) m_nSelectFolderOffsetStart = i;
			m_nSelectFolderOffsetEnd = i;
			CSoundIconItem::AddItem( m_pVecSoundList[i], i );
		}
	}
	if( m_nSelectFolderOffsetEnd != -1 ) m_nSelectFolderOffsetEnd++;
	m_ScrollPos = CSize( 0, 0 );
	ResetScroll();
	Invalidate();

	return S_OK;
}

LRESULT CSoundManagerPaneView::OnChangeFolderNameFromExplorer( WPARAM wParam, LPARAM lParam )
{
	m_szCurrentFolder = (char*)wParam;
	CString szSourceName = (char*)lParam;
	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
		if( m_pVecSoundList[i]->szFolder == szSourceName ) {
			m_pVecSoundList[i]->szFolder = m_szCurrentFolder;
		}
	}
	return S_OK;
}

LRESULT CSoundManagerPaneView::OnMoveFileFromExplorer( WPARAM wParam, LPARAM lParam )
{
	char szFullPath[512] = { 0, };
	char szName[512] = { 0, };
	CString szBeforName = (char*)wParam;
	CString szAfterName = (char*)lParam;
	_GetPath( szFullPath, _countof(szFullPath), szBeforName );
	szFullPath[ strlen(szFullPath) - 1 ] = 0;
	_GetFullFileName( szName, _countof(szName), szBeforName );
	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
		if( _stricmp( szFullPath, m_pVecSoundList[i]->szFolder ) == NULL &&
			_stricmp( szName, m_pVecSoundList[i]->szFileName ) == NULL ) {
				memset( szFullPath, 0, sizeof(szFullPath) );
				_GetPath( szFullPath, _countof(szFullPath), szAfterName );
				szFullPath[ strlen(szFullPath) - 1 ] = 0;
				m_pVecSoundList[i]->szFolder = szFullPath;
		}
	}
	return S_OK;
}

void CSoundManagerPaneView::OnDestroy()
{
	__super::OnDestroy();

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

bool CSoundManagerPaneView::CalcAccompanimentFile( const char *szSoundName, std::vector<CString> &szVecList )
{
	FILE *fp = NULL;
	// 자기자신도 리스트에 ADd
	szVecList.push_back( szSoundName );

	char szBuf[512] = { 0, };
	CString szPath;

	_GetPath( szBuf, _countof(szBuf), szSoundName );
	szPath = szBuf;

	// 스킨이름과 같은 이름의 ani 복사해준다. 
	CString szAniName;
	memset( szBuf, 0, sizeof(szBuf) );
	_GetFileName( szBuf, _countof(szBuf), szSoundName );
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
	_GetFileName( szBuf, _countof(szBuf), szSoundName );
	szActName.Format( "%s%s.act", szPath, szBuf );
	fopen_s( &fp, szActName, "rb" );
	if( fp ) {
		szVecList.push_back( szActName );
		fclose(fp);
		fp = NULL;
	}



	// 스킨에서 사용하는 msh,texture 파일 복사해준다.

	EtObjectHandle Handle = EternityEngine::CreateStaticObject( szSoundName );
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

void CSoundManagerPaneView::OnSoundadd()
{
	CString szFileList;
	TCHAR szFilter[] = _T( "All Support File (*.wav;*.mp3;*.ogg;*.wmv;*.wma)|*.wav;*.mp3;*.ogg;*.wmv;*.wma|Wave File (*.wav)|*.wav|MPEG Audio Layer-3 File (*.mp3)|*.mp3|Ogg voris File (*.ogg)|*.ogg|Windows Media Video File (*.wmv)|*.wmv|Windows Media Audio File (*.wma)|*.wma|All Files (*.*)|*.*||" );

	CSoundPrevDlg dlg(TRUE, _T("wav;mp3;ogg;wmv;wma"), _T("*.wav;*.mp3;*.ogg;*.wmv;*.wma"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_ENABLESIZING, szFilter);

	dlg.GetOFN().lpstrFile = szFileList.GetBuffer(100000);
	dlg.GetOFN().nMaxFile = 100000;

	if( dlg.DoModal() == IDOK ) {
		std::vector<CString> szVecFileList;
		ParseFileList( szFileList, "wav", szVecFileList );
		ParseFileList( szFileList, "mp3", szVecFileList );
		ParseFileList( szFileList, "ogg", szVecFileList );
		ParseFileList( szFileList, "wmv", szVecFileList );
		ParseFileList( szFileList, "wma", szVecFileList );
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
				if( CheckAnyoneUseSound( szTemp[0] ) == true ) {
					if( MessageBox( "Sound 다른 섹터 어딘가에서 사용중입니다.\n그래두 덮어쓸래?", "질문", MB_YESNO ) == IDNO ) continue;
				}

				for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
					if( _stricmp( szTemp[0], m_pVecSoundList[i]->szFileName ) == NULL ) {
						// 같은 폴더에 있는 파일일 경우랑 폴더가 틀릴 경우랑 따로 처리해줘야한다.
						CString szExistPath = m_pVecSoundList[i]->szFolder;
						if( _stricmp( szExistPath, GetCurrentDir() ) == NULL ) {
							m_pVecSoundList[i]->cPrevFlag = 0;
						}
						else {
							bExist = false;
							CString szExistFullName;
							szExistFullName.Format( "%s\\%s", szExistPath, szTemp[0] );
							DeleteFile( szExistFullName );

							DeleteInSoundStruct( i );
						}
						break;
					}
				}
			}
			char szFileName[512] = { 0, };
			CString szErrorFileName;
			CString szErrorStr;

			_GetFullFileName( szFileName, _countof(szFileName), szVecFileList[k] );
			szFullName.Format( "%s\\%s", GetCurrentDir(), szFileName );

			BOOL bResult = CopyFile( szVecFileList[k], szFullName, !bExist );

			if( bResult == false ) {
				MessageBox( "Sound 복사할 수 없어요!!", "Error", MB_OK );
				continue;
			}

			if( bExist == TRUE ) continue;	
			SetFileAttributes( szFullName, FILE_ATTRIBUTE_NORMAL );

			_GetFullFileName( szFileName, _countof(szFileName), szVecFileList[k] );

			SoundStruct *pStruct;
			pStruct = new SoundStruct;
			pStruct->szFileName = szFileName;
			pStruct->szFolder = GetCurrentDir();
			pStruct->cPrevFlag = 0;

			m_pVecSoundList.push_back( pStruct );
			m_MapSoundList.insert( make_pair( pStruct->szFileName, pStruct->szFolder ) );
			CSoundIconItem::AddItem( pStruct, (int)m_pVecSoundList.size() - 1 );
		}
		m_nThreadStatus = 1;
		ResetScroll();
		Invalidate();
	}
	else szFileList.ReleaseBuffer();
}

void CSoundManagerPaneView::OnSoundremove()
{
	if( m_nSelectIndex == -1 ) return;
	CFileServer::GetInstance().UpdateUseSoundList();
	if( CheckAnyoneUseSound( m_pVecSoundList[m_nSelectIndex]->szFileName ) == true ) {
		MessageBox( "삭제하려고 한 스킨이 다른 섹터에서 사용중입니다.\n", "에러", MB_OK );
		return;
	}

	CString szFullName;
	szFullName.Format( "%s\\%s", GetCurrentDir(), m_pVecSoundList[m_nSelectIndex]->szFileName );

	CString szErrorFileName;

	if( DeleteFile( szFullName ) == false ) {
		szErrorFileName = "다음 파일을 삭재하지 못했습니다.\n";
		szErrorFileName = szFullName;
		MessageBox( szErrorFileName, "에러", MB_OK );
	}

	// 지울때는 쓰레드 잠시 종료시키구.. 기존 쓰레드 Process 인덱스 기억해노쿠.. 지운 후 다시 쓰레드 가동
	DeleteInSoundStruct( m_nSelectIndex );
	m_nSelectIndex = -1;

	CSoundIconItem::DeleteAllItems();
	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
		if( _stricmp( GetCurrentDir(), m_pVecSoundList[i]->szFolder ) == NULL )
			CSoundIconItem::AddItem( m_pVecSoundList[i], i );
	}

	ResetScroll();
	Invalidate();
}

void CSoundManagerPaneView::OnSoundMode()
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_SOUNDMODE, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	if( m_nViewFileType == pCombo->GetCurSel() ) return;
	m_nViewFileType = pCombo->GetCurSel();
	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CSoundManagerPaneView::OnSoundRefresh()
{
	CWnd *pWnd = GetPaneWnd( MNG_EXPLORER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 3 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}


void CSoundManagerPaneView::OnUpdateSoundMode(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CSoundManagerPaneView::OnUpdateSoundAdd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}

void CSoundManagerPaneView::OnUpdateSoundRemove(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
	if( m_nSelectIndex == -1 ) pCmdUI->Enable( false );
}

void CSoundManagerPaneView::OnUpdateSoundRefresh(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( CFileServer::GetInstance().IsConnect() );
}


BOOL CSoundManagerPaneView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void CSoundManagerPaneView::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
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
					szTemp = m_pVecSoundList[m_nSelectIndex]->szFolder + "\\" + m_pVecSoundList[m_nSelectIndex]->szFileName;
					pDropWnd->SendMessage( UM_SOUNDPANE_DROPITEM, (WPARAM)szTemp.GetBuffer(), lPos );
				}
			}
		}
	}
	if( nReceiverState & IR_MOUSE_MOVE ) {
		if( m_bDragClickFlag == true && m_bDragBegin == false && m_bLBDown == true && IsInMouseRect( this ) && m_nSelectIndex != -1 && m_pDragImage == NULL && m_pVecSoundList[m_nSelectIndex]->cPrevFlag != -1 ) {
			CPoint p;
			GetCursorPos( &p );
			m_bDragBegin = true;

			SetCapture();
			m_pDragImage = new CImageList;
			m_pDragImage->Create( 64, 64, ILC_COLOR32|ILC_MASK, 1, 1 );
//			if( m_pVecSoundList[m_nSelectIndex]->pImage ) 
				m_pDragImage->Add( m_pBackImagedcBitmap, RGB( 0, 0, 0 ) );

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
				pDropWnd->SendMessage( UM_SOUNDPANE_DROPMOVEITEM, lPos );
			}
		}
		m_bDragClickFlag = false;
	}
}

FMOD_RESULT F_CALLBACK SoundManagerPlayCallback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if( type == FMOD_CHANNEL_CALLBACKTYPE_END ) {
		int nLoopCount;
		FMOD_Channel_GetLoopCount( channel, &nLoopCount );

		std::vector<CSoundIconItem> *pvecList = &CSoundIconItem::s_vecList;
//		CString szStr = (char*)( (int*)&command);
//		if( command < 0 || command >= (int)pvecList->size() ) return FMOD_OK;
		/*
		CWnd *pWnd = GetPaneWnd( SOUNDMNG_PANE );
		if( !pWnd ) return FMOD_OK;
		CSoundIconItem *pIcon = NULL;// = &(*pvecList)[command];
		for( DWORD i=0; i<pvecList->size(); i++ ) {
			if( (*pvecList)[i].m_nIndex == command ) {
				pIcon = &(*pvecList)[i];
				break;
			}
		}
		if( pIcon == NULL ) return FMOD_OK;
		if( !pIcon->GetStruct()->pChannel ) return FMOD_OK;
		if( pIcon->GetStruct()->pChannel->GetChannel() != ((FMOD::Channel*)channel) ) return FMOD_OK;
		pIcon->GetStruct()->pChannel->GetGroup()->RemoveChannel( pIcon->GetStruct()->pChannel->GetIndex() );
		pIcon->GetStruct()->pChannel = NULL;
		*/
		void *pUserData;
		((FMOD::Channel *)channel)->getUserData( &pUserData );
//		int nIndex = *(int*)pUserData;
//		OutputDebug( "LoopCount : %d, - %d\n", nLoopCount, nIndex );

		CWnd *pWnd = GetPaneWnd( SOUNDMNG_PANE );
		if( !pWnd ) return FMOD_OK;
		SoundStruct *pStruct = dynamic_cast<SoundStruct*>(static_cast<SoundStruct*>(pUserData));//((CSoundManagerPaneView*)pWnd)->FindSoundStructFromIndex( nIndex );
		if( !pStruct ) return FMOD_OK;
		if( !pStruct->pChannel ) return FMOD_OK;
		pStruct->pChannel->SetChannel( NULL );
		pStruct->pChannel->GetGroup()->RemoveChannel( pStruct->pChannel->GetIndex() );
		pStruct->pChannel = NULL;


//		CWnd *pWnd = GetPaneWnd( SOUNDMNG_PANE );
		if( pWnd ) pWnd->Invalidate();
	}
	return FMOD_OK;
}

SoundStruct *CSoundManagerPaneView::FindSoundStructFromIndex( int nIndex )
{
	if( nIndex < 0 || nIndex >= (int)m_pVecSoundList.size() ) return NULL;
	return m_pVecSoundList[nIndex];
}

void CSoundManagerPaneView::OnLButtonDown(UINT nFlags, CPoint point)
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
	std::vector<CSoundIconItem> *pvecList = &CSoundIconItem::s_vecList;
	CRect rcIconRect;
	m_nSelectIndex = -1;
	CPoint p = point;
	p.x += m_ScrollPos.cx;
	p.y += m_ScrollPos.cy;
	m_bDragClickFlag = false;
	int nIconIndex = -1;
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
			nIconIndex = i;
			m_bDragClickFlag = true;
			break;
		}
	}
	if( m_nSelectIndex != -1 ) {
		CSoundIconItem *pIcon = &(*pvecList)[nIconIndex];
		rcRect = pIcon->GetSoundButtonRect( &rcIconRect );
		if( p.x > rcRect.left && p.x < rcRect.right && p.y > rcRect.top && p.y < rcRect.bottom ) {
			if( !pIcon->GetStruct()->pChannel ) {
				CString szFullName;
				szFullName.Format( "%s\\%s", pIcon->GetStruct()->szFolder, pIcon->GetStruct()->szFileName );

				CEtSoundEngine *pSystem = CRenderBase::GetInstance().GetSoundEngine();
				int nIndex = pSystem->LoadSound( szFullName.GetBuffer(), false, false );
				if( nIndex != -1 ) {
					pIcon->GetStruct()->pChannel = pSystem->PlaySound( "SoundManager", nIndex );
					pIcon->GetStruct()->pChannel->SetCallback( SoundManagerPlayCallback, (void*)pIcon->GetStruct() );
				}
			}
			else {
				if( pIcon->GetStruct()->pChannel->IsPause() )
					pIcon->GetStruct()->pChannel->Resume();
				else pIcon->GetStruct()->pChannel->Pause();
			}
		}
	}
	Invalidate();
	RefreshFullName();

	__super::OnLButtonDown(nFlags, point);
}

LRESULT CSoundManagerPaneView::OnGetPreviewImage( WPARAM wParam, LPARAM lParam )
{
	char *szFileName = (char *)wParam;
	if( szFileName == NULL ) return 0;

	for( DWORD i=0; i<m_pVecSoundList.size(); i++ ) {
		if( _stricmp( m_pVecSoundList[i]->szFileName, szFileName ) == NULL && m_pVecSoundList[i]->cPrevFlag == 1 ) {
			return (LRESULT)m_pVecSoundList[i];
		}
	}
	return 0;
}

bool CSoundManagerPaneView::CheckAnyoneUseSound( CString szSoundName )
{
	return CFileServer::GetInstance().IsUseSound( szSoundName );
}

void CSoundManagerPaneView::DeleteInSoundStruct( DWORD dwIndex )
{
	bool bThread = false;
	DWORD dwPrevThreadIndex = -1;
	if( s_hThreadHandle ) {
		s_bExitThread = true;
		while(1) {
			Sleep(1);
			if( m_nThreadStatus == -1 ) break;
		}
//		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		dwPrevThreadIndex = m_dwThreadIndex;
		m_dwThreadIndex = 0;
		bThread = true;
	}

	m_MapSoundList.erase( m_pVecSoundList[dwIndex]->szFileName );
	SAFE_DELETE( m_pVecSoundList[dwIndex] );
	m_pVecSoundList.erase( m_pVecSoundList.begin() + dwIndex );

	if( bThread ) {
		m_nThreadStatus = dwPrevThreadIndex;
		s_bExitThread = false;
		s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
	}
}

void CSoundManagerPaneView::RefreshFullName()
{
	CXTPControlEdit *pEdit = (CXTPControlEdit *)m_wndToolBar2.GetControls()->FindControl( xtpControlEdit, ID_SKINNAME, TRUE, FALSE );

	CString szFullName;

	if( m_nSelectIndex != -1 ) {
		CString szTemp = m_pVecSoundList[m_nSelectIndex]->szFolder + "\\" + m_pVecSoundList[m_nSelectIndex]->szFileName;
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

void CSoundManagerPaneView::KillThread()
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