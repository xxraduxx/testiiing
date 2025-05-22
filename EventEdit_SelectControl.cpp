// EventEdit_SelectControl.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventEdit_SelectControl.h"
#include "EventSignalManager.h"
#include "UnionValueProperty.h"


// CEventEdit_SelectControl

IMPLEMENT_DYNCREATE(CEventEdit_SelectControl, CFormView)

CEventEdit_SelectControl::CEventEdit_SelectControl()
	: CFormView(CEventEdit_SelectControl::IDD)
{
	m_bActivate = false;
}

CEventEdit_SelectControl::~CEventEdit_SelectControl()
{
}

void CEventEdit_SelectControl::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CEventEdit_SelectControl, CFormView)
END_MESSAGE_MAP()


// CEventEdit_SelectControl diagnostics

#ifdef _DEBUG
void CEventEdit_SelectControl::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventEdit_SelectControl::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventEdit_SelectControl message handlers

void CEventEdit_SelectControl::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.AddColumn( "Name", 255 );
	m_ListCtrl.AddColumn( "Color", 50, LVCFMT_CENTER );
	m_ListCtrl.AddColumn( "Parameter Count", 100, LVCFMT_RIGHT );

	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );
}

bool SignalCompareProc( CEventSignalItem *pItem1, CEventSignalItem *pItem2 )
{
	return ( strcmp( pItem1->GetName(), pItem2->GetName() ) > 0 ) ? false : true;
}

void CEventEdit_SelectControl::PreProcess()
{
	m_ListCtrl.DeleteAllItems();

	std::vector<CEventSignalItem *> pVecList;
	for( DWORD i=0; i<CEventSignalManager::GetInstance().GetSignalItemCount(); i++ ) {
		pVecList.push_back( CEventSignalManager::GetInstance().GetSignalItem(i) );
	}
	std::sort( pVecList.begin(), pVecList.end(), SignalCompareProc );

	CString szStr;
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		CEventSignalItem *pItem = pVecList[i];
		m_ListCtrl.InsertItem( i, pItem->GetName() );

		szStr.Format( "%d|%d|%d|%d", pItem->GetBackgroundColor(), pItem->GetBackgroundSideColor(), pItem->GetSelectColor(), pItem->GetSelectSideColor() );
		m_ListCtrl.SetItemText( i, 1, szStr );

		szStr.Format( "%d", pItem->GetParameterCount() );
		m_ListCtrl.SetItemText( i, 2, szStr );
	}
}

bool CEventEdit_SelectControl::PostProcess()
{
	POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
	std::vector<CString> szVecSelectItemList;
	while(p) {
		int nIndex = m_ListCtrl.GetNextSelectedItem(p);
		szVecSelectItemList.push_back( m_ListCtrl.GetItemText( nIndex, 0 ) );
	}
	if( szVecSelectItemList.size() == 0 ) {
		MessageBox( "고칠꺼 선택해줘~", "에러", MB_OK );
		return false;
	}

	// 복사해서 넣어준다.
	switch( s_nStartType ) {
		case 1:	// Modify
			{
				// 취소가 있을 수 있으므로 복사해서 넣어준다.
				CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( szVecSelectItemList[0] );
				if( pItem == NULL ) assert(0);
				s_szCurrentSignalName = pItem->GetName();

				s_pCurrentSignalItem->SetName( pItem->GetName() );
				s_pCurrentSignalItem->SetBackgroundColor( pItem->GetBackgroundColor() );
				s_pCurrentSignalItem->SetBackgroundSideColor( pItem->GetBackgroundSideColor() );
				s_pCurrentSignalItem->SetSelectColor( pItem->GetSelectColor() );
				s_pCurrentSignalItem->SetSelectSideColor( pItem->GetSelectSideColor() );
			}
			break;
		case 2:	// Delete
			{
				CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( szVecSelectItemList[0] );
				if( pItem == NULL ) assert(0);

				s_szCurrentSignalName = pItem->GetName();
			}
			break;
		case 3:	// Create Parameter
		case 4:	// Modify Parameter
			{
				CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( szVecSelectItemList[0] );
				s_pCurrentSignalItem = pItem;

				s_pCurrentPropertyVariable = new CUnionValueProperty( CUnionValueProperty::Unknown);
			}
			break;
		case 5:	// Delete Parameter
			{
				CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( szVecSelectItemList[0] );
				s_pCurrentSignalItem = pItem;
			}
			break;
	}

	return true;
}

CEventEditWizardCommon::VIEW_TYPE CEventEdit_SelectControl::GetNextView() 
{ 
	switch( s_nStartType ) {
		case 1: // Modify Signal
			return CREATE_SIGNAL;
		case 2: // Delete Signal
			return FINISH;
		case 3:	// Create Parameter
			return CREATE_PARAMETER;
		case 4:	// Modify Parameter
		case 5: // Delete Parameter
			return SELECT_PARAMETER;
		default:
			assert(0);
	}

	return UNKNOWN_TYPE;
}