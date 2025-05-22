// NaviPropertyPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "NaviPropertyPaneView.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "RenderBase.h"


// CNaviPropertyPaneView
PropertyGridBaseDefine g_NavigationPropertyDefine[] = {
	{ "Common", "Show Navigation Mesh", CUnionValueProperty::Boolean, "네비게이션 메쉬 볼래?", TRUE },
	{ "Common", "Mesh Count", CUnionValueProperty::Integer, "폴리곤 갯수", FALSE },
//	{ "Common", "Build Prop Attribute", CUnionValueProperty::Boolean, "프랍속성 만들까?", TRUE },
	{ "Brush", "Auto Diagonal", CUnionValueProperty::Boolean, "대각선 자동 생성할까?", TRUE },
	{ NULL },
};

IMPLEMENT_DYNCREATE(CNaviPropertyPaneView, CFormView)

CNaviPropertyPaneView::CNaviPropertyPaneView()
	: CFormView(CNaviPropertyPaneView::IDD)
{
	m_bActivate = false;

}

CNaviPropertyPaneView::~CNaviPropertyPaneView()
{
	ResetPropertyGrid();
}

void CNaviPropertyPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNaviPropertyPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
	ON_WM_CREATE()
	ON_COMMAND(ID_ATTR_1, &CNaviPropertyPaneView::OnAttr1)
	ON_COMMAND(ID_ATTR_2, &CNaviPropertyPaneView::OnAttr2)
	ON_COMMAND(ID_ATTR_3, &CNaviPropertyPaneView::OnAttr3)
	ON_COMMAND(ID_ATTR_4, &CNaviPropertyPaneView::OnAttr4)
	ON_UPDATE_COMMAND_UI(ID_ATTR_1, &CNaviPropertyPaneView::OnUpdateAttr1)
	ON_UPDATE_COMMAND_UI(ID_ATTR_2, &CNaviPropertyPaneView::OnUpdateAttr2)
	ON_UPDATE_COMMAND_UI(ID_ATTR_3, &CNaviPropertyPaneView::OnUpdateAttr3)
	ON_UPDATE_COMMAND_UI(ID_ATTR_4, &CNaviPropertyPaneView::OnUpdateAttr4)
	ON_COMMAND(ID_ATTR_MAKENAVI, &CNaviPropertyPaneView::OnAttrMakeNavi)
	ON_COMMAND(ID_ATTR_GEN_ATT4, &CNaviPropertyPaneView::OnAttrGenAttr4)
	ON_UPDATE_COMMAND_UI(ID_ATTR_MAKENAVI, &CNaviPropertyPaneView::OnUpdateMakeNavi)
	ON_UPDATE_COMMAND_UI(ID_ATTR_GEN_ATT4, &CNaviPropertyPaneView::OnUpdateGenAttr4)
END_MESSAGE_MAP()


// CNaviPropertyPaneView diagnostics

#ifdef _DEBUG
void CNaviPropertyPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CNaviPropertyPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CNaviPropertyPaneView message handlers


void CNaviPropertyPaneView::OnInitialUpdate()
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

BOOL CNaviPropertyPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CNaviPropertyPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	int nTop = 0;
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
		nTop += sz.cy;
	}

	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, nTop, cx, cy - nTop, SWP_FRAMECHANGED );
	}
}

LRESULT CNaviPropertyPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();

//	PropertyGridBaseDefine *pDefine = NULL;
	RefreshPropertyGrid( g_NavigationPropertyDefine );

	return S_OK;
}

LRESULT CNaviPropertyPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CNaviPropertyPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				pVariable->SetVariable( (BOOL)CGlobalValue::GetInstance().m_bShowNavigation );
//				pVariable->SetVariable( (BOOL)pSector->IsGenerationNavigationMesh() );
			}
			break;
		case 1:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				pVariable->SetVariable( (BOOL)pSector->GetNavigationFaceCount() );
			}
			break;
		case 2:
			pVariable->SetVariable( (bool)CGlobalValue::GetInstance().m_bAutoDiagonal );
			break;
	}
}

void CNaviPropertyPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			{
				/*
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				pSector->GenerationNavigationMesh( pVariable->GetVariableBool() );
				*/
				CGlobalValue::GetInstance().ShowNavigation( pVariable->GetVariableBool() );
			}
			break;
		case 1:
			break;
			/*
		case 2:
			if( pVariable->GetVariableBool() )
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				pSector->BuildPropAttribute();
				pVariable->SetVariable( FALSE );
			}
			break;
			*/
		case 2:
			CGlobalValue::GetInstance().m_bAutoDiagonal = pVariable->GetVariableBool();
			break;
	}
	CGlobalValue::GetInstance().RefreshRender();
}

void CNaviPropertyPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}

int CNaviPropertyPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_ATTRLIST);

	m_wndToolBar.GetControls()->SetControlType( 0, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 1, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 2, xtpControlButton );
	m_wndToolBar.GetControls()->SetControlType( 3, xtpControlButton );

	return 0;
}

void CNaviPropertyPaneView::OnAttr1()
{
	// TODO: Add your command handler code here
	CGlobalValue::GetInstance().m_cSelectAttribute = 0x01;
}

void CNaviPropertyPaneView::OnAttr2()
{
	// TODO: Add your command handler code here
	CGlobalValue::GetInstance().m_cSelectAttribute = 0x02;
}

void CNaviPropertyPaneView::OnAttr3()
{
	// TODO: Add your command handler code here
	CGlobalValue::GetInstance().m_cSelectAttribute = 0x04;
}

void CNaviPropertyPaneView::OnAttr4()
{
	// TODO: Add your command handler code here
	CGlobalValue::GetInstance().m_cSelectAttribute = 0x08;
}

void CNaviPropertyPaneView::OnUpdateAttr1(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( CGlobalValue::GetInstance().m_cSelectAttribute == 0x01 ? 1 : 0 );
}

void CNaviPropertyPaneView::OnUpdateAttr2(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( CGlobalValue::GetInstance().m_cSelectAttribute == 0x02 ? 1 : 0 );
}

void CNaviPropertyPaneView::OnUpdateAttr3(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( CGlobalValue::GetInstance().m_cSelectAttribute == 0x04 ? 1 : 0 );
}

void CNaviPropertyPaneView::OnUpdateAttr4(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( CGlobalValue::GetInstance().m_cSelectAttribute == 0x08 ? 1 : 0 );
}

void CNaviPropertyPaneView::OnAttrMakeNavi()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;

	pSector->GenerationNavigationMesh( true );
	CGlobalValue::GetInstance().SetModify();
}

void CNaviPropertyPaneView::OnAttrGenAttr4()
{
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector == NULL ) return;
	pSector->BuildPropAttribute();
}

void CNaviPropertyPaneView::OnUpdateMakeNavi(CCmdUI *pCmdUI)
{
}

void CNaviPropertyPaneView::OnUpdateGenAttr4(CCmdUI *pCmdUI)
{
}
