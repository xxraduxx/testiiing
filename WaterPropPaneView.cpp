// WaterPropPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "WaterPropPaneView.h"
#include "UserMessage.h"
#include "TEtWorldWater.h"
#include "TEtWorldWaterRiver.h"
#include "GlobalValue.h"


// CWaterPropPaneView
IMPLEMENT_DYNCREATE(CWaterPropPaneView, CFormView)

CWaterPropPaneView::CWaterPropPaneView()
	: CFormView(CWaterPropPaneView::IDD)
{
	m_bActivate = false;
	m_nPropType = 0;
}

CWaterPropPaneView::~CWaterPropPaneView()
{
}

void CWaterPropPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWaterPropPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
	ON_MESSAGE( UM_TILEPANE_DROPITEM, OnDropItem )
END_MESSAGE_MAP()


// CWaterPropPaneView diagnostics

#ifdef _DEBUG
void CWaterPropPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CWaterPropPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CWaterPropPaneView message handlers

void CWaterPropPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );
	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

BOOL CWaterPropPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CWaterPropPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

//extern PropertyGridBaseDefine g_PropPropertyDefine[];
LRESULT CWaterPropPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	m_pWater = NULL;
	if( wParam == 0 ) return S_OK;

	m_nPropType = (int)wParam;
	switch( wParam ) {
		case 1: 
			m_pWater = (CTEtWorldWater*)lParam;
			RefreshPropertyGrid( m_pWater->GetPropertyList() ); 
			break;
		case 2: 
			m_pRiver = (CTEtWorldWaterRiver*)lParam;
			RefreshPropertyGrid( m_pRiver->GetPropertyList() ); 
			break;
	}

	if( !CGlobalValue::GetInstance().IsCheckOutMe() ) {
		for( DWORD i=0; i<m_pVecVariable.size(); i++ ) {
			SetReadOnly( i, true );
		}
	}

	return S_OK;
}

LRESULT CWaterPropPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CWaterPropPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( m_nPropType ) {
		case 1: m_pWater->OnSetValue( pVariable, dwIndex, this ); break;
		case 2: m_pRiver->OnSetValue( pVariable, dwIndex, this ); break;
	}
}

void CWaterPropPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( m_nPropType ) {
		case 1: m_pWater->OnChangeValue( pVariable, dwIndex, this ); break;
		case 2: m_pRiver->OnChangeValue( pVariable, dwIndex, this ); break;
	}
}

void CWaterPropPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( m_nPropType ) {
		case 1: m_pWater->OnSelectChangeValue( pVariable, dwIndex, this ); break;
		case 2: m_pRiver->OnSelectChangeValue( pVariable, dwIndex, this );  break;
	}
}

LRESULT CWaterPropPaneView::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	if( !CGlobalValue::GetInstance().IsCheckOutMe() ) return S_OK;

	CPoint p;
	p.x = (long)lParam >> 16;
	p.y = (long)( lParam << 16 ) >> 16;
	char *szFileName = (char *)wParam;
	char szName[256] = { 0, };
	_GetFullFileName( szName, _countof(szName), szFileName );

	CRect rcRect;
	for( DWORD i=0; i<m_pVecItem.size(); i++ ) {
		rcRect = m_pVecItem[i]->GetItemRect();
		if( p.x < rcRect.left || p.x > rcRect.right || p.y < rcRect.top || p.y > rcRect.bottom ) continue;
		if( (*m_ppVecVariable)[i]->GetType() != CUnionValueProperty::String_FileOpen ) continue;
		(*m_ppVecVariable)[i]->SetVariable( szName );
		OnChangeValue( (*m_ppVecVariable)[i], i );

		switch( m_nPropType ) {
			case 1: SendMessage( UM_REFRESH_PANE_VIEW, m_nPropType, (LPARAM)m_pWater ); break;
			case 2: SendMessage( UM_REFRESH_PANE_VIEW, m_nPropType, (LPARAM)m_pRiver ); break;
		}

		break;
	}
	

	return S_OK;
}