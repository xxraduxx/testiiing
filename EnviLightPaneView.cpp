// EnviLightPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EnviLightPaneView.h"
#include "UserMessage.h"
#include "EnviLightBase.h"
#include "EnviControl.h"
#include "RenderBase.h"
#include "FileServer.h"


// CEnviLightPaneView

IMPLEMENT_DYNCREATE(CEnviLightPaneView, CFormView)

CEnviLightPaneView::CEnviLightPaneView()
: CFormView(CEnviLightPaneView::IDD)
{
	m_bActivate = false;
	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
	m_hRoot = NULL;

	m_nDirCount = 1;
	m_nSpotCount = 1;
	m_nPointCount = 1;
}

CEnviLightPaneView::~CEnviLightPaneView()
{
	CEnviLightBase::Reset();
	ResetPropertyGrid();
	SAFE_DELETE( m_pContextMenu );
}

void CEnviLightPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
}

BEGIN_MESSAGE_MAP(CEnviLightPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnNotifyGrid )
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEnviLightPaneView::OnTvnSelchangedTree1)
	ON_COMMAND(ID_ENVILIGHT_ADDDIRECTIONLIGHT, &CEnviLightPaneView::OnEnvilightAdddirectionlight)
	ON_COMMAND(ID_ENVILIGHT_ADDPOINTLIGHT, &CEnviLightPaneView::OnEnvilightAddpointlight)
	ON_COMMAND(ID_ENVILIGHT_ADDSPOTLIGHT, &CEnviLightPaneView::OnEnvilightAddspotlight)
	ON_COMMAND(ID_ENVILIGHT_REMOVELIGHT, &CEnviLightPaneView::OnEnvilightRemovelight)
	ON_MESSAGE( UM_PROPERTY_PANE_SET_MODIFY, OnSetModify )
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CEnviLightPaneView diagnostics

#ifdef _DEBUG
void CEnviLightPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEnviLightPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEnviLightPaneView message handlers

void CEnviLightPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );

	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0);

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_ENVIICON );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 4, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );

	m_Tree.EnableMultiSelect();
	m_Tree.DeleteAllItems();
	m_Tree.SetImageList( &m_ImageList, TVSIL_NORMAL );
	CEnviLightBase::Reset();
	m_hRoot = m_Tree.InsertItem( "Light Folder", 2, 3 );


	SendMessage( UM_REFRESH_PANE_VIEW );
}


void CEnviLightPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_Tree ) {
		m_Tree.SetWindowPos( NULL, 0, 0, cx, cy / 2, SWP_FRAMECHANGED );
	}
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, cy/2, cx, cy, SWP_FRAMECHANGED );
	}
}

BOOL CEnviLightPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

LRESULT CEnviLightPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( !m_PropertyGrid ) return S_OK;
	ResetPropertyGrid();
//	RefreshPropertyGrid( g_EnviPropertyDefine );


	m_Tree.DeleteAllItems();
	m_Tree.SetImageList( &m_ImageList, TVSIL_NORMAL );

	CEnviLightBase::Reset();

	m_hRoot = m_Tree.InsertItem( "Light Folder", 2, 3 );

	m_nDirCount = 1;
	m_nSpotCount = 1;
	m_nPointCount = 1;

	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	if( pElement ) {
		SLightInfo *pInfo;
		CString szLightName;
		for( DWORD i=0; i<pElement->GetInfo().GetLightCount(); i++ ) {
			pInfo = pElement->GetInfo().GetLight(i)->pInfo;
			szLightName.Empty();
			switch( pInfo->Type ) {
				case LT_DIRECTIONAL: 
					szLightName.Format( "Direction Light - %02d", m_nDirCount );
					m_nDirCount++;
					break;
				case LT_POINT:
					szLightName.Format( "Point Light - %02d", m_nPointCount );
					m_nPointCount++;
					break;
				case LT_SPOT:
					szLightName.Format( "Spot Light - %02d", m_nSpotCount );
					m_nSpotCount++;
					break;
			}
			HTREEITEM hItem = m_Tree.InsertItem( szLightName, 0, 1, m_hRoot );
			CEnviLightBase::AddLight( hItem, pElement, i );
		}
	}

	m_Tree.Expand( m_hRoot, TVE_EXPAND );

	CRenderBase::GetInstance().InitializeEnviLight( pElement );
	CRenderBase::GetInstance().InitializeEnviLensFlare( pElement );
	CGlobalValue::GetInstance().RefreshRender();

	return S_OK;
}


LRESULT CEnviLightPaneView::OnNotifyGrid( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CEnviLightPaneView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	CalcSelectItemList();

	CEnviLightBase::s_pFocus = NULL;
	ResetPropertyGrid();
	if( m_SelectItemList.GetCount() == 1 ) {
		CEnviLightBase *pBase = CEnviLightBase::FindLight( m_SelectItemList.GetHead() );
		if( pBase ) {
			pBase->SetFocus( true );
			RefreshPropertyGrid( pBase->GetPropertyDefine() );
		}
	}
	CGlobalValue::GetInstance().RefreshRender();

	*pResult = 0;
}

void CEnviLightPaneView::CalcSelectItemList()
{
	m_SelectItemList.RemoveAll();
	m_Tree.GetSelectedList( m_SelectItemList );
}

void CEnviLightPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CalcSelectItemList();

	if( m_SelectItemList.GetCount() == 1 ) {
		CEnviLightBase *pBase = CEnviLightBase::FindLight( m_SelectItemList.GetHead() );

		pBase->OnSetValue( pVariable, dwIndex );
	}
}

void CEnviLightPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CalcSelectItemList();

	if( m_SelectItemList.GetCount() == 1 ) {
		CEnviLightBase *pBase = CEnviLightBase::FindLight( m_SelectItemList.GetHead() );

		pBase->OnChangeValue( pVariable, dwIndex );
		CRenderBase::GetInstance().InitializeEnviLight( pBase->GetElement(), false );
		CRenderBase::GetInstance().InitializeEnviLensFlare( pBase->GetElement(), false );
		CGlobalValue::GetInstance().RefreshRender();
	}
}

void CEnviLightPaneView::OnEnvilightAdddirectionlight()
{
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	if( !pElement ) return;

	SLightInfo LightInfo;
	LightInfo.Type = LT_DIRECTIONAL;
	LightInfo.Diffuse = EtColor( 0.5f, 0.5f, 0.5f, 1.f );
	LightInfo.Specular = EtColor( 1.f, 1.f, 1.f, 1.f );
	LightInfo.Direction = EtVector3( -1.f, -1.f, -1.f );

	EtVec3Normalize( &LightInfo.Direction, &LightInfo.Direction );

	CString szLightName;
	szLightName.Format( "Direction Light - %02d", m_nDirCount );
	HTREEITEM hItem = m_Tree.InsertItem( szLightName, 0, 1, m_hRoot );
	pElement->GetInfo().AddLight( LightInfo );
	CEnviLightBase::AddLight( hItem, pElement, pElement->GetInfo().GetLightCount() - 1 );

	m_Tree.Expand( m_hRoot, TVE_EXPAND );
	m_Tree.SelectItems( hItem, hItem, TRUE );
	m_Tree.FocusItem( hItem );

	CRenderBase::GetInstance().InitializeEnviLight( pElement, true );
	CRenderBase::GetInstance().InitializeEnviLensFlare( pElement, true );
	CGlobalValue::GetInstance().RefreshRender();

}

void CEnviLightPaneView::OnEnvilightAddpointlight()
{
	// TODO: Add your command handler code here
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	if( !pElement ) return;

	MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();

	SLightInfo LightInfo;
	LightInfo.Type = LT_POINT;
	LightInfo.Diffuse = EtColor( 0.5f, 0.5f, 0.5f, 1.f );
	LightInfo.Specular = EtColor( 1.f, 1.f, 1.f, 1.f );
	LightInfo.Position = pCamera->m_vPosition + ( pCamera->m_vZAxis * 500.f );
	LightInfo.fRange = 1000.f;

	CString szLightName;
	szLightName.Format( "Point Light - %02d", m_nPointCount );
	HTREEITEM hItem = m_Tree.InsertItem( szLightName, 0, 1, m_hRoot );
	pElement->GetInfo().AddLight( LightInfo );
	CEnviLightBase::AddLight( hItem, pElement, pElement->GetInfo().GetLightCount() - 1 );

	m_Tree.Expand( m_hRoot, TVE_EXPAND );
	m_Tree.SelectItems( hItem, hItem, TRUE );
	m_Tree.FocusItem( hItem );

	CRenderBase::GetInstance().InitializeEnviLight( pElement, true );
	CRenderBase::GetInstance().InitializeEnviLensFlare( pElement, true );
	CGlobalValue::GetInstance().RefreshRender();
}

void CEnviLightPaneView::OnEnvilightAddspotlight()
{
	// TODO: Add your command handler code here
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	if( !pElement ) return;

	MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();

	SLightInfo LightInfo;
	LightInfo.Type = LT_SPOT;
	LightInfo.Diffuse = EtColor( 0.5f, 0.5f, 0.5f, 1.f );
	LightInfo.Specular = EtColor( 1.f, 1.f, 1.f, 1.f );
	LightInfo.Position = pCamera->m_vPosition + ( pCamera->m_vZAxis * 500.f );
	LightInfo.Direction = pCamera->m_vZAxis;
	LightInfo.fTheta = cos( EtToRadian( 15.0f ) );
	LightInfo.fPhi = cos( EtToRadian( 30.0f ) );
	
	EtVec3Normalize( &LightInfo.Direction, &LightInfo.Direction );

	CString szLightName;
	szLightName.Format( "Spot Light - %02d", m_nSpotCount );
	HTREEITEM hItem = m_Tree.InsertItem( szLightName, 0, 1, m_hRoot );
	pElement->GetInfo().AddLight( LightInfo );
	CEnviLightBase::AddLight( hItem, pElement, pElement->GetInfo().GetLightCount() - 1 );

	m_Tree.Expand( m_hRoot, TVE_EXPAND );
	m_Tree.SelectItems( hItem, hItem, TRUE );
	m_Tree.FocusItem( hItem );

	CRenderBase::GetInstance().InitializeEnviLight( pElement, true );
	CRenderBase::GetInstance().InitializeEnviLensFlare( pElement, true );
	CGlobalValue::GetInstance().RefreshRender();
}

void CEnviLightPaneView::OnEnvilightRemovelight()
{
	// TODO: Add your command handler code here
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	if( !pElement ) return;
	CalcSelectItemList();

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	while(p) {
		hti = m_SelectItemList.GetNext( p );

		CEnviLightBase *pBase = CEnviLightBase::FindLight( hti );
		if( !pBase ) continue;
		for( DWORD i=0; i<pElement->GetInfo().GetLightCount(); i++ ) {
			if( pElement->GetInfo().GetLight(i)->pInfo == pBase->GetLightInfo() ) {
				pElement->GetInfo().RemoveLight(i);
				break;
			}
		}
		CEnviLightBase::RemoveLight( hti );
		m_Tree.DeleteItem( hti );
	}

	CRenderBase::GetInstance().InitializeEnviLight( pElement, true );
	CRenderBase::GetInstance().InitializeEnviLensFlare( pElement, true );
	CGlobalValue::GetInstance().RefreshRender();
}

void CEnviLightPaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
	CRect rcRect;
	GetClientRect( &rcRect );
//	if( !( point.x > rcRect.left && point.x < rcRect.right && point.y > rcRect.top && point.y < rcRect.bottom ) ) return;
	CMenu *pMenu = m_pContextMenu->GetSubMenu(3);
	if( !CFileServer::GetInstance().IsConnect() ) return;
	if( CEnviControl::GetInstance().GetActiveElement() ) {
		if( CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName() == "Edit Set" ) {
			pMenu->EnableMenuItem( ID_ENVILIGHT_ADDDIRECTIONLIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
			pMenu->EnableMenuItem( ID_ENVILIGHT_ADDPOINTLIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
			pMenu->EnableMenuItem( ID_ENVILIGHT_ADDSPOTLIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
			pMenu->EnableMenuItem( ID_ENVILIGHT_REMOVELIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

			pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
			return;
		}
	}

	CalcSelectItemList();

	int nSelectCount = (int)m_SelectItemList.GetCount();
	if( nSelectCount == 0 ) {
		pMenu->EnableMenuItem( ID_ENVILIGHT_REMOVELIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	}
	else {
		pMenu->EnableMenuItem( ID_ENVILIGHT_ADDDIRECTIONLIGHT, MF_BYCOMMAND | MF_ENABLED );

		// 스팟이랑 포인트는 일단 막아놓는다.
		pMenu->EnableMenuItem( ID_ENVILIGHT_ADDPOINTLIGHT, MF_BYCOMMAND | MF_ENABLED );
		pMenu->EnableMenuItem( ID_ENVILIGHT_ADDSPOTLIGHT, MF_BYCOMMAND | MF_ENABLED );

		pMenu->EnableMenuItem( ID_ENVILIGHT_REMOVELIGHT, MF_BYCOMMAND | MF_ENABLED );
	}

	pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}


LRESULT CEnviLightPaneView::OnSetModify( WPARAM wParam, LPARAM lParam )
{
	CUnionValueProperty *pVariable = (CUnionValueProperty *)lParam;
	if( lParam == NULL ) 
		ModifyItem( wParam, (LPARAM)m_pVecVariable[wParam] );
	else ModifyItem( wParam, (LPARAM)lParam );
	return S_OK;
}