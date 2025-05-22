// SoundListPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundListPaneView.h"
#include "UserMessage.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "TEtWorldSound.h"
#include "TEtWorldSoundEnvi.h"

// CSoundListPaneView

IMPLEMENT_DYNCREATE(CSoundListPaneView, CFormView)

CSoundListPaneView::CSoundListPaneView()
	: CFormView(CSoundListPaneView::IDD)
{
	m_bActivate = false;

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_SOUNDTREEICON );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 4, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );
}

CSoundListPaneView::~CSoundListPaneView()
{
}

void CSoundListPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
}

BEGIN_MESSAGE_MAP(CSoundListPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CSoundListPaneView::OnTvnSelchangedTree1)
	ON_MESSAGE( UM_SOUNDPANE_DROPITEM, OnDropItem )
	ON_MESSAGE( UM_SOUNDPANE_DROPMOVEITEM, OnDropMoveItem )
	ON_MESSAGE( UM_SOUNDLIST_PANE_ADDSOUNDENVI, OnAddSoundEnvi )
	ON_MESSAGE( UM_SOUNDLIST_PANE_SELCHANGED, OnSelectChange )
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, &CSoundListPaneView::OnTvnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, &CSoundListPaneView::OnTvnEndlabeleditTree1)
END_MESSAGE_MAP()


// CSoundListPaneView diagnostics

#ifdef _DEBUG
void CSoundListPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSoundListPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSoundListPaneView message handlers

void CSoundListPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;
}

BOOL CSoundListPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

void CSoundListPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_TreeCtrl ) {
		m_TreeCtrl.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CSoundListPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_TreeCtrl.DeleteAllItems();
	m_TreeCtrl.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_TreeCtrl.EnableMultiSelect( true );

	m_hRootItem = m_TreeCtrl.InsertItem( "Sound", 0, 1 );
	m_hBGMItem = m_TreeCtrl.InsertItem( "BGM", 2, 3, m_hRootItem );
	m_hEnviItem = m_TreeCtrl.InsertItem( "Environment", 0, 1, m_hRootItem );
	m_hPropItem = m_TreeCtrl.InsertItem( "Prop Sound List", 0, 1, m_hRootItem );

	m_TreeCtrl.SelectItem( m_hRootItem );
	m_TreeCtrl.Expand( m_hRootItem, TVE_EXPAND );


	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return S_OK;

	CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();

	for( DWORD i=0; i<pSound->GetEnviCount(); i++ ) {
		CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)pSound->GetEnviFromIndex(i);
		m_TreeCtrl.InsertItem( pEnvi->GetName(), 2, 3, m_hEnviItem );
	}
	m_TreeCtrl.Expand( m_hEnviItem, TVE_EXPAND );
	// Prop 읽어와서 Sound 객체가 박혀있는 놈들 셋팅해준다.

	return S_OK;
}

void CSoundListPaneView::CalcSelectItem()
{
	m_SelectItemList.RemoveAll();
	m_TreeCtrl.GetSelectedList( m_SelectItemList );
}


void CSoundListPaneView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	CalcSelectItem();
	CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
	if( pWnd ) {
		if( m_SelectItemList.GetCount() == 0 ) {
			pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		}
		else if( m_SelectItemList.GetCount() == 1 ) {
			int nFlag = 0;
			CGlobalValue::GetInstance().m_pSelectSound = NULL;

			if( m_SelectItemList.GetHead() == m_hRootItem ) nFlag = 1;
			else if( m_SelectItemList.GetHead() == m_hBGMItem ) nFlag = 2;
			else if( m_SelectItemList.GetHead() == m_hEnviItem ) nFlag = 3;
			else if( m_SelectItemList.GetHead() == m_hPropItem ) nFlag = 4;
			else {
				CString szStr = m_TreeCtrl.GetItemText( m_SelectItemList.GetHead() );

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector ) {
						CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
						if( pSound ) {
							CEtWorldSoundEnvi *pEnvi = pSound->GetEnviFromName( szStr );
							if( pEnvi ) {
								CGlobalValue::GetInstance().m_pSelectSound = pEnvi;
								nFlag = 5;
							}
						}
					}
				}
			}
			CGlobalValue::GetInstance().RefreshRender();
			pWnd->SendMessage( UM_REFRESH_PANE_VIEW, nFlag );
		}
		else {
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		}
	}


	*pResult = 0;
}


LRESULT CSoundListPaneView::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)lParam >> 16;
	p.y = (long)( lParam << 16 ) >> 16;

	char *szFileName = (char *)wParam;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return S_OK;
	CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
	if( !pSound ) return S_OK;

	char szTemp[_MAX_PATH];
	_GetFullFileName( szTemp, _countof(szTemp), szFileName );

	CRect rcRect;
	m_TreeCtrl.GetItemRect( m_hBGMItem, rcRect, FALSE );

	if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {

		// SetBGM 하지말고 좀 구리지만 OnChangevalue 안에서 되게 이케 해주자.
		CUnionValueProperty Variable( CUnionValueProperty::String );
		Variable.SetVariable( szTemp );
		pSound->OnChangeValue( &Variable, 0 );
		CGlobalValue::GetInstance().SetModify();

		CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2 );
		return S_OK;
	}

	HTREEITEM hti;
	hti = m_TreeCtrl.GetChildItem( m_hEnviItem );
	while(hti) {
		m_TreeCtrl.GetItemRect( hti, rcRect, FALSE );
		if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {
			CString szItem = m_TreeCtrl.GetItemText( hti );

			CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)pSound->GetEnviFromName( szItem );
			if( pEnvi ) {
				CUnionValueProperty Variable( CUnionValueProperty::String );
				Variable.SetVariable( szTemp );
				pEnvi->OnChangeValue( &Variable, 0 );

				CGlobalValue::GetInstance().SetModify();

				CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 5 );
				return S_OK;
			}
		}

		hti = m_TreeCtrl.GetNextSiblingItem( hti );
	}


	return S_OK;
}

LRESULT CSoundListPaneView::OnDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	CPoint p;
	p.x = (long)wParam >> 16;
	p.y = (long)( wParam << 16 ) >> 16;

	CRect rcRect;

	m_TreeCtrl.GetItemRect( m_hBGMItem, rcRect, FALSE );
	if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {
		m_TreeCtrl.SelectItems( m_hBGMItem, m_hBGMItem );
		return S_OK;
	}
	else m_TreeCtrl.SelectItems( 0, 0 );

	HTREEITEM hti;
	hti = m_TreeCtrl.GetChildItem( m_hEnviItem );
	while(hti) {
		m_TreeCtrl.GetItemRect( hti, rcRect, FALSE );
		if( p.x >= rcRect.left && p.x <= rcRect.right && p.y >= rcRect.top && p.y <= rcRect.bottom ) {
			m_TreeCtrl.SelectItems( hti, hti );
			return S_OK;
		}

		hti = m_TreeCtrl.GetNextSiblingItem( hti );
	}

	return S_OK;
}

LRESULT CSoundListPaneView::OnAddSoundEnvi( WPARAM wParam, LPARAM lParam )
{
	char *szName = (char*)wParam;
	HTREEITEM hti = m_TreeCtrl.InsertItem( szName, 2, 3, m_hEnviItem );
	m_TreeCtrl.Expand( m_hEnviItem, TVE_EXPAND );
	m_TreeCtrl.SelectItems( hti, hti );
	return S_OK;
}
void CSoundListPaneView::OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_szPrevLabelString = pTVDispInfo->item.pszText;

	*pResult = 0;
}

void CSoundListPaneView::OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	if( pTVDispInfo->item.pszText == NULL ) return;
	if( m_hRootItem == pTVDispInfo->item.hItem ) return;
	if( m_hBGMItem == pTVDispInfo->item.hItem ) return;
	if( m_hEnviItem == pTVDispInfo->item.hItem ) return;
	if( m_hPropItem == pTVDispInfo->item.hItem ) return;
	if( m_TreeCtrl.GetParentItem( pTVDispInfo->item.hItem ) != m_hEnviItem ) return;

	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;
	CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
	if( !pSound ) return;
	if( pSound->GetEnviFromName( pTVDispInfo->item.pszText ) ) return;

	CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)pSound->GetEnviFromName( m_szPrevLabelString );
	if( !pEnvi ) return;

	pEnvi->SetName( pTVDispInfo->item.pszText );
	m_TreeCtrl.SetItemText( pTVDispInfo->item.hItem, pTVDispInfo->item.pszText );

	*pResult = 0;
}

LRESULT CSoundListPaneView::OnSelectChange( WPARAM wParam, LPARAM lParam )
{
	char *szName = (char*)wParam;
	if( szName == NULL ) return S_OK;
	HTREEITEM hti = m_TreeCtrl.FindItem( szName, FALSE, TRUE, FALSE, m_hEnviItem );
	if( !hti ) return S_OK;
	CalcSelectItem();
	if( m_SelectItemList.GetCount() == 1 && m_SelectItemList.GetHead() == hti ) {
		CWnd *pWnd = GetPaneWnd( SOUNDPROP_PANE );
		pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 5 );
	}
	else m_TreeCtrl.SelectItems( hti, hti );


	return S_OK;
}
