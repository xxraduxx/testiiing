// EventAreaPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventAreaPaneView.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"
#include "FileServer.h"
#include "GlobalValue.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "UserMessage.h"
#include "EtTrigger.h"
#include "PluginManager.h"
#include "EtEngine.h"
#include "EventRenderView.h"


// CEventAreaPaneView

IMPLEMENT_DYNCREATE(CEventAreaPaneView, CFormView)

CEventAreaPaneView::CEventAreaPaneView()
	: CFormView(CEventAreaPaneView::IDD)
{
	m_bActivate = false;
	m_nSortedCol = 0;
	m_bAscending = true;
	m_bSorting = false;
	m_pCurAction = NULL;
}

CEventAreaPaneView::~CEventAreaPaneView()
{
	SAFE_DELETE( m_pCurAction );
}

void CEventAreaPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CEventAreaPaneView, CFormView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( UM_EVENT_PANE_SELCHANGED, OnSelChanged )
	ON_MESSAGE( UM_EVENT_PANE_CONTROL_CHANGED, OnControlChanged )
	ON_MESSAGE( UM_EVENT_PANE_REFRESH_INFO, OnRefreshInfo )
	ON_COMMAND(ID_EVENT_LIST, &CEventAreaPaneView::OnEventList)
	ON_UPDATE_COMMAND_UI(ID_EVENT_LIST, &CEventAreaPaneView::OnUpdateEventList)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST1, &CEventAreaPaneView::OnLvnBeginlabeleditList1)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, &CEventAreaPaneView::OnLvnEndlabeleditList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CEventAreaPaneView::OnLvnItemchangedList1)
	ON_WM_KEYDOWN()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, &CEventAreaPaneView::OnLvnKeydownList1)
	ON_MESSAGE( UM_ENABLE_PANE_VIEW, OnEnablePane )
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CEventAreaPaneView::OnNMDblclkList1)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CEventAreaPaneView::OnHdnItemclickList1)
END_MESSAGE_MAP()


// CEventAreaPaneView diagnostics

#ifdef _DEBUG
void CEventAreaPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventAreaPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventAreaPaneView message handlers

int CEventAreaPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_EVENTLIST);

	m_wndToolBar.GetControls()->SetControlType( 0, xtpControlComboBox );

	return 0;
}

void CEventAreaPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Area Name"), LVCFMT_LEFT, 110 );
	m_ListCtrl.InsertColumn( 1, _T("Min"), LVCFMT_LEFT, 40 );
	m_ListCtrl.InsertColumn( 2, _T("Max"), LVCFMT_LEFT, 40 );
	m_ListCtrl.InsertColumn( 3, _T("ID"), LVCFMT_LEFT, 35 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	HWND hWndHeader = m_ListCtrl.GetDlgItem(0)->GetSafeHwnd();
	m_header.SubclassWindow(hWndHeader);

	SendMessage( UM_REFRESH_PANE_VIEW );
}

BOOL CEventAreaPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

void CEventAreaPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}

	if( m_ListCtrl ) {
		m_ListCtrl.SetWindowPos( NULL, 0, nTop, cx, cy - nTop, SWP_FRAMECHANGED );
	}
}

LRESULT CEventAreaPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_ListCtrl.DeleteAllItems();

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return S_OK;

	CXTPControl *pCtrl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_EVENT_LIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pCtrl;
	pCombo->ResetContent();

	CString szControlName = CGlobalValue::GetInstance().m_szSelectControlName;
	CString szTemp;
	for( DWORD i=0; i<pSector->GetControlCount(); i++ ) {
		szTemp = ((CTEtWorldEventControl*)pSector->GetControlFromIndex(i))->GetName();
		pCombo->InsertString( pCombo->GetCount(), szTemp );

		// 선택된게 암것두 없을떄는 첫번째껄 선택하게 해준다.
		if( szControlName.IsEmpty() && i == 0 ) {
			CGlobalValue::GetInstance().m_szSelectControlName = szTemp;
			szControlName = szTemp;
		}

		if( szTemp == szControlName ) {
			pCombo->SetCurSel(i);
		}
	}

	CEtWorldEventControl *pControl = pSector->GetControlFromName( szControlName );
	if( pControl == NULL ) return S_OK;

	char szBuf[64] = { 0, };
	for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
		CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);
		int nItem = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pArea->GetName() );

		_itoa_s( (int)pArea->GetMin()->y, szBuf, 10 );
		m_ListCtrl.SetItemText( nItem, 1, szBuf );

		_itoa_s( (int)pArea->GetMax()->y, szBuf, 10 );
		m_ListCtrl.SetItemText( nItem, 2, szBuf );

		_itoa_s( (int)pArea->GetCreateUniqueID(), szBuf, 10 );
		m_ListCtrl.SetItemText( nItem, 3, szBuf );
	}
	CWnd *pWnd = GetPaneWnd( EVENTPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return S_OK;
}

void CEventAreaPaneView::OnEventList()
{
	// TODO: Add your command handler code here
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_EVENT_LIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;
	CString szStr;
	if( pCombo->GetCurSel() == -1 ) return;
	pCombo->GetLBText( pCombo->GetCurSel(), szStr );

	CGlobalValue::GetInstance().m_szSelectControlName = szStr;
	SendMessage( UM_REFRESH_PANE_VIEW );
	CGlobalValue::GetInstance().RefreshRender();
}

void CEventAreaPaneView::OnUpdateEventList(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	BOOL bEnable = TRUE;
	if( !CFileServer::GetInstance().IsConnect() ) bEnable = FALSE;
	pCmdUI->Enable( bEnable );
}

void CEventAreaPaneView::OnLvnBeginlabeleditList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_szPrevEditString = m_ListCtrl.GetItemText( pDispInfo->item.iItem, pDispInfo->item.iSubItem );
	*pResult = 0;
}

void CEventAreaPaneView::OnLvnEndlabeleditList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	CString szNewName = pDispInfo->item.pszText;
	if( szNewName.IsEmpty() ) return;
	CEtWorldEventControl *pControl = pSector->GetControlFromName( CGlobalValue::GetInstance().m_szSelectControlName );
//	if( pControl->GetAreaFromName( szNewName ) ) return;

	CEtWorldEventArea *pArea = pControl->GetAreaFromIndex( pDispInfo->item.iItem );
	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementModifyEventArea( 2 );
		m_pCurAction->SetDesc( "Rename EventArea" );

		((CActionElementModifyEventArea*)m_pCurAction)->SetEventArea( pArea );
	}

	pArea->SetName( szNewName );

	m_pCurAction->AddAction();
	m_pCurAction = NULL;

	m_ListCtrl.SetItemText( pDispInfo->item.iItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText );
	CGlobalValue::GetInstance().RefreshRender();
	CGlobalValue::GetInstance().SetModify();
	SendMessage( UM_REFRESH_PANE_VIEW );

	*pResult = 0;
}

void CEventAreaPaneView::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_bSorting ) return;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CString szStr = m_ListCtrl.GetItemText( pNMLV->iItem, 3 );

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	CEtWorldEventControl *pControl = pSector->GetControlFromName( CGlobalValue::GetInstance().m_szSelectControlName );
	if( pNMLV->uNewState == (LVIS_FOCUSED|LVIS_SELECTED) )
		CGlobalValue::GetInstance().m_pSelectEvent = pControl->GetAreaFromCreateUniqueID( atoi(szStr.GetBuffer()) );
	else CGlobalValue::GetInstance().m_pSelectEvent = NULL;

	CGlobalValue::GetInstance().RefreshRender();

	CWnd *pWnd = GetPaneWnd( EVENTPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pControl, (LPARAM)CGlobalValue::GetInstance().m_pSelectEvent );
}


LRESULT CEventAreaPaneView::OnSelChanged( WPARAM wParam, LPARAM lParam )
{
	CString szStr;
	for( int i=0; i<m_ListCtrl.GetItemCount(); i++ ) {
		szStr = m_ListCtrl.GetItemText( i, 3 );
		if( atoi( szStr.GetBuffer() ) == (int)wParam ) {
			m_ListCtrl.SetItemState( i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED );
			break;
		}
	}
	return S_OK;
}

LRESULT CEventAreaPaneView::OnControlChanged( WPARAM wParam, LPARAM lParam )
{
	CXTPControl *pControl = m_wndToolBar.GetControls()->FindControl( xtpControlComboBox, ID_EVENT_LIST, TRUE, FALSE );
	CXTPControlComboBox* pCombo = (CXTPControlComboBox*)pControl;

	if( pCombo->GetCurSel() == (int)wParam ) return S_OK;
	pCombo->SetCurSel( (int)wParam );
	pCombo->GetLBText( (int)wParam, CGlobalValue::GetInstance().m_szSelectControlName );
	SendMessage( UM_REFRESH_PANE_VIEW );
	return S_OK;
}

LRESULT CEventAreaPaneView::OnRefreshInfo( WPARAM wParam, LPARAM lParam )
{
	CEtWorldEventArea *pArea = CGlobalValue::GetInstance().m_pSelectEvent;
	if( pArea == NULL ) return S_OK;

	CString szStr;
	char szBuf[64];
	for( int i=0; i<m_ListCtrl.GetItemCount(); i++ ) {
		szStr = m_ListCtrl.GetItemText( i, 0 );
		if( strcmp( szStr, pArea->GetName() ) == NULL ) {
			_itoa_s( (int)pArea->GetMin()->y, szBuf, 10 );
			m_ListCtrl.SetItemText( i, 1, szBuf );

			_itoa_s( (int)pArea->GetMax()->y, szBuf, 10 );
			m_ListCtrl.SetItemText( i, 2, szBuf );
			break;
		}
	}
	return S_OK;
}


void CEventAreaPaneView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CEventAreaPaneView::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if( CGlobalValue::GetInstance().IsCheckOutMe() && pLVKeyDow->wVKey == VK_DELETE ) {
		SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
		if( Index == -1 ) return;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
		if( pSector == NULL ) return;

		if( CGlobalValue::GetInstance().m_szSelectControlName.IsEmpty() || CGlobalValue::GetInstance().m_pSelectEvent == NULL ) return;

		if( m_pCurAction == NULL ) {
			m_pCurAction = new CActionElementAddRemoveEventArea( false );
			m_pCurAction->SetDesc( "Remove EventArea" );
		}

		pSector->RemoveArea( CGlobalValue::GetInstance().m_szSelectControlName, CGlobalValue::GetInstance().m_pSelectEvent->GetCreateUniqueID(), m_pCurAction );

		CGlobalValue::GetInstance().m_pSelectEvent = NULL;
		CGlobalValue::GetInstance().RefreshRender();

		m_pCurAction->AddAction();
		m_pCurAction = NULL;

		CEtWorldEventControl *pControl = pSector->GetControlFromName( CGlobalValue::GetInstance().m_szSelectControlName );
		SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pControl, NULL );
	}
}

LRESULT CEventAreaPaneView::OnEnablePane( WPARAM wParam, LPARAM lParam )
{
	m_ListCtrl.EnableWindow( (BOOL)wParam );
	return S_OK;
}
void CEventAreaPaneView::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::EVENT && CGlobalValue::GetInstance().m_pSelectEvent ) {
		CPluginManager::GetInstance().CallDll( EVENTAREA_DLL, CGlobalValue::GetInstance().m_pSelectEvent->GetControl()->GetUniqueID(), "OnLButtonDoubleClick",
			CGlobalValue::GetInstance().m_pParentView, GetEtEngine(), CGlobalValue::GetInstance().m_pSelectEvent );
	}


	if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::TRIGGER ) return;
	if( CGlobalValue::GetInstance().m_nPickType != CEtTrigger::EventArea ) return;

	CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_TRIGGERPROP_PANE_PICK_OBJECT, (WPARAM)CGlobalValue::GetInstance().m_pSelectEvent );
}

void CEventAreaPaneView::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CEventAreaPaneView::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;

	// set sort image for header and sort column.
	m_ListCtrl.SetSortImage(m_nSortedCol, m_bAscending);

	CXTSortClass csc(&m_ListCtrl, m_nSortedCol);
	csc.Sort(m_bAscending, ( m_nSortedCol == 0 ) ? xtSortString : xtSortInt );
}
