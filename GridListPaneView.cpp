// GridListPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "GridListPaneView.h"
#include "TEtWorld.h"
#include "UserMessage.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "DummyView.h"

#include "resource.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"

#include "PaneDefine.h"
#include "MainFrm.h"


// CGridListPaneView

IMPLEMENT_DYNCREATE(CGridListPaneView, CFormView)

CGridListPaneView::CGridListPaneView()
	: CFormView(CGridListPaneView::IDD)
{
	m_bActivate = false;
	m_nSortedCol = 0;
	m_bAscending = true;
	m_bSorting = false;
}

CGridListPaneView::~CGridListPaneView()
{
}

void CGridListPaneView::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGridListPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_GRIDLISTPANE_SELECT, OnSelectGrid )
	ON_WM_ERASEBKGND()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CGridListPaneView::OnLvnItemchangedList1)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CGridListPaneView::OnHdnItemclickList1)
END_MESSAGE_MAP()


// CGridListPaneView diagnostics

#ifdef _DEBUG
void CGridListPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CGridListPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CGridListPaneView message handlers


// CGridListPaneView 메시지 처리기입니다.

void CGridListPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_ListCtrl ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_ListCtrl.MoveWindow( &rcRect );
	}
}

void CGridListPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 100 );
	m_ListCtrl.InsertColumn( 1, _T("Sector Count"), LVCFMT_LEFT, 40 );
	m_ListCtrl.InsertColumn( 2, _T("Sector List"), LVCFMT_LEFT, 75 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	HWND hWndHeader = m_ListCtrl.GetDlgItem(0)->GetSafeHwnd();
	m_header.SubclassWindow(hWndHeader);
}

LRESULT CGridListPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( !CTEtWorld::IsActive() || wParam == -1 ) {
		m_ListCtrl.DeleteAllItems();
		return S_OK;
	}

	AddGridList( (CTEtWorldGrid*)CTEtWorld::GetInstance().GetGrid() );
	Invalidate();
	return S_OK;
}
	
BOOL CGridListPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return FALSE;
	//	return CFormView::OnEraseBkgnd(pDC);
}


void CGridListPaneView::AddGridList( CTEtWorldGrid *pGrid )
{
	if( pGrid == NULL ) return;
	if( _stricmp( pGrid->GetName(), "Root" ) ) {
		CString szStr;
		int nItemID;
		nItemID = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pGrid->GetName() );
		szStr.Format( "%d", pGrid->GetGridX() * pGrid->GetGridY() );
		m_ListCtrl.SetItemText( nItemID, 1, szStr );
		szStr.Empty();
		for( DWORD j=0; j<pGrid->GetGridY(); j++ ) {
			for( DWORD i=0; i<pGrid->GetGridX(); i++ ) {
				CTEtWorldSector *pSector = (CTEtWorldSector *)pGrid->GetSector( SectorIndex( i, j ) );
				if( !pSector ) continue;
				szStr += pSector->GetDescription();
				szStr += "; ";
			}
		}
		m_ListCtrl.SetItemText( nItemID, 2, szStr );
	}

	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		AddGridList( (CTEtWorldGrid*)pGrid->GetChildFromIndex(i) );
	}
}

void CGridListPaneView::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if( m_bSorting ) return;

	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) return;
	// TODO: Add your control notification handler code here
	if( GetFocus() == this || GetFocus() == &m_ListCtrl ) {
		CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
		pView->EnableTabView( -1, false );
		pView->EnableTabView( CGlobalValue::GRID, true );

		int nSelectCount = m_ListCtrl.GetSelectedCount();
		if( nSelectCount == 0 ) return;

		POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
		for( int i=0; i<nSelectCount; i++ ) {
			int nIndex = m_ListCtrl.GetNextSelectedItem(p);
			CString szStr = m_ListCtrl.GetItemText( nIndex, 0 );

			CTEtWorldGrid *pGrid = (CTEtWorldGrid *)CTEtWorld::GetInstance().FindGrid( szStr );
			if( !pGrid ) break;
			int nCenterX = pGrid->GetGridX() / 2;
			int nCenterY = pGrid->GetGridY() / 2;
			if( nCenterX < 1 ) nCenterX = 1;
			if( nCenterY < 1 ) nCenterY = 1;
			EtVector3 vMin, vMax;
			CTEtWorld::GetInstance().GetGridRect( pGrid, nCenterX, nCenterY, &vMin, &vMax );

			CDummyView::TabViewStruct *pStruct = ((CEtWorldPainterView*)CGlobalValue::GetInstance().m_pParentView)->GetDummyView()->GetTabViewStruct( CGlobalValue::GRID );
			if( !pStruct ) break;
			if( !pStruct->pView ) break;

			CGlobalValue::GetInstance().m_szSelectGrid = szStr;
			CGlobalValue::GetInstance().m_SelectGrid = SectorIndex( 0, 0 );

			EtVector3 vResult = vMin + ( vMax - vMin ) / 2.f;
			pStruct->pView->SendMessage( UM_GRIDPANE_CHANGECAMERA, (WPARAM)&vResult );

			CWnd *pWnd = GetPaneWnd( GRID_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

			pWnd = GetPaneWnd( GRIDDESC_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

			pView->EnableTabView( -1, true );

			break;
		}
	}


	*pResult = 0;
}

LRESULT CGridListPaneView::OnSelectGrid( WPARAM wParam, LPARAM lParam )
{
	CString szStr = (char*)wParam;
	LVFINDINFO Info;
	Info.flags = LVFI_PARTIAL | LVFI_STRING;
	Info.psz = szStr.GetBuffer();
	int nIndex = m_ListCtrl.FindItem( &Info );
	if( nIndex == -1 ) return S_OK;
	m_ListCtrl.SetSelectionMark( nIndex );
	m_ListCtrl.SetItemState( nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_ListCtrl.EnsureVisible( nIndex, FALSE );
	return S_OK;
}

void CGridListPaneView::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bSorting = true;
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	if ( phdr->iItem == m_nSortedCol )
		SortColumn( phdr->iItem, !m_bAscending );
	else {
		SortColumn( phdr->iItem, m_header.GetAscending() ? true : false );
	}
	*pResult = 0;
	m_bSorting = false;
}


void CGridListPaneView::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;

	// set sort image for header and sort column.
	m_ListCtrl.SetSortImage(m_nSortedCol, m_bAscending);

	CXTSortClass csc(&m_ListCtrl, m_nSortedCol);
	csc.Sort(m_bAscending, ( m_nSortedCol == 1 ) ? xtSortInt : xtSortString);
}
