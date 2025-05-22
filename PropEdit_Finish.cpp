// PropEdit_Finish.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropEdit_Finish.h"
#include "PropSignalManager.h"
#include "UnionValueProperty.h"


// CPropEdit_Finish

IMPLEMENT_DYNCREATE(CPropEdit_Finish, CFormView)

CPropEdit_Finish::CPropEdit_Finish()
	: CFormView(CPropEdit_Finish::IDD)
	, m_bCheckExport(FALSE)
	, m_szExportFileName("")
{

}

CPropEdit_Finish::~CPropEdit_Finish()
{
}

void CPropEdit_Finish::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_bCheckExport);
	DDX_Text(pDX, IDC_EDIT1, m_szExportFileName);
}

BEGIN_MESSAGE_MAP(CPropEdit_Finish, CFormView)
	ON_BN_CLICKED(IDC_CHECK1, &CPropEdit_Finish::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CPropEdit_Finish::OnBnClickedButton1)
END_MESSAGE_MAP()


// CPropEdit_Finish diagnostics

#ifdef _DEBUG
void CPropEdit_Finish::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEdit_Finish::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropEdit_Finish message handlers

void CPropEdit_Finish::OnBnClickedCheck1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
	GetDlgItem( IDC_EDIT1 )->EnableWindow( m_bCheckExport );
	GetDlgItem( IDC_BUTTON1 )->EnableWindow( m_bCheckExport );

}

void CPropEdit_Finish::PreProcess()
{
	char szStr[2048] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "PropLastExportFullName", szStr, 2048 );
	DWORD dwValue;
	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "PropExportHeader", dwValue );
	m_szExportFileName = szStr;
	m_bCheckExport = ( dwValue == TRUE );

	GetDlgItem( IDC_EDIT1 )->EnableWindow( m_bCheckExport );
	GetDlgItem( IDC_BUTTON1 )->EnableWindow( m_bCheckExport );

	UpdateData( FALSE );
}

bool CPropEdit_Finish::PostProcess()
{
	UpdateData();
	if( m_bCheckExport == TRUE ) {
		if( m_szExportFileName.IsEmpty() ) {
			MessageBox( "익스포트 할 파일을 지정해주세요", "에러", MB_OK );
			return false;
		}
		FILE *fp;
		fopen_s( &fp, m_szExportFileName, "wt" );
		if( fp == NULL ) {
			MessageBox( "파일을 쓸 수 없습니다.", "에러", MB_OK );
			return false;
		}
		fclose(fp);
	}

	switch( s_nStartType ) {
		case 0:
			CPropSignalManager::GetInstance().AddSignalItem( s_pCurrentSignalItem );
			s_pCurrentSignalItem = NULL;
			break;
		case 1:
			{
				CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromName( s_szCurrentSignalName );

				pItem->SetName( s_pCurrentSignalItem->GetName() );
				pItem->SetUniqueIndex( s_pCurrentSignalItem->GetUniqueIndex() );

				SAFE_DELETE( s_pCurrentSignalItem );
			}
			break;
		case 2:
			CPropSignalManager::GetInstance().RemoveSignalItem( s_szCurrentSignalName );
			break;
		case 3:
			if( s_pCurrentSignalItem->AddParameter( s_pCurrentPropertyVariable, true ) == false ) {
				SAFE_DELETE( s_pCurrentPropertyVariable );
				MessageBox( "더이상 파라메터 추가를 할 수 없어요!!\n리빌드와 제저장이 필요해요!!", "에러", MB_OK );
			}
			s_pCurrentSignalItem = NULL;
			s_pCurrentPropertyVariable = NULL;
			break;
		case 4:
			{
				CUnionValueProperty *pVariable = s_pCurrentSignalItem->GetParameterFromName( s_szCurrentPropertyName );
				*pVariable = *s_pCurrentPropertyVariable;

				s_pCurrentSignalItem = NULL;
				SAFE_DELETE( s_pCurrentPropertyVariable );
			}
			break;
		case 5:
			s_pCurrentSignalItem->RemoveParameter( s_szCurrentPropertyName );
			break;
	}

	if( m_bCheckExport == TRUE ) { 
		CPropSignalManager::GetInstance().ExportHeader( m_szExportFileName );
		SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "PropLastExportFullName", m_szExportFileName.GetBuffer() );
	}
	SetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "PropExportHeader", (DWORD)m_bCheckExport );
	return true;
}

void CPropEdit_Finish::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR szFilter[] = _T( "C/C++ Header File (*.h)|*.h|All Files (*.*)|*.*||" );
	CFileDialog dlg( FALSE, _T("h"), _T("*.h"), OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_ENABLESIZING , szFilter, this );

	dlg.m_ofn.lpstrTitle = "Export EtWorldPainter Prop Properties Refrence Header File";
	if( dlg.DoModal() == IDOK ) {
		m_szExportFileName = dlg.GetPathName();
		UpdateData( FALSE );
	}
}
