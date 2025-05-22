#include "StdAfx.h"
#include "TEtWorld.h"
#include "Stream.h"
#include "FileServer.h"
#include <direct.h>
#include "GlobalValue.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "RenderBase.h"
#include "PropertyGridCreator.h"


CTEtWorld g_World;
CTEtWorld::CTEtWorld()
{
	m_bAccesThread = false;

#ifdef _CHECK_MISSING_INDEX
	TCHAR szCurFileName[MAX_PATH]= {0,};
	::GetModuleFileName( NULL, szCurFileName, MAX_PATH );
	TCHAR szDrive[MAX_PATH] = { 0, };
	TCHAR szDir[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	TCHAR szExt[MAX_PATH] = { 0, };
	_tsplitpath( szCurFileName, szDrive, szDir, szFileName, szExt );

	CString _szCurPath;
	_szCurPath.Format( _T("%s%s\\Log"), szDrive, szDir );

	if( GetFileAttributes( _szCurPath.GetBuffer() ) != FILE_ATTRIBUTE_DIRECTORY )
		CreateDirectory( _szCurPath.GetBuffer(), NULL);

	LogWnd::CreateLog(false,LogWnd::LOG_TARGET_FILE);
#endif
}

CTEtWorld::~CTEtWorld()
{
	while(1) {
		if( m_bAccesThread == false ) break;
		Sleep(1);
	}
}

void CTEtWorld::Destroy()
{
	while(1) {
		if( m_bAccesThread == false ) break;
		Sleep(1);
	}
	CEtWorld::Destroy();
}

bool CTEtWorld::Initialize( const char *szWorldFolder, const char *szGridName )
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize World.." );

	bool bResult = CEtWorld::Initialize( szWorldFolder, NULL );
	if( bResult == false ) return false;

	// 읽는다.
	CString szFullPath;
	std::vector< CFileNameString > szVecList;
	szFullPath.Format( "%s\\Grid", CFileServer::GetInstance().GetWorkingFolder() );
	_FindFolder( szFullPath, szVecList );

	std::vector<CTEtWorldGrid *> pVecList;
	for( DWORD i=0; i<szVecList.size(); i++ ) {
		CTEtWorldGrid *pGrid = (CTEtWorldGrid *)AllocGrid();
		if( pGrid->LoadGrid( szVecList[i].c_str() ) == false ) {
			SAFE_DELETE( pGrid );
			continue;
		}
		if( pGrid->GetParentName().IsEmpty() ) continue;
		pVecList.push_back( pGrid );
	}

	int nProgressCount = 0;
	int nProgressMax = (int)pVecList.size();
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		CTEtWorldGrid *pGrid = pVecList[i];
		CTEtWorldGrid *pParent = NULL;
		pParent = (CTEtWorldGrid *)FindGrid( pGrid->GetParentName() );
		if( pParent == NULL ) {
			bool bInvalidParentName = true;
			if( std::find( szVecList.begin() ,szVecList.end(), pGrid->GetParentName().GetBuffer() ) == szVecList.end() ) {
				m_pGrid->InsertChild( pGrid );
				pGrid->SetParent( m_pGrid );
				pVecList.erase( pVecList.begin() + i );
				i--;
				continue;
			}
			else {
				pVecList.push_back( pGrid );
				pVecList.erase( pVecList.begin() + i );
				i--;
				continue;
			}
		}

		pParent->InsertChild( pGrid );
		pGrid->SetParent( pParent );
		pVecList.erase( pVecList.begin() + i );
		i--;
		nProgressCount++;
		CGlobalValue::GetInstance().UpdateProgress( nProgressMax, nProgressCount );
	}

	((CTEtWorldGrid*)m_pGrid)->LoadCommonInfo( false );

	// Event Area Type 들 읽는다.
	/*
	std::vector<std::string> szVecEdfList;
	FindFileListInDirectory( szWorldFolder, "*.edf", szVecEdfList, false, false );

	FILE *fp;
	char szCategory[512];
	char szTag[512];
	char szType[512];
	char szDescription[512];
	char szDefaultEnable[512];

	char *szTypeStr[] = { 
		"INT",
		"FLOAT",
		"CHAR",
		"STRING",
		"VECTOR",
		"COLOR",
		"PTR",
		"BOOLEAN",
		"INT_MINMAX",
		"INT_COMBO",
		"STRING_FF",
		"VECTOR2",
		"VECTOR4",
		"VECTOR4COLOR",
		"FLOAT_MINMAX",
		NULL,
	};

	for( DWORD i=0; i<szVecList.size(); i++ ) {
		DefineEventControlStruct Struct;

		Struct.szControlName = szVecEdfList[i];
		
		CString szFullPath;
		std::vector< std::string > szVecList;
		szFullPath.Format( "%s\\%s", CFileServer::GetInstance().GetWorkingFolder(), szVecEdfList[i].c_str() );

		fopen_s( &fp, szFullPath, "rt" );

		while( !feof(fp) ) {
			PropertyGridBaseDefine Define;

			memset( szCategory, 0, sizeof(szCategory) );
			memset( szTag, 0, sizeof(szTag) );
			memset( szType, 0, sizeof(szType) );
			memset( szDescription, 0, sizeof(szDescription) );
			memset( szDefaultEnable, 0, sizeof(szDefaultEnable) );
			fscanf_s( fp, "%s, %s, %s, %s, %s", szCategory, sizeof(szCategory), szTag, sizeof(szTag), szType, sizeof(szType), szDescription, sizeof(szDescription), szDefaultEnable, sizeof(szDefaultEnable) );
			
			bool bFailed = false;
			for( int j=0; ; j++ ) {
				if( szTypeStr[j] == NULL ) {
					bFailed = true;
					break;
				}
				if( strcmp( szType, szTypeStr[j] ) == NULL ) {
					Define.Type = (CUnionValueProperty::Type)j;
					break;
				}
			}
			if( bFailed ) continue;

			Define.szCategory = new char[strlen(szCategory) + 1];
			strcpy_s( Define.szCategory, strlen(szCategory) + 1, szCategory );

			Define.szStr = new char[strlen(szTag) + 1];
			strcpy_s( Define.szCategory, strlen(szTag) + 1, szTag );

			Define.szDescription = new char[strlen(szDescription) + 1];
			strcpy_s( Define.szDescription, strlen(szDescription) + 1, szDescription );

			if( strcmp( szDefaultEnable, "TRUE" ) == NULL ) Define.bEnable = TRUE;
			else Define.bEnable = FALSE;

			Struct.VecDefine.push_back( Define );
		}
		m_VecDefineEventControlList.push_back( Struct );
	}
	*/
	return true;
}

CEtWorldGrid *CTEtWorld::AllocGrid()
{
	return new CTEtWorldGrid( this );
}

bool CTEtWorld::CreateEmptyWorld()
{
	FILE *fp;
	// 이미 정보파일이 있으면 생성 못하게..
	_chdir( CFileServer::GetInstance().GetWorkingFolder() );
	fopen_s( &fp, "Infomation", "rb" );
	if( fp != NULL ) {
		fclose(fp);
		return false;
	}
	

	fopen_s( &fp, "Infomation", "wb" );
	
	// Heade Write
	InfoFile_Header Header;
	sprintf_s( Header.szStr, CEtWorld::s_szWorldHeaderString );
	Header.nVersion = CEtWorld::s_nWorldVersion;

	fwrite( &Header, sizeof(InfoFile_Header), 1, fp );
	fclose(fp);

	return true;
}

bool CTEtWorld::CreateEmptyGrid( const char *szGridName, const char *szParentName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize )
{
	if( IsExistGrid( szGridName ) == true ) return false;
	CEtWorldGrid *pParent = FindGrid( szParentName );
	if( pParent == NULL ) return false;

	CTEtWorldGrid *pGrid = (CTEtWorldGrid *)AllocGrid();
	if( pGrid->CreateEmptyGrid( szGridName, szParentName, dwX, dwY, dwWidth, dwHeight, dwTileSize ) == false ) {
		SAFE_DELETE( pGrid );
		return false;
	}
	pGrid->SetParent( pParent );
	pParent->InsertChild( pGrid );

	CString szPath;
	szPath.Format( "%s\\Grid\\%s", CFileServer::GetInstance().GetWorkingFolder(), szGridName );
	CFileServer::GetInstance().ApplyNewFiles( szPath );

	return true;
}

void CTEtWorld::CalcGridHeightPos( CEtWorldGrid *pGrid, float fValue, float &fMaxValue )
{
	float fCurValue = fValue;
	float fTemp = 0.f;

	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		CEtWorldGrid *pChild = pGrid->GetChildFromIndex(i);
		((CTEtWorldGrid*)pChild)->SetYPos( fCurValue );
		CalcGridHeightPos( pChild, fCurValue, fTemp );
		fCurValue += fTemp + (float)( pChild->GetGridY() * pChild->GetGridHeight() )  + 100.f;
		fTemp = 0.f;
	}
	float fMax = fCurValue - fValue;
	if( fMaxValue < fMax ) fMaxValue = fMax;
}

void CTEtWorld::GetGridRect( CEtWorldGrid *pGrid, DWORD dwX, DWORD dwY, D3DXVECTOR3 *vMin, D3DXVECTOR3 *vMax )
{
	EtVector2 vOffset = EtVector2( 0.f, 0.f );
	CEtWorldGrid *pCurGrid = pGrid;
	while(1) {
		CEtWorldGrid *pParent = pCurGrid->GetParent();
		if( pParent == NULL ) break;
		float fMaxWidth = (float)( pParent->GetGridWidth() * pParent->GetGridX() );

		pCurGrid = pParent;

		vOffset.x += fMaxWidth + 1000.f;
	}

	float fTemp = 0.f;
	CalcGridHeightPos( GetGrid(), 0.f, fTemp );

	vOffset.y = ((CTEtWorldGrid*)pGrid)->GetYPos();

	vOffset.x += (float)( pGrid->GetGridWidth() * pGrid->GetGridX() / 2.f );
	vOffset.y += (float)( pGrid->GetGridHeight() * pGrid->GetGridY() / 2.f );

	vMin->x = -( ( pGrid->GetGridX() / 2.f ) * (float)pGrid->GetGridWidth() ) + (float)( dwX * pGrid->GetGridWidth() );
	vMax->x = vMin->x + (float)pGrid->GetGridWidth();
	vMin->z = -( ( pGrid->GetGridY() / 2.f ) * ( float)pGrid->GetGridHeight() ) + (float)( dwY * pGrid->GetGridHeight() );
	vMax->z = vMin->z + (float)pGrid->GetGridHeight();

	vMin->x += vOffset.x;
	vMax->x += vOffset.x;
	vMin->z -= vOffset.y;
	vMax->z -= vOffset.y;

	vMin->y = 0.f;
	vMax->y = 0.f;

}


void CTEtWorld::DrawGrid( CEtWorldGrid *pGrid )
{
	EtVector3 vTemp[2];
	CString szDesc;
	for( DWORD i=0; i<pGrid->GetGridX(); i++ ) {
		for( DWORD j=0; j<pGrid->GetGridY(); j++ ) {
			GetGridRect( pGrid, i, j, &vTemp[0], &vTemp[1] );
			DrawGrid( vTemp[0].x, vTemp[0].z, vTemp[1].x, vTemp[1].z, 0xFFFFFFFF, pGrid, SectorIndex( i, j ) );
		}
	}
	// 선그려준다.
	EtVector3 vLine[2];
	GetGridRect( pGrid, pGrid->GetGridX() - 1, pGrid->GetGridY() - 1, &vTemp[0], &vTemp[1] );
	vLine[0].x = vTemp[1].x;
	vLine[0].y = 0.f;
	vLine[0].z = vTemp[0].z + ( ( vTemp[1].z - vTemp[0].z ) / 2.f );



	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		CEtWorldGrid *pChild = pGrid->GetChildFromIndex(i);
		GetGridRect( pChild, 0, pChild->GetGridY() - 1, &vTemp[0], &vTemp[1] );
		vLine[1].x = vTemp[0].x;
		vLine[1].y = 0.f;
		vLine[1].z = vTemp[0].z + ( ( vTemp[1].z - vTemp[0].z ) / 2.f );
		EternityEngine::DrawLine3D( vLine[0], vLine[1], 0xFFFFFF00 );

		DrawGrid( pChild );
	}
}

void CTEtWorld::RenderGrid()
{
	// 루트 월드 그려주구
	MatrixEx *pCross = CRenderBase::GetInstance().GetLastUpdateCamera();
	float fRootSize = 200.f;
	DrawGrid( -fRootSize, -fRootSize, fRootSize, fRootSize, 0xFFFF00FF, NULL, SectorIndex() );

	// 차일드 월드 그려준다.
	DrawGrid( GetGrid() );

	EtVector3 vTemp[2];

	// Render Any User CheckOut Grid
	std::vector<CFileServer::WorkingUserStruct> *pVecList = CFileServer::GetInstance().GetLoginUserList();
	CFileServer::LogStruct *pStruct;
	for( DWORD i=0; i<pVecList->size(); i++ ) {
		pStruct = &(*pVecList)[i].LogStruct;
		if( pStruct->CheckOutGridIndex == -1 ) continue;
		if( pStruct->szCheckOutGridName == NULL ) continue;

		CEtWorldGrid *pGrid = CTEtWorld::FindGrid( pStruct->szCheckOutGridName );
		if( pGrid == NULL ) continue;
		GetGridRect( pGrid, pStruct->CheckOutGridIndex.nX, pStruct->CheckOutGridIndex.nY, &vTemp[0], &vTemp[1] );

		DrawGrid( vTemp[0].x, vTemp[0].z, vTemp[1].x, vTemp[1].z, 0x40FF2222, NULL, SectorIndex() );
		EternityEngine::DrawLine3D( EtVector3( vTemp[0].x, 0.f, vTemp[0].z ), EtVector3( vTemp[1].x, 0.f, vTemp[1].z ), 0xFFFF2222 );
		EternityEngine::DrawLine3D( EtVector3( vTemp[1].x, 0.f, vTemp[0].z ), EtVector3( vTemp[0].x, 0.f, vTemp[1].z ), 0xFFFF2222 );
	}

	// Render Select Grid
	SectorIndex SelectGrid = CGlobalValue::GetInstance().m_SelectGrid;
	CString szSelectGrid = CGlobalValue::GetInstance().m_szSelectGrid;
	CEtWorldGrid *pGrid = CTEtWorld::GetInstance().FindGrid( szSelectGrid );
	if( szSelectGrid && SelectGrid != -1 && pGrid ) {
		GetGridRect( pGrid, SelectGrid.nX, SelectGrid.nY, &vTemp[0], &vTemp[1] );
		DrawGrid( vTemp[0].x, vTemp[0].z, vTemp[1].x, vTemp[1].z, D3DCOLOR_ARGB( 70, 30, 50, 250 ), NULL, SectorIndex() );
	}
	// Render CheckOut Grid
	SectorIndex CheckOutGrid = CGlobalValue::GetInstance().m_CheckOutGrid;
	CString szCheckOutGrid = CGlobalValue::GetInstance().m_szCheckOutGrid;
	pGrid = CTEtWorld::GetInstance().FindGrid( szCheckOutGrid );
	if( szCheckOutGrid && CheckOutGrid != -1 && pGrid ) {
		GetGridRect( pGrid, CheckOutGrid.nX, CheckOutGrid.nY, &vTemp[0], &vTemp[1] );
		DrawGrid( vTemp[0].x, vTemp[0].z, vTemp[1].x, vTemp[1].z, D3DCOLOR_ARGB( 70, 255, 50, 30 ), NULL, SectorIndex() );
	}
}

void CTEtWorld::DrawGrid( float fLeft, float fTop, float fRight, float fBottom, DWORD dwColor, CEtWorldGrid *pGrid, SectorIndex Index )
{
	EtVector3 vRect[4];
	vRect[0] = EtVector3( fLeft, 0.f, fTop );
	vRect[1] = EtVector3( fRight, 0.f, fTop );
	vRect[2] = EtVector3( fRight, 0.f, fBottom );
	vRect[3] = EtVector3( fLeft, 0.f, fBottom );

	static EtVector2 vTexCoord[4] = { EtVector2( 0.f, 1.f ), EtVector2( 1.f, 1.f ), EtVector2( 1.f, 0.f ), EtVector2( 0.f, 0.f ) };

	if( pGrid && Index != -1 ) {
		CTEtWorldSector *pSector = (CTEtWorldSector *)pGrid->GetSector( Index );
		if( pSector && !pSector->IsEmpty() ) {
			EtBaseTexture *pTexture;
			pTexture = pSector->GetThumbnail();
			if( pTexture ) {
				EternityEngine::DrawQuad3DWithTex( vRect, vTexCoord, pTexture, NULL );
			}
		}
	}

	EternityEngine::DrawLine3D( EtVector3( fLeft, 0.f, fTop ), EtVector3( fRight , 0.f, fTop ), dwColor );
	EternityEngine::DrawLine3D( EtVector3( fLeft, 0.f, fBottom ), EtVector3( fRight , 0.f, fBottom ), dwColor );
	EternityEngine::DrawLine3D( EtVector3( fLeft, 0.f, fTop ), EtVector3( fLeft , 0.f, fBottom ), dwColor );
	EternityEngine::DrawLine3D( EtVector3( fRight, 0.f, fTop ), EtVector3( fRight , 0.f, fBottom ), dwColor );
	if( dwColor >> 24  < 255 ) {
		EternityEngine::DrawTriangle3D( vRect[0], vRect[1], vRect[2], dwColor );
		EternityEngine::DrawTriangle3D( vRect[0], vRect[2], vRect[3], dwColor );
	}
}

void CTEtWorld::RenderHeightmap( SectorIndex Index )
{
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = GetSector( Index );
	if( pSector == NULL ) return;

	pSector->Show( true );
	pSector->Render( CRenderBase::GetInstance().GetLocalTime() );
}

bool CTEtWorld::IsEmptySector( const char *szGridName, SectorIndex Index )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName );
	if( !pGrid ) return false;
	return ((CTEtWorldGrid*)pGrid)->IsEmptySector( Index );
}

CTEtWorldSector *CTEtWorld::GetSector( SectorIndex Index, const char *szGridName )
{
	CEtWorldGrid *pGrid = NULL;
	if( szGridName == NULL ) pGrid = m_pCurrentGrid;
	else {
		pGrid = FindGrid( szGridName, m_pGrid );
	}
	if( pGrid == NULL ) return NULL;

	CEtWorldSector *pSector = ((CTEtWorldGrid*)pGrid)->GetSector( Index );
	return (CTEtWorldSector *)pSector;
}

void CTEtWorld::SetCurrentGrid( const char *szGridName )
{
	if( szGridName == NULL ) {
		m_pCurrentGrid = NULL;
		m_szCurrentGrid.clear();
	}
	else {
		m_pCurrentGrid = FindGrid( szGridName );
		m_szCurrentGrid = szGridName;
	}
}

CTEtWorldGrid *CTEtWorld::GetCurrentGrid()
{
	return (CTEtWorldGrid *)m_pCurrentGrid;
}

const char *CTEtWorld::GetCurrentGridName()
{
	return m_szCurrentGrid.c_str();
}

void CTEtWorld::RefreshCommonInfo()
{
	m_bAccesThread = true;
	((CTEtWorldGrid*)m_pGrid)->LoadCommonInfo();
	m_bAccesThread = false;
}

bool CTEtWorld::IsExistGrid( CString szGridName )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName, m_pGrid );
	if( pGrid == NULL ) return false;
	return true;
}

bool CTEtWorld::DeleteFileInSubDirectory( CString szPath )
{
	CString szFullPath;
	std::vector< CString > szVecDir;
	szFullPath.Format( "%s\\*.*", szPath );

	bool bResult = true;
	// 찾아서 지워준다.
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile( szFullPath, &FindFileData );
	while( hFind != INVALID_HANDLE_VALUE ) {
		if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if( strcmp( FindFileData.cFileName, "." ) && strcmp( FindFileData.cFileName, ".." ) ) {
				CString szStr;
				szStr.Format( "%s\\%s", szPath, FindFileData.cFileName );
				szVecDir.push_back( szStr );

				DeleteFileInSubDirectory( szStr );
			}
		}
		else {
			CString szStr;
			szStr.Format( "%s\\%s", szPath, FindFileData.cFileName );
			SetFileAttributes( szStr, FILE_ATTRIBUTE_NORMAL );
			if( DeleteFile( szStr ) == FALSE ) bResult = false;
		}

		if( !FindNextFile( hFind, &FindFileData ) ) break;
	}

	FindClose( hFind );

	if( szVecDir.size() > 0 ) {
		for( int i=(int)szVecDir.size() - 1; i>= 0 ; i-- ) {
			if( _rmdir( szVecDir[i] ) != 0 ) bResult = false;
		}
	}
	if( _rmdir( szPath ) != 0 ) bResult = false;

	return bResult;
}

bool CTEtWorld::DestroyGrid( const char *szGridName )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName, m_pGrid );
	if( pGrid == NULL ) return false;

	// 파일 각각 다찾아서 지워주구 폴더두 지워버리는 코드
	CString szFullPath;
	std::vector< CString > szVecDir;
	szFullPath.Format( "%s\\Grid\\%s", CFileServer::GetInstance().GetWorkingFolder(), szGridName );

	// 찾아서 지워준다.
	DeleteFileInSubDirectory( szFullPath );

	// 차일드가 잇을경우엔 지울놈의 부모밑으로 넣어준다.
	CEtWorldGrid *pParent = pGrid->GetParent();
	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		CEtWorldGrid *pChild = (CEtWorldGrid *)pGrid->GetChildFromIndex(i);
		pChild->SetParent( pParent );
		((CTEtWorldGrid*)pChild)->WriteGridInfo();
	}

	// 닫았다가 다시 열어준다.
	Destroy();
	CTEtWorld::Initialize( CFileServer::GetInstance().GetWorkingFolder(), NULL );
	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

bool CTEtWorld::IsDestroyGrid( const char *szGridName )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName, m_pGrid );
	if( pGrid == NULL ) return false;
	if( pGrid->GetParent() == NULL ) return false;

	CFileServer::GetInstance().UpdateLoginList();
	for( DWORD i=0; i<pGrid->GetGridX(); i++ ) {
		for( DWORD j=0; j<pGrid->GetGridY(); j++ ) {
			if( CFileServer::GetInstance().IsCanCheckOut( szGridName, SectorIndex( i, j ) ) == false ) return false;
		}
	}
	return true;
}

bool CTEtWorld::MoveGrid( const char *szGridName, const char *szParentName )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName, m_pGrid );
	CEtWorldGrid *pParentName = FindGrid( szParentName, m_pGrid );
	if( pGrid == NULL || pParentName == NULL ) return false;

	if( IsDestroyGrid( szGridName ) == false ) return false;

	pGrid->SetParent( pParentName );
	if( ((CTEtWorldGrid*)pGrid)->WriteGridInfo() == false ) return false;

	// 닫았다가 다시 열어준다.
	Destroy();
	CTEtWorld::Initialize( CFileServer::GetInstance().GetWorkingFolder(), NULL );

	return true;
}

bool CTEtWorld::ChangeGridName( const char *szGridName, const char *szNewName )
{
	CEtWorldGrid *pGrid = FindGrid( szGridName, m_pGrid );
	if( pGrid == NULL ) return false;
	if( FindGrid( szNewName, m_pGrid ) ) return false;

	CString szFullPath;
	CString szNewFullPath;
	szFullPath.Format( "%s\\Grid\\%s", CFileServer::GetInstance().GetWorkingFolder(), szGridName );
	szNewFullPath.Format( "%s\\Grid\\%s", CFileServer::GetInstance().GetWorkingFolder(), szNewName );
	if( rename( szFullPath, szNewFullPath ) != 0 ) return false;

	pGrid->SetName( szNewName );

	for( DWORD i=0; i<pGrid->GetChildCount(); i++ ) {
		CTEtWorldGrid *pChild = (CTEtWorldGrid *)pGrid->GetChildFromIndex(i);
		if( pChild->WriteGridInfo() == false ) {
			MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, "이름 바꾸기 실패!!\n실패한 그리드는 루트로 갑니다요~", "에러", MB_OK );
		}
	}

	// 닫았다가 다시 열어준다.
	Destroy();
	CTEtWorld::Initialize( CFileServer::GetInstance().GetWorkingFolder(), NULL );
	CGlobalValue::GetInstance().RefreshRender();

	return true;
}

void CTEtWorld::LoadNavMesh()
{
	for( DWORD i=0; i< m_pCurrentGrid->GetGridX(); i++ )
	{
		for( DWORD j=0; j< m_pCurrentGrid->GetGridY(); j++ )
		{
			CTEtWorldSector *pSector = (CTEtWorldSector *)m_pCurrentGrid->GetSector( float(i), float(j) );
			if( pSector )
			{
				pSector->LoadNavMesh();
			}
		}
	}
}
