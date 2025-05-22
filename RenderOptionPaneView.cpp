// RenderOptionPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "RenderOptionPaneView.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "RenderBase.h"
#include "GlobalValue.h"


// CRenderOptionPaneView
PropertyGridBaseDefine g_RenderOptionPropertyDefine[] = {
	{ "Render", "Show Prop", CUnionValueProperty::Boolean, "프랍 Show/Hide", TRUE },
	{ "Render", "Prop Alpha", CUnionValueProperty::Float, "프랍 알파값|0.f|1.f|0.01f", TRUE },
	{ "Render", "Show Water", CUnionValueProperty::Boolean, "물 Show/Hide", TRUE },
	{ "Render", "Show Collision", CUnionValueProperty::Boolean, "프랍 컬리젼 Show/Hide", TRUE },
	{ "Render", "Show Navigation", CUnionValueProperty::Boolean, "네비게이션 Show/Hide", TRUE },
	{ "Render", "Show Attribute", CUnionValueProperty::Boolean, "바닥속성 Show/Hide", TRUE },
	{ "Render", "Draw Revision EmptyMesh Prop", CUnionValueProperty::Boolean, "메쉬가 없는 프랍 박스 보정 그리기", TRUE },
	{ "Render", "Show EventArea", CUnionValueProperty::Boolean, "이벤트 에이리어 Show/Hide", TRUE },
	{ NULL },
};

IMPLEMENT_DYNCREATE(CRenderOptionPaneView, CFormView)

CRenderOptionPaneView::CRenderOptionPaneView()
	: CFormView(CRenderOptionPaneView::IDD)
{
	m_bActivate = false;
}

CRenderOptionPaneView::~CRenderOptionPaneView()
{
	ResetPropertyGrid();
}

void CRenderOptionPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRenderOptionPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CRenderOptionPaneView diagnostics

#ifdef _DEBUG
void CRenderOptionPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRenderOptionPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CRenderOptionPaneView message handlers

void CRenderOptionPaneView::OnInitialUpdate()
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

BOOL CRenderOptionPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CRenderOptionPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CRenderOptionPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();

	//	PropertyGridBaseDefine *pDefine = NULL;
	RefreshPropertyGrid( g_RenderOptionPropertyDefine );

	return S_OK;
}

LRESULT CRenderOptionPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CRenderOptionPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowProp );
			break;
		case 1:
			SetReadOnly( pVariable, !CGlobalValue::GetInstance().m_bShowProp );
			pVariable->SetVariable( CGlobalValue::GetInstance().m_fPropAlpha );
			break;
		case 2:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowWater );
			break;
		case 3:
			SetReadOnly( pVariable, !CGlobalValue::GetInstance().m_bShowProp );
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowPropCollision );
			break;
		case 4:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowNavigation );
			break;
		case 5:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowAttribute );
			break;
		case 6:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bDrawRevisionEmptyMeshProp );
			break;
		case 7:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bShowEventArea );
			break;
	}
}

void CRenderOptionPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			CGlobalValue::GetInstance().ShowProp( pVariable->GetVariableBool() );
			SetReadOnly( 1, !pVariable->GetVariableBool() );
			SetReadOnly( 3, !pVariable->GetVariableBool() );
			break;
		case 1:
			CGlobalValue::GetInstance().SetPropAlpha( pVariable->GetVariableFloat() );
			break;
		case 2:
			CGlobalValue::GetInstance().ShowWater( pVariable->GetVariableBool() );
			break;
		case 3:
			CGlobalValue::GetInstance().ShowPropCollision( pVariable->GetVariableBool() );
			break;
		case 4:
			CGlobalValue::GetInstance().ShowNavigation( pVariable->GetVariableBool() );
			break;
		case 5:
			CGlobalValue::GetInstance().ShowAttribute( pVariable->GetVariableBool() );
			break;
		case 6:
			CGlobalValue::GetInstance().DrawRevisionEmptyMeshProp( pVariable->GetVariableBool() );
			break;
		case 7:
			CGlobalValue::GetInstance().ShowEventArea( pVariable->GetVariableBool() );
			break;
	}
	CGlobalValue::GetInstance().RefreshRender();
}

void CRenderOptionPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}

