// WaterPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "WaterPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "UserMessage.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "TEtWorldWater.h"
#include "TEtWorldWaterRiver.h"

// CWaterPaneView

IMPLEMENT_DYNCREATE(CWaterPaneView, CFormView)

CWaterPaneView::CWaterPaneView()
	: CFormView(CWaterPaneView::IDD)
{
	m_bActivate = false;
}

CWaterPaneView::~CWaterPaneView()
{
}

void CWaterPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
}

BEGIN_MESSAGE_MAP(CWaterPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CWaterPaneView::OnTvnSelchangedTree1)
	ON_WM_CREATE()
	ON_COMMAND(ID_BAKEWATERALPHA, &CWaterPaneView::OnBakewateralpha)
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_WATER_PANE_SELECTRIVER, OnSelectRiver )
END_MESSAGE_MAP()


// CWaterPaneView diagnostics

#ifdef _DEBUG
void CWaterPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CWaterPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CWaterPaneView message handlers

int CWaterPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_WATER_TOOLBAR);

	return 0;
}

void CWaterPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_BITMAP4 );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 4, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );

	m_Tree.EnableMultiSelect();
	m_Tree.DeleteAllItems();
	m_Tree.SetImageList( &m_ImageList, TVSIL_NORMAL );

	m_hRoot = m_Tree.InsertItem( "Global Water", 0, 1 );
}

BOOL CWaterPaneView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CWaterPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() ) {
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_MRUWIDTH|LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}
	if( m_Tree ) {
		m_Tree.SetWindowPos( NULL, 0, nTop, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CWaterPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_Tree.DeleteAllItems();
	m_Tree.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_Tree.EnableMultiSelect( true );

	m_hRoot = m_Tree.InsertItem( "Global Water", 0, 1 );

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return S_OK;
	CTEtWorldWater *pWater = (CTEtWorldWater *)pSector->GetWater();

	for( DWORD i=0; i<pWater->GetRiverCount(); i++ ) {
		CTEtWorldWaterRiver *pRiver = (CTEtWorldWaterRiver *)pWater->GetRiverFromIndex(i);
		m_Tree.InsertItem( pRiver->GetName(), 2, 3, m_hRoot );
	}
	m_Tree.Expand( m_hRoot, TVE_EXPAND );
	return S_OK;
}

LRESULT CWaterPaneView::OnSelectRiver( WPARAM wParam, LPARAM lParam )
{
	m_Tree.SelectItems( NULL, NULL );
	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectRiverList.size(); i++ ) {
		CTEtWorldWaterRiver *pRiver = CGlobalValue::GetInstance().m_pVecSelectRiverList[i];
		HTREEITEM hti = m_Tree.FindItem( pRiver->GetName(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_Tree.SetItemState( hti, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		}
	}
	return S_OK;
}

void CWaterPaneView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtWorldWater *pWater = (CTEtWorldWater *)pSector->GetWater();

	CalcSelectItemList();
	CWnd *pWnd = GetPaneWnd( WATERPROP_PANE );

	if( m_SelectItemList.GetCount() == 1 ) {

		if( m_SelectItemList.GetHead() == m_hRoot ) {
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1, (LPARAM)pWater );
		}
		else {
			CString szStr = m_Tree.GetItemText( m_SelectItemList.GetHead() );
			CEtWorldWaterRiver *pRiver = pWater->GetRiverFromName( szStr.GetBuffer() );
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2, (LPARAM)pRiver );
		}
	}
	else {
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
}

void CWaterPaneView::CalcSelectItemList()
{
	m_SelectItemList.RemoveAll();
	m_Tree.GetSelectedList( m_SelectItemList );
}

void CWaterPaneView::OnBakewateralpha()
{
	// TODO: Add your command handler code here
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtWorldWater *pWater = (CTEtWorldWater *)pSector->GetWater();

	if( !pWater ) return;
	CalcSelectItemList();

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	while(p) {
		hti = m_SelectItemList.GetNext( p );
		if( hti == m_hRoot ) {
			pWater->GenerationAlphaMap();
		}
		else {
			CString szStr = m_Tree.GetItemText( hti );
			CTEtWorldWaterRiver *pRiver = (CTEtWorldWaterRiver *)pWater->GetRiverFromName( szStr.GetBuffer() );
			if( pRiver )
				pRiver->GenerationAlphaMap();
		}
	}

	CGlobalValue::GetInstance().SetModify();
	CGlobalValue::GetInstance().RefreshRender();
}