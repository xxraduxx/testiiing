// DeleteGridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "DeleteGridDlg.h"
#include "TEtWorld.h"


// CDeleteGridDlg dialog

IMPLEMENT_DYNAMIC(CDeleteGridDlg, CDialog)

CDeleteGridDlg::CDeleteGridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteGridDlg::IDD, pParent)
{

}

CDeleteGridDlg::~CDeleteGridDlg()
{
}

void CDeleteGridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
}


BEGIN_MESSAGE_MAP(CDeleteGridDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDeleteGridDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDeleteGridDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDeleteGridDlg message handlers

BOOL CDeleteGridDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	InsertGridName( CTEtWorld::GetInstance().GetGrid() );

	CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
	if( szGridName.IsEmpty() ) szGridName = "Root";
	int nSelectIndex = m_Combo.FindString( -1, szGridName );
	if( nSelectIndex != -1 ) {
		m_Combo.SetCurSel( nSelectIndex );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteGridDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString szStr;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szStr );
	if( CTEtWorld::GetInstance().IsDestroyGrid( szStr ) == false ) {
		MessageBox( "삭제할 수 없는 그리드입니다.\n누군가 사용중입니다.", "Error", MB_OK );
		return;
	}
	if( CTEtWorld::GetInstance().DestroyGrid( szStr ) == false ) {
		MessageBox( "그리드 삭제를 실패했습니다.", "Error", MB_OK );
	}
	OnOK();
}

void CDeleteGridDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CDeleteGridDlg::InsertGridName( CEtWorldGrid *pGrid )
{
	if( strcmp( pGrid->GetName(), "Root" ) != NULL )
		m_Combo.InsertString( m_Combo.GetCount(), pGrid->GetName() );
	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		InsertGridName( pGrid->GetChildFromIndex(i) );
	}
}