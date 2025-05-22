// PropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropertiesDlg.h"
#include "RenderBase.h"


// CPropertiesDlg dialog

IMPLEMENT_DYNAMIC(CPropertiesDlg, CDialog)

CPropertiesDlg::CPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPropertiesDlg::IDD, pParent)
	, m_szShaderFolder(_T(""))
{
	m_bDisableCancel = false;
}

CPropertiesDlg::~CPropertiesDlg()
{
}

void CPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDIT1, m_szShaderFolder);
	DDX_Text(pDX, IDC_EDIT5, m_szViewerLocation);
	DDX_Text(pDX, IDC_EDIT6, m_szActionLocation);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPropertiesDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPropertiesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPropertiesDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CPropertiesDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON9, &CPropertiesDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CPropertiesDlg::OnBnClickedButton10)
END_MESSAGE_MAP()


// CPropertiesDlg message handlers

BOOL CPropertiesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_szShaderFolder = CRenderBase::GetInstance().GetShaderFolder();
	m_szViewerLocation = CGlobalValue::GetInstance().m_szViewerLocation;
	m_szActionLocation = CGlobalValue::GetInstance().m_szActionLocation;

	UpdateData( FALSE );

	if( m_bDisableCancel == true )
		GetDlgItem( IDCANCEL )->EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CPropertiesDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
	if( m_szShaderFolder.IsEmpty() ) {
		MessageBox( "폴더를 설정해 주세요", "에러", MB_OK );
		return;
	}

	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ShaderFolder", m_szShaderFolder.GetBuffer() );
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ViewerLocation", m_szViewerLocation.GetBuffer() );
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ActionLocation", m_szActionLocation.GetBuffer() );

	if( CRenderBase::GetInstance().GetShaderFolder() != m_szShaderFolder )
		CRenderBase::GetInstance().ChangeShaderFolder( m_szShaderFolder );

	CGlobalValue::GetInstance().m_szViewerLocation = m_szViewerLocation;
	CGlobalValue::GetInstance().m_szActionLocation = m_szActionLocation;

	OnOK();
}

void CPropertiesDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_bDisableCancel == true ) return;
	OnCancel();
}

void CPropertiesDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Shader Directory"));
	if( !m_szShaderFolder.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_szShaderFolder);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_szShaderFolder = Dlg.GetSelPath();
		UpdateData(FALSE);
	}
}

void CPropertiesDlg::OnBnClickedButton9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR szFilter[] = _T( "Execute File(*.exe)|*.exe|All Files (*.*)|*.*||" );
	CFileDialog Dlg(TRUE, _T("exe"), _T("*.exe"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	UpdateData(TRUE);
//	Dlg.SetWindowText(_T("Select EtViewer Execute File"));

	if( Dlg.DoModal() == IDOK ) {
		m_szViewerLocation = Dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CPropertiesDlg::OnBnClickedButton10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR szFilter[] = _T( "Execute File(*.exe)|*.exe|All Files (*.*)|*.*||" );
	CFileDialog Dlg(TRUE, _T("exe"), _T("*.exe"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	UpdateData(TRUE);
//	Dlg.SetWindowText(_T("Select EtActionTool Execute File"));

	if( Dlg.DoModal() == IDOK ) {
		m_szActionLocation = Dlg.GetPathName();
		UpdateData(FALSE);
	}
}