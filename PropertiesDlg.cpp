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

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_szShaderFolder = CRenderBase::GetInstance().GetShaderFolder();
	m_szViewerLocation = CGlobalValue::GetInstance().m_szViewerLocation;
	m_szActionLocation = CGlobalValue::GetInstance().m_szActionLocation;

	UpdateData( FALSE );

	if( m_bDisableCancel == true )
		GetDlgItem( IDCANCEL )->EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CPropertiesDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData();
	if( m_szShaderFolder.IsEmpty() ) {
		MessageBox( "������ ������ �ּ���", "����", MB_OK );
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( m_bDisableCancel == true ) return;
	OnCancel();
}

void CPropertiesDlg::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	TCHAR szFilter[] = _T( "Execute File(*.exe)|*.exe|All Files (*.*)|*.*||" );
	CFileDialog Dlg(TRUE, _T("exe"), _T("*.exe"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	UpdateData(TRUE);
//	Dlg.SetWindowText(_T("Select EtActionTool Execute File"));

	if( Dlg.DoModal() == IDOK ) {
		m_szActionLocation = Dlg.GetPathName();
		UpdateData(FALSE);
	}
}