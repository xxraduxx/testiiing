// PropListPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropListPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "UserMessage.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "PropRenderView.h"
#include "DummyView.h"
#include "EtTrigger.h"
#include "PropSignalManager.h"
#include "PropSignalItem.h"
#include <map>
using namespace std;

// CPropListPaneView

IMPLEMENT_DYNCREATE(CPropListPaneView, CFormView)

CPropListPaneView::CPropListPaneView()
	: CFormView(CPropListPaneView::IDD)
{
	m_bActivate = false;
	m_nSortedCol = 0;
	m_bAscending = true;
	m_bSorting = false;
}

CPropListPaneView::~CPropListPaneView()
{
}

void CPropListPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CPropListPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CPropListPaneView::OnLvnItemchangedList1)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, &CPropListPaneView::OnLvnKeydownList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CPropListPaneView::OnNMDblclkList1)
	ON_MESSAGE( UM_ENABLE_PANE_VIEW, OnEnablePane )
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CPropListPaneView::OnHdnItemclickList1)
END_MESSAGE_MAP()


// CPropListPaneView diagnostics

#ifdef _DEBUG
void CPropListPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropListPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropListPaneView message handlers

void CPropListPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_ListCtrl ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_ListCtrl.MoveWindow( &rcRect );
	}
}

void CPropListPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 100 );
	m_ListCtrl.InsertColumn( 1, _T("Type"), LVCFMT_LEFT, 50 );
	m_ListCtrl.InsertColumn( 2, _T("ID"), LVCFMT_LEFT, 30 );
	m_ListCtrl.InsertColumn( 3, _T("Lock"), LVCFMT_LEFT, 30 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	HWND hWndHeader = m_ListCtrl.GetDlgItem(0)->GetSafeHwnd();
	m_header.SubclassWindow(hWndHeader);
}

LRESULT CPropListPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
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
		std::string szTypeStr;
		char szIndex[32];
		for( DWORD i=0; i<pSector->GetPropCount(); i++ ) {
			CEtWorldProp *pProp = pSector->GetPropFromIndex(i);
			nIndex = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pProp->GetPropName() );
/*
			switch( pProp->GetClassID() ) {
				case CTEtWorldProp::Unknown: szTypeStr = "Unknown"; break;
				case CTEtWorldProp::Static: szTypeStr = "Static"; break;
				case CTEtWorldProp::Action: szTypeStr = "Action"; break;
				case CTEtWorldProp::Broken: szTypeStr = "Broken"; break;
				case CTEtWorldProp::Trap: szTypeStr = "Trap"; break;
				case CTEtWorldProp::Light: szTypeStr = "Light"; break;
				case CTEtWorldProp::Chest: szTypeStr = "Chest"; break;
				case CTEtWorldProp::Operation: szTypeStr = "Operation"; break;
			}
*/
			CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex( pProp->GetClassID() );
			if( pItem ) szTypeStr = pItem->GetName().GetBuffer();
			m_ListCtrl.SetItemText( nIndex, 1, szTypeStr.c_str() );

			sprintf_s( szIndex, "%d", pProp->GetCreateUniqueID() );
			m_ListCtrl.SetItemText( nIndex, 2, szIndex );

			sprintf_s( szIndex, "%s", pProp->IsControlLock() ? "¡î" : "" );
			m_ListCtrl.SetItemText( nIndex, 3, szIndex );
		}
	}
	else if( wParam == 1 ) {
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		if( Sector == -1 ) return S_OK;

		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector == NULL ) return S_OK;

		std::map<int, int> szMapTemp;
		for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
			szMapTemp.insert( make_pair( CGlobalValue::GetInstance().m_pVecSelectPropList[i]->GetCreateUniqueID(), -1 ) );
		}

		for( int i=0; i<m_ListCtrl.GetItemCount(); i++ ) {
			m_ListCtrl.SetItemState( i, 0, LVIS_SELECTED| LVIS_FOCUSED );
			CString szStr = m_ListCtrl.GetItemText( i, 2 );
			std::map<int, int>::iterator it = szMapTemp.find( atoi( szStr ) );
			if( it != szMapTemp.end() ) {
				m_ListCtrl.SetSelectionMark( i );
				m_ListCtrl.SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				m_ListCtrl.EnsureVisible( i, FALSE );
			}
		}
		/*
		for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPropList.size(); i++ ) {
			int nIndex = pSector->GetPropIndexFromPtr( CGlobalValue::GetInstance().m_pVecSelectPropList[i] );
			if( nIndex != -1 ) {
				m_ListCtrl.SetSelectionMark( nIndex );
				m_ListCtrl.SetItemState( nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				m_ListCtrl.EnsureVisible( nIndex, FALSE );
			}
		}
		*/
	}
	else if( wParam == 2 ) {
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		if( Sector == -1 ) return S_OK;

		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( pSector == NULL ) return S_OK;

		CEtWorldProp *pProp = pSector->GetPropFromCreateUniqueID( (DWORD)lParam );
		if( pProp ) {
			int nSelectCount = m_ListCtrl.GetSelectedCount();
			if( nSelectCount == 1 ) {
				POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
				int nIndex = m_ListCtrl.GetNextSelectedItem(p);
				char szIndex[32];
				sprintf_s( szIndex, "%s", pProp->IsControlLock() ? "¡î" : "" );
				m_ListCtrl.SetItemText( nIndex, 3, szIndex );
			}	
		}
	}
//	UpdateData( FALSE );

	return S_OK;
}
void CPropListPaneView::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
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

		for( DWORD i=0; i<pSector->GetPropCount(); i++ )
			((CTEtWorldProp*)pSector->GetPropFromIndex(i))->SetSelect( false );
		CGlobalValue::GetInstance().m_pVecSelectPropList.clear();

		int nSelectCount = m_ListCtrl.GetSelectedCount();
		int nIndex;
		CString szStr;
		POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
		for( int i=0; i<nSelectCount; i++ ) {
			nIndex = m_ListCtrl.GetNextSelectedItem(p);
			szStr = m_ListCtrl.GetItemText( nIndex, 2 );
			CEtWorldProp *pProp = pSector->GetPropFromCreateUniqueID( atoi( szStr.GetBuffer() ) );
			if( pProp == NULL ) continue;
			CGlobalValue::GetInstance().m_pVecSelectPropList.push_back( pProp );
			((CTEtWorldProp*)pProp)->SetSelect( true );

		}

		CWnd *pWnd = GetPaneWnd( PROPPROP_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, ( CGlobalValue::GetInstance().m_pVecSelectPropList.size() > 0 ) ? 1 : 0 );
		CGlobalValue::GetInstance().RefreshRender();
	}
}

BOOL CPropListPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CPropListPaneView::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::PROP && CGlobalValue::GetInstance().IsCheckOutMe() && pLVKeyDow->wVKey == VK_DELETE ) {
		CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
		if( pView && pView->GetDummyView()->GetTabViewStruct( CGlobalValue::PROP ) ) {
			CPropRenderView *pPropView = (CPropRenderView *)pView->GetDummyView()->GetTabViewStruct( CGlobalValue::PROP )->pView;
			if( pPropView ) pPropView->DeleteProp();
		}
	}
}

void CPropListPaneView::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::TRIGGER ) return;
	if( CGlobalValue::GetInstance().m_nPickType != CEtTrigger::Prop ) return;
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;


	int nSelectCount = m_ListCtrl.GetSelectedCount();
	if( nSelectCount != 1 ) return;
	POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
	int nIndex = m_ListCtrl.GetNextSelectedItem(p);
	CString szStr = m_ListCtrl.GetItemText( nIndex, 2 );
	CEtWorldProp *pProp = pSector->GetPropFromCreateUniqueID( atoi(szStr.GetBuffer()) );
	CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)pProp );
}

LRESULT CPropListPaneView::OnEnablePane( WPARAM wParam, LPARAM lParam )
{
	m_ListCtrl.EnableWindow( (BOOL)wParam );
	return S_OK;
}
void CPropListPaneView::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CPropListPaneView::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;

	// set sort image for header and sort column.
	m_ListCtrl.SetSortImage(m_nSortedCol, m_bAscending);

	CXTSortClass csc(&m_ListCtrl, m_nSortedCol);
	csc.Sort(m_bAscending, ( m_nSortedCol == 2 ) ? xtSortInt : xtSortString);
}
