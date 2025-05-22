// DecalListPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "DecalListPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "UserMessage.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "BrushRenderView.h"
#include "TEtWorldDecal.h"
#include "DummyView.h"
#include <map>
using namespace std;

// CDecalListPaneView

IMPLEMENT_DYNCREATE(CDecalListPaneView, CFormView)

CDecalListPaneView::CDecalListPaneView()
	: CFormView(CDecalListPaneView::IDD)
{
	m_bActivate = false;
	m_nSortedCol = 0;
	m_bAscending = true;
	m_bSorting = false;
}

CDecalListPaneView::~CDecalListPaneView()
{
}

void CDecalListPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CDecalListPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_ENABLE_PANE_VIEW, OnEnablePane )
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CDecalListPaneView::OnHdnItemclickList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CDecalListPaneView::OnLvnItemchangedList1)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, &CDecalListPaneView::OnLvnKeydownList1)
END_MESSAGE_MAP()


// CDecalListPaneView diagnostics

#ifdef _DEBUG
void CDecalListPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDecalListPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CDecalListPaneView message handlers

void CDecalListPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_ListCtrl ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_ListCtrl.MoveWindow( &rcRect );
	}
}

void CDecalListPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 150 );
	m_ListCtrl.InsertColumn( 1, _T("Position"), LVCFMT_LEFT, 30 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	HWND hWndHeader = m_ListCtrl.GetDlgItem(0)->GetSafeHwnd();
	m_header.SubclassWindow(hWndHeader);
}

LRESULT CDecalListPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( wParam == -1 ) {
		m_ListCtrl.DeleteAllItems();
		return S_OK;
	}
	else if( wParam == 0 ) {
		m_ListCtrl.DeleteAllItems();
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		if( Sector == -1 ) return S_OK;

		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector == NULL ) return S_OK;

		int nIndex;
		char szPos[256];
		for( DWORD i=0; i<pSector->GetDecalCount(); i++ ) {
			CEtWorldDecal *pDecal = pSector->GetDecalFromIndex(i);
			nIndex = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pDecal->GetTextureName() );

			sprintf_s( szPos, "%f, %f", pDecal->GetPosition().x, pDecal->GetPosition().y );
			m_ListCtrl.SetItemText( nIndex, 1, szPos );
		}
	}
	else if( wParam == 1 ) {
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		if( Sector == -1 ) return S_OK;

		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector == NULL ) return S_OK;

		for( int i=0; i<m_ListCtrl.GetItemCount(); i++ ) {
			m_ListCtrl.SetItemState( i, 0, LVIS_SELECTED| LVIS_FOCUSED );
			CString szStr = m_ListCtrl.GetItemText( i, 1 );

			EtVector2 vPos;
			for( int j=0; j<2; j++ ) {
				const char *pTempStr = _GetSubStrByCount( 0, szStr.GetBuffer(), ',' );
				vPos.x = (float)atof( pTempStr );

				pTempStr = _GetSubStrByCount( 1, szStr.GetBuffer(), ',' );
				vPos.y = (float)atof( pTempStr );
			}
			if( CGlobalValue::GetInstance().m_pSelectDecal ) {
				float fDist = EtVec2Length( &EtVector2( CGlobalValue::GetInstance().m_pSelectDecal->GetPosition() - vPos ) );
				if( fDist < 0.01f ) {
					m_ListCtrl.SetSelectionMark( i );
					m_ListCtrl.SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
					m_ListCtrl.EnsureVisible( i, FALSE );
				}
			}
		}
	}

	return S_OK;
}

BOOL CDecalListPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return CFormView::OnEraseBkgnd(pDC);
}


LRESULT CDecalListPaneView::OnEnablePane( WPARAM wParam, LPARAM lParam )
{
	m_ListCtrl.EnableWindow( (BOOL)wParam );
	return S_OK;
}

void CDecalListPaneView::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_bSorting = true;
	if ( phdr->iItem == m_nSortedCol )
		SortColumn( phdr->iItem, !m_bAscending );
	else {
		SortColumn( phdr->iItem, m_header.GetAscending() ? true : false );
	}
	*pResult = 0;
	m_bSorting = false;
}


void CDecalListPaneView::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;

	// set sort image for header and sort column.
	m_ListCtrl.SetSortImage(m_nSortedCol, m_bAscending);

	CXTSortClass csc(&m_ListCtrl, m_nSortedCol);
	csc.Sort(m_bAscending, ( m_nSortedCol == 2 ) ? xtSortInt : xtSortString);
}


void CDecalListPaneView::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_bSorting ) return;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;


	if( GetFocus() == this || GetFocus() == &m_ListCtrl ) {
		SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
		if( Index == -1 ) return;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
		if( pSector == NULL ) return;

		for( DWORD i=0; i<pSector->GetDecalCount(); i++ )
			((CTEtWorldDecal*)pSector->GetDecalFromIndex(i))->SetSelect( false );
		CGlobalValue::GetInstance().m_pSelectDecal = NULL;

		int nSelectCount = m_ListCtrl.GetSelectedCount();
		int nIndex;
		CString szStr;
		POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
		for( int i=0; i<nSelectCount; i++ ) {
			nIndex = m_ListCtrl.GetNextSelectedItem(p);
			szStr = m_ListCtrl.GetItemText( nIndex, 1 );

			EtVector2 vPos;
			for( int j=0; j<2; j++ ) {
				const char *pTempStr = _GetSubStrByCount( 0, szStr.GetBuffer(), ',' );
				vPos.x = (float)atof( pTempStr );

				pTempStr = _GetSubStrByCount( 1, szStr.GetBuffer(), ',' );
				vPos.y = (float)atof( pTempStr );
			}


			for( DWORD j=0; j<pSector->GetDecalCount(); j++ ) {
				// 일치하지 않을수 있으므로 대충 비슷하면 체크하는식으로 합시다!
				float fDist = EtVec2Length( &EtVector2( pSector->GetDecalFromIndex(j)->GetPosition() - vPos ) );
				if( fDist < 0.01f ) {
					CGlobalValue::GetInstance().m_pSelectDecal = (CTEtWorldDecal*)pSector->GetDecalFromIndex(j);
					CGlobalValue::GetInstance().m_pSelectDecal->SetSelect( true );
					break;
				}
			}
			break;
		}

		CWnd *pWnd = GetPaneWnd( LAYER_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

		CGlobalValue::GetInstance().RefreshRender();
	}
}

void CDecalListPaneView::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::BRUSH && CGlobalValue::GetInstance().IsCheckOutMe() && pLVKeyDow->wVKey == VK_DELETE ) {
		CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
		if( pView && pView->GetDummyView()->GetTabViewStruct( CGlobalValue::BRUSH ) ) {
			CBrushRenderView *pBrushView = (CBrushRenderView *)pView->GetDummyView()->GetTabViewStruct( CGlobalValue::BRUSH )->pView;
			if( pBrushView ) pBrushView->DeleteDecal();
		}
	}
}
