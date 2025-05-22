// MoveToGridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "MoveToGridDlg.h"
#include "TEtWorld.h"


// CMoveToGridDlg dialog

IMPLEMENT_DYNAMIC(CMoveToGridDlg, CDialog)

CMoveToGridDlg::CMoveToGridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveToGridDlg::IDD, pParent)
{

}

CMoveToGridDlg::~CMoveToGridDlg()
{
}

void CMoveToGridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_OrigCombo);
	DDX_Control(pDX, IDC_COMBO2, m_TarCombo);
	DDX_Control(pDX, IDC_COMBO3, m_SelCombo);
}


BEGIN_MESSAGE_MAP(CMoveToGridDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMoveToGridDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CMoveToGridDlg::OnCbnSelchangeCombo3)
END_MESSAGE_MAP()


// CMoveToGridDlg message handlers

BOOL CMoveToGridDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_OrigCombo.EnableWindow( FALSE );
	// TODO:  Add extra initialization here
	m_SelCombo.ResetContent();
	m_OrigCombo.ResetContent();
	m_TarCombo.ResetContent();
	InsertGridName( CTEtWorld::GetInstance().GetGrid(), m_SelCombo );
	InsertGridName( CTEtWorld::GetInstance().GetGrid(), m_OrigCombo );
	InsertGridName( CTEtWorld::GetInstance().GetGrid(), m_TarCombo );

	int nIndex = m_SelCombo.FindString( -1, "Root" );
	if( nIndex != -1 ) m_SelCombo.DeleteString( nIndex );

	CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
	if( szGridName.IsEmpty() ) {
		if( m_SelCombo.GetCount() == 0 ) {
			MessageBox( "이동시킬 그리드가 없습니다.", "에러", MB_OK );
			return FALSE;
		}
		m_SelCombo.GetLBText( 0, szGridName );
	}

	CEtWorldGrid *pGrid = CTEtWorld::GetInstance().FindGrid( szGridName );
	nIndex = m_SelCombo.FindString( -1, szGridName );
	if( nIndex != -1 ) m_SelCombo.SetCurSel( nIndex );

	nIndex = m_OrigCombo.FindString( -1, pGrid->GetParent()->GetName() );
	if( nIndex != -1 ) m_OrigCombo.SetCurSel( nIndex );

	nIndex = m_TarCombo.FindString( -1, szGridName );
	if( nIndex != -1 ) m_TarCombo.DeleteString( nIndex );
	nIndex = m_TarCombo.FindString( -1, pGrid->GetParent()->GetName() );
	if( nIndex != -1 ) m_TarCombo.SetCurSel( nIndex );

	
	CString szStr;
	for( int i=0; i<m_TarCombo.GetCount(); i++ ) {
		m_TarCombo.GetLBText( i, szStr );
		pGrid = CTEtWorld::GetInstance().FindGrid( szStr );
		if( IsExistParent( pGrid, szGridName ) == true ) {
			m_TarCombo.DeleteString( i );
			i--;
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool CMoveToGridDlg::IsExistParent( CEtWorldGrid *pGrid, CString &szGridName )
{
	if( strcmp( pGrid->GetName(), szGridName ) == NULL ) return true;
	if( pGrid->GetParent() )
		return IsExistParent( pGrid->GetParent(), szGridName );
	return false;
}


void CMoveToGridDlg::InsertGridName( CEtWorldGrid *pGrid, CComboBox &ComboBox )
{
	ComboBox.AddString( pGrid->GetName() );

	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		InsertGridName( pGrid->GetChildFromIndex(i), ComboBox );
	}
}
void CMoveToGridDlg::OnBnClickedOk()
{
	CString szGridName;
	CString szParentName;
	m_SelCombo.GetLBText( m_SelCombo.GetCurSel(), szGridName );
	m_TarCombo.GetLBText( m_TarCombo.GetCurSel(), szParentName );

	if( CTEtWorld::GetInstance().MoveGrid( szGridName, szParentName ) == false ) {
		MessageBox( "그리드 이동이 실패하였습니다.", "Error", MB_OK );
	}

	OnOK();
}

void CMoveToGridDlg::OnCbnSelchangeCombo3()
{
	// TODO: Add your control notification handler code here
	m_TarCombo.ResetContent();
	InsertGridName( CTEtWorld::GetInstance().GetGrid(), m_TarCombo );

	
	CString szGridName;
	m_SelCombo.GetLBText( m_SelCombo.GetCurSel(), szGridName );
	if( szGridName.IsEmpty() ) return;


	CEtWorldGrid *pGrid = CTEtWorld::GetInstance().FindGrid( szGridName );

	int nIndex = m_OrigCombo.FindString( -1, pGrid->GetParent()->GetName() );
	if( nIndex != -1 ) m_OrigCombo.SetCurSel( nIndex );

	nIndex = m_TarCombo.FindString( -1, szGridName );
	if( nIndex != -1 ) m_TarCombo.DeleteString( nIndex );
	nIndex = m_TarCombo.FindString( -1, pGrid->GetParent()->GetName() );
	if( nIndex != -1 ) m_TarCombo.SetCurSel( nIndex );


	CString szStr;
	for( int i=0; i<m_TarCombo.GetCount(); i++ ) {
		m_TarCombo.GetLBText( i, szStr );
		pGrid = CTEtWorld::GetInstance().FindGrid( szStr );
		if( IsExistParent( pGrid, szGridName ) == true ) {
			m_TarCombo.DeleteString( i );
			i--;
		}
	}
}
