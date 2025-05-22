// RandomDungeonDefaultPaneView.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "RandomDungeonDefaultPaneView.h"
#include "UserMessage.h"
#include "TEtRandomDungeon.h"
#include "EtMaze.h"


PropertyGridBaseDefine g_DungeonDefaultPropertyDefine[] = {
	{ "Default", "Width", CUnionValueProperty::Integer, "���� �� ����|5|200|1", TRUE },
	{ "Default", "Height", CUnionValueProperty::Integer, "���� �� ����|5|200|1", TRUE },
	{ "Default", "Level", CUnionValueProperty::Integer, "�� ��|1|10|1", TRUE },
	{ "Default", "Sparseness", CUnionValueProperty::Integer, "�� ���⵵ ( 0:����, 100:���)|0|100|1|1", TRUE },
	{ "Default", "Randomness", CUnionValueProperty::Integer, "�� ���� ����Ȯ��|0|100|1", TRUE },
	{ "Default", "Deadends Remove", CUnionValueProperty::Integer, "���� �� �̾��� Ȯ��|0|100|1", TRUE },
	{ "Default", "Seed", CUnionValueProperty::Integer, "���� �õ� ( 0�� ���� )", TRUE },
	{ "Default", "Current Seed", CUnionValueProperty::Integer, "���� �õ�", FALSE },
	{ NULL },
};


// CRandomDungeonDefaultPaneView

IMPLEMENT_DYNCREATE(CRandomDungeonDefaultPaneView, CFormView)

CRandomDungeonDefaultPaneView::CRandomDungeonDefaultPaneView()
	: CFormView(CRandomDungeonDefaultPaneView::IDD)
{
	m_bActivate = false;
}

CRandomDungeonDefaultPaneView::~CRandomDungeonDefaultPaneView()
{
	ResetPropertyGrid();
}

void CRandomDungeonDefaultPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRandomDungeonDefaultPaneView, CFormView)
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnNotifyGrid )
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CRandomDungeonDefaultPaneView �����Դϴ�.

#ifdef _DEBUG
void CRandomDungeonDefaultPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CRandomDungeonDefaultPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CRandomDungeonDefaultPaneView �޽��� ó�����Դϴ�.

LRESULT CRandomDungeonDefaultPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	RefreshPropertyGrid( g_DungeonDefaultPropertyDefine );
	return S_OK;
}

LRESULT CRandomDungeonDefaultPaneView::OnNotifyGrid( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CRandomDungeonDefaultPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if( m_PropertyGrid ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_PropertyGrid.MoveWindow( &rcRect );
	}
}

void CRandomDungeonDefaultPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );

	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0);
	SendMessage( UM_REFRESH_PANE_VIEW );
}


void CRandomDungeonDefaultPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_DungeonSize.nX );
			break;
		case 1:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_DungeonSize.nY );
			break;
		case 2:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_DungeonSize.nZ );
			break;
		case 3:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_nDungeonSparseness );
			break;
		case 4:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_nDungeonRandomness );
			break;
		case 5:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_nDungeonDeadendRemove );
			break;
		case 6:
			pVariable->SetVariable( CGlobalValue::GetInstance().m_nDungeonSeed );
			break;
		case 7:
			if( CTEtRandomDungeon::GetInstance().GetMaze() )
				pVariable->SetVariable( CTEtRandomDungeon::GetInstance().GetCurSeed() );
			break;
	}
}

void CRandomDungeonDefaultPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			CGlobalValue::GetInstance().m_DungeonSize.nX = pVariable->GetVariableInt();
			break;
		case 1:
			CGlobalValue::GetInstance().m_DungeonSize.nY = pVariable->GetVariableInt();
			break;
		case 2:
			CGlobalValue::GetInstance().m_DungeonSize.nZ = pVariable->GetVariableInt();
			break;
		case 3:
			CGlobalValue::GetInstance().m_nDungeonSparseness = pVariable->GetVariableInt();
			break;
		case 4:
			CGlobalValue::GetInstance().m_nDungeonRandomness = pVariable->GetVariableInt();
			break;
		case 5:
			CGlobalValue::GetInstance().m_nDungeonDeadendRemove = pVariable->GetVariableInt();
			break;
		case 6:
			CGlobalValue::GetInstance().m_nDungeonSeed = pVariable->GetVariableInt();
			break;
	}
}
