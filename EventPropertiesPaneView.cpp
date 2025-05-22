// EventPropertiesPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventPropertiesPaneView.h"
#include "UserMessage.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"

// CEventPropertiesPaneView
IMPLEMENT_DYNCREATE(CEventPropertiesPaneView, CFormView)

CEventPropertiesPaneView::CEventPropertiesPaneView()
	: CFormView(CEventPropertiesPaneView::IDD)
{
	m_bActivate = false;
}

CEventPropertiesPaneView::~CEventPropertiesPaneView()
{
	ResetPropertyGrid();
}

void CEventPropertiesPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEventPropertiesPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify )
END_MESSAGE_MAP()


// CEventPropertiesPaneView diagnostics

#ifdef _DEBUG
void CEventPropertiesPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventPropertiesPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventPropertiesPaneView message handlers

void CEventPropertiesPaneView::OnInitialUpdate()
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

BOOL CEventPropertiesPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CEventPropertiesPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CEventPropertiesPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	m_pSelectArea = NULL;

	if( wParam == 0 ) return S_OK;
	CTEtWorldEventControl *pControl = (CTEtWorldEventControl *)wParam;
	m_pSelectArea = (CTEtWorldEventArea *)lParam;
	if( m_pSelectArea )
		RefreshPropertyGrid( m_pSelectArea->OnInitProperty() );

	return S_OK;
}

LRESULT CEventPropertiesPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CEventPropertiesPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	if( m_pSelectArea ) m_pSelectArea->OnSetValue( pVariable, dwIndex );
}

void CEventPropertiesPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	if( m_pSelectArea ) m_pSelectArea->OnChangeValue( pVariable, dwIndex );
	CGlobalValue::GetInstance().SetModify();
	CGlobalValue::GetInstance().RefreshRender();
}

void CEventPropertiesPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	//CGlobalValue::GetInstance().SetModify();
}
