// PropPropertiesPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropPropertiesPaneView.h"
#include "UserMessage.h"
#include "TEtWorldProp.h"
#include "TEtWorld.h"
#include "EtWorldSector.h"
#include "TEtWorldSector.h"
#include "PropRenderView.h"
#include "PaneDefine.h"
#include "MainFrm.h"


// CPropPropertiesPaneView
IMPLEMENT_DYNCREATE(CPropPropertiesPaneView, CFormView)

CPropPropertiesPaneView::CPropPropertiesPaneView()
	: CFormView(CPropPropertiesPaneView::IDD)
{
	m_bActivate = false;
}

CPropPropertiesPaneView::~CPropPropertiesPaneView()
{
	ResetPropertyGrid();
}

void CPropPropertiesPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropPropertiesPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CPropPropertiesPaneView diagnostics

#ifdef _DEBUG
void CPropPropertiesPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropPropertiesPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropPropertiesPaneView message handlers

void CPropPropertiesPaneView::OnInitialUpdate()
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

BOOL CPropPropertiesPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CPropPropertiesPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

extern PropertyGridBaseDefine g_PropPropertyDefine[];
LRESULT CPropPropertiesPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	if( wParam == 0 ) return S_OK;

	/////////////////
	bool bSameClass = true;
	int nClass = -1;
	DNVector(CEtWorldProp *) *pVecList = &CGlobalValue::GetInstance().m_pVecSelectPropList;
	if( pVecList->size() == 0 ) return S_OK;
	for( DWORD i=0; i<pVecList->size(); i++ ) {
		CTEtWorldProp *pProp = (CTEtWorldProp *)(*pVecList)[i];
		if( nClass == -1 ) nClass = pProp->GetClassID();
		else if( nClass != pProp->GetClassID() ) bSameClass = false;
	}
	/////////////////

	int nCommonPropCount = 0;
	for( DWORD i=0;; i++ ) {
		if( g_PropPropertyDefine[i].szCategory == NULL ) break;
		nCommonPropCount++;
	}

	if( bSameClass != true ) {
		RefreshPropertyGrid( g_PropPropertyDefine );
		for( DWORD i=nCommonPropCount; i<m_pVecVariable.size(); i++ ) {
			SetReadOnly( i, true );
		}
	}
	else {
		RefreshPropertyGrid( ((CTEtWorldProp *)(*pVecList)[0])->GetPropertyDefine() );
		if( pVecList->size() > 1 ) {
			for( DWORD i=nCommonPropCount; i<m_pVecVariable.size(); i++ ) {
				SetReadOnly( i, true );
			}
		}
	}

	if( pVecList->size() > 1 ) {
		SetReadOnly( nCommonPropCount - 3, true );
	}

	return S_OK;
}

LRESULT CPropPropertiesPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CPropPropertiesPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	// Set 할때는 첫번쨰 하나만 되게 해준다.
	DNVector(CEtWorldProp *) *pVecList = &CGlobalValue::GetInstance().m_pVecSelectPropList;
	CTEtWorldProp *pProp = (CTEtWorldProp *)(*pVecList)[0];
	pProp->OnSetValue( pVariable, dwIndex, this );
}

void CPropPropertiesPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	DNVector(CEtWorldProp *) *pVecList = &CGlobalValue::GetInstance().m_pVecSelectPropList;

	CActionElementModifyProp *pCurAction = NULL;
	if( dwIndex < 3 ) {
		pCurAction = new CActionElementModifyProp( (char)dwIndex );
		switch( dwIndex ) {
			case 0: pCurAction->SetDesc( "Modify Prop - Position" ); break;
			case 1: pCurAction->SetDesc( "Modify Prop - Rotation" ); break;
			case 2: pCurAction->SetDesc( "Modify Prop - Scale" ); break;
		}
	}

	for( DWORD i=0; i<pVecList->size(); i++ ) {
		CTEtWorldProp *pProp = (CTEtWorldProp *)(*pVecList)[i];

		if( dwIndex == 1 )
			*(pProp->GetPreRotation()) = *(pProp->GetRotation());

		if( pCurAction ) pCurAction->AddProp( pProp );

		pProp->OnChangeValue( pVariable, dwIndex, this );
	}
	if( pCurAction ) pCurAction->AddAction();

	if( dwIndex == 8 ) {
		int nRefreshUniqeuID = 0;
		if( !pVecList->empty() ) {
			CTEtWorldProp *pProp = (CTEtWorldProp *)(*pVecList)[0];
			nRefreshUniqeuID = pProp->GetCreateUniqueID();
		}
		CWnd *pWnd = GetPaneWnd( PROPLIST_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2, nRefreshUniqeuID );
	}

	CGlobalValue::GetInstance().SetModify();
}

void CPropPropertiesPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	m_nCurrentSelectIndex = dwIndex;
	if( pVariable->IsDefaultEnable() == FALSE ) m_nCurrentSelectIndex = -1;
}
