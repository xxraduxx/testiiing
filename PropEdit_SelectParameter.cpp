// PropEdit_SelectParameter.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropEdit_SelectParameter.h"
#include "UnionValueProperty.h"

// CPropEdit_SelectParameter

IMPLEMENT_DYNCREATE(CPropEdit_SelectParameter, CFormView)

CPropEdit_SelectParameter::CPropEdit_SelectParameter()
	: CFormView(CPropEdit_SelectParameter::IDD)
{
	m_bActivate = false;
}

CPropEdit_SelectParameter::~CPropEdit_SelectParameter()
{
}

void CPropEdit_SelectParameter::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CPropEdit_SelectParameter, CFormView)
END_MESSAGE_MAP()


// CPropEdit_SelectParameter diagnostics

#ifdef _DEBUG
void CPropEdit_SelectParameter::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEdit_SelectParameter::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropEdit_SelectParameter message handlers

CPropEditWizardCommon::VIEW_TYPE CPropEdit_SelectParameter::GetNextView()
{
	switch( s_nStartType ) {
		case 4:	return CREATE_PARAMETER;
		case 5:	return FINISH;
		default:
			assert(0);
	}
	return UNKNOWN_TYPE;
}

bool ParameterCompareProc2( CUnionValueProperty *pVariable1, CUnionValueProperty *pVariable2 )
{
	return ( strcmp( pVariable1->GetDescription(), pVariable2->GetDescription() ) > 0 ) ? false : true;
}

void CPropEdit_SelectParameter::PreProcess()
{
	m_ListCtrl.DeleteAllItems();

	std::vector<CUnionValueProperty *> pVecList;
	for( DWORD i=0; i<s_pCurrentSignalItem->GetParameterCount(); i++ ) {
		pVecList.push_back( s_pCurrentSignalItem->GetParameter(i) );
	}
	std::sort( pVecList.begin(), pVecList.end(), ParameterCompareProc2 );

	CString szStr;
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		CUnionValueProperty *pItem = pVecList[i];
		m_ListCtrl.InsertItem( i, pItem->GetDescription() );

		m_ListCtrl.SetItemText( i, 1, szStr );

		switch( pItem->GetType() ) {
			case CUnionValueProperty::Integer:
				szStr = "INT";
				if( _GetSubStrByCount( 3, (char*)pItem->GetSubDescription() ) ) szStr += " - min, max";
				break;
			case CUnionValueProperty::Float:
				szStr = "FLOAT";
				if( _GetSubStrByCount( 3, (char*)pItem->GetSubDescription() ) ) szStr += " - min, max";
				break;
			case CUnionValueProperty::Color:
				szStr = "INT - Color";
				break;
			case CUnionValueProperty::Integer_Combo:
				szStr = "INT - Combo";
				break;
			case CUnionValueProperty::Vector3:
				szStr = "VECTOR3";
				break;
			case CUnionValueProperty::Vector2:
				szStr = "VECTOR2";
				break;
			case CUnionValueProperty::Vector4:
				szStr = "VECTOR4";
				break;
			case CUnionValueProperty::Vector4Color:
				szStr = "VECTOR4 - Color";
				break;
			case CUnionValueProperty::Boolean:
				szStr = "BOOL";
				break;
			case CUnionValueProperty::String:
				szStr = "STRING";
				break;
			case CUnionValueProperty::String_FileOpen:
				szStr = "STRING - FileName";
				break;
		}

		m_ListCtrl.SetItemText( i, 1, szStr );

		m_ListCtrl.SetItemText( i, 2, _GetSubStrByCount( 0, (char*)pItem->GetSubDescription() ) );
	}
}

bool CPropEdit_SelectParameter::PostProcess()
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


	switch( s_nStartType ) {
		case 4:
			{
				CUnionValueProperty *pVariable = s_pCurrentSignalItem->GetParameterFromName( szVecSelectItemList[0] );
				s_szCurrentPropertyName = pVariable->GetDescription();

				*s_pCurrentPropertyVariable = *pVariable;
			}
			break;
		case 5:
			{
				CUnionValueProperty *pVariable = s_pCurrentSignalItem->GetParameterFromName( szVecSelectItemList[0] );
				s_szCurrentPropertyName = pVariable->GetDescription();
			}
			break;
	}
	return true;
}

void CPropEdit_SelectParameter::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ListCtrl.AddColumn( "Name", 150 );
	m_ListCtrl.AddColumn( "Type", 100, LVCFMT_CENTER );
	m_ListCtrl.AddColumn( "Description", 220, LVCFMT_LEFT );

	m_ListCtrl.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );
}
