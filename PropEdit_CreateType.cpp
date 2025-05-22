// PropEdit_CreateType.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropEdit_CreateType.h"
#include "PropSignalManager.h"
#include "PropSignalItem.h"


// CPropEdit_CreateType

IMPLEMENT_DYNCREATE(CPropEdit_CreateType, CFormView)

CPropEdit_CreateType::CPropEdit_CreateType()
	: CFormView(CPropEdit_CreateType::IDD)
{

}

CPropEdit_CreateType::~CPropEdit_CreateType()
{
}

void CPropEdit_CreateType::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_szSignalName);
	DDX_Control(pDX, IDC_COMBO1, m_Combo );
	DDV_MaxChars(pDX, m_szSignalName, 32);
}

BEGIN_MESSAGE_MAP(CPropEdit_CreateType, CFormView)
END_MESSAGE_MAP()


// CPropEdit_CreateType diagnostics

#ifdef _DEBUG
void CPropEdit_CreateType::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEdit_CreateType::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropEdit_CreateType message handlers

// CPropEdit_CreateType message handlers

void CPropEdit_CreateType::PreProcess()
{
	m_szSignalName = s_pCurrentSignalItem->GetName();
	int nSelectIndex = 0;
	char szBuf[128];
	for( DWORD i=0; i<256; i++ ) {
		if( CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex(i) != NULL ) {
			if( s_nStartType == 1 ) {
				if( i != s_pCurrentSignalItem->GetUniqueIndex() ) continue;
				else {
					nSelectIndex = m_Combo.GetCount();
				}
			}
			else continue;
		}
		_itoa_s( i, szBuf, 10 );
		m_Combo.InsertString( m_Combo.GetCount(), szBuf );
	}
	m_Combo.SetCurSel( nSelectIndex );
	UpdateData( FALSE );
}

bool CPropEdit_CreateType::PostProcess()
{
	UpdateData();

	if( m_szSignalName.IsEmpty() ) {
		MessageBox( "사용할 프랍 클래스 이름을 입력해 주세요.", "에러", MB_OK );
		return false;
	}
	bool bImpossibleChar = false;
	for( int i=0; i<m_szSignalName.GetLength(); i++ ) {
		if( m_szSignalName[i] < 'A' || m_szSignalName[i] > 'z' ) {
			bImpossibleChar = true;
			break;
		}
	}
	if( bImpossibleChar ) {
		MessageBox( "이름에 사용 불가능한 문자가 있습니다.", "에러", MB_OK );
		return false;
	}


	if( CPropSignalManager::GetInstance().IsExistSignalItem( m_szSignalName, ( s_nStartType == 1 ) ? s_szCurrentSignalName : "" ) == true ) {
		MessageBox( "중복되는 이름이 있습니다.", "에러", MB_OK );
		return false;
	}

	CString szUniqueID;
	int nUniqueIndex;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szUniqueID );
	nUniqueIndex = atoi( szUniqueID );

	if( CPropSignalManager::GetInstance().IsUseUniqueIndex( ( s_nStartType == 1 ) ? s_szCurrentSignalName : "", nUniqueIndex ) == true ) {
		MessageBox( "이미 할당된 클래스 아이디입니다.", "에러", MB_OK );
		return false;
	}

	s_pCurrentSignalItem->SetName( m_szSignalName );
	s_pCurrentSignalItem->SetUniqueIndex( nUniqueIndex );

	return true;
}
