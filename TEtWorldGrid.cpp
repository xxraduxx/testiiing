#include "StdAfx.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"

CTEtWorldGrid::CTEtWorldGrid( CEtWorld *pWorld )
: CEtWorldGrid( pWorld )
{
	m_fYPos = 0.f;
}

CTEtWorldGrid::~CTEtWorldGrid()
{
}


bool CTEtWorldGrid::IsEmptySector( SectorIndex Index )
{
	return ((CTEtWorldSector*)m_ppSector[Index.nX][Index.nY])->IsEmpty();
}

void CTEtWorldGrid::CreateSector( SectorIndex Index )
{
	CTEtWorldSector *pSector = (CTEtWorldSector *)GetSector( Index );
	if( pSector == NULL ) return;

	pSector->CreateSector();
}

void CTEtWorldGrid::EmptySector( SectorIndex Index )
{
	CTEtWorldSector *pSector = (CTEtWorldSector *)GetSector( Index );
	if( pSector == NULL ) return;

	pSector->EmptySector();
}


void CTEtWorldGrid::LoadCommonInfo( bool bLoadThumbnail )
{
	for( DWORD i=0; i<GetGridY(); i++ ) {
		for( DWORD j=0; j<GetGridX(); j++ ) {
			if( strcmp( CGlobalValue::GetInstance().m_szSelectGrid, GetName() ) == NULL && 
				CGlobalValue::GetInstance().m_CheckOutGrid == SectorIndex( j, i ) ) {
					if( CGlobalValue::GetInstance().m_nActiveView != CGlobalValue::GRID ) continue;
			}
			CTEtWorldSector *pSector = (CTEtWorldSector *)GetSector( SectorIndex( j, i ) );
			pSector->FreeSector( CTEtWorldSector::SRL_PREVIEW );
			if( bLoadThumbnail ) 
				pSector->LoadSector( CTEtWorldSector::SRL_PREVIEW );
			else {
				pSector->DefaultInfoFile( CTEtWorldSector::IFT_READ );
			}
		}
	}

	for( DWORD i=0; i<m_pVecChild.size(); i++ ) {
		((CTEtWorldGrid*)m_pVecChild[i])->LoadCommonInfo( bLoadThumbnail );
	}
}
CEtWorldSector *CTEtWorldGrid::AllocSector()
{
	return (CEtWorldSector *)new CTEtWorldSector;
}

bool CTEtWorldGrid::LoadGrid( const char *szName )
{
	FILE *fp;
	CString szFullName;
	szFullName.Format( "%s\\Grid\\%s\\GridInfo.ini", CFileServer::GetInstance().GetWorkingFolder(), szName );

	fopen_s( &fp, szFullName, "rb" );
	if( fp == NULL ) return false;

	InfoFile_Header Header;
	memset( &Header, 0, sizeof(InfoFile_Header) );
	fread( &Header, sizeof(InfoFile_Header), 1, fp );
	fclose(fp);

	szFullName.Format( "%s\\Grid\\%s\\GridDesc.ini", CFileServer::GetInstance().GetWorkingFolder(), szName );
	LoadGridDescription( szFullName.GetBuffer() );

	m_szParentName = Header.szParentName;
	return Initialize( szName, Header.dwGridX, Header.dwGridY, Header.dwGridWidth, Header.dwGridHeight, Header.dwTileSize );
}

bool CTEtWorldGrid::CreateEmptyGrid( CString szGridName, CString szParentName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize )
{
	FILE *fp;
	CString szFullName;
	szFullName.Format( "%s\\Grid\\%s\\GridInfo.ini", CFileServer::GetInstance().GetWorkingFolder(), szGridName );

	// 이미 정보파일이 있으면 생성 못하게 한다.
	fopen_s( &fp, szFullName, "rb" );
	if( fp != NULL ) {
		fclose(fp);
		return false;
	}


	fopen_s( &fp, szFullName, "wb" );

	// Header Write
	InfoFile_Header Header;
	memset( &Header, 0, sizeof(InfoFile_Header) );
	sprintf_s( Header.szParentName, szParentName );
	Header.dwGridX = dwX;
	Header.dwGridY = dwY;
	Header.dwGridWidth = dwWidth;
	Header.dwGridHeight = dwHeight;
	Header.dwTileSize = dwTileSize;

	fwrite( &Header, sizeof(InfoFile_Header), 1, fp );
	fclose(fp);

	Initialize( szGridName, dwX, dwY, dwWidth, dwHeight, dwTileSize );
	WriteGridDescription();
	return true;
}

bool CTEtWorldGrid::WriteGridInfo()
{
	FILE *fp;
	CString szFullName;
	szFullName.Format( "%s\\Grid\\%s\\GridInfo.ini", CFileServer::GetInstance().GetWorkingFolder(), GetName() );

	fopen_s( &fp, szFullName, "wb" );
	if( fp == NULL ) return false;

	// Header Write
	InfoFile_Header Header;
	memset( &Header, 0, sizeof(InfoFile_Header) );
	sprintf_s( Header.szParentName, m_pParent->GetName() );
	Header.dwGridX = m_dwGridX;
	Header.dwGridY = m_dwGridX;
	Header.dwGridWidth = m_dwGridWidth;
	Header.dwGridHeight = m_dwGridHeight;
	Header.dwTileSize = m_dwTileSize;

	fwrite( &Header, sizeof(InfoFile_Header), 1, fp );
	fclose(fp);

	WriteGridDescription();
	return true;
}

void CTEtWorldGrid::LoadGridDescription( const char *szName )
{
	m_szDescription.Empty();
	FILE *fp;

	fopen_s( &fp, szName, "rb" );

	if( !fp ) return;

	ReadCString( &m_szDescription, fp );

	fclose(fp);
}

void CTEtWorldGrid::WriteGridDescription()
{
	FILE *fp;
	CString szFullName;
	szFullName.Format( "%s\\Grid\\%s\\GridDesc.ini", CFileServer::GetInstance().GetWorkingFolder(), GetName() );

	fopen_s( &fp, szFullName.GetBuffer(), "wb" );
	if( !fp ) return;

	WriteCString( &m_szDescription, fp );

	fclose(fp);
}
