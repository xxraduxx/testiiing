// NationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "NationDlg.h"


// CNationDlg dialog

IMPLEMENT_DYNAMIC(CNationDlg, CDialog)

CNationDlg::CNationDlg(CWnd* pParent /*=NULL*/, CFileServer::ProfileStruct *pStruct)
	: CDialog(CNationDlg::IDD, pParent)
	, m_szNationName("")
	, m_szNationPath("")
	, m_szNationResource("")
{
	m_pProfile = pStruct;
	m_pNation = NULL;
}

CNationDlg::~CNationDlg()
{
}

void CNationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_szNationName);
	DDX_Text(pDX, IDC_EDIT4, m_szNationPath);
	DDX_Text(pDX, IDC_EDIT5, m_szNationResource);
}


BEGIN_MESSAGE_MAP(CNationDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CNationDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CNationDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CNationDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CNationDlg message handlers

BOOL CNationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if( m_pNation ) {
		m_szNationName = m_pNation->szNationStr;
		m_szNationPath = m_pNation->szNationPath;
		m_szNationResource = m_pNation->szNationResPath;
	}

	UpdateData( FALSE );


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNationDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_szNationPath.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_szNationPath);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_szNationPath = Dlg.GetSelPath();
		UpdateData(FALSE);
	}
}

void CNationDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_szNationResource.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_szNationResource);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_szNationResource = Dlg.GetSelPath();
		UpdateData(FALSE);
	}
}


void CNationDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
	if( m_szNationName.IsEmpty() ) {
		MessageBox( "이름을 입력해 주세여", "Error", MB_OK );
		return;
	}
	if( m_szNationPath.IsEmpty() ) {
		MessageBox( "경로 설정해주세요", "Error", MB_OK );
		return;
	}
	if( m_pProfile ) {
		for( DWORD i=0; i<m_pProfile->VecNation.size(); i++ ) {
			if( m_pNation == &m_pProfile->VecNation[i] ) continue;
			if( _stricmp( m_pProfile->VecNation[i].szNationStr, m_szNationName ) == NULL ) {
				MessageBox( "국가이름이 중복됩니다.", "Error", MB_OK );
				return;
			}
		}
	}

	if( m_pNation ) {
		m_pNation->szNationStr = m_szNationName;
		m_pNation->szNationPath = m_szNationPath;
		m_pNation->szNationResPath = m_szNationResource;
	}

	OnOK();
}
