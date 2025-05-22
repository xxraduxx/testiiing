// TriggerValueDefinePaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerValueDefinePaneView.h"
#include "UserMessage.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "TEtTrigger.h"
#include "FileServer.h"
#include "TriggerValueDlg.h"


// CTriggerValueDefinePaneView

IMPLEMENT_DYNCREATE(CTriggerValueDefinePaneView, CFormView)

CTriggerValueDefinePaneView::CTriggerValueDefinePaneView()
	: CFormView(CTriggerValueDefinePaneView::IDD)
{
	m_bActivate = false;
	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );

}

CTriggerValueDefinePaneView::~CTriggerValueDefinePaneView()
{
	SAFE_DELETE( m_pContextMenu );
}

void CTriggerValueDefinePaneView::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTriggerValueDefinePaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_TRIGGERVALUEDEFINE_ADDVALUE, &CTriggerValueDefinePaneView::OnTriggerValueDefineAddValue)
	ON_COMMAND(ID_TRIGGERVALUEDEFINE_DELETEVALUE, &CTriggerValueDefinePaneView::OnTriggerValueDefineDeleteValue)
	ON_COMMAND(ID_TRIGGERVALUEDEFINE_MODIFYVALUE, &CTriggerValueDefinePaneView::OnTriggerValueDefineModifyValue)
END_MESSAGE_MAP()


// CTriggerValueDefinePaneView diagnostics

#ifdef _DEBUG
void CTriggerValueDefinePaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTriggerValueDefinePaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTriggerValueDefinePaneView message handlers
void CTriggerValueDefinePaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_ListCtrl ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_ListCtrl.MoveWindow( &rcRect );
	}
}

void CTriggerValueDefinePaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.InsertColumn( 0, _T("Value"), LVCFMT_LEFT, 140 );
	m_ListCtrl.InsertColumn( 1, _T("Type"), LVCFMT_LEFT, 50 );
	m_ListCtrl.InsertColumn( 2, _T("Default"), LVCFMT_LEFT, 50 );
	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

LRESULT CTriggerValueDefinePaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( wParam == -1 ) {
		m_ListCtrl.DeleteAllItems();
		return S_OK;
	}
	else if( wParam == 0 ) {
		SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
		if( Index == -1 ) return S_OK;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
		if( pSector == NULL ) return S_OK;
		CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
		if( pTrigger == NULL ) return S_OK;

		for( DWORD i=0; i<pTrigger->GetDefineValueCount(); i++ ) {
			SetItemListCtrl( pTrigger->GetDefineValueFromIndex(i), -1 );
		}
	}
	//	UpdateData( FALSE );

	return S_OK;
}

BOOL CTriggerValueDefinePaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

void CTriggerValueDefinePaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
	if( CFileServer::GetInstance().IsConnect() == false ) return;

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(7);

	
	pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_ADDVALUE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_DELETEVALUE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_MODIFYVALUE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	
	if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
		pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_ADDVALUE, MF_BYCOMMAND | MF_ENABLED );
	}

	if( m_ListCtrl.GetSelectedCount() == 1 ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_MODIFYVALUE, MF_BYCOMMAND | MF_ENABLED );
		}
	}
	if( m_ListCtrl.GetSelectedCount() >= 1 ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			pSubMenu->EnableMenuItem( ID_TRIGGERVALUEDEFINE_DELETEVALUE, MF_BYCOMMAND | MF_ENABLED );
		}
	}

	CPoint p = point;
	ScreenToClient( &p );
	CRect rcRect;
	m_ListCtrl.GetWindowRect( &rcRect );
	ScreenToClient( &rcRect );

	if( p.x < rcRect.left || p.x > rcRect.right || p.y < rcRect.top || p.y > rcRect.bottom ) return;

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CTriggerValueDefinePaneView::OnTriggerValueDefineAddValue()
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	CEtTrigger::DefineValue *pValue = pTrigger->AddDefineValue( "Value" );
	if( pValue == NULL ) {
		MessageBox( "Define Value 는 최대 256개까지밖에 못넣어요..", "에러", MB_OK );
		return;
	}
	CString szName;
	szName.Format( "Value #%02d", pValue->nUniqueID );
	pValue->szName = szName;

	CTriggerValueDlg Dlg;
	Dlg.SetValue( pValue );

	if( Dlg.DoModal() != IDOK ) {
		pTrigger->DeleteDefineValueFromName( pValue->szName.c_str() );
		return;
	}
	CGlobalValue::GetInstance().SetModify();

	SetItemListCtrl( pValue, -1 );
}

void CTriggerValueDefinePaneView::OnTriggerValueDefineDeleteValue()
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	int nSelectCount = m_ListCtrl.GetSelectedCount();
	int nItemID;
	CString szStr;
	POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
	std::vector<int> nVecList;
	for( int i=0; i<nSelectCount; i++ ) {
		nItemID = m_ListCtrl.GetNextSelectedItem(p);
		szStr = m_ListCtrl.GetItemText( nItemID, 0 );
		pTrigger->DeleteDefineValueFromName( szStr );
		nVecList.push_back( nItemID );
//		m_ListCtrl.DeleteItem( nItemID );
	}
	std::sort( nVecList.begin(), nVecList.end() );
	for( int i=(int)nVecList.size()-1; i>=0; i-- ) {
		m_ListCtrl.DeleteItem( nVecList[i] );
	}
	CGlobalValue::GetInstance().SetModify();
}

void CTriggerValueDefinePaneView::OnTriggerValueDefineModifyValue()
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	if( m_ListCtrl.GetSelectedCount() != 1 ) return;

	int nSelectCount = m_ListCtrl.GetSelectedCount();
	int nItemID;
	CString szStr;
	POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
	for( int i=0; i<nSelectCount; i++ ) {
		nItemID = m_ListCtrl.GetNextSelectedItem(p);
		szStr = m_ListCtrl.GetItemText( nItemID, 0 );
		break;
	}

	CEtTrigger::DefineValue *pValue = pTrigger->GetDefineValueFromName( szStr );
	CString szName;
	szName.Format( "Value #%02d", pValue->nUniqueID );

	CTriggerValueDlg Dlg;
	Dlg.SetModify( true );
	Dlg.SetValue( pValue );

	if( Dlg.DoModal() != IDOK ) return;

	SetItemListCtrl( pValue, nItemID );
	CGlobalValue::GetInstance().SetModify();
}

void CTriggerValueDefinePaneView::SetItemListCtrl( CEtTrigger::DefineValue *pValue, int nItemID )
{
	if( nItemID == -1 )
		nItemID = m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), pValue->szName.c_str() );

	const char *szParam[] = { "Integer", "Float", "String" };

	m_ListCtrl.SetItemText( nItemID, 0, pValue->szName.c_str() );
	m_ListCtrl.SetItemText( nItemID, 1, szParam[ pValue->ParamType ] );
	switch( pValue->ParamType ) {
		case CEtTrigger::Integer:
			{
				char szStr[256] = { 0, };
				_itoa_s( pValue->nValue, szStr, 10 );
				m_ListCtrl.SetItemText( nItemID, 2, szStr );
			}
			break;
		case CEtTrigger::Float:
			{
				char szStr[256] = { 0, };
				int nDecimal;
				int nSign;
				_fcvt_s( szStr, sizeof(szStr), pValue->fValue, 2, &nDecimal, &nSign );
				m_ListCtrl.SetItemText( nItemID, 2, szStr );
			}
			break;
		case CEtTrigger::String:
			m_ListCtrl.SetItemText( nItemID, 2, pValue->szValue );
			break;
	}
}