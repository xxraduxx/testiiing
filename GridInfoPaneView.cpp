// GridInfoPaneView.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "GridInfoPaneView.h"
#include "UserMessage.h"
#include "GlobalValue.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"


PropertyGridBaseDefine g_GridPropertyDefine[] = {
	{ "World Info", "Select Grid Name", CUnionValueProperty::String, "���� �̸�", FALSE },	
	{ "World Info", "World Size", CUnionValueProperty::Vector2, "���� ũ��(����:�׸��� ��)", FALSE },
	{ "World Info", "Width", CUnionValueProperty::Integer, "�׸��� ������ ����(����:����)", FALSE },
	{ "World Info", "Height", CUnionValueProperty::Integer, "�׸��� ������ ����(����:����)", FALSE },
	{ "World Info", "Tile Size", CUnionValueProperty::Integer, "Ÿ�� ������(����:��ġ����)", FALSE },
	{ "Sector Info", "Sector Name", CUnionValueProperty::String, "���� �̸�", FALSE },
	{ NULL },
};


// CGridInfoPaneView

IMPLEMENT_DYNCREATE(CGridInfoPaneView, CFormView)

CGridInfoPaneView::CGridInfoPaneView()
	: CFormView(CGridInfoPaneView::IDD)
{
	m_bActivate = false;
}

CGridInfoPaneView::~CGridInfoPaneView()
{
	ResetPropertyGrid();
}

void CGridInfoPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGridInfoPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnNotifyGrid )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CGridInfoPaneView �����Դϴ�.

#ifdef _DEBUG
void CGridInfoPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CGridInfoPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CGridInfoPaneView �޽��� ó�����Դϴ�.
	
void CGridInfoPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );

	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0);
	SendMessage( UM_REFRESH_PANE_VIEW );

}

void CGridInfoPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if( m_PropertyGrid ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_PropertyGrid.MoveWindow( &rcRect );
	}
}

LRESULT CGridInfoPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( CFileServer::GetInstance().IsConnect() == false ) {
		ResetPropertyGrid();
		return S_OK;
	}
	RefreshPropertyGrid( g_GridPropertyDefine );
	return S_OK;
}

LRESULT CGridInfoPaneView::OnNotifyGrid( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CGridInfoPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			{
				SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
				CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
				CEtWorldGrid *pGrid = CTEtWorld::GetInstance().FindGrid( szGridName );
				CFileServer::GetInstance().UpdateLoginList();
				if( CFileServer::GetInstance().GetLoginUserList()->size() > 0 ) {
					MessageBox( "�� �۾��� �α����� ������ ����� �����մϴ�.", "����", MB_OK );

					SendMessage( UM_REFRESH_PANE_VIEW );
					CGlobalValue::GetInstance().RefreshRender();

					break;
				}
				if( pGrid == NULL ) {
					pVariable->SetVariable( (char*)szGridName.GetBuffer() );
				}
				else {
					if( CTEtWorld::GetInstance().ChangeGridName( szGridName, pVariable->GetVariableString() ) == false ) {
						pVariable->SetVariable( (char*)szGridName.GetBuffer() );
					}
				}
				SendMessage( UM_REFRESH_PANE_VIEW );
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 5:
			{
				SectorIndex Index = CGlobalValue::GetInstance().m_CheckOutGrid;
				CString szGridName = CGlobalValue::GetInstance().m_szCheckOutGrid;
				if( Index == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index, szGridName );
				if( pSector == NULL ) break;
				pSector->SetDescription( pVariable->GetVariableString() );
				
			}
			break;
	}
}

void CGridInfoPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{

	switch( dwIndex ) {
		case 0:
			{
				SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
				CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
				CEtWorldGrid *pGrid = CTEtWorld::GetInstance().FindGrid( szGridName );
				if( pGrid == NULL ) {
					pVariable->SetVariable( "Root" );
					SetReadOnly( 1, false );
					SetReadOnly( 2, false );
					SetReadOnly( 3, false );
					SetReadOnly( 4, false );
					SetReadOnly( 5, false );
					break;
				}
				pVariable->SetVariable( (char*)pGrid->GetName() );
				SetReadOnly( 1, true );
				SetReadOnly( 2, true );
				SetReadOnly( 3, true );
				SetReadOnly( 4, true );
				SetReadOnly( 5, true );

				if( pGrid == NULL ) {
					SetReadOnly( pVariable, true );
				}
				else SetReadOnly( pVariable, false );
			}
			break;
		case 1:
			CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
			pVariable->SetVariable( D3DXVECTOR2( (float)CTEtWorld::GetInstance().GetGridX(), (float)CTEtWorld::GetInstance().GetGridY() ) );
			CTEtWorld::GetInstance().SetCurrentGrid( NULL );
			break;
		case 2:
			CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
			pVariable->SetVariable( (int)CTEtWorld::GetInstance().GetGridWidth() );
			CTEtWorld::GetInstance().SetCurrentGrid( NULL );
			break;
		case 3:
			CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
			pVariable->SetVariable( (int)CTEtWorld::GetInstance().GetGridHeight() );
			CTEtWorld::GetInstance().SetCurrentGrid( NULL );
			break;
		case 4:
			CTEtWorld::GetInstance().SetCurrentGrid( CGlobalValue::GetInstance().m_szSelectGrid );
			pVariable->SetVariable( (int)CTEtWorld::GetInstance().GetGridTileSize() );
			CTEtWorld::GetInstance().SetCurrentGrid( NULL );
			break;
		case 5:
			{
				SetReadOnly( pVariable, true );
				SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
				CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
				CString szStr;
				if( Index == -1 || szGridName.IsEmpty() ) szStr = "None";
				else {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index, szGridName );
					if( pSector ) {
						if( pSector->IsEmpty() ) szStr = "Empty";
						else szStr = pSector->GetDescription();
					}
				}
				pVariable->SetVariable( szStr.GetBuffer() );

				if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
					SetReadOnly( pVariable, false );
				}
			}
			break;
	}
}
BOOL CGridInfoPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	return FALSE;
//	return __super::OnEraseBkgnd(pDC);
}
