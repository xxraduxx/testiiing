// NewProfileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "NewProfileDlg.h"
#include "FileServer.h"


// CNewProfileDlg dialog

IMPLEMENT_DYNAMIC(CNewProfileDlg, CDialog)

CNewProfileDlg::CNewProfileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewProfileDlg::IDD, pParent)
	, m_szProfileName(_T(""))
{

}

CNewProfileDlg::~CNewProfileDlg()
{
}

void CNewProfileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ProfileName);
	DDX_Text(pDX, IDC_EDIT1, m_szProfileName);
}


BEGIN_MESSAGE_MAP(CNewProfileDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CNewProfileDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNewProfileDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CNewProfileDlg message handlers

void CNewProfileDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
	if( CFileServer::GetInstance().IsExistProfile( m_szProfileName ) ) {
		if( m_szDefaultProfileName != m_szProfileName ) {
			MessageBox( "같은 이름의 프로파일이 존제합니다.", "에러", MB_OK );
			return;
		}
	}
	OnOK();
}

void CNewProfileDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CNewProfileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if( !m_szDefaultProfileName.IsEmpty() ) {
		m_szProfileName = m_szDefaultProfileName;
		UpdateData( FALSE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
