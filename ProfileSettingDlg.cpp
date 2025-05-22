// ProfileSettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ProfileSettingDlg.h"
#include "NewProfileDlg.h"
#include "FileServer.h"
#include "NationDlg.h"


// CProfileSettingDlg dialog

IMPLEMENT_DYNAMIC(CProfileSettingDlg, CDialog)

CProfileSettingDlg::CProfileSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProfileSettingDlg::IDD, pParent)
	, m_LocalPath(_T(""))
	, m_LocalResPath(_T(""))
	, m_LoginInfoPath(_T(""))
{
	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_BITMAP7 );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 4, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );
}

CProfileSettingDlg::~CProfileSettingDlg()
{
	SAFE_DELETE( m_pContextMenu );
}

void CProfileSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_BUTTON1, m_NewButton);
	DDX_Control(pDX, IDC_BUTTON2, m_DelButton);
	DDX_Control(pDX, IDC_BUTTON9, m_PropButton);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
	DDX_Text(pDX, IDC_EDIT1, m_LocalPath);
	DDX_Text(pDX, IDC_EDIT4, m_LocalResPath);
	DDX_Text(pDX, IDC_EDIT6, m_LoginInfoPath);
}


BEGIN_MESSAGE_MAP(CProfileSettingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CProfileSettingDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CProfileSettingDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CProfileSettingDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CProfileSettingDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CProfileSettingDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON9, &CProfileSettingDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDOK, &CProfileSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProfileSettingDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CProfileSettingDlg::OnCbnSelchangeCombo1)
	ON_CBN_EDITCHANGE(IDC_COMBO1, &CProfileSettingDlg::OnCbnEditchangeCombo1)
	ON_EN_UPDATE(IDC_EDIT1, &CProfileSettingDlg::OnEnUpdateEdit1)
	ON_EN_UPDATE(IDC_EDIT4, &CProfileSettingDlg::OnEnUpdateEdit4)
	ON_EN_UPDATE(IDC_EDIT6, &CProfileSettingDlg::OnEnUpdateEdit6)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROFILEEDIT_ADDNATION, &CProfileSettingDlg::OnProfileeditAddnation)
	ON_COMMAND(ID_PROFILEEDIT_REMOVENATION, &CProfileSettingDlg::OnProfileeditRemovenation)
	ON_COMMAND(ID_PROFILEEDIT_MODIFYNATION, &CProfileSettingDlg::OnProfileeditModifynation)
END_MESSAGE_MAP()


// CProfileSettingDlg message handlers

BOOL CProfileSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_List.AddColumn( "Nation", 80, LVCFMT_LEFT );
	m_List.AddColumn( "Path", 189, LVCFMT_LEFT );
	m_List.AddColumn( "Resource", 189, LVCFMT_LEFT );
	m_List.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_FLATSB );

	RefreshProfileList( "" );

	m_NewButton.SetBitmap( CSize( 15, 15 ), IDB_BITMAP7 );
	m_DelButton.SetBitmap( CSize( 15, 15 ), IDB_BITMAP8 );
	m_PropButton.SetBitmap( CSize( 15, 15 ), IDB_BITMAP9 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProfileSettingDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CNewProfileDlg Dlg;
	if( Dlg.DoModal() != IDOK ) return;

	CString szProfileName = Dlg.m_szProfileName;
	CFileServer::GetInstance().AddProfile( szProfileName.GetBuffer() );

	RefreshProfileList( szProfileName );

}

void CProfileSettingDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CString szProfileName;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szProfileName );

	CFileServer::GetInstance().RemoveProfile( szProfileName );

	RefreshProfileList( "" );
}

void CProfileSettingDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_LocalPath.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_LocalPath);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_LocalPath = Dlg.GetSelPath();
		UpdateData(FALSE);

		CString szProfileName;
		int nSel = m_Combo.GetCurSel();
		m_Combo.GetLBText( nSel, szProfileName );

		CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
		if( pStruct ) pStruct->szLocalPath = m_LocalPath;

	}
}

void CProfileSettingDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_LocalResPath.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_LocalResPath);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_LocalResPath = Dlg.GetSelPath();
		UpdateData(FALSE);

		CString szProfileName;
		int nSel = m_Combo.GetCurSel();
		m_Combo.GetLBText( nSel, szProfileName );

		CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
		if( pStruct ) pStruct->szResPath = m_LocalResPath;

	}
}

void CProfileSettingDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog Dlg;
	UpdateData(TRUE);
	Dlg.SetTitle(_T("Select Working Project Directory"));
	if( !m_LoginInfoPath.IsEmpty() ) {
		TCHAR path[_MAX_PATH];
		STRCPY_S(path, _MAX_PATH, m_LoginInfoPath);
		Dlg.SetSelPath(path);
	}
	if( Dlg.DoModal() == IDOK ) {
		m_LoginInfoPath = Dlg.GetSelPath();
		UpdateData(FALSE);

		CString szProfileName;
		int nSel = m_Combo.GetCurSel();
		m_Combo.GetLBText( nSel, szProfileName );

		CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
		if( pStruct ) pStruct->szLoginPath = m_LoginInfoPath;

	}
}

void CProfileSettingDlg::RefreshProfileList( const char *szSelectProfileName )
{
	int nItemIndex = -1;
	m_Combo.ResetContent();
	for( DWORD i=0; i<CFileServer::GetInstance().GetProfileCount(); i++ ) {
		CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile(i);
		m_Combo.InsertString( i, pStruct->szProfileName );
		if( _stricmp( pStruct->szProfileName, szSelectProfileName ) == NULL ) {
			nItemIndex = i;
		}
	}
	if( nItemIndex == -1 ) nItemIndex = 0;
	m_Combo.SetCurSel( nItemIndex );
	RefreshProfileInfo();
}

void CProfileSettingDlg::RefreshProfileInfo()
{
	GetDlgItem( IDC_EDIT1 )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON3 )->EnableWindow( FALSE );
	GetDlgItem( IDC_LIST1 )->EnableWindow( FALSE );

	CString szProfileName;
	int nCurSel = m_Combo.GetCurSel();
	if( nCurSel == -1 ) return;
	m_Combo.GetLBText( m_Combo.GetCurSel(), szProfileName );

	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( pStruct == NULL ) {
		m_LocalPath = "";
		m_LocalResPath = "";
		m_LoginInfoPath = "";
		m_List.DeleteAllItems();
		UpdateData( FALSE );
		return;
	}
	GetDlgItem( IDC_EDIT1 )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON3 )->EnableWindow( TRUE );
	GetDlgItem( IDC_LIST1 )->EnableWindow( TRUE );

	m_LocalPath = pStruct->szLocalPath;
	m_LocalResPath = pStruct->szResPath;
	m_LoginInfoPath = pStruct->szLoginPath;


	m_List.DeleteAllItems();
	for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
		CFileServer::NationStruct *pNation = &pStruct->VecNation[i];
		m_List.InsertItem( i, pNation->szNationStr );
		m_List.SetItemText( i, 1, pNation->szNationPath );
		m_List.SetItemText( i, 2, pNation->szNationResPath );
	}

	UpdateData( FALSE );
}

void CProfileSettingDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CFileServer::GetInstance().SaveProfile();

	OnOK();
}

void CProfileSettingDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	//OnCancel();
}

void CProfileSettingDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	RefreshProfileInfo();
}

void CProfileSettingDlg::OnCbnEditchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

void CProfileSettingDlg::OnEnUpdateEdit1()
{
	UpdateData( TRUE );

	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );

	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( pStruct ) pStruct->szLocalPath = m_LocalPath;
}

void CProfileSettingDlg::OnEnUpdateEdit4()
{
	UpdateData( TRUE );

	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );

	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( pStruct ) pStruct->szResPath = m_LocalResPath;
}

void CProfileSettingDlg::OnEnUpdateEdit6()
{
	UpdateData( TRUE );

	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );

	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( pStruct ) pStruct->szLoginPath = m_LoginInfoPath;
}

void CProfileSettingDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
	CRect rcRect;
	m_List.GetClientRect( &rcRect );
	m_List.ClientToScreen( &rcRect );
	CPoint p2 = point;
	ClientToScreen( &p2 );
	if( !( point.x > rcRect.left && point.x < rcRect.right && point.y > rcRect.top && point.y < rcRect.bottom ) ) return;
	CMenu *pMenu = m_pContextMenu->GetSubMenu(8);

	pMenu->EnableMenuItem( ID_PROFILEEDIT_ADDNATION, MF_BYCOMMAND | MF_ENABLED );

	pMenu->EnableMenuItem( ID_PROFILEEDIT_REMOVENATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pMenu->EnableMenuItem( ID_PROFILEEDIT_MODIFYNATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	if( m_List.GetSelectedCount() == 1 ) {
		pMenu->EnableMenuItem( ID_PROFILEEDIT_MODIFYNATION, MF_BYCOMMAND | MF_ENABLED );
	}
	if( m_List.GetSelectedCount() >= 1 ) {
		pMenu->EnableMenuItem( ID_PROFILEEDIT_REMOVENATION, MF_BYCOMMAND | MF_ENABLED );
	}


	pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void CProfileSettingDlg::OnProfileeditAddnation()
{
	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );
	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( !pStruct ) return;

	CNationDlg Dlg( NULL, pStruct );

	CFileServer::NationStruct Nation;
	Dlg.SetNation( &Nation );
	if( Dlg.DoModal() != IDOK ) return;

	pStruct->VecNation.push_back( Nation );

	RefreshProfileInfo();
}

void CProfileSettingDlg::OnProfileeditRemovenation()
{
	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );
	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( !pStruct ) return;

	int nSelectCount = m_List.GetSelectedCount();
	int nItemID;
	CString szStr;
	POSITION p = m_List.GetFirstSelectedItemPosition();
	int nOffset = 0;
	std::vector<int> nVecList;
	for( int i=0; i<nSelectCount; i++ ) {
		nItemID = m_List.GetNextSelectedItem(p);
		szStr = m_List.GetItemText( nItemID, 0 );
		for( DWORD j=0; j<pStruct->VecNation.size(); j++ ) {
			if( _stricmp( pStruct->VecNation[j].szNationStr, szStr ) == NULL ) {
				pStruct->VecNation.erase( pStruct->VecNation.begin() + j );
				break;
			}
		}
		nVecList.push_back(nItemID);
	}
	std::sort( nVecList.begin(), nVecList.end() );
	for( int i=(int)nVecList.size()-1; i>=0; i-- ) {
		m_List.DeleteItem( nVecList[i] );
	}
}

void CProfileSettingDlg::OnProfileeditModifynation()
{
	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );
	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( !pStruct ) return;

	int nSelectCount = m_List.GetSelectedCount();
	if( nSelectCount < 1 ) return;

	int nItemID;
	CString szStr;
	POSITION p = m_List.GetFirstSelectedItemPosition();
	for( int i=0; i<nSelectCount; i++ ) {
		nItemID = m_List.GetNextSelectedItem(p);
		szStr = m_List.GetItemText( nItemID, 0 );
		break;
	}

	CFileServer::NationStruct *pNation = NULL;
	for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
		if( _stricmp( pStruct->VecNation[i].szNationStr, szStr ) == NULL ) {
			pNation = &pStruct->VecNation[i];
			break;
		}
	}

	if( pNation == NULL ) return;
	CNationDlg Dlg( NULL, pStruct );

	Dlg.SetNation( pNation );
	if( Dlg.DoModal() != IDOK ) return;

	RefreshProfileInfo();
}

void CProfileSettingDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	CNewProfileDlg Dlg;

	CString szProfileName;
	int nSel = m_Combo.GetCurSel();
	m_Combo.GetLBText( nSel, szProfileName );
	Dlg.SetDefaultProfileName( szProfileName );

	if( Dlg.DoModal() != IDOK ) return;

	CFileServer::ProfileStruct *pStruct = CFileServer::GetInstance().GetProfile( szProfileName );
	if( !pStruct ) return;
	pStruct->szProfileName = Dlg.m_szProfileName;

	RefreshProfileList( szProfileName );
}

