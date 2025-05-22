// RandomDungeonDefaultPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "RandomDungeonDefaultPaneView.h"
#include "UserMessage.h"
#include "TEtRandomDungeon.h"
#include "EtMaze.h"


PropertyGridBaseDefine g_DungeonDefaultPropertyDefine[] = {
	{ "Default", "Width", CUnionValueProperty::Integer, "가로 블럭 갯수|5|200|1", TRUE },
	{ "Default", "Height", CUnionValueProperty::Integer, "세로 블럭 갯수|5|200|1", TRUE },
	{ "Default", "Level", CUnionValueProperty::Integer, "층 수|1|10|1", TRUE },
	{ "Default", "Sparseness", CUnionValueProperty::Integer, "길 복잡도 ( 0:복잡, 100:희박)|0|100|1|1", TRUE },
	{ "Default", "Randomness", CUnionValueProperty::Integer, "길 방향 랜덤확률|0|100|1", TRUE },
	{ "Default", "Deadends Remove", CUnionValueProperty::Integer, "막힌 길 이어줄 확률|0|100|1", TRUE },
	{ "Default", "Seed", CUnionValueProperty::Integer, "랜덤 시드 ( 0은 랜덤 )", TRUE },
	{ "Default", "Current Seed", CUnionValueProperty::Integer, "현제 시드", FALSE },
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


// CRandomDungeonDefaultPaneView 진단입니다.

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


// CRandomDungeonDefaultPaneView 메시지 처리기입니다.

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

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_PropertyGrid ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_PropertyGrid.MoveWindow( &rcRect );
	}
}

void CRandomDungeonDefaultPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
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
