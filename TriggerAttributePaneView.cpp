// TriggerAttributePaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerAttributePaneView.h"
#include "UserMessage.h"
#include "EtTriggerObject.H"


PropertyGridBaseDefine g_TriggerPropertyDefine[] = {
	{ "Common", "Repeat Trigger", CUnionValueProperty::Boolean, "Repeat Trigger Process", TRUE },
	{ NULL },
};

// CTriggerAttributePaneView

IMPLEMENT_DYNCREATE(CTriggerAttributePaneView, CFormView)

CTriggerAttributePaneView::CTriggerAttributePaneView()
	: CFormView(CTriggerAttributePaneView::IDD)
{
	m_bActivate = false;
	m_pTriggerObj = NULL;
}

CTriggerAttributePaneView::~CTriggerAttributePaneView()
{
	ResetPropertyGrid();
}

void CTriggerAttributePaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTriggerAttributePaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CTriggerAttributePaneView diagnostics

#ifdef _DEBUG
void CTriggerAttributePaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTriggerAttributePaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTriggerAttributePaneView message handlers


void CTriggerAttributePaneView::OnInitialUpdate()
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

BOOL CTriggerAttributePaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CTriggerAttributePaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CTriggerAttributePaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	m_pTriggerObj = (CEtTriggerObject*)wParam;
	if( wParam == 0 ) return S_OK;

	RefreshPropertyGrid( g_TriggerPropertyDefine );

	return S_OK;
}

LRESULT CTriggerAttributePaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CTriggerAttributePaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( m_pTriggerObj->IsRepeat() );
			break;
	}
}

void CTriggerAttributePaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			m_pTriggerObj->SetRepeat( pVariable->GetVariableBool() );
			break;
	}
}

void CTriggerAttributePaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}
