// CreateProejctDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreateProjectDlg.h"
#include "FileServer.h"
#include "TEtWorld.h"


// CCreateProjectDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CCreateProjectDlg, CDialog)

CCreateProjectDlg::CCreateProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateProjectDlg::IDD, pParent)
	, m_szCreateFolder(_T(""))
{

}

CCreateProjectDlg::~CCreateProjectDlg()
{
}

void CCreateProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_szCreateFolder);
}


BEGIN_MESSAGE_MAP(CCreateProjectDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CCreateProjectDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CCreateProjectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CCreateProjectDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CCreateProjectDlg �޽��� ó�����Դϴ�.

void CCreateProjectDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	OnCancel();
}

void CCreateProjectDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData( TRUE );
	CFileServer::GetInstance().SetWorkingFolder( m_szCreateFolder.GetBuffer() );

	if( CTEtWorld::GetInstance().CreateEmptyWorld() == false ) {
		MessageBox( "���� ����!!", "Error", MB_OK );
		OnOK();
		return;
	}
	if( CFileServer::GetInstance().CreateDefaultFile() == false ) {
		MessageBox( "���� ����!!", "Error", MB_OK );
		OnOK();
		return;
	}
	OnOK();
}

void CCreateProjectDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg(this);
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_szCreateFolder.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_szCreateFolder);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_szCreateFolder = Dlg.GetSelPath();
		UpdateData(FALSE);
	}
}
