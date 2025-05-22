// CreateGridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreateGridDlg.h"
#include "TEtWorld.h"
#include "FileServer.h"


// CCreateGridDlg dialog

IMPLEMENT_DYNAMIC(CCreateGridDlg, CDialog)

CCreateGridDlg::CCreateGridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateGridDlg::IDD, pParent)
	, m_dwSizeX(10)
	, m_dwSizeY(10)
	, m_dwWidth(2000)
	, m_dwHeight(2000)
	, m_dwTileSize(200)
	, m_szGridName(_T(""))
{

}

CCreateGridDlg::~CCreateGridDlg()
{
}

void CCreateGridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_dwSizeX);
	DDX_Text(pDX, IDC_EDIT4, m_dwSizeY);
	DDX_Text(pDX, IDC_EDIT5, m_dwWidth);
	DDX_Text(pDX, IDC_EDIT6, m_dwHeight);
	DDX_Text(pDX, IDC_EDIT8, m_dwTileSize);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
	DDX_Text(pDX, IDC_EDIT2, m_szGridName);
}


BEGIN_MESSAGE_MAP(CCreateGridDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CCreateGridDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CCreateGridDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCreateGridDlg message handlers

void CCreateGridDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

void CCreateGridDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData( TRUE );
	CString szParent;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szParent );

	if( m_szGridName.IsEmpty() ) {
		MessageBox( "이름을 넣어주세요", "Error", MB_OK );
		return;
	}
	if( strcmp( m_szGridName, "Root" ) == NULL ) {
		MessageBox( "쓸 수 없는 이름입니다.", "Error", MB_OK );
		return;
	}
	if( m_szGridName.GetLength() >= 64 ) {
		MessageBox( "64자를 넘을 수 없습니다.", "Error", MB_OK );
		return;
	}

	bool bResult = true;
	char szTemp[64] = { 0, };
	sprintf_s( szTemp, m_szGridName );
	for( DWORD i=0; i<(DWORD)m_szGridName.GetLength(); i++ ) {
		if( szTemp[i] == ' ' || 
			szTemp[i] == '\\' ||
			szTemp[i] == ':' ||
			szTemp[i] == '*' ||
			szTemp[i] == '?' ||
			szTemp[i] == '"' ||
			szTemp[i] == '<' ||
			szTemp[i] == '>' ||
			szTemp[i] == '|' ) bResult = false;
	}
	if( bResult == false ) {
		MessageBox( "파일 이름에 사용할 수 없는 문자가 있습니다.", "Error", MB_OK );
		return;
	}

	if( CTEtWorld::GetInstance().IsExistGrid( m_szGridName ) == true ) {
		MessageBox( "같은 이름의 그리드가 있습니다.", "Error", MB_OK );
		return;
	}
	if( CTEtWorld::GetInstance().IsExistGrid( szParent ) == false ) {
		MessageBox( "부모가 없습니다.", "Error", MB_OK );
		return;
	}

	if( CFileServer::GetInstance().CreateGridDefaultFile( m_szGridName, m_dwSizeX, m_dwSizeY, m_dwWidth, m_dwHeight ) == false ) {
		CTEtWorld::GetInstance().DestroyGrid( m_szGridName );
		MessageBox( "생성 실패!!\n폴더에 같은 이름이 있거나 파일을 생성할 수 없습니다.", "Error", MB_OK );
		OnOK();
		return;
	}

	if( CTEtWorld::GetInstance().CreateEmptyGrid( m_szGridName, szParent, m_dwSizeX, m_dwSizeY, m_dwWidth, m_dwHeight, m_dwTileSize ) == false )  {
		MessageBox( "생성 실패!!", "Error", MB_OK );
		OnOK();
		return;
	}
	OnOK();
}

BOOL CCreateGridDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CEtWorldGrid *pGrid = CTEtWorld::GetInstance().GetGrid();
	InsertParentName( pGrid );

	CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
	if( szGridName.IsEmpty() ) szGridName = "Root";
	int nSelectIndex = m_Combo.FindString( -1, szGridName );
	if( nSelectIndex == -1 ) nSelectIndex = m_Combo.FindString( -1, "Root" );
	if( nSelectIndex != -1 ) {
		m_Combo.SetCurSel( nSelectIndex );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCreateGridDlg::InsertParentName( CEtWorldGrid *pGrid )
{
	m_Combo.AddString( pGrid->GetName() );


	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		InsertParentName( pGrid->GetChildFromIndex(i) );
	}
}
