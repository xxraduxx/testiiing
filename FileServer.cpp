#include "StdAfx.h"
#include "FileServer.h"
#include <direct.h>
#include "TEtWorld.h"
#include <winsock2.h>
#include <process.h>
#include "UserMessage.h"
#include "MainFrm.h"
#include "EtWorldPainter.h"
#include "PaneDefine.h"
#include "PerfCheck.h"
#include "TEtRandomDungeon.h"
#include "TEtWorldSector.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "EnviControl.h"
#include "EtResourceMng.h"
#include "ScriptControl.h"
#include <shlwapi.h>
#include "DNTableFile.h"
#include "PluginManager.h"

bool CFileServer::s_bExitThread = false;
HANDLE CFileServer::s_hThreadEndRequest = NULL;
HANDLE CFileServer::s_hThreadHandle = NULL;
CFileServer g_FileServer;
CFileServer::CFileServer()
{
	char szStr[512] = { 0, };

	m_bConnected = false;
	
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "WorkingFolder", szStr );
	m_szWorkingFolder = szStr;
	

	memset( szStr, 0, sizeof(szStr) );
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "User", szStr );
	m_szUserName = szStr;

	memset( szStr, 0, sizeof(szStr) );
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Pass", szStr );
	m_szPass = szStr;

	memset( szStr, 0, sizeof(szStr) );
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Profile", szStr );
	m_szProfile = szStr;

	DWORD dwServerType = 0;
	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "ServerType", dwServerType );
	m_ServerType = (ServerTypeEnum)dwServerType;

	RefreshProfile();

	m_pStream = NULL;

	m_dwThreadIndex = 0;
	m_bUpdateLoginInfo = false;

	m_pEventSignalMng = NULL;
	m_pPropSignalMng = NULL;
}

CFileServer::~CFileServer()
{
	SAFE_DELETE( m_pPluginMng );
	SAFE_DELETE( m_pEventSignalMng );
	SAFE_DELETE( m_pPropSignalMng );
	EndLoginInfoThread();
}

bool CFileServer::Initialize()
{
	return true;
}

bool CFileServer::IsCanServerType( ServerTypeEnum Type )
{
	return true;
}

void CFileServer::SetWorkingFolder( char *szStr )
{
	m_szWorkingFolder = szStr;
//	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "WorkingFolder", m_szWorkingFolder.GetBuffer() );
}

char *CFileServer::GetWorkingFolder()
{
//	ProfileStruct *pStruct = GetProfile( m_szProfile );
//	if( pStruct == NULL ) return "";
	
	return m_szWorkingFolder.GetBuffer();

	/*
	char szStr[512] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "WorkingFolder", szStr );
	m_szWorkingFolder = szStr;
	return m_szWorkingFolder.GetBuffer();
	*/
}

void CFileServer::SetUserName( char *szStr )
{
	m_szUserName = szStr;
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "User", m_szUserName.GetBuffer() );
}

char *CFileServer::GetLocalWorkingFolder()
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( pStruct == NULL ) return "";
	return pStruct->szLocalPath.GetBuffer();
}

char *CFileServer::GetUserName()
{
	char szStr[512] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "User", szStr );
	m_szUserName = szStr;
	return m_szUserName.GetBuffer();
}

void CFileServer::SetUseProfile( char *szStr )
{
	m_szProfile = szStr;
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Profile", m_szProfile.GetBuffer() );
}

char *CFileServer::GetUseProfile()
{
	char szStr[512] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Profile", szStr );
	m_szProfile = szStr;
	return m_szProfile.GetBuffer();
}

void CFileServer::SetPassword( char *szStr )
{
	m_szPass = szStr;
	SetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Pass", m_szPass.GetBuffer() );
}

char *CFileServer::GetPassword()
{
	char szStr[512] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "Pass", szStr );
	m_szPass = szStr;
	return m_szPass.GetBuffer();
}

char *CFileServer::GetLoginFolder()
{
	return m_szLoginFolder.GetBuffer();
}


CFileServer::ServerTypeEnum CFileServer::GetServerType()
{
	DWORD dwType = 0;
	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "ServerType", dwType );
	m_ServerType = (ServerTypeEnum)dwType;
	return m_ServerType;
}

void CFileServer::SetServerType( ServerTypeEnum Type )
{
	m_ServerType = Type;
	SetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "ServerType", m_ServerType );
}


bool CFileServer::Connect()
{
	/*
	char *pPtr[100];
	for( int i=0; i<100; i++ ) {
		pPtr[i] = new char[10000000];
		memset( pPtr[i], 0, 10000000 );
	}
	for( int i=0; i<100; i++ ) {
		delete []pPtr[i];
	}
	*/

	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( pStruct == NULL ) return "";


	switch( m_ServerType ) {
		case NetworkAccess:
			{
				ProfileStruct *pStruct = GetProfile( m_szProfile );
				OutputDebug( "Profile : %s\n", m_szProfile );
				if( pStruct == NULL ) {
					CFileServer::GetInstance().LockNation( "" );
					return false;
				}
				if( m_szLockNation.IsEmpty() ) {
					m_szWorkingFolder = pStruct->szLocalPath;
					m_szLoginFolder = pStruct->szLoginPath;
				}
				else {
					for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
						if( _stricmp( pStruct->VecNation[i].szNationStr, m_szLockNation ) == NULL ) {
							m_szWorkingFolder = pStruct->VecNation[i].szNationPath;
							break;
						}
					}
				}

//				m_szWorkingFolder = pStruct->szLocalPath;
			}
			break;
	}
	AddResourcePath();

	if( CTEtWorld::GetInstance().Initialize( GetWorkingFolder(), NULL ) == false ) {
		CFileServer::GetInstance().LockNation( "" );
		return false;
	}

	if( CreateLoginFile() == false ) {
		CTEtWorld::GetInstance().Destroy();
		CTEtRandomDungeon::GetInstance().Destroy();
		CFileServer::GetInstance().LockNation( "" );
		return false;
	}

	CEnviControl::GetInstance().Connect();
	CScriptControl::GetInstance().RefreshDefineScript();

	m_pEventSignalMng = new CEventSignalManager;
	m_pEventSignalMng->ChangeWorkingFolder( GetLocalWorkingFolder() );
	m_pEventSignalMng->Load();

	m_pPropSignalMng = new CPropSignalManager;
	m_pPropSignalMng->ChangeWorkingFolder( GetLocalWorkingFolder() );
	m_pPropSignalMng->Load();

	BeginLoginInfoThread();

	m_bConnected = true;

	CString szStr;
	szStr.Format( "Profile - %s - ( Nation : %s )", CFileServer::GetInstance().GetUseProfile(), ( m_szLockNation.IsEmpty() ) ? "Local" : m_szLockNation );
	((CMainFrame*)AfxGetMainWnd())->SetWindowText( szStr );

	CWnd *pWnd = GetPaneWnd( GRIDLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );

	SAFE_DELETE( m_pPluginMng );
	m_pPluginMng = new CPluginManager();

	/*
	char szMouleName[512] = { 0, };
	char szConfigFileName[512] = { 0, };

	GetModuleFileName( GetModuleHandle( NULL ), szMouleName, 512 );
	_GetPath( szConfigFileName, szMouleName );
	sprintf_s( szMouleName, "%sPlugins", szConfigFileName );
	*/

	m_pPluginMng->Initialize( CEtResourceMng::GetInstance().GetFullPath( "Plugins" ).c_str() );
	m_pPluginMng->RegisterResource();

	return true;
}

void CFileServer::Disconnect()
{
	SAFE_DELETE( m_pEventSignalMng );
	SAFE_DELETE( m_pPropSignalMng );
	CTEtWorld::GetInstance().Destroy();
	CTEtRandomDungeon::GetInstance().Destroy();
	CEnviControl::GetInstance().Disconnect();
	DeleteLoginFile();
	EndLoginInfoThread();
	m_bConnected = false;

	RemoveResourcePath();

	m_szWorkingFolder.Empty();
	CString szStr;
	szStr.Format( "Disconnect" );
	((CMainFrame*)AfxGetMainWnd())->SetWindowText( szStr );

	CWnd *pWnd = GetPaneWnd( GRIDLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );

	SAFE_DELETE( m_pPluginMng );
}

bool CFileServer::IsNeedLogin()
{
	// 네트웍 폴더에서 작업할땐 return true
	// 아님 return false
	char *pWorkingPath = GetWorkingFolder();
	if( pWorkingPath[0] == '\\' && pWorkingPath[1] == '\\' ) return true;
	return false;
}

bool CFileServer::CreateLoginFile()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Create Login Info.." );
	if( !( PathIsDirectory( GetLoginFolder() ) != FALSE ) ) return true;

	char szStr[512];
	std::string szIP;
	LogStruct Struct;

	GetLocalIP( szIP );
	if( IsNeedLogin() ) {
		sprintf_s( szStr, "%s\\Resource\\login\\%s.log", GetWorkingFolder(), m_szUserName );
	}
	else {
		sprintf_s( szStr, "%s\\%s.log", GetLoginFolder(), m_szUserName );
	}

	fopen_s( &m_pStream, szStr, "rb" );

	// 비정상 종료했을 경우 로그인 로그가 남아있게된다.
	if( m_pStream != NULL ) {
		// 아이피 체크해서 현제 아이피랑 유저네임이랑 같으면
		// 로긴시킨다.
		memset( &Struct, 0, sizeof(LogStruct) );
		fread( &Struct, sizeof(LogStruct), 1, m_pStream );

		fclose( m_pStream );
		m_pStream = NULL;

		if( strcmp( szIP.c_str(), Struct.szIP ) != NULL ) return false;
	}

	// Write Login Info
	fopen_s( &m_pStream, szStr, "wb" );
	if( m_pStream == NULL ) return false;


	memset( &Struct, 0, sizeof(LogStruct) );
	sprintf_s( Struct.szIP, szIP.c_str(), szIP.size() );
	Struct.CheckOutGridIndex = -1;

	fwrite( &Struct, sizeof(LogStruct), 1, m_pStream );

	fclose( m_pStream );
	m_pStream = NULL;

	return true;
}

void CFileServer::DeleteLoginFile()
{
	if( !( PathIsDirectory( GetLoginFolder() ) != FALSE ) ) return;
//	if( CGlobalValue::GetInstance().m_SelectGrid != -1 ) return;
	char szStr[512];
	if( IsNeedLogin() )
		sprintf_s( szStr, "%s\\Resource\\login\\%s.log", GetWorkingFolder(), m_szUserName );
	else sprintf_s( szStr, "%s\\%s.log", GetLoginFolder(), m_szUserName );
	DeleteFile( szStr );
}


bool CFileServer::CreateDefaultFile()
{
	_chdir( GetWorkingFolder() );
	if( _mkdir( "Resource" ) != 0 ) return false;
	_chdir( "Resource" );
	if( IsNeedLogin() ) {
		// 로그인 정보 폴더
		if( _mkdir( "login" ) != 0 ) {
			_chdir( ".." );
			return false;
		}
	}
	// TileList 폴더
	if( _mkdir( "Tile" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( "Tile" );
	if( _mkdir( "Grass" ) != 0 ) {
		_chdir("..");
		return false;
	}
	if( _mkdir( "Water" ) != 0 ) {
		_chdir("..");
		return false;
	}
	if( _mkdir( "Decal" ) != 0 ) {
		_chdir("..");
		return false;
	}

	_chdir( ".." );

	// Envi 폴더
	if( _mkdir( "Envi" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	// Prop 리소스 폴더
	if( _mkdir( "Prop" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	if( _mkdir( "PropInfo" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( "Prop" );
	if( _mkdir( "Light" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	if( _mkdir( "Camera" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( ".." );
	// Trigger 폴더
	if( _mkdir( "Trigger" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( "Trigger" );
	if( _mkdir( "Condition" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	if( _mkdir( "Action" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	if( _mkdir( "Event" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( ".." );

	// Sound 리소스 폴더
	if( _mkdir( "Sound" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	// EventArea Infomation 폴더
	if( _mkdir( "EventInfo" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	// 랜덤던젼 리소스 폴더
	if( _mkdir( "RandomRes" ) != 0 ) {
		_chdir( ".." );
		return false;
	}
	_chdir( ".." );
	if( _mkdir( "Grid" ) != 0 ) {
		_chdir( ".." );
		return false;
	}


	// Default Texture 복사
	/*
	CString szFullName;
	CString szDefaultTexture = "DefaultLayerTexture.dds";
	FILE *fp;

	HRSRC hrc = FindResource( NULL, MAKEINTRESOURCE( IDR_DEFAULTDDS ), "DDS" );
	HGLOBAL hGlobal = LoadResource( NULL, hrc );
	long nLength = SizeofResource( NULL, hrc );
	void *pPtr = LockResource( hGlobal );

	szFullName.Format( "%s\\Resource\\Tile\\%s", CFileServer::GetInstance().GetWorkingFolder(), szDefaultTexture );
	fopen_s( &fp, szFullName, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );

	// Grass Texture 복사
	szDefaultTexture = "GrassTexture.dds";

	hrc = FindResource( NULL, MAKEINTRESOURCE( IDR_GRASSDDS ), "DDS" );
	hGlobal = LoadResource( NULL, hrc );
	nLength = SizeofResource( NULL, hrc );
	pPtr = LockResource( hGlobal );

	szFullName.Format( "%s\\Resource\\Tile\\Grass\\%s", CFileServer::GetInstance().GetWorkingFolder(), szDefaultTexture );
	fopen_s( &fp, szFullName, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );

	// Light Skin 복사
	szDefaultTexture = "Light.skn";

	hrc = FindResource( NULL, MAKEINTRESOURCE( IDR_LIGHT_SKN ), "DDS" );
	hGlobal = LoadResource( NULL, hrc );
	nLength = SizeofResource( NULL, hrc );
	pPtr = LockResource( hGlobal );

	szFullName.Format( "%s\\Resource\\Prop\\Light\\%s", CFileServer::GetInstance().GetWorkingFolder(), szDefaultTexture );
	fopen_s( &fp, szFullName, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );

	// Light Mesh 복사
	szDefaultTexture = "__Light.msh";

	hrc = FindResource( NULL, MAKEINTRESOURCE( IDR_LIGHT_MSH ), "DDS" );
	hGlobal = LoadResource( NULL, hrc );
	nLength = SizeofResource( NULL, hrc );
	pPtr = LockResource( hGlobal );

	szFullName.Format( "%s\\Resource\\Prop\\Light\\%s", CFileServer::GetInstance().GetWorkingFolder(), szDefaultTexture );
	fopen_s( &fp, szFullName, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );

	// Light DDS 복사
	szDefaultTexture = "__Light.dds";

	hrc = FindResource( NULL, MAKEINTRESOURCE( IDR_LIGHT_DDS ), "DDS" );
	hGlobal = LoadResource( NULL, hrc );
	nLength = SizeofResource( NULL, hrc );
	pPtr = LockResource( hGlobal );

	szFullName.Format( "%s\\Resource\\Prop\\Light\\%s", CFileServer::GetInstance().GetWorkingFolder(), szDefaultTexture );
	fopen_s( &fp, szFullName, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );
	*/
	CopyResourceToFile( IDR_DEFAULTDDS, "Resource\\Tile\\DefaultLayerTexture.dds" );
	CopyResourceToFile( IDR_GRASSDDS, "Resource\\Tile\\Grass\\GrassTexture.dds" );
	CopyResourceToFile( IDR_LIGHT_SKN, "Resource\\Prop\\Light\\Light.skn" );
	CopyResourceToFile( IDR_LIGHT_MSH, "Resource\\Prop\\Light\\__Light.msh" );
	CopyResourceToFile( IDR_LIGHT_DDS, "Resource\\Prop\\Light\\__Light.dds" );
	CopyResourceToFile( IDR_CAMERA_SKN, "Resource\\Prop\\Camera\\Camera.skn" );
	CopyResourceToFile( IDR_CAMERA_MSH, "Resource\\Prop\\Camera\\Camera.msh" );


	return true;
}

void CFileServer::CopyResourceToFile( int nResourceID, const char *szOutputFilePath )
{
	HRSRC hrc = FindResource( NULL, MAKEINTRESOURCE( nResourceID ), "DDS" );
	HGLOBAL hGlobal = LoadResource( NULL, hrc );
	long nLength = SizeofResource( NULL, hrc );
	void *pPtr = LockResource( hGlobal );
	FILE *fp;

	char szTargetFile[256];
	sprintf_s( szTargetFile, "%s\\%s", CFileServer::GetInstance().GetWorkingFolder(), szOutputFilePath );
	fopen_s( &fp, szTargetFile, "wb" );
	fwrite( pPtr, nLength, 1, fp );
	fclose( fp );

	UnlockResource( hGlobal );
}

bool CFileServer::CreateGridDefaultFile( const char *szGridName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight )
{
	CString szFullPath;
	szFullPath.Format( "%s\\Grid", GetWorkingFolder() );
	_chdir( szFullPath );

	if( _mkdir( szGridName ) != 0 ) return false;
	_chdir( szGridName );

	char szStr[64];
	for( DWORD i=0; i<dwX; i++ ) {
		for( DWORD j=0; j<dwY; j++ ) {
			sprintf_s( szStr, "%d_%d", i, j );
			if( _mkdir( szStr ) != 0 ) return false;
		}
	}

	_chdir( ".." );
	_chdir( ".." );


	return true;
}

void CFileServer::UpdateLoginList()
{
	ScopeLock<CSyncLock> Lock(m_SyncLock);

	char szStr[512];
	char szTemp[512];
	char szBuf[256];

	if( m_bConnected == false ) {
		m_vecLoginList.clear();
		return;
	}

	if( IsNeedLogin() )
		sprintf_s( szStr, "%s\\Resource\\login", GetWorkingFolder() );
	else sprintf_s( szStr, "%s", GetLoginFolder() );
	std::vector<std::string> szvecNameList;

	FindFileListInDirectory( szStr, "*.log", szvecNameList );
	m_vecLoginList.clear();

	FILE *fp;
	WorkingUserStruct Struct;
	for( DWORD i=0; i<szvecNameList.size(); i++ ) {
		memcpy( szBuf, szvecNameList[i].c_str(), szvecNameList[i].size() - 4 );
		szBuf[szvecNameList[i].size() - 4 ] = 0;
		Struct.szUserName = szBuf;
		if( strcmp( szBuf, m_szUserName.GetBuffer() ) == NULL ) continue;

		sprintf_s( szTemp, "%s\\%s", szStr, szvecNameList[i].c_str() );

		fp = NULL;
		fopen_s( &fp, szTemp, "rb" );
		if( fp == NULL ) continue;
		fread( &Struct.LogStruct, sizeof(LogStruct), 1, fp );
		fclose(fp);

		m_vecLoginList.push_back( Struct );
	}
	m_bUpdateLoginInfo = true;
}

void CFileServer::UpdateUseTextureList( CEtWorldGrid *pGrid )
{
	if( pGrid == NULL ) {
		pGrid = CTEtWorld::GetInstance().GetGrid();
		m_szMapUseTextureList.clear();
	}
	DWORD dwWidth = pGrid->GetGridX();
	DWORD dwHeight = pGrid->GetGridY();

	for( DWORD i=0; i<dwHeight; i++ ) {
		for( DWORD j=0; j<dwWidth; j++ ) {
			if( ((CTEtWorldGrid*)pGrid)->IsEmptySector( SectorIndex( j, i ) ) == true ) continue;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( SectorIndex( j, i ), pGrid->GetName() );
			if( pSector == NULL ) continue;
			if( pSector->IsEmpty() ) continue;

			pSector->GetUseTextureList( m_szMapUseTextureList );
		}
	}
	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		UpdateUseTextureList( pGrid->GetChildFromIndex(i) );
	}
}

void CFileServer::UpdateUseSkinList( CEtWorldGrid *pGrid )
{
	if( pGrid == NULL ) {
		pGrid = CTEtWorld::GetInstance().GetGrid();
		m_szMapUseSkinList.clear();
	}
	DWORD dwWidth = pGrid->GetGridX();
	DWORD dwHeight = pGrid->GetGridY();

	for( DWORD i=0; i<dwHeight; i++ ) {
		for( DWORD j=0; j<dwWidth; j++ ) {
			if( ((CTEtWorldGrid*)pGrid)->IsEmptySector( SectorIndex( j, i ) ) == true ) continue;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( SectorIndex( j, i ), pGrid->GetName() );
			if( pSector == NULL ) continue;
			if( pSector->IsEmpty() ) continue;

			pSector->GetUsePropList( m_szMapUseSkinList );
		}
	}
	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		UpdateUseSkinList( pGrid->GetChildFromIndex(i) );
	}
}

void CFileServer::UpdateUseSoundList( CEtWorldGrid *pGrid )
{
}

bool CFileServer::IsUseTexture( CString szName, CString *szResult )
{
	std::map<CString,CString>::iterator it = m_szMapUseTextureList.find( szName );
	if( it != m_szMapUseTextureList.end() ) {
		if( szResult ) *szResult += it->second;
		return true;
	}
	return false;
}

bool CFileServer::IsUseSkin( CString szName, CString *szResult )
{
	std::map<CString,CString>::iterator it = m_szMapUseSkinList.find( szName );
	if( it != m_szMapUseSkinList.end() ) {
		if( szResult ) *szResult += it->second;
		return true;
	}
	return false;
}

bool CFileServer::IsUseSound( CString szName, CString *szResult )
{
	std::map<CString,CString>::iterator it = m_szMapUseSoundList.find( szName );
	if( it != m_szMapUseSoundList.end() ) {
		if( szResult ) *szResult += it->second;
		return true;
	}
	return false;
}

void CFileServer::BeginLoginInfoThread()
{
	CWnd *pWnd = GetPaneWnd( LOGINLIST_PANE );
	s_bExitThread = false;
	s_hThreadEndRequest = CreateEvent( NULL, FALSE, FALSE, NULL );
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)pWnd, 0, &m_dwThreadIndex );
}

void CFileServer::EndLoginInfoThread()
{
//	WaitForSingleObject( m_hThreadHandle, INFINITE );
	if( s_hThreadHandle ) {
		SetEvent( s_hThreadEndRequest );
//		WaitForSingleObject( s_hThreadHandle, INFINITE );
		for( DWORD i=0; i<1000; i++ ) { Sleep(1); }
		TerminateThread( s_hThreadHandle, 0 );
		CloseHandle( s_hThreadHandle );
		CloseHandle( s_hThreadEndRequest );
		s_hThreadHandle = NULL;
		s_hThreadEndRequest = NULL;
	
		m_dwThreadIndex = 0;

		UpdateLoginList();
		CWnd *pWnd = GetPaneWnd( LOGINLIST_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	}
}

UINT __stdcall CFileServer::BeginThread( void *pParam )
{
	/*
	if( CFileServer::GetInstancePtr() && !CFileServer::GetInstance().IsNeedLogin() ) {
		_endthreadex( 0 );
		return 0;
	}
	*/
	while(1) {
		if( !CFileServer::IsActive() ) break;
//		if( CFileServer::s_bExitThread ) break;
		CFileServer::GetInstance().UpdateLoginList();

		CWnd *pWnd = (CWnd *)pParam;
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
		if( CFileServer::GetInstance().IsConnect() == false ) break;
		CGlobalValue::GetInstance().RefreshRender();

		switch( CGlobalValue::GetInstance().m_nActiveView ) {
			case 0:
				CTEtWorld::GetInstance().RefreshCommonInfo();
				break;
			case 1:
				break;
		}

		bool bBreak = false;
		for( int i=0; i<100; i++ ) {
//			if( CFileServer::s_bExitThread ) break;
			if( WaitForSingleObject( CFileServer::s_hThreadEndRequest, 0 ) == WAIT_OBJECT_0 ) {
				OutputDebug( "정상으로2\n" );
				bBreak = true;
				break;
			}
			if( CFileServer::s_hThreadHandle == NULL ) {
				bBreak = true;
				break;
			}

			Sleep(50);
		}
		if( bBreak ) break;
	}
	_endthreadex( 0 );
	return 0;
}

void CFileServer::GetLocalIP( std::string &szIP )
{
	char szStr[512];

	WSADATA winsockInfo;
	WSAStartup( MAKEWORD( 2, 2 ), &winsockInfo );

	gethostname( szStr, 512 );
	HOSTENT *p = NULL;
	p = gethostbyname( szStr );

	struct in_addr *ptr;
	char **listptr = p->h_addr_list;

	if( p->h_addrtype == AF_INET ) {
		while( ( ptr = reinterpret_cast<struct in_addr*>(*listptr++) ) != NULL ) {
			szIP = inet_ntoa( *ptr );
			break;
		}
	}

	WSACleanup();
}

bool CFileServer::CheckOut()
{
	if( m_bConnected == false ) return false;
	if( m_pStream ) return false;
	if( CGlobalValue::GetInstance().m_SelectGrid == -1 ) return false;

	CFileServer::GetInstance().UpdateLoginList();
//	CTEtWorld::GetInstance().RefreshCommonInfo();
	if( IsCanCheckOut( CGlobalValue::GetInstance().m_szSelectGrid, CGlobalValue::GetInstance().m_SelectGrid ) == false ) return false;

	CGlobalValue::GetInstance().m_CheckOutGrid = CGlobalValue::GetInstance().m_SelectGrid;
	CGlobalValue::GetInstance().m_szCheckOutGrid = CGlobalValue::GetInstance().m_szSelectGrid;

	if( !( PathIsDirectory( GetLoginFolder() ) != FALSE ) ) return true;

	char szStr[512];
	LogStruct Struct;
	memset( &Struct, 0, sizeof(LogStruct) );
	if( IsNeedLogin() )
		sprintf_s( szStr, "%s\\Resource\\login\\%s.log", GetWorkingFolder(), m_szUserName );
	else sprintf_s( szStr, "%s\\%s.log", GetLoginFolder(), m_szUserName );

	fopen_s( &m_pStream, szStr, "r+b" );
	if( m_pStream == NULL ) return false;
	fseek( m_pStream, 0, SEEK_SET );
	fread( &Struct, sizeof(LogStruct), 1, m_pStream );
	fseek( m_pStream, 0, SEEK_SET );
	Struct.CheckOutGridIndex = CGlobalValue::GetInstance().m_CheckOutGrid;
	sprintf_s( Struct.szCheckOutGridName, CGlobalValue::GetInstance().m_szCheckOutGrid );

	fwrite( &Struct, sizeof(LogStruct), 1, m_pStream );

	fclose(m_pStream);
	m_pStream = NULL;

	return true;
}

bool CFileServer::CheckIn()
{
	if( m_bConnected == false ) return false;
	if( m_pStream ) return false;
	if( CGlobalValue::GetInstance().m_szCheckOutGrid.IsEmpty() ) return false;
	if( CGlobalValue::GetInstance().m_CheckOutGrid == -1 ) return false;

	CGlobalValue::GetInstance().m_SelectGrid = CGlobalValue::GetInstance().m_CheckOutGrid;
	CGlobalValue::GetInstance().m_szSelectGrid = CGlobalValue::GetInstance().m_szCheckOutGrid;
	CGlobalValue::GetInstance().m_CheckOutGrid = -1;
	CGlobalValue::GetInstance().m_szCheckOutGrid.Empty();

	CTEtWorldGrid *pGrid = (CTEtWorldGrid*)CTEtWorld::GetInstance().FindGrid( CGlobalValue::GetInstance().m_szSelectGrid );
	if( pGrid ) pGrid->WriteGridDescription();

	if( !( PathIsDirectory( GetLoginFolder() ) != FALSE ) ) return true;

	char szStr[512];
	LogStruct Struct;
	if( IsNeedLogin() )
		sprintf_s( szStr, "%s\\Resource\\login\\%s.log", GetWorkingFolder(), m_szUserName );
	else sprintf_s( szStr, "%s\\%s.log", GetLoginFolder(), m_szUserName );

	fopen_s( &m_pStream, szStr, "r+b" );
	fseek( m_pStream, 0, SEEK_SET );
	fread( &Struct, sizeof(LogStruct), 1, m_pStream );
	fseek( m_pStream, 0, SEEK_SET );
	Struct.CheckOutGridIndex = -1;
	fwrite( &Struct, sizeof(LogStruct), 1, m_pStream );

	fclose(m_pStream);
	m_pStream = NULL;

	return true;
}

bool CFileServer::IsCanCheckOut( const char *szGridName, SectorIndex Index )
{
	for( DWORD i=0; i<m_vecLoginList.size(); i++ ) {
		if( m_vecLoginList[i].LogStruct.CheckOutGridIndex == -1 ) continue;
		if( m_vecLoginList[i].LogStruct.szCheckOutGridName == NULL ) continue;

		if( strcmp( m_vecLoginList[i].LogStruct.szCheckOutGridName, szGridName ) == NULL && 
			m_vecLoginList[i].LogStruct.CheckOutGridIndex == Index ) return false;
	}
	return true;
}

bool CFileServer::IsCanConnectSetting()
{
	switch( m_ServerType ) {
		case NetworkAccess:
			{
				ProfileStruct *pStruct = GetProfile( m_szProfile );
				if( pStruct == NULL ) return false;

				CString szStr;
				szStr.Format( "%s\\Infomation", pStruct->szLocalPath );
				return ( PathFileExists( szStr.GetBuffer() ) == TRUE ) ? true : false;
			}
			break;
	}
	return false;
}

bool CFileServer::ApplyNewFiles( const char *szLocalPath )
{
	return true;
}

bool CFileServer::ApplyDeleteFiles( const char *szLocalPath )
{
	return true;
}

void CFileServer::RefreshProfile()
{
	CString szSubKey = REG_SUBKEY;
	szSubKey += "\\Profile";

	DWORD dwCount = 0;
	GetRegistryNumber( HKEY_CURRENT_USER, szSubKey.GetBuffer(), "Count", dwCount );

	CString szTemp;
	char szStr[512] = { 0, };
	for( DWORD i=0; i<dwCount; i++ ) {
		ProfileStruct Struct;
		szTemp.Format( "%s\\Profile %d", szSubKey, i + 1 );
		if( GetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "ProfileName", szStr, 512 ) == ERROR_SUCCESS )
			Struct.szProfileName = szStr;
		if( GetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "LocalPath", szStr, 512 ) == ERROR_SUCCESS )
			Struct.szLocalPath = szStr;
		if( GetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "ResourcePath", szStr, 512 ) == ERROR_SUCCESS )
			Struct.szResPath = szStr;
		if( GetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "LoginPath", szStr, 512 ) == ERROR_SUCCESS )
			Struct.szLoginPath = szStr;

		DWORD dwNationCount = 0;
		GetRegistryNumber( HKEY_CURRENT_USER, szTemp.GetBuffer(), "NationCount", dwNationCount );

		CString szTemp2;
		NationStruct Nation;
		for( DWORD j=0; j<dwNationCount; j++ ) {
			szTemp2.Format( "%s\\Nation %d", szTemp, j + 1 );

			if( GetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "Nation", szStr, 512 ) == ERROR_SUCCESS )
				Nation.szNationStr = szStr;
			if( GetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "LocalPath", szStr, 512 ) == ERROR_SUCCESS )
				Nation.szNationPath = szStr;
			if( GetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "ResourcePath", szStr, 512 ) == ERROR_SUCCESS )
				Nation.szNationResPath = szStr;
			Struct.VecNation.push_back( Nation );
		}

		m_VecProfileList.push_back( Struct );
	}
}

void CFileServer::SaveProfile()
{
	CString szSubKey = REG_SUBKEY;
	szSubKey += "\\Profile";
	DeleteRegistry( HKEY_CURRENT_USER, szSubKey.GetBuffer() );

	AddRegistryKey( HKEY_CURRENT_USER, REG_SUBKEY, "Profile" );

	DWORD dwCount = (DWORD)m_VecProfileList.size();
	SetRegistryNumber( HKEY_CURRENT_USER, szSubKey.GetBuffer(), "Count", dwCount );

	CString szTemp;
	for( DWORD i=0; i<m_VecProfileList.size(); i++ ) {
		szTemp.Format( "Profile %d", i + 1 );
		AddRegistryKey( HKEY_CURRENT_USER, szSubKey.GetBuffer(), szTemp.GetBuffer() );

		szTemp.Format( "%s\\Profile %d", szSubKey, i + 1 );

		ProfileStruct *pStruct = &m_VecProfileList[i];
		SetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "ProfileName", pStruct->szProfileName.GetBuffer() );
		SetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "LocalPath", pStruct->szLocalPath.GetBuffer() );
		SetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "ResourcePath", pStruct->szResPath.GetBuffer() );
		SetRegistryString( HKEY_CURRENT_USER, szTemp.GetBuffer(), "LoginPath", pStruct->szLoginPath.GetBuffer() );

		DWORD dwNationCount = (DWORD)pStruct->VecNation.size();
		SetRegistryNumber( HKEY_CURRENT_USER, szTemp.GetBuffer(), "NationCount", dwNationCount );
		CString szTemp2;
		for( DWORD j=0; j<dwNationCount; j++ ) {
			szTemp2.Format( "Nation %d", j + 1 );
			AddRegistryKey( HKEY_CURRENT_USER, szTemp.GetBuffer(), szTemp2.GetBuffer() );

			szTemp2.Format( "%s\\Nation %d", szTemp, j + 1 );

			NationStruct *pNation = &pStruct->VecNation[j];
			SetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "Nation", pNation->szNationStr.GetBuffer() );
			SetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "LocalPath", pNation->szNationPath.GetBuffer() );
			SetRegistryString( HKEY_CURRENT_USER, szTemp2.GetBuffer(), "ResourcePath", pNation->szNationResPath.GetBuffer() );
		}
	}
}


CFileServer::ProfileStruct *CFileServer::GetProfile( const char *szProfileName )
{
	for( DWORD i=0; i<m_VecProfileList.size(); i++ ) {
		if( _stricmp( m_VecProfileList[i].szProfileName, szProfileName ) == NULL ) return &m_VecProfileList[i];
	}
	return NULL;
}

bool CFileServer::AddProfile( const char *szProfileName )
{
	if( IsExistProfile( szProfileName ) ) return false;
	ProfileStruct Struct;
	Struct.szProfileName = szProfileName;

	m_VecProfileList.push_back( Struct );
	return true;
}

bool CFileServer::RemoveProfile( const char *szProfileName )
{
	if( !IsExistProfile( szProfileName ) ) return false;

	for( DWORD i=0; i<m_VecProfileList.size(); i++ ) {
		if( _stricmp( m_VecProfileList[i].szProfileName, szProfileName ) == NULL ) {
			m_VecProfileList.erase( m_VecProfileList.begin() + i );
			return true;
		}
	}
	return false;
}

bool CFileServer::IsExistProfile( const char *szProfileName )
{
	for( DWORD i=0; i<m_VecProfileList.size(); i++ ) {
		if( _stricmp( m_VecProfileList[i].szProfileName, szProfileName ) == NULL ) return true;
	}
	return false;
}


DWORD CFileServer::GetProfileCount()
{
	return (DWORD)m_VecProfileList.size();
}

CFileServer::ProfileStruct *CFileServer::GetProfile( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecProfileList.size() ) return NULL;
	return &m_VecProfileList[dwIndex];
}


void CFileServer::AddResourcePath()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Add Resource Path.." );
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	CString szWorkPath;

	if( !m_szLockNation.IsEmpty() ) {
		NationStruct *pNation;
		for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
			pNation = &pStruct->VecNation[i];
			if( _stricmp( pNation->szNationStr, m_szLockNation ) ) continue;

			// Nation 별 MapData Path 등록
			if( !pNation->szNationPath.IsEmpty() ) {
				szWorkPath = pNation->szNationPath;
				CString szFolder;
				szFolder.Format( "%s\\Resource\\Tile", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Prop", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

				/*
				szFolder.Format( "%s\\Resource\\Envi", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );
				*/

				szFolder.Format( "%s\\Resource\\Sound", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

				/*
				szFolder.Format( "%s\\Resource\\Trigger\\Condition", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Trigger\\Action", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Trigger\\Event", szWorkPath );
				CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );
				*/
			}

			// Nation 별 Resource Path 등록
			if( !pNation->szNationResPath.IsEmpty() ) {
				CEtResourceMng::GetInstance().AddResourcePath( pNation->szNationResPath, true );
			}
			break;
		}
	}

	// 로컬 MapData Path 등록
	szWorkPath = pStruct->szLocalPath;
	if( !szWorkPath.IsEmpty() ) {
		CString szFolder;
		szFolder.Format( "%s\\Resource\\Tile", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Prop", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Envi", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Sound", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Condition", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Action", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Event", szWorkPath );
		CEtResourceMng::GetInstance().AddResourcePath( szFolder, true );
	}

	// 로컬 Resource Path 등록
	if( !pStruct->szResPath.IsEmpty() ) {
		CEtResourceMng::GetInstance().AddResourcePath( pStruct->szResPath, true );
	}

	if( CTEtWorldSector::s_pPropSOX == NULL )
		CTEtWorldSector::s_pPropSOX = LoadSox( "PropTable.dnt", true );

}

void CFileServer::RemoveResourcePath()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Remove Resource Path.." );
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( pStruct == NULL ) return;
	CString szWorkPath;

	if( !m_szLockNation.IsEmpty() ) {
		NationStruct *pNation;
		for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
			pNation = &pStruct->VecNation[i];
			if( _stricmp( pNation->szNationStr, m_szLockNation ) ) continue;

			// Nation 별 MapData Path 해재
			if( !pNation->szNationPath.IsEmpty() ) {
				szWorkPath = pNation->szNationPath;
				CString szFolder;
				szFolder.Format( "%s\\Resource\\Tile", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Prop", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

				/*
				szFolder.Format( "%s\\Resource\\Envi", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );
				*/

				szFolder.Format( "%s\\Resource\\Sound", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

				/*
				szFolder.Format( "%s\\Resource\\Trigger\\Condition", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Trigger\\Action", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

				szFolder.Format( "%s\\Resource\\Trigger\\Event", szWorkPath );
				CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );
				*/
			}

			// Nation 별 Resource Path 등록
			if( !pNation->szNationResPath.IsEmpty() ) {
				CEtResourceMng::GetInstance().RemoveResourcePath( pNation->szNationResPath, true );
			}
			break;
		}
	}

	// 로컬 MapData Path 등록
	szWorkPath = pStruct->szLocalPath;
	if( !szWorkPath.IsEmpty() ) {
		CString szFolder;
		szFolder.Format( "%s\\Resource\\Tile", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Prop", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Envi", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Sound", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Condition", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Action", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );

		szFolder.Format( "%s\\Resource\\Trigger\\Event", szWorkPath );
		CEtResourceMng::GetInstance().RemoveResourcePath( szFolder, true );
	}

	// 로컬 Resource Path 등록
	if( !pStruct->szResPath.IsEmpty() ) {
		CEtResourceMng::GetInstance().RemoveResourcePath( pStruct->szResPath, true );
	}
	CEtResource::FlushWaitDelete();

	SAFE_DELETE( CTEtWorldSector::s_pPropSOX );
}

void CFileServer::AddNationResourcePath()
{
	if( m_szLockNation.IsEmpty() ) return;
	OutputDebug( "AddNationResourcePath\n" );
}

void CFileServer::RemoveNationResourcePath()
{
	if( m_szLockNation.IsEmpty() ) return;
	OutputDebug( "RemoveNationResourcePath\n" );
}

int CFileServer::GetNationCount()
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return 0;
	return (int)pStruct->VecNation.size();
}

CString CFileServer::GetNationName( int nIndex )
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return "";
	if( nIndex < 0 || nIndex >= (int)pStruct->VecNation.size() ) return "";
	return pStruct->VecNation[nIndex].szNationStr;
}

CString CFileServer::GetNationResourceFolder( int nIndex )
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return "";
	if( nIndex < 0 || nIndex >= (int)pStruct->VecNation.size() ) return "";
	return pStruct->VecNation[nIndex].szNationPath;
}

CString CFileServer::GetNationWorkingFolder( int nIndex )
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return "";
	if( nIndex < 0 || nIndex >= (int)pStruct->VecNation.size() ) return "";
	return pStruct->VecNation[nIndex].szNationResPath;

}

bool CFileServer::IsExistNationSector( char *szGridName, SectorIndex Index )
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return false;

	CString szPath;
	for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
		szPath.Format( "%s\\Grid\\%s\\GridInfo.ini", pStruct->VecNation[i].szNationPath, szGridName );
		if( PathFileExists( szPath.GetBuffer() ) == TRUE ) {
			szPath.Format( "%s\\Grid\\%s\\%d_%d", pStruct->VecNation[i].szNationPath, szGridName, Index.nX, Index.nY );
			if( PathFileExists( szPath.GetBuffer() ) == TRUE ) {
				return true;
			}
		}
	}
	return false;
}

void CFileServer::LockNation( CString szNation )
{
	m_szLockNation = szNation;
}

void CFileServer::UnlockNation()
{
	m_szLockNation.Empty();
}

void CFileServer::GetNationSectorNationName( char *szGridName, SectorIndex Index, std::vector<CString> &VecNationNameList )
{
	ProfileStruct *pStruct = GetProfile( m_szProfile );
	if( !pStruct ) return;

	CString szPath;
	for( DWORD i=0; i<pStruct->VecNation.size(); i++ ) {
		szPath.Format( "%s\\Grid\\%s\\GridInfo.ini", pStruct->VecNation[i].szNationPath, szGridName );
		if( PathFileExists( szPath.GetBuffer() ) == TRUE ) {
			szPath.Format( "%s\\Grid\\%s\\%d_%d", pStruct->VecNation[i].szNationPath, szGridName, Index.nX, Index.nY );
			if( PathFileExists( szPath.GetBuffer() ) == TRUE ) {
				VecNationNameList.push_back( pStruct->VecNation[i].szNationStr );
			}
		}
	}
}


void CFileServer::GetDepthPath( const char *szPath, char *szBuf, int nDepth )
{
	if( nDepth == 0 ) {
		strcpy( szBuf, szPath );
		return;
	}

	int nLength = (int)strlen(szPath);
	int nCount = 0;
	for( int i=nLength-1; i>=0; i-- ) {
		if( szPath[i] == '\\' ) {
			nCount++;
			if( nCount == nDepth ) {
				strncpy( szBuf, szPath, i );
				szBuf[i] = 0;
				return;
			}
		}
	}
}

int CFileServer::GetDepthPathCount( const char *szPath )
{
	int nLength = (int)strlen(szPath);
	int nCount = 0;
	for( int i=nLength-1; i>=0; i-- ) {
		if( szPath[i] == '\\' ) {
			nCount++;
		}
	}
	return nCount;
}

DNTableFileFormat *CFileServer::LoadSox( const char *szFileName, bool bGenerationInverseFindMap )
{
	DNTableFileFormat *pSox = NULL;
	std::vector<CFileNameString> szVecList;
	FindExtFileList( szFileName, szVecList );
	pSox = new DNTableFileFormat;
	pSox->SetGenerationInverseLabel( "_Name" );
	for( DWORD i=0; i<szVecList.size(); i++ ) {
		pSox->Load( szVecList[i].c_str(), false );
	}
	return pSox;
}

void CFileServer::FindExtFileList( const char *szFileName, std::vector<CFileNameString> &szVecList )
{
	char szTemp[_MAX_PATH] = { 0, };
	char szName[256] = { 0, };
	char szExt[256] = { 0, };
	_GetFileName( szName, _countof(szName), szFileName );
	_GetExt( szExt, _countof(szExt), szFileName );
	sprintf_s( szTemp, "%s*.%s", szName, szExt );
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "ext", szTemp, szVecList );

}
