// ConnectSettingDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ConnectSettingDlg.h"
#include "ProfileSettingDlg.h"


// CConnectSettingDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CConnectSettingDlg, CDialog)

CConnectSettingDlg::CConnectSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectSettingDlg::IDD, pParent)
	, m_szUserName(_T(""))
	, m_szLocalPath(_T(""))
{
	m_bCreateEmptyProject = false;
}

CConnectSettingDlg::~CConnectSettingDlg()
{
}

void CConnectSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT4, m_szUserName);
	DDX_Text(pDX, IDC_EDIT6, m_szLocalPath);
	DDX_Control(pDX, IDC_CHECK1, m_CreateProject);
	DDX_Control(pDX, IDC_COMBO1, m_ProfileCombo);
}


BEGIN_MESSAGE_MAP(CConnectSettingDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CConnectSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CConnectSettingDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_RADIO1, &CConnectSettingDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CConnectSettingDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_CHECK1, &CConnectSettingDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON10, &CConnectSettingDlg::OnBnClickedButton10)
END_MESSAGE_MAP()


// CConnectSettingDlg 메시지 처리기입니다.

void CConnectSettingDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	CButton *pButton = (CButton *)GetDlgItem( IDC_RADIO1 );
	if( pButton->GetCheck() == BST_CHECKED ) 
		CFileServer::GetInstance().SetServerType( CFileServer::NetworkAccess );

	switch( CFileServer::GetInstance().GetServerType() ) {
		case CFileServer::NetworkAccess:
			{
//				CFileServer::GetInstance().SetWorkingFolder( m_szFolderName.GetBuffer() );
				int nCurSel = m_ProfileCombo.GetCurSel();
				CString szProfileName;
				m_ProfileCombo.GetLBText( nCurSel, szProfileName );
				CFileServer::GetInstance().SetUseProfile( szProfileName.GetBuffer() );
				CFileServer::GetInstance().SetUserName( m_szUserName.GetBuffer() );
			}
			break;
	}
	m_bCreateEmptyProject = ( m_CreateProject.GetCheck() == BST_CHECKED ) ? true : false;

	if( !m_bCreateEmptyProject && !CFileServer::GetInstance().IsCanConnectSetting() ) {
		MessageBox( "Connect 설정이 올바르지 않습니다.", "Error", MB_OK );
		return;
	}

	OnOK();
}

void CConnectSettingDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

BOOL CConnectSettingDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	switch( CFileServer::GetInstance().GetServerType() ) {
		case CFileServer::NetworkAccess:
			{
				RefreshProfileList();
				CString szStr = CFileServer::GetInstance().GetUseProfile();
				CString szTemp;
				for( int i=0; i<m_ProfileCombo.GetCount(); i++ ) {
					m_ProfileCombo.GetLBText( i, szTemp );
					if( _stricmp( szTemp, szStr ) == NULL ) {
						m_ProfileCombo.SetCurSel(i);
						break;
					}
				}
				m_szUserName = CFileServer::GetInstance().GetUserName();
			}
			break;
	}

	GetDlgItem( IDC_RADIO2 )->EnableWindow( FALSE );

	CButton *pButton;
	switch( CFileServer::GetInstance().GetServerType() ) {
		case CFileServer::NetworkAccess: pButton = (CButton *)GetDlgItem( IDC_RADIO1 ); break;
	}
	pButton->SetCheck( TRUE );

	RefreshEnableControl( CFileServer::GetInstance().GetServerType() );
	UpdateData( FALSE );

	/*
	// http://eye-ad:8080/svn/sandbox
	const svn_version_t *pVersion = svn_client_version();
	if( pVersion->major < 1 || pVersion->minor < 5 || pVersion->patch < 4 ) {
		GetDlgItem( IDC_RADIO2 )->EnableWindow( FALSE );
	}

	svn_repos_t* pRepos;
	apr_pool_t *parentpool = svn_pool_create(NULL);
	svn_utf_initialize(parentpool);
	const char * deststr = NULL;
	svn_utf_cstring_to_utf8(&deststr, "dummy", parentpool);
	svn_utf_cstring_from_utf8(&deststr, "dummy", parentpool);

	svn_error_t *Err = svn_config_ensure(NULL, parentpool);
	apr_pool_t *pool = svn_pool_create (parentpool);

//	svn_repos_open( &pRepos, "http://eye-ad:8080/svn", pPool );
	*/
//	SVN_AUTH_PARAM_NO_AUTH_CACHE();


	CRect rcRect;
	GetWindowRect( &rcRect );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CConnectSettingDlg::RefreshEnableControl( CFileServer::ServerTypeEnum Type )
{
	switch( Type ) {
		case CFileServer::NetworkAccess:
			GetDlgItem( IDC_EDIT5 )->EnableWindow( FALSE );
			GetDlgItem( IDC_EDIT6 )->EnableWindow( FALSE );
			GetDlgItem( IDC_EDIT8 )->EnableWindow( FALSE );
			GetDlgItem( IDC_EDIT10 )->EnableWindow( FALSE );
			GetDlgItem( IDC_BUTTON2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_BUTTON9 )->EnableWindow( FALSE );

			GetDlgItem( IDC_COMBO1 )->EnableWindow( TRUE );
			GetDlgItem( IDC_EDIT4 )->EnableWindow( TRUE);
			GetDlgItem( IDC_BUTTON10 )->EnableWindow( TRUE );
			break;
	}
}

void CConnectSettingDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	RefreshEnableControl( CFileServer::NetworkAccess );
}

void CConnectSettingDlg::OnBnClickedRadio2()
{
}

void CConnectSettingDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
}

void CConnectSettingDlg::OnBnClickedButton10()
{
	CProfileSettingDlg Dlg;
	if( Dlg.DoModal() != IDOK ) return;
	CString szPrevProfile;
	int nCurSel = m_ProfileCombo.GetCurSel();

	RefreshProfileList();

	if( nCurSel == -1 ) nCurSel = 0;
	if( nCurSel >= m_ProfileCombo.GetCount() ) nCurSel = m_ProfileCombo.GetCount() - 1;
	m_ProfileCombo.SetCurSel( nCurSel );
}

void CConnectSettingDlg::RefreshProfileList()
{
	m_ProfileCombo.ResetContent();
	for( DWORD i=0; i<CFileServer::GetInstance().GetProfileCount(); i++ ) {
		CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile(i);
		m_ProfileCombo.InsertString( i, pStruct->szProfileName );
	}
}