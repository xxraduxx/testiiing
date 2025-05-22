// ExportPropInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ExportPropInfoDlg.h"


// CExportPropInfoDlg dialog

IMPLEMENT_DYNAMIC(CExportPropInfoDlg, CDialog)

CExportPropInfoDlg::CExportPropInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportPropInfoDlg::IDD, pParent)
{

}

CExportPropInfoDlg::~CExportPropInfoDlg()
{
}

void CExportPropInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_MaxFolder);
}


BEGIN_MESSAGE_MAP(CExportPropInfoDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CExportPropInfoDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CExportPropInfoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CExportPropInfoDlg message handlers

void CExportPropInfoDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg(this);
	Dlg.SetTitle(_T("Select 3ds Max File Directory"));
	if( Dlg.DoModal() == IDOK ) {
		m_MaxFolder.SetWindowText( Dlg.GetSelPath() );
		UpdateData(FALSE);
	}
}

void CExportPropInfoDlg::OnBnClickedOk()
{
	UpdateData();

	CString szStr;
	m_MaxFolder.GetWindowText( szStr );

	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "LastMaxFolder", szStr.GetBuffer() );

	m_szMaxFolder = szStr;

	OnOK();
}

BOOL CExportPropInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	char szStr[2048] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "LastMaxFolder", szStr, 2048 );
	m_MaxFolder.SetWindowText( szStr );
	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
