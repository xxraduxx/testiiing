// CreateEnviDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreateEnviDlg.h"
#include "EnviControl.h"


// CCreateEnviDlg dialog

IMPLEMENT_DYNAMIC(CCreateEnviDlg, CDialog)

CCreateEnviDlg::CCreateEnviDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateEnviDlg::IDD, pParent)
	, m_szName(_T(""))
	, m_szCopyTo( _T("") )
{

}

CCreateEnviDlg::~CCreateEnviDlg()
{
}

void CCreateEnviDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_szName);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
}


BEGIN_MESSAGE_MAP(CCreateEnviDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CCreateEnviDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &CCreateEnviDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCreateEnviDlg message handlers

void CCreateEnviDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

void CCreateEnviDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );

	CString szCopyTo;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szCopyTo );
	m_szCopyTo = szCopyTo;

	if( CEnviControl::GetInstance().IsExistElement( m_szName ) ) {
		MessageBox( "같은게 잇음", "에러", MB_OK );
		return;
	}
	OnOK();

}

BOOL CCreateEnviDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Combo.ResetContent();
	// TODO:  Add extra initialization here
	m_Combo.InsertString( 0, "None" );
	for( DWORD i=0; i<CEnviControl::GetInstance().GetCount(); i++ ) {
		m_Combo.InsertString( m_Combo.GetCount(), CEnviControl::GetInstance().GetElement(i)->GetInfo().GetName() );
	}

	m_Combo.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
