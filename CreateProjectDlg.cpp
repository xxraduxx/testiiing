// CreateProejctDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreateProjectDlg.h"
#include "FileServer.h"
#include "TEtWorld.h"


// CCreateProjectDlg 대화 상자입니다.

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


// CCreateProjectDlg 메시지 처리기입니다.

void CCreateProjectDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

void CCreateProjectDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData( TRUE );
	CFileServer::GetInstance().SetWorkingFolder( m_szCreateFolder.GetBuffer() );

	if( CTEtWorld::GetInstance().CreateEmptyWorld() == false ) {
		MessageBox( "생성 실패!!", "Error", MB_OK );
		OnOK();
		return;
	}
	if( CFileServer::GetInstance().CreateDefaultFile() == false ) {
		MessageBox( "생성 실패!!", "Error", MB_OK );
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
