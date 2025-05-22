// ManagerExplorerPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ManagerExplorerPaneView.h"
#include "UserMessage.h"
#include "FileServer.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include <Direct.h>


// CManagerExplorerPaneView

IMPLEMENT_DYNCREATE(CManagerExplorerPaneView, CFormView)

CManagerExplorerPaneView::CManagerExplorerPaneView()
	: CFormView(CManagerExplorerPaneView::IDD)
{
	m_bActivate = false;
	m_hRootTree = NULL;
	m_hNationRootTree = NULL;
	m_nCurrentManager = 0;

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_BITMAP2 );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 2, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
}

CManagerExplorerPaneView::~CManagerExplorerPaneView()
{
	SAFE_DELETE( m_pContextMenu );
}

void CManagerExplorerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
}

BEGIN_MESSAGE_MAP(CManagerExplorerPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CManagerExplorerPaneView::OnTvnSelchangedTree1)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MANAGEREXPLORER_CREATEFOLDER, &CManagerExplorerPaneView::OnManagerexplorerCreatefolder)
	ON_COMMAND(ID_MANAGEREXPLORER_DELETEFOLDER, &CManagerExplorerPaneView::OnManagerexplorerDeletefolder)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, &CManagerExplorerPaneView::OnTvnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, &CManagerExplorerPaneView::OnTvnEndlabeleditTree1)
END_MESSAGE_MAP()


// CManagerExplorerPaneView 진단입니다.

#ifdef _DEBUG
void CManagerExplorerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CManagerExplorerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CManagerExplorerPaneView 메시지 처리기입니다.

void CManagerExplorerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

}

BOOL CManagerExplorerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return FALSE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CManagerExplorerPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_TreeCtrl ) {
		m_TreeCtrl.MoveWindow( 0, 0, cx, cy );
	}
}

LRESULT CManagerExplorerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	SAFE_DELETE_VEC( m_VecDirList );
	if( !m_TreeCtrl ) return S_OK;
	m_TreeCtrl.DeleteAllItems();
	m_TreeCtrl.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_TreeCtrl.EnableMultiSelect( true );
	m_nCurrentManager = (int)wParam;
	if( wParam == NULL ) return S_OK;
	if( !CFileServer::GetInstance().IsConnect() ) return S_OK;

	CString szFolder;

	m_hRootTree = m_TreeCtrl.InsertItem( "Server", 0, 1 );

	if( CFileServer::GetInstance().IsLockNation() ) {
		CString szRootName;
		szRootName.Format( "Server - %s", CFileServer::GetInstance().GetLockNation() );
		m_hNationRootTree = m_TreeCtrl.InsertItem( szRootName, 0, 1 );

		szFolder = GetCurrentDefaultNationFolder();
		RefreshFolder( szFolder, m_hNationRootTree );
	}

	szFolder = GetCurrentDefaultFolder();
	RefreshFolder( szFolder, m_hRootTree );

	m_TreeCtrl.SelectItem( m_hRootTree );
	m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
	m_TreeCtrl.Expand( m_hNationRootTree, TVE_EXPAND );
	/*
	CWnd *pWnd = GetCurrentWnd();
	if( pWnd ) pWnd->SendMessage( UM_EXPLORER_CHANGE_FOLOR, NULL );
	*/

	Invalidate();

	return S_OK;
}

void CManagerExplorerPaneView::CalcSelectItem()
{
	m_SelectItemList.RemoveAll();
	m_TreeCtrl.GetSelectedList( m_SelectItemList );
}

bool CManagerExplorerPaneView::CheckIgnoreFolder( char *szName )
{
	switch( m_nCurrentManager ) {
		case 1:	// Texture Manager
			if( _stricmp( szName, "Server" ) == NULL ) return false;
			if( _stricmp( szName, "Grass" ) == NULL ) return false;
			if( _stricmp( szName, "Water" ) == NULL ) return false;
			if( _stricmp( szName, "Decal" ) == NULL ) return false;
			if( strstr( szName, "Server - " ) ) return false;
			break;
		case 2:	// Skin Manager( Prop )
			if( _stricmp( szName, "Server" ) == NULL ) return false;
			if( _stricmp( szName, "Light" ) == NULL ) return false;
			if( _stricmp( szName, "Camera" ) == NULL ) return false;
			if( strstr( szName, "Server - " ) ) return false;
			break;
		case 3:	// Sound Manager
			if( _stricmp( szName, "Server" ) == NULL ) return false;
			if( strstr( szName, "Server - " ) ) return false;
			break;
	}
	return true;
}

void CManagerExplorerPaneView::RefreshFolder( CString &szFolder, HTREEITEM hParentItem )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char szFindFile[512];

	sprintf_s( szFindFile, "%s\\*.*", szFolder );

	hFind = FindFirstFile( szFindFile, &FindFileData );
	while( hFind != INVALID_HANDLE_VALUE ) {
		if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if( strcmp( FindFileData.cFileName, "." ) != NULL && strcmp( FindFileData.cFileName, ".." ) != NULL && FindFileData.cFileName[0] != '.' ) {
				bool bExist = false;
				for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
					if( m_VecDirList[i].hParent == hParentItem ) {
						CString szStr = m_TreeCtrl.GetItemText( m_VecDirList[i].hItem );
						if( _stricmp( szStr, FindFileData.cFileName ) == NULL ) {
							szStr.Format( "%s\\%s", szFolder, FindFileData.cFileName );
							RefreshFolder( szStr, m_VecDirList[i].hItem );

							bExist = true;
							break;
						}
					}
				}
				if( !bExist ) {
					DirectoryStruct Struct;
					Struct.szFullPath.Format( "%s\\%s", szFolder, FindFileData.cFileName );
					Struct.hItem = m_TreeCtrl.InsertItem( FindFileData.cFileName, 0, 1, hParentItem );
					Struct.hParent = hParentItem;
					m_VecDirList.push_back( Struct );

					RefreshFolder( Struct.szFullPath, Struct.hItem );
				}
			}
		}

		if( !FindNextFile( hFind, &FindFileData ) ) break;
	}

	FindClose( hFind );
}
void CManagerExplorerPaneView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CalcSelectItem();
	if( m_SelectItemList.GetCount() == 0 ) {
	}
	else if( m_SelectItemList.GetCount() == 1 ) {
		CString szFullPath;
		for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
			if( m_VecDirList[i].hItem == m_SelectItemList.GetHead() ) {
				szFullPath = m_VecDirList[i].szFullPath;
				break;
			}
		}
		if( szFullPath.IsEmpty() ) {
			if( m_SelectItemList.GetHead() == m_hRootTree )
				szFullPath = GetCurrentDefaultFolder();
			else if( m_SelectItemList.GetHead() == m_hNationRootTree )
				szFullPath = GetCurrentDefaultNationFolder();
		}

		CWnd *pWnd = GetCurrentWnd();
		if( pWnd ) pWnd->SendMessage( UM_EXPLORER_CHANGE_FOLDER, (WPARAM)szFullPath.GetBuffer() );

	}
	else {}


	*pResult = 0;
}

CWnd *CManagerExplorerPaneView::GetCurrentWnd()
{
	switch( m_nCurrentManager ) {
		case 1: return GetPaneWnd( TILEMNG_PANE );	break;
		case 2: return GetPaneWnd( SKINMNG_PANE );	break;
		case 3:	return GetPaneWnd( SOUNDMNG_PANE );	break;
	}
	return NULL;
}

CString CManagerExplorerPaneView::GetCurrentDefaultFolder()
{
	CString szStr;
	switch( m_nCurrentManager ) {
		case 1: szStr.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetLocalWorkingFolder() ); break;
		case 2: szStr.Format( "%s\\Resource\\Prop", CFileServer::GetInstance().GetLocalWorkingFolder() ); break;
		case 3: szStr.Format( "%s\\Resource\\Sound", CFileServer::GetInstance().GetLocalWorkingFolder() ); break;
	}
	return szStr;
}

CString CManagerExplorerPaneView::GetCurrentDefaultNationFolder()
{
	CString szStr;
	switch( m_nCurrentManager ) {
		case 1: szStr.Format( "%s\\Resource\\Tile", CFileServer::GetInstance().GetWorkingFolder() ); break;
		case 2: szStr.Format( "%s\\Resource\\Prop", CFileServer::GetInstance().GetWorkingFolder() ); break;
		case 3: szStr.Format( "%s\\Resource\\Sound", CFileServer::GetInstance().GetWorkingFolder() ); break;
	}
	return szStr;
}

void CManagerExplorerPaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( CFileServer::GetInstance().IsConnect() == false ) return;
	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(1);

	pSubMenu->EnableMenuItem( ID_MANAGEREXPLORER_CREATEFOLDER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_MANAGEREXPLORER_DELETEFOLDER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	
	CalcSelectItem();
	if( m_SelectItemList.GetCount() == 1 ) {
		if( m_SelectItemList.GetHead() == m_hRootTree || m_SelectItemList.GetHead() == m_hNationRootTree ) {
			pSubMenu->EnableMenuItem( ID_MANAGEREXPLORER_CREATEFOLDER, MF_BYCOMMAND | MF_ENABLED );
		}
		else {
			pSubMenu->EnableMenuItem( ID_MANAGEREXPLORER_CREATEFOLDER, MF_BYCOMMAND | MF_ENABLED );
			pSubMenu->EnableMenuItem( ID_MANAGEREXPLORER_DELETEFOLDER, MF_BYCOMMAND | MF_ENABLED );
		}
	}

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void CManagerExplorerPaneView::OnManagerexplorerCreatefolder()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CalcSelectItem();
	CString szFolder;
	for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
		if( m_VecDirList[i].hItem == m_SelectItemList.GetHead() ) {
			szFolder = m_VecDirList[i].szFullPath;
			break;
		}
	}
	if( szFolder.IsEmpty() ) szFolder = GetCurrentDefaultFolder();

	CString szStr;
	std::vector<CString> szVecFolderList;
	GetFolderName( szFolder, szVecFolderList );

	int i = 0;
	while(1) {
		szStr.Format( "Group%03d", i );
		bool bExist = false;
		for( DWORD j=0; j<szVecFolderList.size(); j++ ) {
			if( _stricmp( szStr, szVecFolderList[j] ) == NULL ) {
				bExist = true;
				break;
			}
		}
		if( bExist == false ) break;
		i++;
	}
	CString szResult;
	szResult.Format( "%s\\%s", szFolder, szStr );
	if( _mkdir( szResult ) != 0 ) {
		MessageBox( "에러", "폴더 만들기 필패!!", MB_OK );
		return;
	}
	DirectoryStruct Struct;
	Struct.hParent = m_SelectItemList.GetHead();
	Struct.hItem = m_TreeCtrl.InsertItem( szStr, 0, 1, m_SelectItemList.GetHead() );
	Struct.szFullPath = szResult;
	m_VecDirList.push_back( Struct );

	m_TreeCtrl.SelectItems( Struct.hItem, Struct.hItem, TRUE );
//	m_TreeCtrl.SelectItem( Struct.hItem );
	m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
	m_TreeCtrl.Expand( m_hNationRootTree, TVE_EXPAND );
}

void CManagerExplorerPaneView::OnManagerexplorerDeletefolder()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_SelectItemList.GetHead() == m_hRootTree ) return;
	if( m_SelectItemList.GetHead() == m_hNationRootTree ) return;
	CString szFullPath, szParentPath;
	HTREEITEM hCurrent, hParent;
	int nIndex = -1;
	for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
		if( m_VecDirList[i].hItem == m_SelectItemList.GetHead() ) {
			szFullPath = m_VecDirList[i].szFullPath;
			hCurrent = m_VecDirList[i].hItem;
			hParent = m_TreeCtrl.GetParentItem( m_VecDirList[i].hItem );
			nIndex = (int)i;
			break;
		}
	}
	if( nIndex == -1 ) return;
	if( szFullPath.IsEmpty() ) return;
	if( CheckIgnoreFolder( m_TreeCtrl.GetItemText( m_VecDirList[nIndex].hItem ).GetBuffer() ) == false ) return;

	for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
		if( m_VecDirList[i].hItem == hParent ) {
			szParentPath = m_VecDirList[i].szFullPath;
			break;
		}
	}
	if( szParentPath.IsEmpty() ) szParentPath = GetCurrentDefaultFolder();

	CString szStr;
	szStr.Format( "%s\\*.*", szFullPath );

	char szSorPath[512] = { 0, };
	char szSorExt[256] = { 0, };

	_GetPath( szSorPath, _countof(szSorPath), szStr );
	_GetFullFileName( szSorExt, _countof(szSorExt), szStr );

	std::vector<std::string> szVecList;
	FindFileListInDirectory( szSorPath, szSorExt, szVecList, false );

	char szTemp1[512] = { 0, }; 
	char szTemp2[512] = { 0, };
	bool bResult = true;
	CWnd *pWnd = GetCurrentWnd();
	for( DWORD i=0; i<szVecList.size(); i++ ) {
		sprintf_s( szTemp1, "%s%s", szSorPath, szVecList[i].c_str() );
		sprintf_s( szTemp2, "%s\\%s", szParentPath, szVecList[i].c_str() );
		if( CopyFile( szTemp1, szTemp2, TRUE ) == false ) bResult = false;
		else {
			if( pWnd ) pWnd->SendMessage( UM_EXPLORER_MOVE_FILE, (WPARAM)szTemp1, (LPARAM)szTemp2 );
			DeleteFile( szTemp1 );
		}
	}

	if( bResult == false ) {
		MessageBox( "하위 폴더에 같은 이름의 파일이 있거나 원본 파일을 지울 수 없습니다.\n폴더 삭제에 실패하였습니다.", "에러", MB_OK );
	}
	else {
		if( _rmdir( szFullPath ) != 0 ) {
			MessageBox( "폴더 삭제에 실패하였습니다.", "에러", MB_OK );
		}
		else {
			m_TreeCtrl.DeleteItem( hCurrent );
			m_TreeCtrl.SelectItems( hParent, hParent, TRUE );
		}
	}
}

void CManagerExplorerPaneView::GetFolderName( CString &szFolder, std::vector<CString> &szVecList )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char szFindFile[512];

	sprintf_s( szFindFile, "%s\\*.*", szFolder );

	hFind = FindFirstFile( szFindFile, &FindFileData );
	while( hFind != INVALID_HANDLE_VALUE ) {
		if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			szVecList.push_back( CString(FindFileData.cFileName) );
		}

		if( !FindNextFile( hFind, &FindFileData ) ) break;
	}

	FindClose( hFind );
}
void CManagerExplorerPaneView::OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_szPrevLabelString = pTVDispInfo->item.pszText;

	*pResult = 0;
}

void CManagerExplorerPaneView::OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_hRootTree == pTVDispInfo->item.hItem ) return;
	if( m_hNationRootTree == pTVDispInfo->item.hItem ) return;
	if( pTVDispInfo->item.pszText == NULL ) return;
	if( CheckIgnoreFolder( pTVDispInfo->item.pszText ) == false ) return;
	if( CheckIgnoreFolder( m_szPrevLabelString.GetBuffer() ) == false ) return;

	// 같은 폴더에 같은 이름의 하위 디렉토리가 잇느지 체크
	HTREEITEM hParent = m_TreeCtrl.GetParentItem( pTVDispInfo->item.hItem );

	CString szFolder;
	for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
		if( m_VecDirList[i].hItem == hParent ) {
			szFolder = m_VecDirList[i].szFullPath;
			break;
		}
	}
	if( szFolder.IsEmpty() ) szFolder = GetCurrentDefaultFolder();

	CString szStr;
	std::vector<CString> szVecFolderList;
	GetFolderName( szFolder, szVecFolderList );
	for( DWORD i=0; i<szVecFolderList.size(); i++ ) {
		if( _stricmp( szVecFolderList[i], pTVDispInfo->item.pszText ) == NULL ) return;
	}
	///////////////////////////////////
	CString szSourceFullPath, szModifyFullPath;
	int nDirIndex = -1;
	for( DWORD i=0; i<m_VecDirList.size(); i++ ) {
		if( m_VecDirList[i].hItem == pTVDispInfo->item.hItem ) {
			szSourceFullPath = m_VecDirList[i].szFullPath;
			nDirIndex = (int)i;
			break;
		}
	}
	szModifyFullPath.Format( "%s\\%s", szFolder, pTVDispInfo->item.pszText );

	if( rename( szSourceFullPath, szModifyFullPath ) != 0 ) return;

	m_TreeCtrl.SetItemText( pTVDispInfo->item.hItem, pTVDispInfo->item.pszText );
	m_VecDirList[nDirIndex].szFullPath = szModifyFullPath;

	CWnd *pWnd = GetCurrentWnd();
	if( pWnd ) pWnd->SendMessage( UM_EXPLORER_CHANGE_FOLDER_NAME, (WPARAM)szModifyFullPath.GetBuffer(), (LPARAM)szSourceFullPath.GetBuffer() );


	*pResult = 0;
}
