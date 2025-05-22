// PropPoolPropertiesView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropPoolPropertiesView.h"
#include "UserMessage.h"
#include "GlobalValue.h"


// CPropPoolPropertiesView
PropertyGridBaseDefine g_PropPoolInfoPropertyDefine[] = {
	{ "Common", "Drawing Type", CUnionValueProperty::Integer_Combo, "드로잉 타입\n클릭할때만 그릴건지 클릭 후 움직일때마다 그릴건지|Click|Click & Move", TRUE },
	{ "Position", "Random Position", CUnionValueProperty::Boolean, "랜덤한 위치값", TRUE },
	{ "Position", "Minimum Distance", CUnionValueProperty::Float, "객체간 최소 간격|0.f|2000.f|1.f", FALSE },
	{ "Position", "Height Dig Percent", CUnionValueProperty::Float, "바닥으로부터 얼마나 무치게 찍을건지|0.f|100.f|0.1f", TRUE },
	{ "Rotation", "Random Rotation - X", CUnionValueProperty::Boolean, "랜덤한 X축 회전값", TRUE },
	{ "Rotation", "Random Rotation - Y", CUnionValueProperty::Boolean, "랜덤한 Y축 회전값", TRUE },
	{ "Rotation", "Random Rotation - Z", CUnionValueProperty::Boolean, "랜덤한 Z축 회전값", TRUE },
	{ "Rotation", "Lock Height Normal", CUnionValueProperty::Boolean, "바닥 노말값 영향을 받게 한다.", TRUE },
	{ "Scale", "Lock Axis", CUnionValueProperty::Boolean, "같은 스케일로 모든 좌표를 적용시킨다.", TRUE },
	{ "Scale", "Random Scale - X", CUnionValueProperty::Vector2, "랜덤 X 스케일값 영역|0.01f|10.f|0.01f", TRUE },	
	{ "Scale", "Random Scale - Y", CUnionValueProperty::Vector2, "랜덤 Y 스케일값 영역|0.01f|10.f|0.01f", FALSE },	
	{ "Scale", "Random Scale - Z", CUnionValueProperty::Vector2, "랜덤 Z 스케일값 영역|0.01f|10.f|0.01f", FALSE },
	{ "Position", "Ignore Distance Size", CUnionValueProperty::Boolean, "Minimum Distance 에서 자기자신의 사이즈를 무시하고 체크하겠는가", TRUE },
	{ NULL },
};


IMPLEMENT_DYNCREATE(CPropPoolPropertiesView, CFormView)

CPropPoolPropertiesView::CPropPoolPropertiesView()
	: CFormView(CPropPoolPropertiesView::IDD)
{
	m_bActivate = false;
}

CPropPoolPropertiesView::~CPropPoolPropertiesView()
{
	ResetPropertyGrid();
}

void CPropPoolPropertiesView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropPoolPropertiesView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CPropPoolPropertiesView diagnostics

#ifdef _DEBUG
void CPropPoolPropertiesView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropPoolPropertiesView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropPoolPropertiesView message handlers

void CPropPoolPropertiesView::OnInitialUpdate()
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

BOOL CPropPoolPropertiesView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
//	return __super::OnEraseBkgnd(pDC);
}

void CPropPoolPropertiesView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CPropPoolPropertiesView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	RefreshPropertyGrid( g_PropPoolInfoPropertyDefine );
	return S_OK;
}

LRESULT CPropPoolPropertiesView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CPropPoolPropertiesView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0: pVariable->SetVariable( (int)CGlobalValue::GetInstance().m_cDrawType );		break;
		case 1:	pVariable->SetVariable( CGlobalValue::GetInstance().m_bRandomPosition );	break;
		case 2:	pVariable->SetVariable( CGlobalValue::GetInstance().m_fMinPropDistance );	break;
		case 3: pVariable->SetVariable( CGlobalValue::GetInstance().m_fHeightDigPer );		break;
		case 4:	pVariable->SetVariable( CGlobalValue::GetInstance().m_bRandomRotationX );	break;
		case 5:	pVariable->SetVariable( CGlobalValue::GetInstance().m_bRandomRotationY );	break;
		case 6:	pVariable->SetVariable( CGlobalValue::GetInstance().m_bRandomRotationZ );	break;
		case 7:	
			pVariable->SetVariable( CGlobalValue::GetInstance().m_bLockHeightNormal );	
			if( pVariable->GetVariableBool() == true ) {
				SetReadOnly( 4, true );
				SetReadOnly( 6, true );
			}
			else {
				SetReadOnly( 4, false );
				SetReadOnly( 6, false );
			}
			break;
		case 8:	
			{
				pVariable->SetVariable( CGlobalValue::GetInstance().m_bLockScaleAxis );		
				if( pVariable->GetVariableBool() == true ) {
					SetReadOnly( 10, true );
					SetReadOnly( 11, true );
				}
				else {
					SetReadOnly( 10, false );
					SetReadOnly( 11, false );
				}
				break;
			}
		case 9: pVariable->SetVariable( D3DXVECTOR2( CGlobalValue::GetInstance().m_rScaleXRange.fMin, CGlobalValue::GetInstance().m_rScaleXRange.fMax ) );	break;
		case 10: pVariable->SetVariable( D3DXVECTOR2( CGlobalValue::GetInstance().m_rScaleYRange.fMin, CGlobalValue::GetInstance().m_rScaleYRange.fMax ) );	break;
		case 11: pVariable->SetVariable( D3DXVECTOR2( CGlobalValue::GetInstance().m_rScaleZRange.fMin, CGlobalValue::GetInstance().m_rScaleZRange.fMax ) );	break;
		case 12: pVariable->SetVariable( CGlobalValue::GetInstance().m_bIgnoreDistanceSize );	break;
	}
}

void CPropPoolPropertiesView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			CGlobalValue::GetInstance().m_cDrawType = (char)pVariable->GetVariableInt();
			break;
		case 1:	
			CGlobalValue::GetInstance().m_bRandomPosition = pVariable->GetVariableBool();	
			if( CGlobalValue::GetInstance().m_bRandomPosition == false )
				SetReadOnly( 2, true );
			else SetReadOnly( 2, false );
			break;
		case 2: CGlobalValue::GetInstance().m_fMinPropDistance = pVariable->GetVariableFloat();	break;
		case 3:	CGlobalValue::GetInstance().m_fHeightDigPer = pVariable->GetVariableFloat();	break;
		case 4:	
			CGlobalValue::GetInstance().m_bRandomRotationX = pVariable->GetVariableBool();	
			if( CGlobalValue::GetInstance().m_bRandomRotationX == true || CGlobalValue::GetInstance().m_bRandomRotationZ == true )
				SetReadOnly( 7, true );
			else SetReadOnly( 7, false );
			break;
		case 5:	CGlobalValue::GetInstance().m_bRandomRotationY = pVariable->GetVariableBool();	break;
		case 6:	
			CGlobalValue::GetInstance().m_bRandomRotationZ = pVariable->GetVariableBool();	
			if( CGlobalValue::GetInstance().m_bRandomRotationX == true || CGlobalValue::GetInstance().m_bRandomRotationZ == true )
				SetReadOnly( 7, true );
			else SetReadOnly( 7, false );
			break;
		case 7:
			CGlobalValue::GetInstance().m_bLockHeightNormal = pVariable->GetVariableBool();	
			if( pVariable->GetVariableBool() == true ) {
				SetReadOnly( 4, true );
				SetReadOnly( 6, true );
			}
			else {
				SetReadOnly( 4, false );
				SetReadOnly( 6, false );
			}
			break;
		case 8:
			CGlobalValue::GetInstance().m_bLockScaleAxis = pVariable->GetVariableBool();
			if( pVariable->GetVariableBool() == true ) {
				SetReadOnly( 10, true );
				SetReadOnly( 11, true );
			}
			else {
				SetReadOnly( 10, false );
				SetReadOnly( 11, false );
			}
			break;
		case 9:
			CGlobalValue::GetInstance().m_rScaleXRange = frange( pVariable->GetVariableVector2().x, pVariable->GetVariableVector2().y );
			break;
		case 10:
			CGlobalValue::GetInstance().m_rScaleYRange = frange( pVariable->GetVariableVector2().x, pVariable->GetVariableVector2().y );
			break;
		case 11:
			CGlobalValue::GetInstance().m_rScaleZRange = frange( pVariable->GetVariableVector2().x, pVariable->GetVariableVector2().y );
			break;
		case 12: 
			CGlobalValue::GetInstance().m_bIgnoreDistanceSize = pVariable->GetVariableBool();
			break;
	}
}

void CPropPoolPropertiesView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}
