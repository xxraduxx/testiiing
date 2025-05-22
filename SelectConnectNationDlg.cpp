// SelectConnectNationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SelectConnectNationDlg.h"
#include "FileServer.h"


// CSelectConnectNationDlg dialog

IMPLEMENT_DYNAMIC(CSelectConnectNationDlg, CDialog)

CSelectConnectNationDlg::CSelectConnectNationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectConnectNationDlg::IDD, pParent)
{

}

CSelectConnectNationDlg::~CSelectConnectNationDlg()
{
}

void CSelectConnectNationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
}


BEGIN_MESSAGE_MAP(CSelectConnectNationDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSelectConnectNationDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSelectConnectNationDlg message handlers

BOOL CSelectConnectNationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_Combo.AddString( "Local" );
	CString szStr;
	int nCount = CFileServer::GetInstance().GetNationCount();
	for( int i=0; i<nCount; i++ ) {
		szStr = CFileServer::GetInstance().GetNationName(i);
		m_Combo.AddString( szStr );
	}

	char szLastConnectNation[512] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "LastConnectNation", szLastConnectNation );

	int nIndex = m_Combo.FindString( -1, szLastConnectNation );
	if( nIndex > -1 ) {
		m_Combo.SetCurSel( nIndex );
	}
	else if( m_Combo.GetCount() > 0 ) m_Combo.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectConnectNationDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString szNation;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szNation );

	if( strcmp( szNation, "Local" ) == NULL ) {
		CFileServer::GetInstance().LockNation( "" );
	}
	else {
		CFileServer::GetInstance().LockNation( szNation );
	}
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "LastConnectNation", szNation.GetBuffer() );

	OnOK();
}
