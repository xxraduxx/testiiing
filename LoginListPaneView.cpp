// LoginListPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "LoginListPaneView.h"
#include "FileServer.h"
#include <Process.h>
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

// CLoginListPaneView

IMPLEMENT_DYNCREATE(CLoginListPaneView, CFormView)

CLoginListPaneView::CLoginListPaneView()
	: CFormView(CLoginListPaneView::IDD)
{

	m_bActivate = false;
}

CLoginListPaneView::~CLoginListPaneView()
{
}

void CLoginListPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CLoginListPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_ERASEBKGND()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CLoginListPaneView::OnLvnItemchangedList1)
END_MESSAGE_MAP()


// CLoginListPaneView 진단입니다.

#ifdef _DEBUG
void CLoginListPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CLoginListPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLoginListPaneView 메시지 처리기입니다.

void CLoginListPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_ListCtrl ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_ListCtrl.MoveWindow( &rcRect );
	}
}

void CLoginListPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 60 );
	m_ListCtrl.InsertColumn( 1, _T("CheckOut Grid"), LVCFMT_LEFT, 40 );
	m_ListCtrl.InsertColumn( 2, _T("IP"), LVCFMT_LEFT, 75 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );
}

void CLoginListPaneView::UpdateLoginList()
{
	std::vector<CFileServer::WorkingUserStruct> *pVecList = CFileServer::GetInstance().GetLoginUserList();

	m_ListCtrl.DeleteAllItems();

	CFileServer::WorkingUserStruct *pStruct;
	int nIndex;
	char szBuf[512];
	for( DWORD i=0; i<pVecList->size(); i++ ) {
		pStruct = &(*pVecList)[i];
		nIndex = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pStruct->szUserName );
		if( pStruct->LogStruct.CheckOutGridIndex != -1 ) {
			SectorIndex Temp = pStruct->LogStruct.CheckOutGridIndex;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Temp, pStruct->LogStruct.szCheckOutGridName );
			sprintf_s( szBuf, "%s (%d-%d) : %s", pStruct->LogStruct.szCheckOutGridName, Temp.nX, Temp.nY, pSector ? pSector->GetDescription() : "(null)");
		}
		else sprintf_s( szBuf, "None" );
		m_ListCtrl.SetItemText( nIndex, 1, szBuf );
		m_ListCtrl.SetItemText( nIndex, 2, pStruct->LogStruct.szIP );
	}
}

LRESULT CLoginListPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	UpdateLoginList();
	return S_OK;
}
BOOL CLoginListPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return FALSE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CLoginListPaneView::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) return;
	// TODO: Add your control notification handler code here
	if( GetFocus() == this || GetFocus() == &m_ListCtrl ) {
		int nSelectCount = m_ListCtrl.GetSelectedCount();
		if( nSelectCount == 0 ) return;

		POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
		for( int i=0; i<nSelectCount; i++ ) {
			int nIndex = m_ListCtrl.GetNextSelectedItem(p);
			CString szStr = m_ListCtrl.GetItemText( nIndex, 1 );
			char szGridName[256] = { 0, };
			sscanf_s( szStr.GetBuffer(), "%s ", szGridName, 256 );

			CTEtWorldGrid *pGrid = (CTEtWorldGrid *)CTEtWorld::GetInstance().FindGrid( szGridName );
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

//			CGlobalValue::GetInstance().m_szSelectGrid = szStr;
//			CGlobalValue::GetInstance().m_SelectGrid = SectorIndex( 0, 0 );

			EtVector3 vResult = vMin + ( vMax - vMin ) / 2.f;
			pStruct->pView->SendMessage( UM_GRIDPANE_CHANGECAMERA, (WPARAM)&vResult );

			CWnd *pWnd = GetPaneWnd( GRID_PANE );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

			break;
		}
	}

	*pResult = 0;
}
