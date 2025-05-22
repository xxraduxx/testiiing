#include "StdAfx.h"
#include "TEtWorldSector.h"
#include "TEtWorldGrid.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include "Resource.h"
#include "PerfCheck.h"
#include "TEtWorldProp.h"
#include "TEtWorldActProp.h"
#include "TEtWorldLightProp.h"
#include "TEtBuffProp.h"
#include "PropPoolControl.h"
#include "RenderBase.h"
#include "EtActionCoreMng.h"
#include "TEtWorldProp.h"
#include "ActionCommander.h"
#include "PropRenderView.h"
#include "EnviControl.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"
#include "TEtWorldSound.h"
#include "TEtWorldSoundEnvi.h"
#include "EtNavigationMeshGenerator.h"
#include "navigationmesh.h"
#include "DNTableFile.h"
#include "TEtTrigger.h"
#include "EtResourceMng.h"
#include "TEtWorldWater.h"
#include "EtWater.h"
#include "KdTree.h"
//#include "EtCollisionPrimitive.h"
#include "EtCollisionMng.h"
#include "EventSignalManager.h"
#include "EventSignalItem.h"
#include "EtOptionController.h"
#include "MainFrm.h"
#include "EtComputeDist.h"
#include "EngineUtil.h"
#include "TEtWorldDecal.h"
#include "PointAxisRenderObject.h"
#include "EventRenderView.h"
#include <shlwapi.h>
#include <map>

using namespace std;
#include "SundriesFunc.h"

extern CSyncLock *g_pEtRenderLock;
DNTableFileFormat *CTEtWorldSector::s_pPropSOX = NULL;
CTEtWorldSector::CTEtWorldSector()
{
	m_bEmpty = true;
	m_szDescription.Empty();
	m_pAlpha = NULL;
	m_pGrass = NULL;
	m_nSectorRWLevel = SRL_EMPTY;
	m_pThumbnail = NULL;
	m_vPickPrevPos = m_vPickPos = EtVector3( 0.f, 0.f, 0.f );
	m_rcLastUpdate = CRect( -1, -1, -1, -1 );

	for( int i=0; i<4; i++ ) {
		m_fGrassWidth[i] = 80.f;
		m_fGrassHeightMin[i] = 50.f;
		m_fGrassHeightMax[i] = 80.f;
	}
	m_fGrassShakeMin = 8.f;
	m_fGrassShakeMax = 15.f;

	m_nAttributeBlockSize = 50;
	m_nDrawAttributeCount = 50;

	m_pNavCell = NULL;
	m_pNavMesh = NULL;

	m_pDecalQuadtree = NULL;

	m_bShowWater = false;
	m_bShowAttribute = false;
	m_bShowNavigation = false;
	m_bShowEventArea = false;

	m_fDecalRotate = 0.f;
	m_vDecalColor = D3DCOLOR_ARGB( 255, 255, 255, 255 );
	m_fDecalAlpha = 0.5f;
}

CTEtWorldSector::~CTEtWorldSector()
{
	m_bLoaded = false;
	SAFE_DELETE( m_pDecalQuadtree );
	SAFE_RELEASE( m_pThumbnail );
	SAFE_DELETEA( m_pAlpha );
	SAFE_DELETEA( m_pGrass );
	SAFE_DELETE_AVEC( m_pVecAttributePrimitive );
	SAFE_DELETE_AVEC( m_bVecAttributePrimitiveType );
	SAFE_DELETE( m_pNavCell );
	SAFE_DELETE( m_pNavMesh );
}

bool CTEtWorldSector::Initialize( CEtWorldGrid *pParentGrid, SectorIndex Index )
{
	bool bResult = CEtWorldSector::Initialize( pParentGrid, Index );
	return bResult;
}

bool CTEtWorldSector::CreateTerrain( STerrainInfo *pInfo )
{
	STerrainInfo Info;
	if( pInfo ) Info = *pInfo;
	else Info = GetCreateTerrainInfo();

	m_Handle = EternityEngine::CreateTerrain( &Info );
	if( !m_Handle ) return false;

	for( DWORD i=0; i<m_nVecBlockTypeList.size(); i++ ) {
		ChangeBlockType( i, (TerrainType)m_nVecBlockTypeList[i] );
	}

	if( m_szVecLayerTexture.size() != m_fVecLayerTextureDistance.size() ) ASSERT( 0 &&"텍스쳐 갯수와 Distance 가 틀려요" );
	for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
		if( m_szVecLayerTexture[i].size() != m_fVecLayerTextureDistance[i].size() ) ASSERT( 0 &&"텍스쳐 갯수와 Distance 가 틀려요" );
		for( DWORD j=0; j<m_szVecLayerTexture[i].size(); j++ ) {
			if( !m_szVecLayerTexture[i][j].IsEmpty() )
				SetTexture( i, j, m_szVecLayerTexture[i][j] );
			ChangeTextureDistance( i, j, m_fVecLayerTextureDistance[i][j] );
			ChangeTextureRotation( i, j, m_fVecLayerTextureRotation[i][j] );
		}
	}
	m_Handle->InitializeBlock( -1 );

	// 지금은 하나만 셋팅하면 되기땜시.. For 돌리지 않쿠 첫번쨰껏만 셋팅한다.
	SetGrassTexture( -1, m_szGrassTexture );
	m_bLoaded = true;

	m_Handle->Show( false );
	return true;
}

STerrainInfo CTEtWorldSector::GetCreateTerrainInfo()
{
	STerrainInfo Info;
	m_fTileSize = (float)m_pParentGrid->GetTileSize();
	Info.nSizeX = GetTileWidthCount() - 1;
	Info.nSizeY = GetTileHeightCount() - 1;
	Info.pHeight = m_pHeight;
	Info.pLayerDensity = m_pAlpha;
	Info.pGrassBuffer = m_pGrass;
	Info.fTileSize = m_fTileSize;
	Info.fHeightMultiply = m_fHeightMultiply;
	Info.fTextureDistance = 5000.f;
	Info.Type = TT_CLIFF;
	memcpy( Info.fGrassWidth, m_fGrassWidth, sizeof(Info.fGrassWidth) );
	memcpy( Info.fMinGrassHeight, m_fGrassHeightMin, sizeof(Info.fMinGrassHeight) );
	memcpy( Info.fMaxGrassHeight, m_fGrassHeightMax, sizeof(Info.fMaxGrassHeight) );
	Info.fMaxShake = m_fGrassShakeMax;
	Info.fMinShake = m_fGrassShakeMin;

	EtVector3 vPos = m_Offset;
	vPos.x -= ( m_pParentGrid->GetGridWidth() * 100.f ) / 2.f;
	vPos.z -= ( m_pParentGrid->GetGridHeight() * 100.f ) / 2.f;
	Info.TerrainOffset = vPos;

	return Info;
}

void CTEtWorldSector::SetDescription( CString szStr )
{
	m_szDescription = szStr;
}

CString CTEtWorldSector::GetWorkingPath()
{
	CString szFullPath;
	szFullPath.Format( "%s\\Grid\\%s\\%d_%d", CFileServer::GetInstance().GetWorkingFolder(), m_pParentGrid->GetName(), m_Index.nX, m_Index.nY );

	return szFullPath;
}

bool CTEtWorldSector::DefaultInfoFile( INFO_FILE_TYPE Type )
{
	FILE *fp;
	CString szFullName;
	szFullName.Format( "%s\\default.ini", GetWorkingPath() );

	switch( Type ) {
		case IFT_CREATE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					m_bEmpty = true;
					return false;
				}
				if( m_szDescription.IsEmpty() )
					m_szDescription.Format( "Sector %d.%d", m_Index.nX, m_Index.nY );

				// Write Description
				WriteCString( &m_szDescription, fp );

				fclose(fp);

				m_bEmpty = false;
				return true;
			}
			break;
		case IFT_WRITE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "기본정보 파일이 읽기전용입니다. - "+ szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "기본정보 파일을 쓸 수 없습니다. - "+ szFullName + "\n";
					return false;
				}

				// Write Description
				WriteCString( &m_szDescription, fp );

				fclose(fp);
			}
			break;
		case IFT_READ:
			{
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) {
					m_bEmpty = true;
					return true;
				}
				m_bEmpty = false;

				m_szDescription.Empty();

				// Read Description
				ReadCString( &m_szDescription, fp );

				fclose(fp);

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "기본정보 파일이 읽기전용입니다. - "+ szFullName + "\n";
				}
				return true;
			}
			break;
		case IFT_DELETE:
			{
				m_szDescription.Empty();
				DeleteFile( szFullName );
			}
			break;
	}
	return true;
}

bool CTEtWorldSector::WriteCollisionHeight( FILE *fp )
{
	bool bLoadProp = false;
	bool bLoadNavi = false;
	if( !CheckExistLoadLevel( SRL_PROP, true ) ) {
		PropInfoFile( IFT_READ );
		bLoadProp = true;
	}
	if( !CheckExistLoadLevel( SRL_NAVIGATION, true ) ) {
		NavigationInfoFile( IFT_READ );
		bLoadNavi = true;
	}
	int nSize = GetTileWidthCount() * GetTileHeightCount();
	short *pCollisionHeight = new short[nSize];
	bool bResult = GenerationCollisionHeight( pCollisionHeight );
	if( bResult ) {
		fwrite( pCollisionHeight, nSize * sizeof(short), 1, fp );
	}
	delete []pCollisionHeight;

	if( bLoadProp ) FreeSector( SRL_PROP );
	if( bLoadNavi ) FreeSector( SRL_NAVIGATION );

	return bResult;
}

bool CTEtWorldSector::HeightmapFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..HeightmapFile", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\Height.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETEA( m_pHeight );

				int nSize = GetTileWidthCount() * GetTileHeightCount();
				m_pHeight = new short[nSize];
				
				for( DWORD i=0; i<GetTileHeightCount(); i++ ) {
					for( DWORD j=0; j<GetTileWidthCount(); j++ ) {
						float fTemp = 0.f;
						m_pHeight[ i * GetTileWidthCount() + j ] = (short)fTemp;
					}
				}

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				fwrite( &m_fHeightMultiply, sizeof(float), 1, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pHeight, nSize * sizeof(short), 1, fp );
				if( !feof(fp) ) {
					int asdf = 0;
				}

				fclose( fp );
				m_nHeightSize = nSize;
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETEA( m_pHeight );
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				int nSize;
				fread( &m_fHeightMultiply, sizeof(float), 1, fp );
				fread( &nSize, sizeof(int), 1, fp );
				m_pHeight = new short[nSize];
				fread( m_pHeight, nSize * sizeof(short), 1, fp );

				fclose( fp );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "높이정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
				m_nHeightSize = nSize;
			}
			break;
		case IFT_WRITE:
			{
				int nSize = GetTileWidthCount() * GetTileHeightCount();
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "높이정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "높이정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				fwrite( & m_fHeightMultiply, sizeof(float), 1, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pHeight, nSize * sizeof(short), 1, fp );

				WriteCollisionHeight( fp );
				fclose( fp );
				m_nHeightSize = nSize;
			}
			break;
		case IFT_DELETE:
			SAFE_DELETEA( m_pHeight );
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::AlphaTableFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..AlphaTableFile", szTempStr[Type] );

	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\alphatable.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETEA( m_pAlpha );

				int nSize = GetTileWidthCount() * GetTileHeightCount();
				m_pAlpha = new DWORD[nSize];

				for( DWORD i=0; i<GetTileHeightCount(); i++ ) {
					for( DWORD j=0; j<GetTileWidthCount(); j++ ) {
						m_pAlpha[ i * GetTileWidthCount() + j ] = D3DCOLOR_ARGB( 255, 0, 0, 0 );
					}
				}

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pAlpha, nSize * sizeof(DWORD), 1, fp );

				fclose( fp );
			}

			break;
		case IFT_READ:
			{
				SAFE_DELETEA( m_pAlpha );
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				int nSize;
				fread( &nSize, sizeof(int), 1, fp );
				m_pAlpha = new DWORD[nSize];
				fread( m_pAlpha, nSize * sizeof(DWORD), 1, fp );

				fclose( fp );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "알파정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_WRITE:
			{
				int nSize = GetTileWidthCount() * GetTileHeightCount();
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "알파정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "알파정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pAlpha, nSize * sizeof(DWORD), 1, fp );

				fclose( fp );
			}
			break;
		case IFT_DELETE:
			SAFE_DELETEA( m_pAlpha );
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::GrassTableFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..GrassTableFile", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\grasstable.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETEA( m_pGrass );
				m_szGrassTexture = "GrassTexture.dds";

				int nSize = ( GetTileWidthCount() - 1 ) * ( GetTileHeightCount() - 1 );
				m_pGrass = new char[nSize];

				memset( m_pGrass, 0, sizeof(char)*nSize );

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				fwrite( m_fGrassWidth, sizeof(m_fGrassWidth), 1, fp );
				fwrite( m_fGrassHeightMin, sizeof(m_fGrassHeightMin), 1, fp );
				fwrite( m_fGrassHeightMax, sizeof(m_fGrassHeightMax), 1, fp );
				fwrite( &m_fGrassShakeMin, sizeof(m_fGrassShakeMin), 1, fp );
				fwrite( &m_fGrassShakeMax, sizeof(m_fGrassShakeMax), 1, fp );

				WriteCString( &m_szGrassTexture, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pGrass, nSize * sizeof(char), 1, fp );

				fclose( fp );
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETEA( m_pGrass );
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				fread( m_fGrassWidth, sizeof(m_fGrassWidth), 1, fp );
				fread( m_fGrassHeightMin, sizeof(m_fGrassHeightMin), 1, fp );
				fread( m_fGrassHeightMax, sizeof(m_fGrassHeightMax), 1, fp );
				fread( &m_fGrassShakeMin, sizeof(m_fGrassShakeMin), 1, fp );
				fread( &m_fGrassShakeMax, sizeof(m_fGrassShakeMax), 1, fp );

				m_szGrassTexture.Empty();
				ReadCString( &m_szGrassTexture, fp );
				int nSize;
				fread( &nSize, sizeof(int), 1, fp );
				m_pGrass = new char[nSize];
				fread( m_pGrass, nSize * sizeof(char), 1, fp );

				fclose( fp );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "잔디정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_WRITE:
			{
				int nSize = ( GetTileWidthCount() - 1 ) * ( GetTileHeightCount() - 1 );
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "잔디정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "잔디정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				fwrite( m_fGrassWidth, sizeof(m_fGrassWidth), 1, fp );
				fwrite( m_fGrassHeightMin, sizeof(m_fGrassHeightMin), 1, fp );
				fwrite( m_fGrassHeightMax, sizeof(m_fGrassHeightMax), 1, fp );
				fwrite( &m_fGrassShakeMin, sizeof(m_fGrassShakeMin), 1, fp );
				fwrite( &m_fGrassShakeMax, sizeof(m_fGrassShakeMax), 1, fp );
				WriteCString( &m_szGrassTexture, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pGrass, nSize * sizeof(char), 1, fp );

				fclose( fp );
			}
			break;
		case IFT_DELETE:
			SAFE_DELETEA( m_pGrass );
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::TextureTableFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..TextureTableFile", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\TexTable.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				CString szDefaultTexture[4] = { "DefaultLayerTexture.dds", "DefaultLayerTexture.dds", "DefaultLayerTexture.dds", "DefaultLayerTexture.dds" };

				// 기본 택스쳐 리스트 생성, 저장
				for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
					SAFE_DELETE_VEC( m_szVecLayerTexture[i] );
				}
				SAFE_DELETE_VEC( m_szVecLayerTexture );

				for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
					SAFE_DELETE_VEC( m_fVecLayerTextureDistance[i] );
				}
				SAFE_DELETE_VEC( m_fVecLayerTextureDistance );

				for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
					SAFE_DELETE_VEC( m_fVecLayerTextureRotation[i] );
				}
				SAFE_DELETE_VEC( m_fVecLayerTextureRotation );

				SAFE_DELETE_VEC( m_nVecBlockTypeList );

				GetBlockCount();
				std::vector<CString> szVecList;
				std::vector<float> fVecList;
				std::vector<float> fVecList2;
				for( int i=0; i<m_nBlockCount; i++ ) {
					m_nVecBlockTypeList.push_back((int)TT_CLIFF);
					szVecList.clear();
					fVecList.clear();
					for( int k=0; k<4; k++ ) {
						szVecList.push_back( szDefaultTexture[k] );
						fVecList.push_back( 5000.f );
						fVecList2.push_back( 0.0f );
					}
					m_szVecLayerTexture.push_back( szVecList );
					m_fVecLayerTextureDistance.push_back( fVecList );
					m_fVecLayerTextureRotation.push_back( fVecList2 );
				}

				// Write Block Info
				fwrite( &m_nBlockCount, sizeof(int), 1, fp );
				for( DWORD i=0; i<m_nVecBlockTypeList.size(); i++ ) {
					fwrite( &m_nVecBlockTypeList[i], sizeof(int), 1, fp );
				}

				// Write Block Texture
				for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
					// 텍스쳐 저장
					int nSize = (int)m_szVecLayerTexture[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						WriteCString( &m_szVecLayerTexture[i][j], fp );
					}
				}
				// Write Block Texture Distance
				for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
					int nSize = (int)m_fVecLayerTextureDistance[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						fwrite( &m_fVecLayerTextureDistance[i][j], sizeof(int), 1, fp );
					}
				}
				// Write Block Texture Rotation
				for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
					int nSize = (int)m_fVecLayerTextureRotation[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						fwrite( &m_fVecLayerTextureRotation[i][j], sizeof(int), 1, fp );
					}
				}

				fclose(fp);
			}
			break;
		case IFT_WRITE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "바닥 택스처 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "바닥 택스처 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				// Write Block Info
				fwrite( &m_nBlockCount, sizeof(int), 1, fp );
				for( int i=0; i<m_nBlockCount; i++ ) {
					fwrite( &m_nVecBlockTypeList[i], sizeof(int), 1, fp );
				}
				// Write Block Texture
				for( int i=0; i<m_nBlockCount; i++ ) {
					int nSize = (int)m_szVecLayerTexture[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						WriteCString( &m_szVecLayerTexture[i][j], fp );
					}
				}
				// Write Block Texture Distance
				for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
					int nSize = (int)m_fVecLayerTextureDistance[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						fwrite( &m_fVecLayerTextureDistance[i][j], sizeof(int), 1, fp );
					}
				}
				// Write Block Texture Rotation
				for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
					int nSize = (int)m_fVecLayerTextureRotation[i].size();
					fwrite( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						fwrite( &m_fVecLayerTextureRotation[i][j], sizeof(int), 1, fp );
					}
				}

				fclose(fp);
			}
			break;
		case IFT_READ:
			{
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return true;

				for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
					SAFE_DELETE_VEC( m_szVecLayerTexture[i] );
				}
				SAFE_DELETE_VEC( m_szVecLayerTexture );

				for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
					SAFE_DELETE_VEC( m_fVecLayerTextureDistance[i] );
				}
				SAFE_DELETE_VEC( m_fVecLayerTextureDistance );

				for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
					SAFE_DELETE_VEC( m_fVecLayerTextureRotation[i] );
				}
				SAFE_DELETE_VEC( m_fVecLayerTextureRotation );

				SAFE_DELETE_VEC( m_nVecBlockTypeList );

				// Read Block Info
				int nCount, nType;
				fread( &nCount, sizeof(int), 1, fp );
				if( nCount != GetBlockCount() ) {
					assert(0);
					fclose(fp);
					return false;
				}

				for( int i=0; i<m_nBlockCount; i++ ) {
					fread( &nType, sizeof(int), 1, fp );
					m_nVecBlockTypeList.push_back( nType );
				}

				// Read Block Texture
				CString szTexture;
				std::vector<CString> szVecList;
				for( int i=0; i<nCount; i++ ) {
					int nSize;
					szVecList.clear();
					fread( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						ReadCString( &szTexture, fp );
						szVecList.push_back( szTexture );
					}
					m_szVecLayerTexture.push_back( szVecList );
				}
				// Read Block Texture Distance
				float fDistance;
				std::vector<float> fVecList;
				for( int i=0; i<nCount; i++ ) {
					int nSize;
					fVecList.clear();
					fread( &nSize, sizeof(int), 1, fp );
					for( int j=0; j<nSize; j++ ) {
						fread( &fDistance, sizeof(int), 1, fp );
						fVecList.push_back( fDistance );
					}
					m_fVecLayerTextureDistance.push_back( fVecList );
				}
				// Read Block Texture Rotation - 하위 호환을 위해서 예외처리를 추가한다.(맨 끝이라서 가능하다.)
				bool bReadRotation = false;
				int nValue = 0;
				int nRead = (int)fread( &nValue, sizeof(int), 1, fp );
				if( nRead != 0 )
				{
					bReadRotation = true;
					nValue = sizeof(int);
					fseek( fp, -nValue, SEEK_CUR );
				}

				float fRotation;
				std::vector<float> fVecList2;
				for( int i=0; i<nCount; i++ ) {
					int nSize;
					fVecList2.clear();
					if( bReadRotation ) fread( &nSize, sizeof(int), 1, fp );
					else nSize = 4;
					for( int j=0; j<nSize; j++ ) {
						if( bReadRotation ) fread( &fRotation, sizeof(int), 1, fp );
						else fRotation = 0.0f;
						fVecList2.push_back( fRotation );
					}
					m_fVecLayerTextureRotation.push_back( fVecList2 );
				}

				fclose(fp);

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "바닥 택스처 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
				return true;
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::DecalInfoFile( INFO_FILE_TYPE Type, bool bReadOnly )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Decal", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\DecalInfo.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				int nSize = 0;
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;
				fwrite( &nSize, sizeof(int), 1, fp );
				fclose( fp );
			}
			break;
		case IFT_READ:
			{
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				int nCount = 0;
				fread( &nCount, sizeof(int), 1, fp );
				for( int i=0; i<nCount; i++ ) {
					CTEtWorldDecal *pDecal = (CTEtWorldDecal *)AllocDecal();
					if( pDecal->ReadDecalInfo( fp ) == false ) {
						SAFE_DELETE( pDecal );
						continue;
					}
					if( bReadOnly ) continue;
					if( pDecal->Initialize( pDecal->GetPosition(), pDecal->GetRadius(), pDecal->GetRotate(), pDecal->GetColor(), pDecal->GetAlpha(), pDecal->GetTextureName() ) == false ) {
						SAFE_DELETE( pDecal );
						continue;
					}
					InsertDecal( pDecal );
				}

				fclose( fp );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "데칼정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_WRITE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "데칼정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "데칼정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				int nCount = (int)GetDecalCount();
				fwrite( &nCount, sizeof(int), 1, fp );
				for( int i=0; i<nCount; i++ ) {
					CTEtWorldDecal *pDecal = (CTEtWorldDecal *)GetDecalFromIndex(i);
					pDecal->WriteDecalInfo( fp );
				}

				fclose( fp );
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::ThumbnailFile( INFO_FILE_TYPE Type )
{
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\Thumbnail.tga", GetWorkingPath() );
	switch( Type ) {
		case IFT_WRITE:
		case IFT_CREATE:
			{
				if( Type == IFT_CREATE ) LoadSector( SRL_TERRAIN );
				m_Handle->Show( true );

				CString szActiveName;
				if( CEnviControl::GetInstance().GetActiveElement() )
					szActiveName = CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName();
				CEnviControl::GetInstance().ActivateElement( "Edit Set" );

				float fSizeX = m_pParentGrid->GetGridWidth() * 100.f;
				float fSizeY = m_pParentGrid->GetGridHeight() * 100.f;

				EtCameraHandle CamHandle;
				SCameraInfo CamInfo;
				CamInfo.fWidth = 256.f;
				CamInfo.fHeight = 256.f;
				CamInfo.fViewWidth = fSizeX;
				CamInfo.fViewHeight = fSizeY;
				CamInfo.Type = CT_ORTHOGONAL;
				CamInfo.Target = CT_RENDERTARGET;
				CamInfo.fFar = 1000000.f;
				CamInfo.fFogFar = CamInfo.fFogNear = 1000000.f;
				CamHandle = EternityEngine::CreateCamera( &CamInfo );
				MatrixEx Cross;

				EtVector3 vCross;
				Cross.RotatePitch( 90.f );
				Cross.m_vPosition = *GetOffset();
				Cross.m_vPosition.y = min( fSizeX, fSizeY ) * 1.2f;

				CamHandle->Update( Cross );
				if( CRenderBase::s_bRenderMainScene == true ) {
					SAFE_RELEASE_SPTR( CamHandle );
					if( szActiveName ) CEnviControl::GetInstance().ActivateElement( szActiveName );
					return false;
				}
				g_pEtRenderLock->Lock();
				for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
					CamHandle->AddRenderSkin( m_pVecPropList[i]->GetPropName(), *((CTEtWorldProp*)m_pVecPropList[i])->GetMatEx() );
				}

				m_Handle->EnableFrustumCull( false );
				CamHandle->Render();
				m_Handle->EnableFrustumCull( true );
	
				EtTextureHandle TextureHandle = CamHandle->GetRenderTargetTexture();
				LPD3DXBUFFER pBuffer = NULL;
				EtBaseTexture *pEtTexture = TextureHandle->GetTexturePtr();
//				SetFileAttributes( szFullName, FILE_ATTRIBUTE_NORMAL );
				HRESULT hr = D3DXSaveTextureToFile( szFullName, D3DXIFF_TGA, pEtTexture, NULL );

				g_pEtRenderLock->UnLock();
				SAFE_RELEASE_SPTR( CamHandle );
				if( szActiveName ) CEnviControl::GetInstance().ActivateElement( szActiveName );

				if( Type == IFT_CREATE ) FreeSector( SRL_TERRAIN );

				if( FAILED(hr) ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "썸네일 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "썸네일 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				SAFE_RELEASE( m_pThumbnail );
				m_pThumbnail = CEtDevice::GetInstance().LoadTextureFromFile( szFullName );
			}
			break;
		case IFT_READ:
			{
				if( m_pThumbnail == NULL )
					m_pThumbnail = CEtDevice::GetInstance().LoadTextureFromFile( szFullName );
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

#ifdef _CHECK_MISSING_INDEX
void CTEtWorldSector::CalcMissingIndex()
{
	// Grid 읽는다.
	FILE* fp;
	int nCount = 0;
	CString szGridFullPath, szGridName;
	std::vector< CFileNameString > szGridVecList;
	
	szGridFullPath.Format( "%s\\Grid", CFileServer::GetInstance().GetWorkingFolder() );
	_FindFolder( szGridFullPath, szGridVecList );
	
	std::vector< CFileNameString >::iterator it;
	it = szGridVecList.begin();

	for( ; it != szGridVecList.end() ; ++it )
	{
		szGridFullPath.Format( "%s\\Grid\\%s\\%d_%d\\PropInfo.ini", CFileServer::GetInstance().GetWorkingFolder() , it->c_str() , m_Index.nX, m_Index.nY);
		
		fopen_s( &fp, szGridFullPath, "rb" );
		if( fp == NULL ) 
		{
			continue;
		}

		int nSize, nPropCreateUniqueCount;
		
		fread( &nPropCreateUniqueCount, sizeof(int), 1, fp );
		fread( &nSize, sizeof(int), 1, fp );

		CGlobalValue::GetInstance().m_bIsFirst = true;

		for( int i=0; i<nSize; i++ ) 
		{
			int nPropSize;
			int nSeekPos;
			char szPropName[64] = { 0, };
			nSeekPos = ftell( fp );

			fread( &nPropSize, sizeof(int), 1, fp );
			fread( szPropName, 64, 1, fp );
			fseek( fp, nSeekPos, SEEK_SET );

			CTEtWorldProp *pProp = (CTEtWorldProp *)AllocProp( GetPropClassID( szPropName ) );
			if( pProp == NULL ) 
				continue;

			szGridName.Format( "%s\\Grid\\%s", CFileServer::GetInstance().GetWorkingFolder() , it->c_str() );
			CGlobalValue::GetInstance().m_szFolderName = szGridName;

			if( pProp->ReadPropInfo_ForTheLog( fp ) == false )
			{
				continue;	
			}
		}
		/*CString str;
		nCount++;
		str.Format("%d\n", nCount);
		TRACE(str);*/
	}
	fclose(fp);
}
#endif

bool CTEtWorldSector::PropInfoFile( INFO_FILE_TYPE Type, bool bReadOnly )
{
#ifdef _CHECK_MISSING_INDEX
	CalcMissingIndex();
#endif

	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Prop", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\PropInfo.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				m_nPropCreateUniqueCount = 0;
				fwrite( &m_nPropCreateUniqueCount, sizeof(int), 1, fp );
				int nSize = (int)m_pVecPropList.size();
				fwrite( &nSize, sizeof(int), 1, fp );

				fclose(fp);
				return true;
			}
			break;
		case IFT_WRITE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "프랍 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "프랍 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				int nSize = (int)m_pVecPropList.size();

				fwrite( &m_nPropCreateUniqueCount, sizeof(int), 1, fp );
				fwrite( &nSize, sizeof(int), 1, fp );

				for( int i=0; i<nSize; i++ ) {
					if( ((CTEtWorldProp*)m_pVecPropList[i])->GetCreateUniqueID() == 0 ) {
						((CTEtWorldProp*)m_pVecPropList[i])->SetCreateUniqueID( i + 1 );
					}
					((CTEtWorldProp*)m_pVecPropList[i])->WritePropInfo( fp );
					CGlobalValue::GetInstance().UpdateProgress( nSize, i );
				}

				fclose(fp);

				// Kd Tree 정보 저장
				// Export 만 해준다.
				CString szCollisionBuildFileName;
				szCollisionBuildFileName.Format( "%s\\ColBuild.ini", GetWorkingPath() );

				for( int i=0; i<nSize; i++ ) {
					if( !((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle() ) continue;
					bool bIgnoreCol = ((CTEtWorldProp*)m_pVecPropList[i])->IsIgnoreBuildColMesh();
					if( !bIgnoreCol && ((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle()->IsShow() == false ) bIgnoreCol = true;
					((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle()->EnableCollision( !bIgnoreCol );
				}
				CEtObject::BuildKdTree();
				CFileStream Stream( szCollisionBuildFileName, CFileStream::OPEN_WRITE );
				CEtCollisionMng::GetInstance().Save( &Stream );

				for( int i=0; i<nSize; i++ ) {
					if( !((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle() ) continue;
					((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle()->EnableCollision( true );
				}

				// SaveIgnorePropFolder
				CString szIngorePropFolderFileName;
				szIngorePropFolderFileName.Format( "%s\\IgnorePropFolderInfo.ini", GetWorkingPath() );
				DeleteFile( szIngorePropFolderFileName );

				if( !m_vecIgnorePropFolder.empty() ) {
					CFileStream Stream( szIngorePropFolderFileName, CFileStream::OPEN_WRITE );
					int nCount = (int)m_vecIgnorePropFolder.size();
					Stream.Write( &nCount, sizeof(int) );
					for( int i=0; i<nCount; i++ ) {
						WriteStdString( m_vecIgnorePropFolder[i], &Stream );
					}
				}

				return true;
			}
			break;
		case IFT_READ:
			{
				LoadIgnorePropFolder( GetWorkingPath() );

				CString szStr = "다음 프랍들을 읽을 수 없습니다.\n\n";
				bool bErrorMessage = false;
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
					DeleteProp( m_pVecPropList[i] );
					SAFE_DELETE( m_pVecPropList[i] );
					i--;
				}
				int nSize;
				fread( &m_nPropCreateUniqueCount, sizeof(int), 1, fp );
				fread( &nSize, sizeof(int), 1, fp );

				for( int i=0; i<nSize; i++ ) {
					int nPropSize;
					int nSeekPos;
					char szPropName[64] = { 0, };
					nSeekPos = ftell( fp );

					fread( &nPropSize, sizeof(int), 1, fp );

					fread( szPropName, 64, 1, fp );
					fseek( fp, nSeekPos, SEEK_SET );

					CTEtWorldProp *pProp = (CTEtWorldProp *)AllocProp( GetPropClassID( szPropName ) );
					if( pProp == NULL ) continue;
					if( bReadOnly ) {
						pProp->SetPropName( szPropName );
						InsertProp( pProp );
						fseek( fp, nPropSize + sizeof(int), SEEK_CUR );
						continue;
					}
					if( pProp->ReadPropInfo( fp ) == false ) {
						szStr += pProp->GetPropName();
						szStr += "   - 프랍 타입이 바뀌었습니다만 일단 읽었습니다.";
						szStr += "\n";
						bErrorMessage = true;
//						SAFE_DELETE( pProp );
//						continue;
					}
					if( !pProp->Initialize( this, pProp->GetPropName(), *pProp->GetPosition(), *pProp->GetRotation(), *pProp->GetScale() ) ) {
						szStr += pProp->GetPropName();
						szStr += "   - 파일이 없습니다.";
						szStr += "\n";
						bErrorMessage = true;
						SAFE_DELETE( pProp );
						continue;
					}
					else InsertProp( pProp );
					if( CheckIgnoreProp( szPropName ) ) {
						pProp->Show( false );
					}
					CGlobalValue::GetInstance().UpdateProgress( nSize, i );
				}
				if( bErrorMessage ) {
					MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, szStr, "에러", MB_OK );
				}
				// 임시 겹치는 유니크아이디를 가진 프랍들이 생겨서..
				CTEtWorldProp *pProp;
				for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
					pProp = (CTEtWorldProp *)m_pVecPropList[i];
					if( GetPropFromCreateUniqueID( pProp->GetCreateUniqueID() ) != pProp ) {
						pProp->SetCreateUniqueID( AddPropCreateUniqueCount() );
					}
				}

				fclose(fp);

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "프랍 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
				return true;
			}
			break;
		case IFT_DELETE:
			{
				DeleteFile( szFullName );
				// Kd Tree 정보 삭제
				CString szCollisionBuildFileName;
				szCollisionBuildFileName.Format( "%s\\ColBuild.ini", GetWorkingPath() );
				DeleteFile( szCollisionBuildFileName );
				CString szIngorePropFolderFileName;
				szIngorePropFolderFileName.Format( "%s\\IgnorePropFolderInfo.ini", GetWorkingPath() );
				DeleteFile( szIngorePropFolderFileName );
			}
			break;
	}

	return true;
}

bool CTEtWorldSector::EventInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Event", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\EventAreaInfo.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				m_nEventAreaCreateUniqueCount = 0;

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				fwrite( &m_nEventAreaCreateUniqueCount, sizeof(int), 1, fp );
				int nControlCount = GetControlCount();
				fwrite( &nControlCount, sizeof(int), 1, fp );

				for( int i=0; i<nControlCount; i++ ) {
					CTEtWorldEventControl *pControl = (CTEtWorldEventControl *)GetControlFromIndex(i);
					if( pControl->SaveControl( fp ) == false ) {
						fclose(fp);
						return false;
					}
				}
				fclose(fp);
				return true;
			}
			break;
		case IFT_WRITE:
			{
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "이벤트영역 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "이벤트영역 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				fwrite( &m_nEventAreaCreateUniqueCount, sizeof(int), 1, fp );
				int nControlCount = GetControlCount();
				fwrite( &nControlCount, sizeof(int), 1, fp );

				for( DWORD i=0; i<GetControlCount(); i++ ) {
					CTEtWorldEventControl *pControl = (CTEtWorldEventControl *)GetControlFromIndex(i);
					if( pControl->SaveControl( fp ) == false ) {
						fclose(fp);
						return false;
					}
				}
				fclose(fp);
				return true;
			}
			break;
		case IFT_READ:
			{
				fopen_s( &fp, szFullName, "rb" );
				if( fp == NULL ) return false;

				fread( &m_nEventAreaCreateUniqueCount, sizeof(int), 1, fp );
				int nControlCount = 0;
				fread( &nControlCount, sizeof(int), 1, fp );

				CTEtWorldEventControl *pControl;
				for( int i=0; i<nControlCount; i++ ) {
					pControl = (CTEtWorldEventControl *)AllocControl();
					if( pControl->LoadControl( fp ) == false ) {
						//MessageBoxA(NULL, "EventAreaInfo::pControl is null!", "Error", MB_OK);
						SAFE_DELETE( pControl );
						continue;
					}
					CEtWorldSector::InsertControl( (CEtWorldEventControl*)pControl );
				}
				fclose(fp);

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "이벤트영역 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
				return true;
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::SoundInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Sound", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\SoundInfo.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETE( m_pSoundInfo );
				m_pSoundInfo = AllocSound();

				return ((CTEtWorldSound*)m_pSoundInfo)->Save( szFullName );
			}
			break;
		case IFT_WRITE:
			{
				bool bResult = ((CTEtWorldSound*)m_pSoundInfo)->Save( szFullName );
				if( bResult == false ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "사운드 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "사운드 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
				}
				return bResult;
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETE( m_pSoundInfo );
				m_pSoundInfo = AllocSound();

				if( ((CTEtWorldSound*)m_pSoundInfo)->Load( szFullName ) == false ) return false;

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "사운드 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}

	return true;
}

bool CTEtWorldSector::AttributeInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Attribute", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\HeightAttribute.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETEA( m_pAttribute );

				int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nSize = nWidthCount * nHeightCount;

				m_pAttribute = new char[nSize];
				memset( m_pAttribute, 0, nSize );

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) return false;

				fwrite( &m_nAttributeBlockSize, sizeof(int), 1, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pAttribute, nSize, 1, fp );

				fclose(fp);

			}
			break;
		case IFT_WRITE:
			{
				int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nSize = nWidthCount * nHeightCount;

				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "바닥 속성 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "바닥 속성 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				fwrite( &m_nAttributeBlockSize, sizeof(int), 1, fp );
				fwrite( &nSize, sizeof(int), 1, fp );
				fwrite( m_pAttribute, nSize, 1, fp );

				fclose(fp);
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETEA( m_pAttribute );
				SAFE_DELETE_PVEC( m_pVecAttributePrimitive );
				SAFE_DELETE_VEC( m_bVecAttributePrimitiveType );

				int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nHeightCount = (int)( GetTileHeightCount() * GetTileSize() ) / m_nAttributeBlockSize;
				int nSize = nWidthCount * nHeightCount;

				m_pAttribute = new char[nSize];

				fopen_s( &fp, szFullName, "rb" );
//				if( fp == NULL ) return false;
				if( fp ) {
					int nTempSize;
					fread( &m_nAttributeBlockSize, sizeof(int), 1, fp );
					fread( &nTempSize, sizeof(int), 1, fp );
					if( nTempSize != nSize ) return false;
					fread( m_pAttribute, nSize, 1, fp );

					fclose(fp);
				}
				else {
					memset( m_pAttribute, 0, nSize );
				}


				EtVector3 vOffset;
				float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
				float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

				vOffset.x = m_Offset.x - ( fWidth / 2.f );
				vOffset.z = m_Offset.z - ( fHeight / 2.f );
				vOffset.y = 0.f;

				int nBlockCountX, nBlockCountY;

				nBlockCountX = nWidthCount / m_nDrawAttributeCount;
				nBlockCountY = nHeightCount / m_nDrawAttributeCount;
				if( nWidthCount % m_nDrawAttributeCount > 0 ) nBlockCountX += 1;
				if( nHeightCount % m_nDrawAttributeCount > 0 ) nBlockCountY += 1;
				SPrimitiveDraw3D *pBuffer;
				bool *pTriangleType;

				for( int m=0; m<nBlockCountY; m++ ) {
					for( int n=0; n<nBlockCountX; n++ ) {
						int nPrimitiveX = m_nDrawAttributeCount;
						int nPrimitiveY = m_nDrawAttributeCount;
						if( n == nBlockCountX - 1 && nWidthCount % m_nDrawAttributeCount > 0 ) {
							nPrimitiveX = nWidthCount % m_nDrawAttributeCount;
						}
						if( m == nBlockCountY - 1 && nHeightCount % m_nDrawAttributeCount > 0 ) {
							nPrimitiveY = nHeightCount % m_nDrawAttributeCount;
						}
						DWORD dwPrimitiveCount = nPrimitiveX * nPrimitiveY * 2;
						m_dwVecAttributePrimitiveCount.push_back( dwPrimitiveCount );
						pBuffer = new SPrimitiveDraw3D[ dwPrimitiveCount * 3 ];
						pTriangleType = new bool[ dwPrimitiveCount / 2 ];

						EtVector3 vPos;
						int nPosX = ( n * m_nDrawAttributeCount );
						int nPosY = ( m * m_nDrawAttributeCount );
						for( int j=0; j<nPrimitiveY; j++ ) {
							for( int i=0; i<nPrimitiveX; i++ ) {

								vPos.x = ( (i+nPosX) * m_nAttributeBlockSize ) + vOffset.x;
								vPos.z = ( (j+nPosY) * m_nAttributeBlockSize ) + vOffset.z;
								vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;

								pBuffer[(j*6) * nPrimitiveX + (i*6)].Position = vPos;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+3)].Position = vPos;

								vPos.x = ( ((i+nPosX)+1) * m_nAttributeBlockSize ) + vOffset.x;
								vPos.z = ( (j+nPosY) * m_nAttributeBlockSize ) + vOffset.z;
								vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+1)].Position = vPos;

								vPos.x = ( ((i+nPosX)+1) * m_nAttributeBlockSize ) + vOffset.x;
								vPos.z = ( ((j+nPosY)+1) * m_nAttributeBlockSize ) + vOffset.z;
								vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+2)].Position = vPos;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+4)].Position = vPos;

								vPos.x = ( (i+nPosX) * m_nAttributeBlockSize ) + vOffset.x;
								vPos.z = ( ((j+nPosY)+1) * m_nAttributeBlockSize ) + vOffset.z;
								vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+5)].Position = vPos;


								DWORD dwColor = 0;
								char cAttr = m_pAttribute[ (j+nPosY) * nWidthCount + (i+nPosX) ];
								if( cAttr & 0x01 ) dwColor = 0x55EE2222;
								if( cAttr & 0x02 ) dwColor = 0x552222EE;
								if( cAttr & 0x04 ) dwColor = 0x5522EE22;
								if( cAttr & 0x08 ) dwColor = 0x55EEEE11;
								if( ( cAttr & 0xf ) == 0xf ) dwColor = 0x55ff00ff;

								pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = dwColor;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = dwColor;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = dwColor;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = dwColor;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = dwColor;
								pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = dwColor;

								pTriangleType[ j * nPrimitiveX + i ] = false;
							}
						}
						m_pVecAttributePrimitive.push_back( pBuffer );
						m_bVecAttributePrimitiveType.push_back( pTriangleType );

					}
				}

				UpdateAttribute( CRect( 0, 0, nWidthCount, nHeightCount ), false );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "바닥 속성 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}

	return true;
}

bool CTEtWorldSector::NavigationInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Navigation", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\Navigation.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
			}
			break;
		case IFT_WRITE:
			{
				bool bGeneration = IsGenerationNavigationMesh();
				GenerationNavigationMesh();

				CFileStream Stream( szFullName, CFileStream::OPEN_WRITE );
				if( !Stream.IsValid() ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "네비게이션 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "네비게이션 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
					return false;
				}

				m_pNavMesh->SaveToStream( &Stream );

				if( !bGeneration )
					GenerationNavigationMesh( false );
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETE( m_pNavMesh );
				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "네비게이션 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
				CFileStream Stream( szFullName, CFileStream::OPEN_READ );
				if( Stream.IsValid() ) {
					m_pNavMesh = new NavigationMesh;
					m_pNavMesh->LoadFromStream( &Stream );
				}
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}

	return true;
}

bool CTEtWorldSector::TriggerInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Trigger", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName, szDefineName;
	szFullName.Format( "%s\\Trigger.ini", GetWorkingPath() );
	szDefineName.Format( "%s\\TriggerDefine.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETE( m_pTrigger );
				m_pTrigger = AllocTrigger();

				((CTEtTrigger*)m_pTrigger)->SaveDefine( szDefineName );
				return ((CTEtTrigger*)m_pTrigger)->Save( szFullName );
			}
			break;
		case IFT_WRITE:
			{
				if( m_pTrigger == NULL ) break;

				bool bResult;
				bResult = ((CTEtTrigger*)m_pTrigger)->SaveDefine( szDefineName );
				if( bResult == false ) {
					DWORD dwAttr = GetFileAttributes( szDefineName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "트리거 선언 정보 파일이 읽기전용입니다. - " + szDefineName + "\n";
					}
					else m_szReadyOnlyMsg += "트리거 선언 정보 파일을 쓸 수 없습니다. - " + szDefineName + "\n";
				}
				bResult = ((CTEtTrigger*)m_pTrigger)->Save( szFullName );
				if( bResult == false ) {
					DWORD dwAttr = GetFileAttributes( szDefineName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "트리거 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "트리거 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";
				}
				return bResult;
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETE( m_pTrigger );
				m_pTrigger = AllocTrigger();

				((CTEtTrigger*)m_pTrigger)->LoadDefine( szDefineName );
				bool bResult = ((CTEtTrigger*)m_pTrigger)->Load( szFullName );

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "트리거 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}

				if( IsReadOnlyFile( szDefineName ) ) {
					m_szReadyOnlyMsg += "트리거 선언 정보 파일이 읽기전용입니다. - " + szDefineName + "\n";
				}
				return bResult;

			}
			break;
		case IFT_DELETE:
			DeleteFile( szDefineName );
			DeleteFile( szFullName );
			break;
	}

	return true;
}

bool CTEtWorldSector::WaterInfoFile( INFO_FILE_TYPE Type )
{
	char *szTempStr[] = { "Create", "Load", "Save", "Delete" };
	CGlobalValue::GetInstance().UpdateInitDesc( "%s Sector..Water", szTempStr[Type] );
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\Water.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
			{
				SAFE_DELETE( m_pWater );
				m_pWater = AllocWater();

				return ((CTEtWorldWater*)m_pWater)->Save( szFullName );
			}
			break;
		case IFT_WRITE:
			{
				if( m_pWater == NULL ) break;

				bool bResult = ((CTEtWorldWater*)m_pWater)->Save( szFullName );
				if( bResult == false ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "물 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "물 정보 파일을 쓸 수 없습니다. - " + szFullName + "\n";

				}
				return bResult;
			}
			break;
		case IFT_READ:
			{
				SAFE_DELETE( m_pWater );
				m_pWater = AllocWater();
				m_pWater->Initialize();

				if( ((CTEtWorldWater*)m_pWater)->Load( szFullName ) == false ) return false;

				if( IsReadOnlyFile( szFullName ) ) {
					m_szReadyOnlyMsg += "물 정보 파일이 읽기전용입니다. - " + szFullName + "\n";
				}
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

bool CTEtWorldSector::SectorSizeInfoFile( INFO_FILE_TYPE Type )
{
	FILE *fp = NULL;
	CString szFullName;
	szFullName.Format( "%s\\SectorSize.ini", GetWorkingPath() );
	switch( Type ) {
		case IFT_CREATE:
		case IFT_WRITE:
			{
				FILE *fp;
				fopen_s( &fp, szFullName, "wb" );
				if( fp == NULL ) {
					DWORD dwAttr = GetFileAttributes( szFullName );
					if( dwAttr & FILE_ATTRIBUTE_READONLY ) {
						m_szReadyOnlyMsg += "섹터 사이즈 파일이 읽기전용입니다. - "+ szFullName + "\n";
					}
					else m_szReadyOnlyMsg += "섹터 사이즈 파일을 쓸 수 없습니다. - "+ szFullName + "\n";
					return false;
				}

				float fHeightCenter, fSize;
				CalcSectorSize( fHeightCenter, fSize );

				fwrite( &fHeightCenter, sizeof(float), 1, fp );
				fwrite( &fSize, sizeof(float), 1, fp );

				fclose(fp);
			}
			break;
		case IFT_READ:
			{
			}
			break;
		case IFT_DELETE:
			DeleteFile( szFullName );
			break;
	}
	return true;
}

void CTEtWorldSector::FlushResource()
{
	// 나중에 리소스 지워준다.
}

bool CTEtWorldSector::CreateSector()
{
	bool bResult = true;
	if( DefaultInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && HeightmapFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && AlphaTableFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && GrassTableFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && TextureTableFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && PropInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && ThumbnailFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && EventInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && SoundInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && AttributeInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && NavigationInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && TriggerInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && WaterInfoFile( IFT_CREATE ) == false ) bResult = false;
	if( bResult == true && DecalInfoFile( IFT_CREATE ) == false ) bResult = false;

	if( bResult == false ) FreeSector( SRL_PREVIEW | SRL_TERRAIN | SRL_PROP | SRL_EVENT | SRL_SOUND | SRL_ATTRIBUTE | SRL_NAVIGATION | SRL_TRIGGER | SRL_WATER );

	CString szPath;
	szPath.Format( "%s\\Grid\\%d_%d", CFileServer::GetInstance().GetWorkingFolder(), GetIndex().nX, GetIndex().nY );
	CFileServer::GetInstance().ApplyNewFiles( szPath );

	return bResult;
}

bool CTEtWorldSector::EmptySector()
{
	bool bResult = true;
	if( DefaultInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && ThumbnailFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && HeightmapFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && GrassTableFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && TextureTableFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && AlphaTableFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && PropInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && EventInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && SoundInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && AttributeInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && NavigationInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && TriggerInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && WaterInfoFile( IFT_DELETE ) == false ) bResult = false;
	if( bResult == true && DecalInfoFile( IFT_DELETE ) == false ) bResult = false;

	FlushResource();
	m_bEmpty = true;

	CString szPath;
	szPath.Format( "%s\\Grid\\%d_%d", CFileServer::GetInstance().GetWorkingFolder(), GetIndex().nX, GetIndex().nY );
	CFileServer::GetInstance().ApplyDeleteFiles( szPath );

	return bResult;
}

bool CTEtWorldSector::CheckExistLoadLevel( SECTOR_RW_LEVEL Level, bool bAdd )
{
	if( bAdd == true ) {
		if( m_nSectorRWLevel & Level ) return true;
		m_nSectorRWLevel |= Level;
		return false;
	}
	else {
		if( m_nSectorRWLevel & Level ) {
			m_nSectorRWLevel &= ~Level;
			return true;
		}
		return false;
	}

	return true;
}

bool CTEtWorldSector::LoadSector( int Level )
{
	m_szReadyOnlyMsg.Empty();
	if( Level & SRL_PREVIEW && !CheckExistLoadLevel( SRL_PREVIEW, true ) ) {
		DefaultInfoFile( IFT_READ );
		if( IsEmpty() ) return true;
		ThumbnailFile( IFT_READ );
	}
	if( Level & SRL_TERRAIN && !CheckExistLoadLevel( SRL_TERRAIN, true ) ) {
		HeightmapFile( IFT_READ );
		AlphaTableFile( IFT_READ );
		TextureTableFile( IFT_READ );
		GrassTableFile( IFT_READ );
		CreateTerrain();

		DecalInfoFile( IFT_READ );
	}
	if( Level & SRL_PROP && !CheckExistLoadLevel( SRL_PROP, true ) ) {
		PropInfoFile( IFT_READ );
	}
	if( Level & SRL_EVENT && !CheckExistLoadLevel( SRL_EVENT, true ) ) {
		EventInfoFile( IFT_READ );
		for( DWORD i=0; i<CEventSignalManager::GetInstance().GetSignalItemCount(); i++ ) {
			CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItem(i);
			InsertControl( pItem->GetName() );
			GetControlFromName( pItem->GetName() )->SetUniqueID(pItem->GetUniqueIndex());
		}
	}
	if( Level & SRL_SOUND && !CheckExistLoadLevel( SRL_SOUND, true ) ) {
		SoundInfoFile( IFT_READ );
	}
	if( Level & SRL_ATTRIBUTE && !CheckExistLoadLevel( SRL_ATTRIBUTE, true ) ) {
		AttributeInfoFile( IFT_READ );
	}
	if( Level & SRL_NAVIGATION && !CheckExistLoadLevel( SRL_NAVIGATION, true ) ) {
		NavigationInfoFile( IFT_READ );
	}
	if( Level & SRL_TRIGGER && !CheckExistLoadLevel( SRL_TRIGGER, true ) ) {
		TriggerInfoFile( IFT_READ );
	}
	if( Level & SRL_WATER && !CheckExistLoadLevel( SRL_WATER, true ) ) {
		WaterInfoFile( IFT_READ );
	}

	return true;
}

void CTEtWorldSector::ShowWarningMesasge()
{
	if( !m_szReadyOnlyMsg.IsEmpty() ) {
		MessageBox( ((CMainFrame*)AfxGetMainWnd())->m_hWnd, m_szReadyOnlyMsg.GetBuffer(), "경고", MB_OK );
	}
}

bool CTEtWorldSector::SaveSector( int Level, bool bWarningMsg )
{
	m_szReadyOnlyMsg.Empty();
	bool bFailed = false;
	if( Level & SRL_PREVIEW && m_nSectorRWLevel & SRL_PREVIEW ) {
		if( !DefaultInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_TERRAIN && m_nSectorRWLevel & SRL_TERRAIN ) {
		bool bResult1 = HeightmapFile( IFT_WRITE );
		bool bResult2 = AlphaTableFile( IFT_WRITE );
		bool bResult3 = TextureTableFile( IFT_WRITE );
		bool bResult4 = GrassTableFile( IFT_WRITE );
		bool bResult5 = ThumbnailFile( IFT_WRITE );
		bool bResult6 = DecalInfoFile( IFT_WRITE );
		SectorSizeInfoFile( IFT_WRITE );
		if( !bResult1 || !bResult2 || !bResult3 || !bResult4 || !bResult5 || !bResult6 ) bFailed = true;
	}
	if( Level & SRL_PROP && m_nSectorRWLevel & SRL_PROP ) {
		bool bResult1 = PropInfoFile( IFT_WRITE );
		bool bResult2 = ThumbnailFile( IFT_WRITE );
		SectorSizeInfoFile( IFT_WRITE );
		if( !bResult1 || !bResult2 ) bFailed = true;
	}
	if( Level & SRL_EVENT && m_nSectorRWLevel & SRL_EVENT ) {
		if( !EventInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_SOUND && m_nSectorRWLevel & SRL_SOUND ) {
		if( !SoundInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_ATTRIBUTE && m_nSectorRWLevel & SRL_ATTRIBUTE ) {
		if( !AttributeInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_NAVIGATION && m_nSectorRWLevel & SRL_NAVIGATION ) {
		if( !NavigationInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_TRIGGER && m_nSectorRWLevel & SRL_TRIGGER ) {
		if( !TriggerInfoFile( IFT_WRITE ) ) bFailed = true;
	}
	if( Level & SRL_WATER && m_nSectorRWLevel & SRL_WATER ) {
		if( !WaterInfoFile( IFT_WRITE ) ) bFailed = true;
	}

	if( bFailed && bWarningMsg && !m_szReadyOnlyMsg.IsEmpty() ) {
		MessageBox( ((CMainFrame*)AfxGetMainWnd())->m_hWnd, m_szReadyOnlyMsg.GetBuffer(), "에러", MB_OK );
	}

	return !bFailed;
}

bool CTEtWorldSector::FreeSector( int Level )
{
	if( Level & SRL_PREVIEW && CheckExistLoadLevel( SRL_PREVIEW, false ) ) {
		m_szDescription.Empty();
	}
	if( Level & SRL_TERRAIN && CheckExistLoadLevel( SRL_TERRAIN, false ) ) {
		SAFE_DELETEA( m_pHeight );
		SAFE_DELETEA( m_pAlpha );
		SAFE_DELETEA( m_pGrass );

		for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
			SAFE_DELETE_VEC( m_szVecLayerTexture[i] );
		}
		SAFE_DELETE_VEC( m_szVecLayerTexture );

		for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
			SAFE_DELETE_VEC( m_fVecLayerTextureDistance[i] );
		}
		SAFE_DELETE_VEC( m_fVecLayerTextureDistance );

		for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
			SAFE_DELETE_VEC( m_fVecLayerTextureRotation[i] );
		}
		SAFE_DELETE_VEC( m_fVecLayerTextureRotation );

		SAFE_DELETE_VEC( m_nVecBlockTypeList );

		SAFE_DELETE( m_pDecalQuadtree );
		float fSize = max( m_pParentGrid->GetGridWidth() * 100.f, m_pParentGrid->GetGridHeight() * 100.f );
		m_pDecalQuadtree = new CEtQuadtree<CEtWorldDecal *>;
		m_pDecalQuadtree->Initialize( EtVector2( m_Offset.x, m_Offset.z ), fSize );

		SAFE_DELETE_PVEC( m_pVecDecalList );

		SAFE_RELEASE_SPTR( m_Handle );
	}
	if( Level & SRL_PROP && CheckExistLoadLevel( SRL_PROP, false ) ) {
		SAFE_DELETE( m_pPropOctree );
		float fSize = max( m_pParentGrid->GetGridWidth() * 100.f, m_pParentGrid->GetGridHeight() * 100.f );
		fSize = max( fSize, GetHeightMultiply() * 32768.f );
		m_pPropOctree = new CEtOctree<CEtWorldProp *>;
		m_pPropOctree->Initialize( m_Offset, fSize );
		m_nPropCreateUniqueCount = 0;

		SAFE_DELETE_PVEC( m_pVecPropList );
	}
	if( Level & SRL_EVENT && CheckExistLoadLevel( SRL_EVENT, false ) ) {
		SAFE_DELETE_PVEC( m_pVecAreaControl );
		m_nEventAreaCreateUniqueCount = 0;
	}
	if( Level & SRL_SOUND && CheckExistLoadLevel( SRL_SOUND, false ) ) {
		SAFE_DELETE( m_pSoundInfo );
	}
	if( Level & SRL_ATTRIBUTE && CheckExistLoadLevel( SRL_ATTRIBUTE, false ) ) {
		SAFE_DELETEA( m_pAttribute );
		SAFE_DELETE_AVEC( m_pVecAttributePrimitive );
		SAFE_DELETE_AVEC( m_bVecAttributePrimitiveType );
	}
	if( Level & SRL_NAVIGATION && CheckExistLoadLevel( SRL_NAVIGATION, false ) ) {
		SAFE_DELETE( m_pNavCell );
		SAFE_DELETE( m_pNavMesh );
	}
	if( Level & SRL_TRIGGER && CheckExistLoadLevel( SRL_TRIGGER, false ) ) {
		SAFE_DELETE( m_pTrigger );
	}
	if( Level & SRL_WATER && CheckExistLoadLevel( SRL_WATER, false ) ) {
		SAFE_DELETE( m_pWater );
	}
	return true;
}

void CTEtWorldSector::Render( LOCAL_TIME LocalTime )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		CTEtWorldProp *pProp = (CTEtWorldProp *)m_pVecPropList[i];
		pProp->Render( LocalTime );
	}
	for( DWORD i=0; i<m_pVecDecalList.size(); i++ ) {
		CTEtWorldDecal *pDecal = (CTEtWorldDecal *)m_pVecDecalList[i];
		pDecal->Render( LocalTime );
	}
	if( m_pWater && m_bShowWater ) m_pWater->Render( LocalTime );
	if( m_bShowAttribute ) DrawAttribute();
	if( m_bShowNavigation ) DrawNavigationMesh();
	if( m_bShowEventArea ) DrawArea( CGlobalValue::GetInstance().m_szSelectControlName );
}

void CTEtWorldSector::UpdateBrush( int nX, int nY, float fRadian, DWORD dwColor )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) vPickPos = m_vPickPos;
	else m_vPickPos = vPickPos;
	m_vPickPos = vPickPos;

	float fCurAngle = 0.f;
	EtVector3 vPos[2];
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	vPickPos -= vOffset;

	vPos[0].x = vPickPos.x + ( cos( EtToRadian( fCurAngle ) ) * ( fRadian / 2.f ) );
	vPos[0].z = vPickPos.z + ( sin( EtToRadian( fCurAngle ) ) * ( fRadian / 2.f ) );
	if( vPos[0].x < 0.f ) vPos[0].x = 0.f;
	if( vPos[0].x >= fWidth ) vPos[0].x = fWidth;
	if( vPos[0].z < 0.f ) vPos[0].z = 0.f;
	if( vPos[0].z >= fHeight ) vPos[0].z = fHeight;
	vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

	for( DWORD i=0;; i++ ) {
		vPos[1].x = vPickPos.x + ( cos( EtToRadian( fCurAngle ) ) * ( fRadian / 2.f ) );
		vPos[1].z = vPickPos.z + ( sin( EtToRadian( fCurAngle ) ) * ( fRadian / 2.f ) );
		if( vPos[1].x < 0.f ) vPos[1].x = 0.f;
		if( vPos[1].x >= fWidth ) vPos[1].x = fWidth;
		if( vPos[1].z < 0.f ) vPos[1].z = 0.f;
		if( vPos[1].z >= fHeight ) vPos[1].z = fHeight;

		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );

		vPos[0] = vPos[1];

		if( fCurAngle >= 360.f ) break;
		fCurAngle += 5.f;
	}
}

bool CTEtWorldSector::BeginRectBrush( int nX, int nY )
{
	if( !m_Handle ) return false;

	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return false;
	m_vPickPrevPos = vPickPos;
	return true;
}

void CTEtWorldSector::PickProp( int nX, int nY, DNVector(CEtWorldProp *) &pVecList )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) {
		CEtWorldProp *pPickObject = NULL;
		EtVector3 Position, Direction;
		CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );
		m_pPropOctree->PickBySize( Position, Direction, pPickObject );
		if( pPickObject ) {
			if( std::find( pVecList.begin(), pVecList.end(), pPickObject ) == pVecList.end() ) {
				pVecList.push_back( pPickObject );
				return;
			}
		}
	}

	CEtWorldProp *pPickObject = NULL;
	EtVector3 Position, Direction;
	float fMinDist;

	fMinDist = FLT_MAX;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );
	for( int i = 0; i < ( int )m_pVecPropList.size(); i++ )
	{
		if( m_pVecPropList[ i ]->IsControlLock() ) continue;

		SOBB BoundingBox;

		m_pVecPropList[ i ]->GetBoundingBox( BoundingBox );
		if( TestLineToOBB( Position, Direction, BoundingBox ) )
		{
			EtMeshHandle hMesh;
			EtMatrix WorldMat;
			float fDist;
			hMesh = m_pVecPropList[ i ]->GetObjectHandle()->GetMesh();
			WorldMat = *m_pVecPropList[ i ]->GetObjectHandle()->GetWorldMat();
			if( hMesh->TestLineToTriangle( Position, Direction, WorldMat, fDist ) )
			{
				if( fDist < fMinDist )
				{
					fMinDist = fDist;
					pPickObject = m_pVecPropList[ i ];
				}
			}
		}
	}
	if( pPickObject )
	{
		if( std::find( pVecList.begin(), pVecList.end(), pPickObject ) == pVecList.end() )
			pVecList.push_back( pPickObject );
	}
}

void CTEtWorldSector::UpdateRectBrush( int nX, int nY, DWORD dwColor, DNVector(CEtWorldProp *) &pVecList )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) {
		CEtWorldProp *pPickObject = NULL;
		EtVector3 Position, Direction;

		CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );
		m_pPropOctree->PickBySize( Position, Direction, pPickObject );

		if( pPickObject ) {
			if( std::find( pVecList.begin(), pVecList.end(), pPickObject ) == pVecList.end() ) {
				pVecList.push_back( pPickObject );
				return;
			}
		}
		else {
			vPickPos = m_vPickPos;
		}
	}
	else m_vPickPos = vPickPos;

	EtVector3 vPos[2];
	EtVector2 vRect[5];
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	vPickPos -= vOffset;
	EtVector3 vPickPrevPos = m_vPickPrevPos - vOffset;

	vRect[0].x = vPickPrevPos.x;
	vRect[0].y = vPickPrevPos.z;
	vRect[1].x = vPickPos.x;
	vRect[1].y = vPickPrevPos.z;
	vRect[2].x = vPickPos.x;
	vRect[2].y = vPickPos.z;
	vRect[3].x = vPickPrevPos.x;
	vRect[3].y = vPickPos.z;
	vRect[4] = vRect[0];

	for( int i=0; i<4; i++ ) {
		vPos[0].x = vRect[i].x;
		vPos[0].z = vRect[i].y;
		vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

		EtVector2 vDir = vRect[i+1] - vRect[i];
		int nCount = (int)EtVec2Length( &vDir ) / 100;
		EtVec2Normalize( &vDir, &vDir );
		for( int j=0; j<nCount; j++ ) {
			vPos[1].x = vPos[0].x + ( vDir.x * 100.f );
			vPos[1].z = vPos[0].z + ( vDir.y * 100.f );
			vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

			EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );
			vPos[0] = vPos[1];
		}
		vPos[1].x = vRect[i+1].x;
		vPos[1].z = vRect[i+1].y;
		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );
	}


	if( m_vPickPrevPos != m_vPickPos ) {
		SAABox Box;
		EtVector3 vTemp[2];
		vTemp[0] = m_vPickPrevPos;
		vTemp[1] = m_vPickPos;
		vTemp[0].y = GetHeightMultiply() * 32000.f;
		vTemp[1].y = GetHeightMultiply() * -32000.f;

		Box.Reset();
		Box.AddPoint( vTemp[0] );
		Box.AddPoint( vTemp[1] );
		m_pPropOctree->Pick( Box, pVecList, false );
	}

	CEtWorldProp *pPickObject = NULL;
	EtVector3 Position, Direction;
	float fMinDist;

	fMinDist = FLT_MAX;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );
	for( int i = 0; i < ( int )m_pVecPropList.size(); i++ )
	{
		if( m_pVecPropList[ i ]->IsControlLock() ) continue;

		SOBB BoundingBox;

		m_pVecPropList[ i ]->GetBoundingBox( BoundingBox );
		if( TestLineToOBB( Position, Direction, BoundingBox ) )
		{
			if( ((CTEtWorldProp*)m_pVecPropList[i])->IsEmptyPickMesh() ) {
				float fDist = EtVec3Length( &EtVector3( ((CTEtWorldProp*)m_pVecPropList[i])->GetMatEx()->m_vPosition - Position ) );
				if( fDist < fMinDist ) {
					fMinDist = fDist;
					pPickObject = m_pVecPropList[i];
				}
			}
			else {
				EtMeshHandle hMesh;
				EtMatrix WorldMat;
				float fDist;
				hMesh = m_pVecPropList[ i ]->GetObjectHandle()->GetMesh();
				WorldMat = *m_pVecPropList[ i ]->GetObjectHandle()->GetWorldMat();
				if( hMesh->TestLineToTriangle( Position, Direction, WorldMat, fDist ) )
				{
					if( fDist < fMinDist )
					{
						fMinDist = fDist;
						pPickObject = m_pVecPropList[ i ];
					}
				}
			}
		}
	}
	if( pPickObject )
	{
		if( std::find( pVecList.begin(), pVecList.end(), pPickObject ) == pVecList.end() )
			pVecList.push_back( pPickObject );
	}
}

int CTEtWorldSector::UpdateAreaControlType( int nX, int nY, const char *szControlName )
{
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return -1;

	m_vPickPos = vPickPos;

	EtVector3 vPos[2];
	EtVector2 vRect[5];
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	vPickPos -= vOffset;

	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( pControl == NULL ) return -1;

	EtVector3 Position, Direction;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );

//	SAABox Box;
	EtVector3 vSize;

	DWORD dwMinIndex = -1;
	float fSize = 0.f;
	float fMinSize = 0.f;
	float fSizeX, fSizeZ;
	EtVector3 vAreaOffset;
	SOBB ResultBox;

	for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
		CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);

		SOBB Box;

		Box.Center = *pArea->GetMin() + ( *pArea->GetMax() - *pArea->GetMin() ) / 2.f;
		Box.Center.y = GetHeight( Box.Center.x, Box.Center.z );
		Box.Center += vOffset;
		Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
		Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
		Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
		Box.Extent[0] = ( pArea->GetMax()->x - pArea->GetMin()->x ) / 2.f;
		Box.Extent[1] = 1.f;//( ( pArea->GetMax()->y - pArea->GetMin()->y ) / 2.f ) + 1.f;
		Box.Extent[2] = ( pArea->GetMax()->z - pArea->GetMin()->z ) / 2.f;
		EtMatrix matRotate;
		EtMatrixRotationY( &matRotate , EtToRadian( pArea->GetRotate() ) );
		EtVec3TransformNormal( &Box.Axis[0], &Box.Axis[0], &matRotate );
		EtVec3TransformNormal( &Box.Axis[1], &Box.Axis[1], &matRotate );
		EtVec3TransformNormal( &Box.Axis[2], &Box.Axis[2], &matRotate );
		Box.CalcVertices();

		if( TestLineToOBB( Position, Direction, Box ) == true ) {
			vSize.x = Box.Extent[0] * 2.f;
			vSize.z = Box.Extent[2] * 2.f;
			fSize = vSize.x * vSize.z;
			if( dwMinIndex == -1 ) {
				fMinSize = fSize;
				dwMinIndex = i;

				fSizeX = vSize.x;
				fSizeZ = vSize.z;
				ResultBox = Box;
			}
			else {
				if( fSize < fMinSize ) {
					fMinSize = fSize;
					dwMinIndex = i;

					fSizeX = vSize.x;
					fSizeZ = vSize.z;
					ResultBox = Box;
				}
			}

		}

		/*
		Box.Min = *pArea->GetMin() + vOffset;
		Box.Max = *pArea->GetMax() + vOffset;
		if( Box.Max.y == 0.f && Box.Min.y == 0.f ) {
			float fMinY = GetHeight( Box.Min.x - vOffset.x, Box.Min.z - vOffset.z ) - 1.f;
			float fMaxY = GetHeight( Box.Max.x - vOffset.x, Box.Max.z - vOffset.z ) + 1.f;
			Box.Max.y = max( fMaxY, fMinY ) ;
			Box.Min.y = min( fMaxY, fMinY ) ;
		}
		if( TestLineToBox( Position, Direction, Box, fDist ) == true ) {
			vSize.x = Box.Max.x - Box.Min.x;
			vSize.z = Box.Max.z - Box.Min.z;
			fSize = vSize.x * vSize.z;
			if( dwMinIndex == -1 ) {
				fMinSize = fSize;
				dwMinIndex = i;

				vAreaOffset.x = ( vPickPos.x + vOffset.x ) - Box.Min.x;
				vAreaOffset.y = 0.f;
				vAreaOffset.z = ( vPickPos.z + vOffset.z ) - Box.Min.z;
				fSizeX = vSize.x;
				fSizeZ = vSize.z;
			}
			else {
				if( fSize < fMinSize ) {
					fMinSize = fSize;
					dwMinIndex = i;

					vAreaOffset.x = ( vPickPos.x + vOffset.x ) - Box.Min.x;
					vAreaOffset.y = 0.f;
					vAreaOffset.z = ( vPickPos.z + vOffset.z ) - Box.Min.z;
					fSizeX = vSize.x;
					fSizeZ = vSize.z;
				}
			}
		}
		*/
	}
	if( dwMinIndex == -1 ) return -1;
	int nControlType = 0;

	SOBB Box;

	Box = ResultBox;
	Box.Center -= ResultBox.Axis[0] * ( ResultBox.Extent[0] - 15.f );
	Box.Extent[0] = 15.f;
	if( TestLineToOBB( Position, Direction, Box ) == true ) nControlType |= 0x01;

	Box = ResultBox;
	Box.Center += ResultBox.Axis[0] * ( ResultBox.Extent[0] - 15.f );
	Box.Extent[0] = 15.f;
	if( TestLineToOBB( Position, Direction, Box ) == true ) nControlType |= 0x02;

	Box = ResultBox;
	Box.Center -= ResultBox.Axis[2] * ( ResultBox.Extent[2] - 15.f );
	Box.Extent[2] = 15.f;
	if( TestLineToOBB( Position, Direction, Box ) == true ) nControlType |= 0x04;

	Box = ResultBox;
	Box.Center += ResultBox.Axis[2] * ( ResultBox.Extent[2] - 15.f );
	Box.Extent[2] = 15.f;
	if( TestLineToOBB( Position, Direction, Box ) == true ) nControlType |= 0x08;
	if( nControlType == 0 ) nControlType = 0x0F;


	return nControlType;
}

CEtWorldEventArea *CTEtWorldSector::UpdateAreaRectBrush( int nX, int nY, DWORD dwColor, const char *szControlName )
{
	if( !m_Handle ) return NULL;
	EtVector3 vPickPos = m_vPickPos;

	EtVector3 vPos[2];
	EtVector2 vRect[5];
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
 	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	vPickPos -= vOffset;
	EtVector3 vPickPrevPos = m_vPickPrevPos - vOffset;

	vRect[0].x = vPickPrevPos.x;
	vRect[0].y = vPickPrevPos.z;
	vRect[1].x = vPickPos.x;
	vRect[1].y = vPickPrevPos.z;
	vRect[2].x = vPickPos.x;
	vRect[2].y = vPickPos.z;
	vRect[3].x = vPickPrevPos.x;
	vRect[3].y = vPickPos.z;
	vRect[4] = vRect[0];

	for( int i=0; i<4; i++ ) {
		vPos[0].x = vRect[i].x;
		vPos[0].z = vRect[i].y;
		vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

		EtVector2 vDir = vRect[i+1] - vRect[i];
		int nCount = (int)EtVec2Length( &vDir ) / 100;
		EtVec2Normalize( &vDir, &vDir );
		for( int j=0; j<nCount; j++ ) {
			vPos[1].x = vPos[0].x + ( vDir.x * 100.f );
			vPos[1].z = vPos[0].z + ( vDir.y * 100.f );
			vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

			EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );
			vPos[0] = vPos[1];
		}
		vPos[1].x = vRect[i+1].x;
		vPos[1].z = vRect[i+1].y;
		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );
	}
	

	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( pControl == NULL ) return NULL;


	EtVector3 Position, Direction;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );

	SAABox Box;
	EtVector3 vSize;

	DWORD dwMinIndex = -1;
	float fSize = 0.f;
	float fMinSize = 0.f;
	float fSizeX, fSizeZ;

	for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
		CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);

		SOBB Box;
		EtVector3 vCenter;

		Box.Center = *pArea->GetMin() + ( *pArea->GetMax() - *pArea->GetMin() ) / 2.f;
		vCenter = Box.Center;
		Box.Center.y = GetHeight( Box.Center.x, Box.Center.z );
		Box.Center += vOffset;
		Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
		Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
		Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
		Box.Extent[0] = ( pArea->GetMax()->x - pArea->GetMin()->x ) / 2.f;
		Box.Extent[1] = 1.f;//( ( pArea->GetMax()->y - pArea->GetMin()->y ) / 2.f ) + 1.f;
		Box.Extent[2] = ( pArea->GetMax()->z - pArea->GetMin()->z ) / 2.f;
		EtMatrix matRotate;
		EtMatrixRotationY( &matRotate , EtToRadian( pArea->GetRotate() ) );
		EtVec3TransformNormal( &Box.Axis[0], &Box.Axis[0], &matRotate );
		EtVec3TransformNormal( &Box.Axis[1], &Box.Axis[1], &matRotate );
		EtVec3TransformNormal( &Box.Axis[2], &Box.Axis[2], &matRotate );
		Box.CalcVertices();


		if( TestLineToOBB( Position, Direction, Box ) == true ) {
			vSize.x = Box.Extent[0] * 2.f;
			vSize.z = Box.Extent[2] * 2.f;
			fSize = vSize.x * vSize.z;
			if( dwMinIndex == -1 ) {
				fMinSize = fSize;
				dwMinIndex = i;

				m_vEventAreaPrevCenter = vCenter;
				m_vAreaOffset = vPickPos - *pArea->GetMin();
				fSizeX = vSize.x;
				fSizeZ = vSize.z;
			}
			else {
				if( fSize < fMinSize ) {
					fMinSize = fSize;
					dwMinIndex = i;

					m_vEventAreaPrevCenter = vCenter;
					m_vAreaOffset = vPickPos - *pArea->GetMin();
					fSizeX = vSize.x;
					fSizeZ = vSize.z;
				}
			}
		}

		/*
		Box.Min = *pArea->GetMin() + vOffset;
		Box.Max = *pArea->GetMax() + vOffset;
		if( Box.Max.y == 0.f && Box.Min.y == 0.f ) {
			float fMinY = GetHeight( Box.Min.x - vOffset.x, Box.Min.z - vOffset.z ) - 1.f;
			float fMaxY = GetHeight( Box.Max.x - vOffset.x, Box.Max.z - vOffset.z ) + 1.f;
			Box.Max.y = max( fMaxY, fMinY );
			Box.Min.y = min( fMaxY, fMinY );
		}
		if( TestLineToBox( Position, Direction, Box, fDist ) == true ) {
			vSize.x = Box.Max.x - Box.Min.x;
			vSize.z = Box.Max.z - Box.Min.z;
			fSize = vSize.x * vSize.z;
			if( dwMinIndex == -1 ) {
				fMinSize = fSize;
				dwMinIndex = i;

				m_vAreaOffset.x = ( vPickPos.x + vOffset.x ) - Box.Min.x;
				m_vAreaOffset.y = 0.f;
				m_vAreaOffset.z = ( vPickPos.z + vOffset.z ) - Box.Min.z;
				fSizeX = vSize.x;
				fSizeZ = vSize.z;
			}
			else {
				if( fSize < fMinSize ) {
					fMinSize = fSize;
					dwMinIndex = i;

					m_vAreaOffset.x = ( vPickPos.x + vOffset.x ) - Box.Min.x;
					m_vAreaOffset.y = 0.f;
					m_vAreaOffset.z = ( vPickPos.z + vOffset.z ) - Box.Min.z;
					fSizeX = vSize.x;
					fSizeZ = vSize.z;
				}
			}
		}
		*/
	}
	if( dwMinIndex == -1 ) return NULL;

	CEtWorldEventArea *pArea = pControl->GetAreaFromIndex( (DWORD)dwMinIndex );
	return pArea;
}


void CTEtWorldSector::ControlEventArea( int nX, int nY, int nControlType )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( CGlobalValue::GetInstance().m_pSelectEvent == NULL ) return;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

	m_vPickPos = vPickPos;

	EtVector3 vPos[2];
	EtVector2 vRect[5];
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	vPickPos -= vOffset;

	CEtWorldEventArea *pArea = CGlobalValue::GetInstance().m_pSelectEvent;
	EtVector3 vMin, vMax;
	EtVector3 vSize;
	vMin = *pArea->GetMin();
	vMax = *pArea->GetMax();
	vSize.x = vMax.x - vMin.x;
	vSize.y = 0.f;//vMax.y - vMin.y;
	vSize.z = vMax.z - vMin.z;

	if( nControlType == 0x0F ) {
		vMin = vPickPos - m_vAreaOffset;
		vMax = vMin + vSize;
	}
	else {
		if( nControlType & 0x01 ) {
			vMin.x = vPickPos.x;
			if( vMin.x > vMax.x - 100.f ) vMin.x = vMax.x - 100.f;
		}
		if( nControlType & 0x02 ) {
			vMax.x = vPickPos.x;
			if( vMax.x < vMin.x + 100.f ) vMax.x = vMin.x + 100.f;
		}
		if( nControlType & 0x04 ) {
			vMin.z = vPickPos.z;
			if( vMin.z > vMax.z - 100.f ) vMin.z = vMax.z - 100.f;
		}
		if( nControlType & 0x08 ) {
			vMax.z = vPickPos.z;
			if( vMax.z < vMin.z + 100.f ) vMax.z = vMin.z + 100.f;
		}
	}

	EtVector3 vResultMin, vResultMax;
	vResultMin.x = min( vMin.x, vMax.x );
	vResultMin.z = min( vMin.z, vMax.z );
	vResultMax.x = max( vMin.x, vMax.x );
	vResultMax.z = max( vMin.z, vMax.z );

	vResultMin.y = vResultMax.y = 0.f;
	if( nControlType != 0x0F ) {
		EtVector3 vCenter;
		vCenter = vResultMin + ( vResultMax - vResultMin ) / 2.f;
		EtVector3 vMove = vCenter - m_vEventAreaPrevCenter;
		EtMatrix matRotate;
		EtMatrixRotationY( &matRotate, EtToRadian( pArea->GetRotate() ) );
		EtVector3 vRotMove;
		EtVec3TransformCoord( &vRotMove, &vMove, &matRotate );
		vMove = vRotMove - vMove;


		vResultMin += vMove;
		vResultMax += vMove;

		m_vEventAreaPrevCenter = vResultMin + ( vResultMax - vResultMin ) / 2.f;
	}

	vResultMin.y = pArea->GetMin()->y;
	vResultMax.y = pArea->GetMax()->y;
	pArea->SetMin( vResultMin );
	pArea->SetMax( vResultMax );
	((CTEtWorldEventArea*)pArea)->ModifyCustomRender();
}

DWORD WinRGB2DXRGB( DWORD dwColor, BYTE Alpha ) 
{
	BYTE r = (BYTE)( ( dwColor << 24 ) >> 24 );
	BYTE g = (BYTE)( ( dwColor << 16 ) >> 24 );
	BYTE b = (BYTE)( ( dwColor << 8 ) >> 24 );
	return D3DCOLOR_ARGB( Alpha, r, g, b );
}

void CTEtWorldSector::DrawArea( const char *szControlName )
{
	if( szControlName && strlen( szControlName ) == 0 ) {
		for( DWORD i=0; i<GetControlCount(); i++ ) {
			DrawArea( ((CTEtWorldEventControl*)GetControlFromIndex(i))->GetName() );
		}
	}

	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( pControl == NULL ) return;
	if( !m_Handle ) return;

	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( CString(szControlName) );
	if( pItem == NULL ) return;

	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	CEtWorldEventArea *pArea;
	EtVector3 vMin, vMax;
	for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
		pArea = pControl->GetAreaFromIndex(i);
		vMin = *pArea->GetMin();
		vMax = *pArea->GetMax();
		if( CGlobalValue::GetInstance().m_pSelectEvent == pArea ) {
			DrawArea( vMin, vMax, pArea->GetRotate(), vOffset, 30.f, WinRGB2DXRGB( pItem->GetSelectColor(), 150 ), WinRGB2DXRGB( pItem->GetSelectSideColor(), 255 ), pArea->GetName() );
		}
		else {
			BYTE r = ( pItem->GetSelectColor() << 24 ) >> 24;
			BYTE g = ( pItem->GetSelectColor() << 16 ) >> 24;
			BYTE b = ( pItem->GetSelectColor() << 8 ) >> 24;
			DrawArea( vMin, vMax, pArea->GetRotate(), vOffset, 30.f, WinRGB2DXRGB( pItem->GetBackgroundColor(), 80 ), WinRGB2DXRGB( pItem->GetBackgroundSideColor(), 180 ), pArea->GetName() );
		}
	}
}

void CTEtWorldSector::DrawArea( EtVector3 &vMin, EtVector3 &vMax, float fRotate, EtVector3 &vOffset, float fYOrder, DWORD dwColor, DWORD dwSideColor, const char *szStr )
{
	EtVector3 vPos[4];
	EtVector2 vRect[5];

	SOBB Box;

	Box.Center = vMin + ( vMax - vMin ) / 2.f;
	Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
	Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
	Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
	Box.Extent[0] = ( vMax.x - vMin.x ) / 2.f;
	Box.Extent[1] = 0.f;//( vMax.y - vMin.y ) / 2.f;
	Box.Extent[2] = ( vMax.z - vMin.z ) / 2.f;
	EtMatrix matRotate;
	EtMatrixRotationY( &matRotate , EtToRadian( fRotate ) );
	EtVec3TransformNormal( &Box.Axis[0], &Box.Axis[0], &matRotate );
	EtVec3TransformNormal( &Box.Axis[1], &Box.Axis[1], &matRotate );
	EtVec3TransformNormal( &Box.Axis[2], &Box.Axis[2], &matRotate );
	Box.CalcVertices();

	vRect[0].x = Box.Vertices[0].x;
	vRect[0].y = Box.Vertices[0].z;
	vRect[1].x = Box.Vertices[1].x;
	vRect[1].y = Box.Vertices[1].z;
	vRect[2].x = Box.Vertices[5].x;
	vRect[2].y = Box.Vertices[5].z;
	vRect[3].x = Box.Vertices[4].x;
	vRect[3].y = Box.Vertices[4].z;
	vRect[4] = vRect[0];

	EtVector3 vDirection[3];
	vDirection[0] = Box.Center + ( Box.Axis[2] * Box.Extent[2] );
	vDirection[1] = vDirection[0];
	vDirection[2] = vDirection[0];
	vDirection[1] += Box.Axis[0] * 20.f;
	vDirection[2] -= Box.Axis[0] * 20.f;
	vDirection[1] -= Box.Axis[2] * 20.f;
	vDirection[2] -= Box.Axis[2] * 20.f;


	vDirection[0].y = GetHeight( vDirection[0].x, vDirection[0].z ) + fYOrder + 1.f;
	vDirection[1].y = GetHeight( vDirection[1].x, vDirection[1].z ) + fYOrder + 1.f;
	vDirection[2].y = GetHeight( vDirection[2].x, vDirection[2].z ) + fYOrder + 1.f;

	EternityEngine::DrawTriangle3D( vDirection[0] + vOffset, vDirection[1] + vOffset, vDirection[2] + vOffset, 0xFFFFFF00 );

	for( int i=0; i<4; i++ ) {
		vPos[0].x = vRect[i].x;
		vPos[0].z = vRect[i].y;
		vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + fYOrder;

		EtVector2 vDir = vRect[i+1] - vRect[i];
		int nCount = (int)( EtVec2Length( &vDir ) / m_fTileSize );
		EtVec2Normalize( &vDir, &vDir );
		for( int j=0; j<nCount; j++ ) {
			vPos[1].x = vPos[0].x + ( vDir.x * m_fTileSize );
			vPos[1].z = vPos[0].z + ( vDir.y * m_fTileSize );
			vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + fYOrder;

			EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwSideColor );
			vPos[0] = vPos[1];
		}
		vPos[1].x = vRect[i+1].x;
		vPos[1].z = vRect[i+1].y;
		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + fYOrder;

		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwSideColor );
	}


	float fSizeX = m_fTileSize * 6.f;
	float fSizeY = fSizeX;

	EtVector2 vDir = vRect[1] - vRect[0];
	float fLengthX = EtVec2Length( &vDir );
	int nCountX = (int)( fLengthX / fSizeX );

	vDir = vRect[3] - vRect[0];
	float fLengthY = EtVec2Length( &vDir );
	int nCountY = (int)( fLengthY / fSizeY );
	if( nCountX == 0 ) {
		nCountX = 1;
		fSizeX = fLengthX;
	}
	if( nCountY == 0 ) {
		nCountY = 1;
		fSizeY = fLengthY;
	}

	for( int j=0; j<nCountY+1; j++ ) {
		for( int i=0; i<nCountX+1; i++ ) {
			float fLeft = fSizeX * i;
			float fTop = fSizeY * j;
			float fRight = fSizeX * ( i + 1 );
			float fBottom = fSizeY * ( j + 1 );

			if( fRight > fLengthX ) fRight = fLengthX;
			if( fBottom > fLengthY ) fBottom = fLengthY;

			vPos[0] = Box.Vertices[0] + ( fLeft * Box.Axis[0] );
			vPos[0] += fTop * Box.Axis[2];
			vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + fYOrder;

			vPos[1] = Box.Vertices[0] + ( fRight * Box.Axis[0] );
			vPos[1] += fTop * Box.Axis[2];
			vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + fYOrder;

			vPos[2] = Box.Vertices[0] + ( fRight * Box.Axis[0] );
			vPos[2] += ( fBottom * Box.Axis[2] );
			vPos[2].y = GetHeight( vPos[2].x, vPos[2].z ) + fYOrder;

			vPos[3] = Box.Vertices[0] + ( fLeft * Box.Axis[0] );
			vPos[3] += ( fBottom * Box.Axis[2] );
			vPos[3].y = GetHeight( vPos[3].x, vPos[3].z ) + fYOrder;

			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[1] + vOffset, vPos[2] + vOffset, dwColor );
			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[2] + vOffset, vPos[3] + vOffset, dwColor );
		}
	}

	if( vMin.y != 0.f || vMax.y != 0.f ) {
		vPos[0] = Box.Vertices[0];
		vPos[1] = Box.Vertices[1];
		vPos[2] = Box.Vertices[5];
		vPos[3] = Box.Vertices[6];

		dwColor -= D3DCOLOR_ARGB( 40, 30, 30, 30 );

		if( vMin.y != 0.f ) {
			for( int i=0; i<4; i++ ) vPos[i].y = GetHeight( Box.Center.x, Box.Center.z ) + vMin.y;
			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[1] + vOffset, vPos[2] + vOffset, dwColor );
			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[3] + vOffset, vPos[2] + vOffset, dwColor );
		}

		if( vMax.y != 0.f ) {
			for( int i=0; i<4; i++ ) vPos[i].y = GetHeight( Box.Center.x, Box.Center.z ) + vMax.y;
			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[1] + vOffset, vPos[2] + vOffset, dwColor );
			EternityEngine::DrawTriangle3D( vPos[0] + vOffset, vPos[3] + vOffset, vPos[2] + vOffset, dwColor );
		}
	}


	EtVector3 vTemp;
	EtMatrix *pmat = CRenderBase::GetInstance().GetCameraHandle()->GetViewProjMat();
	vTemp = vMin + vOffset;
	vTemp.y = GetHeight( vMin.x, vMin.z );
	EtVec3TransformCoord( &vTemp, &vTemp, pmat );
	if( vTemp.z <= 1.0f ) {
		EtVector2 vTextPos;
		vTextPos.x = ( vTemp.x + 1.f ) / 2.f;
		vTextPos.y = 1.f - ( ( vTemp.y + 1.f ) / 2.f );


		char szTempStr[256] = { 0, };
		sprintf_s( szTempStr, "%s ( Max:%.2f, Min:%.2f )", szStr, vMax.y, vMin.y );
		EternityEngine::DrawText2D( vTextPos, szTempStr );
	}
}

CEtWorldSoundEnvi *CTEtWorldSector::UpdateSoundRectBrush( int nX, int nY, DWORD dwColor )
{
	if( !m_Handle ) return NULL;
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;


	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) vPickPos = m_vPickPos;
	else m_vPickPos = vPickPos;

	EtVector3 vPos[6];

	vPickPos -= vOffset;
	EtVector3 vPickPrevPos = m_vPickPrevPos - vOffset;
	float fRadian = EtVec3Length( &( vPickPrevPos - vPickPos ) );

	float fCurAngle = 0.f;
	vPos[0].x = vPickPrevPos.x + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
	vPos[0].z = vPickPrevPos.z + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
	if( vPos[0].x < 0.f || vPos[0].x >= fWidth || vPos[0].z < 0.f || vPos[0].z >= fHeight ) vPos[0].y = 0.f;
	vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

	vPos[2].x = vPickPrevPos.x + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
	vPos[2].y = vPickPrevPos.y + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
	vPos[2].z = vPickPrevPos.z;

	vPos[4].z = vPickPrevPos.z + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
	vPos[4].y = vPickPrevPos.y + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
	vPos[4].x = vPickPrevPos.x;


	for( DWORD i=0;; i++ ) {
		vPos[1].x = vPickPrevPos.x + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
		vPos[1].z = vPickPrevPos.z + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
		if( vPos[1].x < 0.f || vPos[1].x >= fWidth || vPos[1].z < 0.f || vPos[1].z >= fHeight ) vPos[1].y = 0.f;

		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;
		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );

		vPos[3].x = vPickPrevPos.x + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
		vPos[3].y = vPickPrevPos.y + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
		vPos[3].z = vPickPrevPos.z;
		EternityEngine::DrawLine3D( ( vPos[2] + vOffset ), ( vPos[3] + vOffset ), dwColor );

		vPos[5].z = vPickPrevPos.z + ( cos( EtToRadian( fCurAngle ) ) * fRadian );
		vPos[5].y = vPickPrevPos.y + ( sin( EtToRadian( fCurAngle ) ) * fRadian );
		vPos[5].x = vPickPrevPos.x;
		EternityEngine::DrawLine3D( ( vPos[4] + vOffset ), ( vPos[5] + vOffset ), dwColor );


		vPos[0] = vPos[1];
		vPos[2] = vPos[3];
		vPos[4] = vPos[5];

		if( fCurAngle >= 360.f ) break;
		fCurAngle += 5.f;
	}

	EtVector3 Position, Direction;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );

	SSphere Sphere;

	float fMinSize = FLT_MAX;
	int nMinIndex = -1;
	for( DWORD i=0; i<m_pSoundInfo->GetEnviCount(); i++ ) {
		CEtWorldSoundEnvi *pEnvi = m_pSoundInfo->GetEnviFromIndex(i);
		Sphere.Center = *pEnvi->GetPosition() + vOffset;
		Sphere.fRadius = pEnvi->GetRange();

		if( TestLineToSphere( Position, Direction, Sphere ) == true ) {
			if( pEnvi->GetRange() < fMinSize ) nMinIndex = i;
		}
	}
	if( nMinIndex != -1 ) return m_pSoundInfo->GetEnviFromIndex( nMinIndex );

	return NULL;
}

void CTEtWorldSector::ControlSound( int nX, int nY, int nControlType )
{
}

short CTEtWorldSector::GetNewHeight( short *pHeight, int x, int y, int nBlurValue, float **ppfTable )
{
	int nTableSize = ( nBlurValue * 2 ) + 1;

	x -= nBlurValue;
	y -= nBlurValue;

	float fValue = 0.f;
	for( int j=0; j<nTableSize; j++ ) {
		for( int i=0; i<nTableSize; i++ ) {
			int nOffset = (y+j) * GetTileWidthCount() + (x+i);

			fValue += pHeight[nOffset] * ppfTable[i][j];
		}
	}
	int nResult = (int)fValue;
	if( nResult > 32768 ) nResult = 32768;
	if( nResult < -32767 ) nResult = -32767;

	return (short)nResult;
}

void CTEtWorldSector::ApplyHeight( CRect &rcRect, int nPtrWidth, int nPtrHeight, short *pHeight, float fEpsilon )
{
	if( !m_Handle ) return;

	CRect rcEditRect = CRect( 0, 0, (int)GetTileWidthCount(), (int)GetTileHeightCount() );
	if( pHeight ) {
		float fWidthRatio = nPtrWidth / (float)rcRect.Width();
		float fHeightRatio = nPtrHeight / (float)rcRect.Height();

		for( int j=rcRect.top; j<=rcRect.bottom; j++ ) {
			for( int i=rcRect.left; i<=rcRect.right; i++ ) {

				if( i < rcEditRect.left || i >= rcEditRect.right ) continue;
				if( j < rcEditRect.top || j >= rcEditRect.bottom ) continue;


				int nOffset = j * GetTileWidthCount() + i;
				int nPtrOffset = ( (int)( ( j - rcRect.top ) * fHeightRatio ) * nPtrWidth ) + (int)( ( i - rcRect.left ) * fWidthRatio );
				m_pHeight[nOffset] = pHeight[nPtrOffset];
			}
		}
	}
	// Blur
	short *pTempHeight = new short[GetTileWidthCount()*GetTileHeightCount()];
	memcpy( pTempHeight, m_pHeight, GetTileWidthCount()*GetTileHeightCount()*sizeof(short) );
	int nBlurValue = 2;
	float **ppfGaussianTable;

	int nTableSize = ( nBlurValue * 2 ) + 1;
	ppfGaussianTable = new float*[nTableSize];
	for( int i=0; i<nTableSize; i++ )
		ppfGaussianTable[i] = new float[nTableSize];
	CalcGaussianTable( nBlurValue, fEpsilon, ppfGaussianTable );

	for( int j=rcRect.top; j<=rcRect.bottom; j++ ) {
		for( int i=rcRect.left; i<=rcRect.right; i++ ) {
			if( i < rcEditRect.left || i >= rcEditRect.right ) continue;
			if( j < rcEditRect.top || j >= rcEditRect.bottom ) continue;

			if( i < nBlurValue ) continue;
			if( i >= rcEditRect.right - nBlurValue ) continue;
			if( j < nBlurValue ) continue;
			if( j >= rcEditRect.bottom - nBlurValue ) continue;

			int nOffset = j * GetTileWidthCount() + i;
			m_pHeight[nOffset] = GetNewHeight( pTempHeight, i, j, nBlurValue, ppfGaussianTable );
		}
	}
	delete []pTempHeight;
	for( int i=0; i<nTableSize; i++ )
		SAFE_DELETEA( ppfGaussianTable[i] );
	SAFE_DELETEA( ppfGaussianTable );

	UpdateHeight( rcRect );


	m_rcLastUpdate.left = ( m_rcLastUpdate.left == -1 ) ? rcRect.left : ( ( m_rcLastUpdate.left > rcRect.left ) ? rcRect.left : m_rcLastUpdate.left );
	m_rcLastUpdate.top = ( m_rcLastUpdate.top == -1 ) ? rcRect.top : ( ( m_rcLastUpdate.top > rcRect.top ) ? rcRect.top : m_rcLastUpdate.top );
	m_rcLastUpdate.right = ( m_rcLastUpdate.right == -1 ) ? rcRect.right : ( ( m_rcLastUpdate.right < rcRect.right ) ? rcRect.right : m_rcLastUpdate.right );
	m_rcLastUpdate.bottom = ( m_rcLastUpdate.bottom == -1 ) ? rcRect.bottom : ( ( m_rcLastUpdate.bottom < rcRect.bottom ) ? rcRect.bottom : m_rcLastUpdate.bottom );

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() ) m_rcLastUpdate.left = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() ) m_rcLastUpdate.top = GetTileHeightCount() - 1;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() ) m_rcLastUpdate.right = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() ) m_rcLastUpdate.bottom = GetTileHeightCount() - 1;

}

void CTEtWorldSector::ApplyHeight( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, bool bMousePos, int nApplyBrushIndex )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( bMousePos == true ) {
		if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

		EtVector3 vOffset;
		float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
		float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
		vOffset.x = m_Offset.x - ( fWidth / 2.f );
		vOffset.z = m_Offset.z - ( fHeight / 2.f );
		vOffset.y = 0.f;
		vPickPos -= vOffset;
	}
	else {
		vPickPos.x = nX * m_fTileSize;
		vPickPos.z = nY * m_fTileSize;
		vPickPos.y = GetHeight( vPickPos.x, vPickPos.z );
	}

	CRect rcRect;
	std::vector<short *> pVecPtrList;
	int nAverage = 0;
	int nSize = (int)( fRadian / m_fTileSize );
	fRadian /= 2.f;
	if( fRadian < m_fTileSize ) {
		nSize = 0;
	}

	CRect rcEditRect = CRect( 0, 0, (int)GetTileWidthCount(), (int)GetTileHeightCount() );
	if( CGlobalValue::GetInstance().m_bLockSideHeight == true ) {
		rcEditRect.DeflateRect( 1, 1, 1, 1 );
	}

	int nSizeX, nSizeY;
	int nCountX, nCountY;
	m_Handle->GetBlockSize( nSizeX, nSizeY ); 
	m_Handle->GetBlockCount( nCountX, nCountY );

	for( int i=0; i<=nSize; i++ ) {
		float fY = vPickPos.z - fRadian + ( i * m_fTileSize );
		int nY = (int)( fY / m_fTileSize  );
		for( int j=0; j<=nSize; j++ ) {
			float fX = vPickPos.x - fRadian + ( j * m_fTileSize );

			int nX = (int)( fX / m_fTileSize );
			if( nSize == 0 ) {
				fX = vPickPos.x;
				fY = vPickPos.z;
				nX = (int)( fX / m_fTileSize );
				nY = (int)( fY / m_fTileSize  );
			}

			if( i == 0 ) rcRect.top = nY;
			if( i == nSize ) rcRect.bottom = nY;
			if( j == 0 ) rcRect.left = nX;
			if( j == nSize ) rcRect.right = nX;
			if( fX < 0.f || fY < 0.f ) continue;
			if( nX < rcEditRect.left || nX >= rcEditRect.right ) continue;
			if( nY < rcEditRect.top || nY >= rcEditRect.bottom ) continue;

			float fLength = EtVec2Length( &( EtVector2( fX, fY ) - EtVector2( vPickPos.x, vPickPos.z ) ) );
			if( fLength > fRadian ) continue;

			// SPACE 바 눌르구 브러시질 할때는 처음 브러쉬된 블럭만 수정되게 한다.
			if( nApplyBrushIndex != -1 ) {
				int nBlockX = (int)( nX / nSizeX );
				int nBlockY = (int)( nY / nSizeY );
				if( ( nBlockY * nCountX ) + nBlockX != nApplyBrushIndex ) continue;
			}

			switch( nModifyType ) {
				case -1:
				case 0:	// 일반 편집
					{
						float fHard = EtToRadian( 90.f ) - ( ( 1.f / fRadian ) * fLength );
						fHard = sin( fHard );
						fHard += ( cos( fHardness ) * fHardness );
						if( fHard >= 1.f ) fHard = 1.f;

						int nOffset = nY * GetTileWidthCount() + nX;
						int nValue = m_pHeight[nOffset];

						if( nModifyType == 0 ) nValue += (short)( fStrong * fHard );
						else nValue -= (short)( fStrong * fHard );
						if( nValue < rHeight.nMin ) nValue = rHeight.nMin;
						else if( nValue > rHeight.nMax ) nValue = rHeight.nMax;
						m_pHeight[nOffset] = (short)nValue;
					}
					break;
				case 1:	// 스무스
					{
						// 각 체크
						EtVector3 vNormal = GetHeightNormal( nX, nY );
						float fDegree = EtToDegree( acos( EtVec3Dot( &vNormal, &EtVector3( 0.f, 1.f, 0.f ) ) ) );
						if( fDegree < rRadian.fMin || fDegree > rRadian.fMax ) continue;

						int nOffset = nY * GetTileWidthCount() + nX;
						pVecPtrList.push_back( &m_pHeight[nOffset] );
						nAverage += m_pHeight[nOffset];
					}
					break;
				case 2:	// 최대, 최소 값으로
				case -2:
					{
						int nOffset = nY * GetTileWidthCount() + nX;
						if( nModifyType == 2 ) m_pHeight[nOffset] = rHeight.nMax;
						else m_pHeight[nOffset] = rHeight.nMin;
					}
					break;
			}
		}
	}
	switch( nModifyType ) {
		case 0:	
			ApplyHeight( rcRect, -1, -1, NULL, 1.f - ( CGlobalValue::GetInstance().m_fBrushHardness * 0.2f ) );
			return;
			break;
		case 1:
			{
				float fAver = nAverage / (float)pVecPtrList.size();
				float fTemp = 0;
				float fTemp2 = fStrong / 2000.f;
				for( DWORD i=0; i<pVecPtrList.size(); i++ ) {
					int nValue = (int)*pVecPtrList[i];
					if( nValue < rHeight.nMin || nValue > rHeight.nMax ) continue;
					fTemp = (float)nValue - fAver;
					nValue -= (int)( fTemp * fTemp2 );
					if( nValue < rHeight.nMin ) nValue = rHeight.nMin;
					else if( nValue > rHeight.nMax ) nValue = rHeight.nMax;
					*pVecPtrList[i] = (short)nValue;
				}
				pVecPtrList.clear();
			}
			break;
	}

//	m_Handle->UpdateHeight( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
	UpdateHeight( rcRect );

	m_rcLastUpdate.left = ( m_rcLastUpdate.left == -1 ) ? rcRect.left : ( ( m_rcLastUpdate.left > rcRect.left ) ? rcRect.left : m_rcLastUpdate.left );
	m_rcLastUpdate.top = ( m_rcLastUpdate.top == -1 ) ? rcRect.top : ( ( m_rcLastUpdate.top > rcRect.top ) ? rcRect.top : m_rcLastUpdate.top );
	m_rcLastUpdate.right = ( m_rcLastUpdate.right == -1 ) ? rcRect.right : ( ( m_rcLastUpdate.right < rcRect.right ) ? rcRect.right : m_rcLastUpdate.right );
	m_rcLastUpdate.bottom = ( m_rcLastUpdate.bottom == -1 ) ? rcRect.bottom : ( ( m_rcLastUpdate.bottom < rcRect.bottom ) ? rcRect.bottom : m_rcLastUpdate.bottom );

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() ) m_rcLastUpdate.left = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() ) m_rcLastUpdate.top = GetTileHeightCount() - 1;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() ) m_rcLastUpdate.right = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() ) m_rcLastUpdate.bottom = GetTileHeightCount() - 1;

}

void CTEtWorldSector::ApplyAlpha( int nLayerIndex, int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, int nApplyBrushIndex )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;
	vPickPos -= vOffset;

	CRect rcRect;
	std::vector<short *> pVecPtrList;
	int nAverage = 0;
	int nSize = (int)( fRadian / m_fTileSize );
	fRadian /= 2.f;
	if( fRadian < m_fTileSize ) {
		nSize = 0;
	}

	int nSizeX, nSizeY;
	int nCountX, nCountY;
	m_Handle->GetBlockSize( nSizeX, nSizeY ); 
	m_Handle->GetBlockCount( nCountX, nCountY );

	for( int i=0; i<=nSize; i++ ) {
		float fY = vPickPos.z - fRadian + ( i * m_fTileSize );
		int nY = (int)( fY / m_fTileSize  );
		for( int j=0; j<=nSize; j++ ) {
			float fX = vPickPos.x - fRadian + ( j * m_fTileSize );

			int nX = (int)( fX / m_fTileSize );
			if( nSize == 0 ) {
				fX = vPickPos.x;
				fY = vPickPos.z;
				nX = (int)( fX / m_fTileSize );
				nY = (int)( fY / m_fTileSize  );
			}

			if( i == 0 ) rcRect.top = nY;
			if( i == nSize ) rcRect.bottom = nY;
			if( j == 0 ) rcRect.left = nX;
			if( j == nSize ) rcRect.right = nX;
			if( fX < 0.f || fY < 0.f ) continue;
			if( nX < 0 || nX >= (int)GetTileWidthCount() ) continue;
			if( nY < 0 || nY >= (int)GetTileHeightCount() ) continue;

			float fLength = EtVec2Length( &( EtVector2( fX, fY ) - EtVector2( vPickPos.x, vPickPos.z ) ) );
			if( fLength > fRadian ) continue;

			int nOffset = nY * GetTileWidthCount() + nX;
			// 높이체크
			if( m_pHeight[nOffset] < rHeight.nMin ||
				m_pHeight[nOffset] > rHeight.nMax ) continue;

			// 각 체크
			EtVector3 vNormal = GetHeightNormal( nX, nY );
			float fDegree = EtToDegree( acos( EtVec3Dot( &vNormal, &EtVector3( 0.f, 1.f, 0.f ) ) ) );
			if( fDegree < rRadian.fMin || fDegree > rRadian.fMax ) continue;

			// SPACE 바 눌르구 브러시질 할때는 처음 브러쉬된 블럭만 수정되게 한다.
			if( nApplyBrushIndex != -1 ) {
 				int nBlockX = (int)( nX / nSizeX );
				int nBlockY = (int)( nY / nSizeY );
   				if( ( nBlockY * nCountX ) + nBlockX != nApplyBrushIndex ) continue;
			}
			
			// Hardness 
			float fRatio = fRadian * fHardness;
			float fHard = 0.f;
			if( fRatio >= fLength ) fHard = 1.f;
			else {
				fHard = 1.f - ( 1.f / ( fRadian - fRatio ) ) * ( fLength - fRatio );
			}
			if( fHard >= 1.f ) fHard = 1.f;
			else if( fHard < 0.f ) fHard = 0.f;


			// 증가된 레이어 만큼 다른 레이어들의 알파값들을 비율체크해서 빼준다.
			D3DXCOLOR Color = D3DXCOLOR( m_pAlpha[nOffset] );
			int nColor[4] = { (int)( Color.a * 255 ), (int)( Color.r * 255 ), (int)( Color.g * 255 ), (int)( Color.b * 255 ) };
			float fValue = 2550 * fStrong * fHard;
			float fDivide = 0;

			nColor[nLayerIndex] += (int)fValue;
			if( nColor[nLayerIndex] > 255 ) nColor[nLayerIndex] = 255;

			for( int k=0; k<4; k++ ) {
				if( nLayerIndex == k ) continue;
				if( nColor[k] == 0 ) continue;
				fDivide++;
			}
			for( int k=0; k<4; k++ ) {
				if( nLayerIndex == k ) continue;
				if( nColor[k] == 0 ) continue;
				if( (float)nColor[k] < ( fValue / fDivide ) ) fDivide -= ( 1.f - ( nColor[k] / ( fValue / fDivide ) ) );
			}
			fValue /= fDivide;
			for( int k=0; k<4; k++ ) {
				if( nLayerIndex == k ) continue;
				nColor[k] -= (int)fValue;
				if( nColor[k] < 0 ) nColor[k] = 0;
			}

			m_pAlpha[nOffset] = D3DCOLOR_ARGB( nColor[0], nColor[1], nColor[2], nColor[3] );
		}
	}

	UpdateAlpha( rcRect );

	m_rcLastUpdate.left = ( m_rcLastUpdate.left == -1 ) ? rcRect.left : ( ( m_rcLastUpdate.left > rcRect.left ) ? rcRect.left : m_rcLastUpdate.left );
	m_rcLastUpdate.top = ( m_rcLastUpdate.top == -1 ) ? rcRect.top : ( ( m_rcLastUpdate.top > rcRect.top ) ? rcRect.top : m_rcLastUpdate.top );
	m_rcLastUpdate.right = ( m_rcLastUpdate.right == -1 ) ? rcRect.right : ( ( m_rcLastUpdate.right < rcRect.right ) ? rcRect.right : m_rcLastUpdate.right );
	m_rcLastUpdate.bottom = ( m_rcLastUpdate.bottom == -1 ) ? rcRect.bottom : ( ( m_rcLastUpdate.bottom < rcRect.bottom ) ? rcRect.bottom : m_rcLastUpdate.bottom );

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() ) m_rcLastUpdate.left = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() ) m_rcLastUpdate.top = GetTileHeightCount() - 1;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() ) m_rcLastUpdate.right = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() ) m_rcLastUpdate.bottom = GetTileHeightCount() - 1;

}

void CTEtWorldSector::ApplyGrass( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, bool bMousePos, int nApplyBrushIndex )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;
	vPickPos -= vOffset;

	CRect rcRect;
	std::vector<short *> pVecPtrList;
	int nAverage = 0;
	int nSize = (int)( fRadian / m_fTileSize );
	fRadian /= 2.f;
	if( fRadian < m_fTileSize ) {
		nSize = 0;
	}

	int nSizeX, nSizeY;
	int nCountX, nCountY;
	m_Handle->GetBlockSize( nSizeX, nSizeY ); 
	m_Handle->GetBlockCount( nCountX, nCountY );

	for( int i=0; i<=nSize; i++ ) {
		float fY = vPickPos.z - fRadian + ( i * m_fTileSize );
		int nY = (int)( fY / m_fTileSize  );
		for( int j=0; j<=nSize; j++ ) {
			float fX = vPickPos.x - fRadian + ( j * m_fTileSize );

			int nX = (int)( fX / m_fTileSize );
			if( nSize == 0 ) {
				fX = vPickPos.x;
				fY = vPickPos.z;
				nX = (int)( fX / m_fTileSize );
				nY = (int)( fY / m_fTileSize  );
			}

			if( i == 0 ) rcRect.top = nY;
			if( i == nSize ) rcRect.bottom = nY;
			if( j == 0 ) rcRect.left = nX;
			if( j == nSize ) rcRect.right = nX;
			if( fX < 0.f || fY < 0.f ) continue;
			if( nX < 0 || nX >= (int)GetTileWidthCount()-1 ) continue;
			if( nY < 0 || nY >= (int)GetTileHeightCount()-1 ) continue;

			float fLength = EtVec2Length( &( EtVector2( fX, fY ) - EtVector2( vPickPos.x, vPickPos.z ) ) );
			if( fLength > fRadian ) continue;

			int nOffset = nY * ( GetTileWidthCount() - 1 ) + nX;
			// 높이체크
			if( m_pHeight[nOffset] < rHeight.nMin ||
				m_pHeight[nOffset] > rHeight.nMax ) continue;

			// 각 체크
			EtVector3 vNormal = GetHeightNormal( nX, nY );
			float fDegree = EtToDegree( acos( EtVec3Dot( &vNormal, &EtVector3( 0.f, 1.f, 0.f ) ) ) );
			if( fDegree < rRadian.fMin || fDegree > rRadian.fMax ) continue;

			// SPACE 바 눌르구 브러시질 할때는 처음 브러쉬된 블럭만 수정되게 한다.
			if( nApplyBrushIndex != -1 ) {
				int nBlockX = (int)( nX / nSizeX );
				int nBlockY = (int)( nY / nSizeY );
				if( ( nBlockY * nCountX ) + nBlockX != nApplyBrushIndex ) continue;
			}

			// Hardness 
			float fRatio = fRadian * fHardness;
			float fHard = 0.f;
			if( fRatio >= fLength ) fHard = 1.f;
			else {
				fHard = 1.f - ( 1.f / ( fRadian - fRatio ) ) * ( fLength - fRatio );
			}
			if( fHard >= 1.f ) fHard = 1.f;
			else if( fHard < 0.f ) fHard = 0.f;


			float fValue = 2550 * fStrong * fHard;
			char cCurFlag = m_pGrass[nOffset];
			char cCurValue[4];
			cCurValue[3] = ( cCurFlag >> 6 ) & 0x03;
			cCurValue[2] = ( cCurFlag >> 4 ) & 0x03;
			cCurValue[1] = ( cCurFlag >> 2 ) & 0x03;
			cCurValue[0] = cCurFlag & 0x03;

			for( int l=0; l<4; l++ ) {
				if( CGlobalValue::GetInstance().m_bEnableGrassIndex[l] == true ) {
					if( nModifyType == 0 ) cCurValue[l] += 1;
					else if( nModifyType == -1 ) cCurValue[l] -= 1;
				}
				if( cCurValue[l] > 2 ) cCurValue[l] = 2;
				else if( cCurValue[l] < 0 ) cCurValue[l] = 0;
			}
			m_pGrass[nOffset] = ((char)((((cCurValue[3])&0x03)<<6)|(((cCurValue[2])&0x03)<<4)|(((cCurValue[1])&0x03)<<2)|((cCurValue[0])&0x03)));
		}
	}

	m_rcLastUpdate.left = ( m_rcLastUpdate.left == -1 ) ? rcRect.left : ( ( m_rcLastUpdate.left > rcRect.left ) ? rcRect.left : m_rcLastUpdate.left );
	m_rcLastUpdate.top = ( m_rcLastUpdate.top == -1 ) ? rcRect.top : ( ( m_rcLastUpdate.top > rcRect.top ) ? rcRect.top : m_rcLastUpdate.top );
	m_rcLastUpdate.right = ( m_rcLastUpdate.right == -1 ) ? rcRect.right : ( ( m_rcLastUpdate.right < rcRect.right ) ? rcRect.right : m_rcLastUpdate.right );
	m_rcLastUpdate.bottom = ( m_rcLastUpdate.bottom == -1 ) ? rcRect.bottom : ( ( m_rcLastUpdate.bottom < rcRect.bottom ) ? rcRect.bottom : m_rcLastUpdate.bottom );

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() - 1 ) m_rcLastUpdate.left = GetTileWidthCount() - 2;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() - 1 ) m_rcLastUpdate.top = GetTileHeightCount() - 2;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() - 1 ) m_rcLastUpdate.right = GetTileWidthCount() - 2;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() - 1 ) m_rcLastUpdate.bottom = GetTileHeightCount() - 2;
}

void CTEtWorldSector::UpdateHeight( CRect &rcRect )
{
	m_Handle->UpdateHeight( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
	m_Handle->UpdateGrassBlock( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void CTEtWorldSector::UpdateAlpha( CRect &rcRect )
{
	m_Handle->UpdateLayer( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void CTEtWorldSector::UpdateTextureCoord( CRect &rcRect )
{
	m_Handle->UpdateTextureCoord( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void CTEtWorldSector::UpdateGrass( CRect &rcRect )
{
	m_Handle->UpdateGrassBlock( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void CTEtWorldSector::ResetUpdateInfo()
{
	if( !m_Handle ) return;
	if( m_rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return;

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() ) m_rcLastUpdate.left = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() ) m_rcLastUpdate.top = GetTileHeightCount() - 1;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() ) m_rcLastUpdate.right = GetTileWidthCount() - 1;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() ) m_rcLastUpdate.bottom = GetTileHeightCount() - 1;

	m_Handle->UpdateNormal( m_rcLastUpdate.left, m_rcLastUpdate.top, m_rcLastUpdate.right, m_rcLastUpdate.bottom );
	m_Handle->UpdateTextureCoord( m_rcLastUpdate.left, m_rcLastUpdate.top, m_rcLastUpdate.right, m_rcLastUpdate.bottom );

	// 찍혀있던 데칼들두 재계산
	int nSizeX, nSizeY;
	int nCountX, nCountY;
	m_Handle->GetBlockSize( nSizeX, nSizeY ); 
	m_Handle->GetBlockCount( nCountX, nCountY );
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	EtVector2 vOffset;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.y = m_Offset.z - ( fHeight / 2.f );

	EtVector2 vPos;

	vPos.x = (float)( m_rcLastUpdate.left + ( m_rcLastUpdate.Width() / 2.f ) ) * GetTileSize();
	vPos.y = (float)( m_rcLastUpdate.top + ( m_rcLastUpdate.Height() / 2.f ) ) * GetTileSize();
	vPos += vOffset;

	float fRadius = ( max( m_rcLastUpdate.Width() , m_rcLastUpdate.Height() ) * GetTileSize() ) / 2.f;
	std::vector<CEtWorldDecal *> pVecList;
	CEtWorldSector::ScanDecal( vPos, fRadius, &pVecList );
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		((CTEtWorldDecal*)pVecList[i])->Reinitialize();
	}

	m_rcLastUpdate = CRect( -1, -1, -1, -1 );
}

void CTEtWorldSector::ResetUpdateGrassInfo()
{
	if( !m_Handle ) return;

	if( m_rcLastUpdate == CRect( -1, -1, -1, -1 ) ) return;
	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= (int)GetTileWidthCount() - 1 ) m_rcLastUpdate.left = GetTileWidthCount() - 2;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= (int)GetTileHeightCount() - 1 ) m_rcLastUpdate.top = GetTileHeightCount() - 2;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= (int)GetTileWidthCount() - 1 ) m_rcLastUpdate.right = GetTileWidthCount() - 2;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= (int)GetTileHeightCount() - 1 ) m_rcLastUpdate.bottom = GetTileHeightCount() - 2;
	m_Handle->UpdateGrassBlock( m_rcLastUpdate.left, m_rcLastUpdate.top, m_rcLastUpdate.right, m_rcLastUpdate.bottom );

	m_rcLastUpdate = CRect( -1, -1, -1, -1 );
}

void CTEtWorldSector::ApplyGrassInfo()
{
	if( !m_Handle ) return;
	for( int i=0; i<m_Handle->GetGrassBlockCount(); i++ ) {
		SGrassBlockInfo Info = *m_Handle->GetGrassInfo( i );
		memcpy( Info.fGrassWidth, m_fGrassWidth, sizeof(Info.fGrassWidth) );
		memcpy( Info.fMinGrassHeight, m_fGrassHeightMin, sizeof(Info.fMinGrassHeight) );
		memcpy( Info.fMaxGrassHeight, m_fGrassHeightMax, sizeof(Info.fMaxGrassHeight) );
		Info.fMinShake = m_fGrassShakeMin;
		Info.fMaxShake = m_fGrassShakeMax;
		m_Handle->SetGrassInfo( i, Info );
	}
}

short *CTEtWorldSector::GetHeightPtr()
{
	return m_pHeight;
}

DWORD *CTEtWorldSector::GetAlphaPtr()
{
	return m_pAlpha;
}

char *CTEtWorldSector::GetGrassPtr()
{
	return m_pGrass;
}

void CTEtWorldSector::GetBlockCount( int &nCountX, int &nCountY )
{
	if( !m_Handle ) return;
	m_Handle->GetBlockCount( nCountX, nCountY );
}

void CTEtWorldSector::GetBlockSize( int &nSizeX, int &nSizeY )
{
	if( !m_Handle ) return;
	m_Handle->GetBlockSize( nSizeX, nSizeY );
}


int CTEtWorldSector::GetCurrentBlock()
{
	return GetSelectBlock( -1, -1, false );
}

int CTEtWorldSector::GetSelectBlock( int nX, int nY, bool bCheckPick )
{
	if( !m_Handle ) return -1;
	EtVector3 vPickPos;
	if( bCheckPick == true ) {
		if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return -1;
	}
	else vPickPos = m_vPickPos;

	int nCountX, nCountY;
	m_Handle->GetBlockCount( nCountX, nCountY );
	int nIndex = 0;
	for( int i=0; i<nCountY; i++ ) {
		for( int j=0; j<nCountX; j++ ) {
			nIndex = i * nCountX + j;
			SAABox Box = *m_Handle->GetBoundingBox( nIndex );
			Box.Max.y += 1000.f;
			Box.Min.y -= 1000.f;
			if( vPickPos.x >= Box.Min.x && vPickPos.x <= Box.Max.x && 
				vPickPos.y >= Box.Min.y && vPickPos.y <= Box.Max.y &&
				vPickPos.z >= Box.Min.z && vPickPos.z <= Box.Max.z ) {
					return nIndex;
			}

		}
	}
	return -1;
}


void CTEtWorldSector::SetHeightMultiply( float fValue )
{
	SAFE_RELEASE_SPTR( m_Handle );
	m_fHeightMultiply = fValue;
	CreateTerrain();
	
	/*
	SaveSector( SRL_TERRAIN );
	FreeSector( SRL_TERRAIN );
	LoadSector( SRL_TERRAIN );
	*/
}

int CTEtWorldSector::GetTextureCount( int nBlockIndex )
{
	if( !m_Handle ) return -1;
	return m_Handle->GetTextureCount( nBlockIndex );
}

const char *CTEtWorldSector::GetTextureName( int nBlockIndex, int nTextureIndex )
{
	if( !m_Handle ) return NULL;

	return m_Handle->GetTextureName( nTextureIndex, nBlockIndex );
}

const char *CTEtWorldSector::GetTextureSemanticName( int nBlockIndex, int nTextureIndex )
{
	if( !m_Handle ) return NULL;

	return m_Handle->GetTextureSemanticName( nTextureIndex, nBlockIndex );
}

void CTEtWorldSector::ReloadTexture()
{
	SAFE_RELEASE_SPTR( m_Handle );
	CreateTerrain( NULL );
}

void CTEtWorldSector::SetTexture( int nBlockIndex, int nTextureIndex, const char *szName )
{
	if( !m_Handle ) return;

	m_Handle->SetTexture( nTextureIndex, szName, nBlockIndex );

	CString szTemp = szName;
	CString szFolder;
	szFolder.Format( "%s\\Resource\\Tile\\", CFileServer::GetInstance().GetWorkingFolder() );
	if( szName != NULL ) {
		const char *pPtr = strstr( szName, szFolder );
		if( pPtr ) {
			szTemp.Format( "%s", szTemp.GetBuffer() + strlen( szFolder ) );
		}
	}
	else szTemp.Empty();
	m_szVecLayerTexture[nBlockIndex][nTextureIndex] = szTemp;
}

const char *CTEtWorldSector::GetGrassTextureName( int nBlockIndex )
{
	if( !m_Handle ) return NULL;
	return m_Handle->GetGrassTexture();
}
 
void CTEtWorldSector::SetGrassTexture( int nBlockIndex, const char *szName )
{
	char szBuf[_MAX_PATH];
	_GetFullFileName( szBuf, _countof(szBuf), szName );
	m_szGrassTexture = szBuf;
	m_Handle->SetGrassTexture( szName );
}

int CTEtWorldSector::GetBlockCount()
{
	int nBlockX = ( GetTileWidthCount() - 1 ) / DEFAULT_TERRAIN_SIZE;
	if( ( GetTileWidthCount() - 1 ) % DEFAULT_TERRAIN_SIZE ) nBlockX++;

	int nBlockY = ( GetTileHeightCount() - 1 ) / DEFAULT_TERRAIN_SIZE;
	if( ( GetTileHeightCount() - 1 ) % DEFAULT_TERRAIN_SIZE ) nBlockY++;
	m_nBlockCount = nBlockX * nBlockY;

	return m_nBlockCount;
}


void CTEtWorldSector::ChangeBlockType( int nBlockIndex, TerrainType Type )
{
	m_Handle->ChangeBlockType( Type, nBlockIndex );
	m_nVecBlockTypeList[nBlockIndex] = (int)Type;

	m_szVecLayerTexture[nBlockIndex].resize( GetTextureCount(nBlockIndex) );
	m_fVecLayerTextureDistance[nBlockIndex].resize( GetTextureCount(nBlockIndex) );
	m_fVecLayerTextureRotation[nBlockIndex].resize( GetTextureCount(nBlockIndex) );

	for( DWORD i=0; i<m_szVecLayerTexture[nBlockIndex].size(); i++ ) {
		if( m_szVecLayerTexture[nBlockIndex][i].IsEmpty() || m_szVecLayerTexture[nBlockIndex][i] == "" ) {
			if( i >= 4 ) {
				m_szVecLayerTexture[nBlockIndex][i] = m_szVecLayerTexture[nBlockIndex][i-4];
				SetTexture( nBlockIndex, i, m_szVecLayerTexture[nBlockIndex][i] );
			}
		}
	}
}

TerrainType CTEtWorldSector::GetBlockType( int nBlockIndex )
{
	return m_Handle->GetBlockType( nBlockIndex );
}

void CTEtWorldSector::Show( bool bShow )
{
	if( !m_Handle ) return;
	m_Handle->Show( bShow );
}

void CTEtWorldSector::GetUseTextureList( std::map<CString,CString> &szMapList )
{
	bool bLoadTexture = true;
	if( m_szVecLayerTexture.size() == 0 ) {
		TextureTableFile( IFT_READ );
		GrassTableFile( IFT_READ );
		DecalInfoFile( IFT_READ, true );
	}
	else bLoadTexture = false;

	std::map<CString, bool> mapCurSearch;
	char szFileName[256] = { 0, };
	for( DWORD l=0; l<m_szVecLayerTexture.size(); l++ ) {
		for( DWORD m=0; m<m_szVecLayerTexture[l].size(); m++ ) {
			_GetFullFileName( szFileName, _countof(szFileName), m_szVecLayerTexture[l][m] );
			std::map<CString,CString>::iterator it = szMapList.find( szFileName );
			if( it != szMapList.end() ) {
				std::map<CString, bool>::iterator it2 = mapCurSearch.find( szFileName );
				if( it2 != mapCurSearch.end() ) continue;

				CString szTemp;
				szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
				it->second += szTemp;
				mapCurSearch.insert( make_pair( szFileName, true ) );
			}
			else {
				CString szTemp;
				szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
				szMapList.insert( make_pair( szFileName, szTemp ) );
				mapCurSearch.insert( make_pair( m_szVecLayerTexture[l][m], false ) );
			}
		}
	}
	int nCount = 0;
	for( DWORD i=0; i<m_pVecDecalList.size(); i++ ) {
		std::map<CString,CString>::iterator it = szMapList.find( m_pVecDecalList[i]->GetTextureName() );
		if( it != szMapList.end() ) {
			std::map<CString, bool>::iterator it2 = mapCurSearch.find( m_pVecDecalList[i]->GetTextureName() );
			if( it2 != mapCurSearch.end() ) continue;

			CString szTemp;
			szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
			it->second += szTemp;
			mapCurSearch.insert( make_pair( m_pVecDecalList[i]->GetTextureName(), true ) );
		}
		else {
			CString szTemp;
			szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
			szMapList.insert( make_pair( m_pVecDecalList[i]->GetTextureName(), szTemp ) );
			mapCurSearch.insert( make_pair( m_pVecDecalList[i]->GetTextureName(), false ) );
		}
	}


	_GetFullFileName( szFileName, _countof(szFileName), m_szGrassTexture );
	std::map<CString,CString>::iterator it = szMapList.find( szFileName );
	if( it != szMapList.end() ) {
		std::map<CString, bool>::iterator it2 = mapCurSearch.find( szFileName );
		if( it2 == mapCurSearch.end() ) {
			CString szTemp;
			szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
			it->second += szTemp;
		}
	}
	else {
		CString szTemp;
		szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
		szMapList.insert( make_pair( szFileName, szTemp ) );
		mapCurSearch.insert( make_pair( szFileName, false ) );
	}



	if( bLoadTexture ) {
		// 기본 택스쳐 리스트
		for( DWORD i=0; i<m_szVecLayerTexture.size(); i++ ) {
			SAFE_DELETE_VEC( m_szVecLayerTexture[i] );
		}
		SAFE_DELETE_VEC( m_szVecLayerTexture );

		for( DWORD i=0; i<m_fVecLayerTextureDistance.size(); i++ ) {
			SAFE_DELETE_VEC( m_fVecLayerTextureDistance[i] );
		}
		SAFE_DELETE_VEC( m_fVecLayerTextureDistance );

		for( DWORD i=0; i<m_fVecLayerTextureRotation.size(); i++ ) {
			SAFE_DELETE_VEC( m_fVecLayerTextureRotation[i] );
		}
		SAFE_DELETE_VEC( m_fVecLayerTextureRotation );

		SAFE_DELETE_VEC( m_nVecBlockTypeList );
		m_szGrassTexture.Empty();

		SAFE_DELETE( m_pDecalQuadtree );
		float fSize = max( m_pParentGrid->GetGridWidth() * 100.f, m_pParentGrid->GetGridHeight() * 100.f );
		m_pDecalQuadtree = new CEtQuadtree<CEtWorldDecal *>;
		m_pDecalQuadtree->Initialize( EtVector2( m_Offset.x, m_Offset.z ), fSize );

		SAFE_DELETE_PVEC( m_pVecDecalList );
	}
}

void CTEtWorldSector::GetUsePropList( std::map<CString,CString> &szMapList )
{
	bool bLoadProp = true;
	if( m_pVecPropList.size() == 0 ) {
		PropInfoFile( IFT_READ, true );
	}
	else bLoadProp = false;

	std::map<CString, bool> mapCurSearch;
	int nCount = 0;
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		std::map<CString,CString>::iterator it = szMapList.find( m_pVecPropList[i]->GetPropName() );
		if( it != szMapList.end() ) {
			std::map<CString, bool>::iterator it2 = mapCurSearch.find( m_pVecPropList[i]->GetPropName() );
			if( it2 != mapCurSearch.end() ) continue;

			CString szTemp;
			szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
			it->second += szTemp;
			mapCurSearch.insert( make_pair( m_pVecPropList[i]->GetPropName(), true ) );
		}
		else {
			CString szTemp;
			szTemp.Format( "%s : (%d,%d)\n", GetParentGrid()->GetName(), m_Index.nX, m_Index.nY );
			szMapList.insert( make_pair( m_pVecPropList[i]->GetPropName(), szTemp ) );
			mapCurSearch.insert( make_pair( m_pVecPropList[i]->GetPropName(), false ) );
		}
	}

	if( bLoadProp ) {
		SAFE_DELETE( m_pPropOctree );
		float fSize = max( m_pParentGrid->GetGridWidth() * 100.f, m_pParentGrid->GetGridHeight() * 100.f );
		fSize = max( fSize, GetHeightMultiply() * 32768.f );
		m_pPropOctree = new CEtOctree<CEtWorldProp *>;
		m_pPropOctree->Initialize( m_Offset, fSize );

		SAFE_DELETE_PVEC( m_pVecPropList );
	}
}

void CTEtWorldSector::ChangeTextureDistance( int nBlockIndex, int nTextureIndex, float fDistance )
{
	m_Handle->SetTextureDistance( nTextureIndex, fDistance, nBlockIndex );

	m_fVecLayerTextureDistance[nBlockIndex][nTextureIndex] = fDistance;
}

float CTEtWorldSector::GetTextureDistance( int nBlockIndex, int nTextureIndex )
{
	return m_Handle->GetTextureDistance( nTextureIndex, nBlockIndex );
}

void CTEtWorldSector::ChangeTextureRotation( int nBlockIndex, int nTextureIndex, float fRotation )
{
	m_Handle->SetTextureRotation( nTextureIndex, fRotation, nBlockIndex );

	m_fVecLayerTextureRotation[nBlockIndex][nTextureIndex] = fRotation;
}

float CTEtWorldSector::GetTextureRotation( int nBlockIndex, int nTextureIndex )
{
	return m_Handle->GetTextureRotation( nTextureIndex, nBlockIndex );
}

CEtWorldProp *CTEtWorldSector::AllocProp( int nClass )
{
	CTEtWorldProp *pProp = NULL;
	switch( nClass ) {
		/*
		case CTEtWorldProp::Action:
		case CTEtWorldProp::Operation:
			pProp = new CTEtWorldActProp;
			break;
		case CTEtWorldProp::Light:
			pProp = new CTEtWorldLightProp;
			break;
		case CTEtWorldProp::Broken:
			pProp = new CTEtWorldBrokenProp;
			break;
		case CTEtWorldProp::Trap:
			pProp = new CTEtWorldTrapProp;
			break;
		case CTEtWorldProp::Chest:
			pProp = new CTEtWorldChestProp;
			break;
		default:
			pProp = new CTEtWorldProp;
			break;
		*/
		case 4:
			pProp = new CTEtWorldLightProp;
			break;
		case 12:
			pProp = new CTEtBuffProp;
			break;
		default:
			pProp = new CTEtWorldActProp;
			break;
	}
	pProp->SetClassID( nClass );
	pProp->AllocCustomInfo();
//	pProp->AllocAdditionalPropInfo();
	return pProp;
}

void CTEtWorldSector::AddProp( int nX, int nY, CPropPoolStage *pPoolStage, CActionElement *pElement, float fRadian, float fHardness, irange rHeight, frange rRadian, bool bRandomPos, bool bRandomRotateX, bool bRandomRotateY, bool bRandomRotateZ, bool bLockHeightProp, bool bLockScaleAxis, frange rScaleX, frange rScaleY, frange rScaleZ, float fHeightDig, float fMinPropDistance, bool bIgnoreDistanceSize )
{
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;
	vPickPos -= vOffset;

	fRadian /= 2.f;
	if( bRandomPos == false ) {
		for( DWORD i=0; i<pPoolStage->GetPropCount(); i++ ) {
			AddProp( pPoolStage, pElement, i, vPickPos, fRadian, rHeight, rRadian, bRandomPos, bRandomRotateX, bRandomRotateY, bRandomRotateZ, bLockHeightProp, bLockScaleAxis, rScaleX, rScaleY, rScaleZ, fHeightDig, fMinPropDistance, bIgnoreDistanceSize );
		}
	}
	else {
		// 중앙과 거리멀수록 나올 확률이 떨어진다.
		int nDrawCount = (int)( fHardness * 100 );
		int nPropCount = pPoolStage->GetPropCount();
		if( nPropCount <= 0 ) return;
		int *pArray = new int[nPropCount*100];

		for( DWORD i=0; i<pPoolStage->GetPropCount(); i++ ) {
			EtVector2 vPosOffset = pPoolStage->GetPropOffset(i);
			int nPer = 100 - (int)( ( EtVec2Length( &vPosOffset ) * 100 ) );
			if( nPer < 0 ) nPer = 0;
			else if( nPer > 100 ) nPer = 100;

			for( int j=0; j<100; j++ ) {
				pArray[ ( i * 100 ) + j ] = ( j <= nPer ) ? i : -1;
			}
		}


		for( int k=0; k<nDrawCount; k++ ) {
			int nIndex = pArray[rand()%(nPropCount*100)];
			if( nIndex == -1 ) continue;
			AddProp( pPoolStage, pElement, (DWORD)nIndex, vPickPos, fRadian, rHeight, rRadian, bRandomPos, bRandomRotateX, bRandomRotateY, bRandomRotateZ, bLockHeightProp, bLockScaleAxis, rScaleX, rScaleY, rScaleZ, fHeightDig, fMinPropDistance, bIgnoreDistanceSize );
		}
		delete []pArray;
	}
}

void CTEtWorldSector::AddProp( CPropPoolStage *pPoolStage, CActionElement *pElement, DWORD dwPropIndex, EtVector3 vPickPos, float fRadian, irange rHeight, frange rRadian, bool bRandomPos, bool bRandomRotateX, bool bRandomRotateY, bool bRandomRotateZ, bool bLockHeightProp, bool bLockScaleAxis, frange rScaleX, frange rScaleY, frange rScaleZ, float fHeightDig, float fMinPropDistance, bool bIgnoreDistanceSize )
{
	EtVector3 vPosition;
	EtVector3 vRotate;
	EtVector3 vScale;
	EtVector2 vPosOffset;
	EtVector3 vNormal;
	CString szPropName;

	szPropName = pPoolStage->GetPropName(dwPropIndex);
	vPosOffset = pPoolStage->GetPropOffset(dwPropIndex);


	vPosition = vPickPos;
	if( bRandomPos ) {
		float fRandomAngle = ( rand()%36000 ) / 100.f;
		float fRandomRadian = float( rand()%(int)( fRadian ) );
		vPosition.x += cos( EtToRadian( fRandomAngle ) ) * fRandomRadian;
		vPosition.z += sin( EtToRadian( fRandomAngle ) ) * fRandomRadian;
	}
	else {
		MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();
		EtVector3 vXVector, vYVector, vZVector;
		vXVector = pCamera->m_vXAxis;
		vYVector = EtVector3( 0.f, 1.f, 0.f );
		EtVec3Cross( &vZVector, &vXVector, &vYVector );

		vPosition += vXVector * ( -vPosOffset.x * fRadian );
		vPosition += vZVector * ( vPosOffset.y * fRadian );
	}
	vPosition.y = GetHeight( vPosition.x, vPosition.z, &vNormal );
	// 최소거리 체크
	if( bRandomPos == true ) {
		EtVector3 vOffset;
		float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
		float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
		vOffset.x = m_Offset.x - ( fWidth / 2.f );
		vOffset.z = m_Offset.z - ( fHeight / 2.f );
		vOffset.y = 0.f;

		DNVector(CEtWorldProp *) pVecAdjacentList;
		SSphere Sphere;
		Sphere.Center = EtVector3( vPosition.x + vOffset.x, vPosition.y, vPosition.z + vOffset.z ); 
		Sphere.fRadius = fMinPropDistance;
		m_pPropOctree->Pick( Sphere, pVecAdjacentList );
		if( bIgnoreDistanceSize ) {
			for( DWORD i=0; i<pVecAdjacentList.size(); i++ ) {
				if( EtVec3Length( &( *pVecAdjacentList[i]->GetPosition() - vPosition ) ) > fMinPropDistance ) {
					pVecAdjacentList.erase( pVecAdjacentList.begin() + i );
					i--;
				}
			}
		}
		if( pVecAdjacentList.size() > 0 ) return;
	}

	CRect rcEditRect = CRect( 0, 0, (int)GetTileWidthCount(), (int)GetTileHeightCount() );
	if( vPosition.x / m_fTileSize < rcEditRect.left || vPosition.x / m_fTileSize > rcEditRect.right ) return;
	if( vPosition.z / m_fTileSize < rcEditRect.top || vPosition.z / m_fTileSize > rcEditRect.bottom ) return;
	if( vPosition.y < rHeight.nMin || vPosition.y > rHeight.nMax ) return;
	float fDegree = EtToDegree( acos( EtVec3Dot( &vNormal, &EtVector3( 0.f, 1.f, 0.f ) ) ) );
	if( fDegree < rRadian.fMin || fDegree > rRadian.fMax ) return;

	if( bRandomRotateX ) {
		float fRandomAngle = ( rand()%36000 ) / 100.f;
		vRotate.x = fRandomAngle;
	}
	else vRotate.x = 0.f;

	if( bRandomRotateY ) {
		float fRandomAngle = ( rand()%36000 ) / 100.f;
		vRotate.y = fRandomAngle;
	}
	else vRotate.y = 0.f;

	if( bRandomRotateZ ) {
		float fRandomAngle = ( rand()%36000 ) / 100.f;
		vRotate.z = fRandomAngle;
	}
	else vRotate.z = 0.f;

	if( bLockHeightProp == true ) {
		EtVector3 vXVector(1.f, 0.f, 0.f );
		EtVector3 vYVector(0.f, 1.f, 0.f );
		EtVector3 vZVector(0.f, 0.f, 1.f );
		float fDotX, fDotZ;
		fDotX = EtVec3Dot( &vXVector, &vNormal );
		fDotZ = EtVec3Dot( &vZVector, &vNormal );

		vRotate.z = 90.f - EtToDegree( acos( fDotX ) );
		vRotate.x = 90.f - EtToDegree( acos( fDotZ ) );
	}
	// scale
	int nScaleRange;
	float fRandom;

	nScaleRange = (int)( ( rScaleX.fMax - rScaleX.fMin ) * 100.f );
	if( nScaleRange > 0 )
		fRandom = rScaleX.fMin +  ( (int)( rand()%nScaleRange ) / 100.f );
	else fRandom = rScaleX.fMin;
	vScale.x = fRandom;
	if( CGlobalValue::GetInstance().m_bLockScaleAxis == true ) {
		vScale.y = vScale.z = vScale.x;
	}
	else {
		nScaleRange = (int)( ( rScaleY.fMax - rScaleY.fMin ) * 100.f );
		if( nScaleRange > 0 )
			fRandom = rScaleY.fMin +  ( (int)( rand()%nScaleRange ) / 100.f );
		else fRandom = rScaleY.fMin;
		vScale.y = fRandom;

		nScaleRange = (int)( ( rScaleZ.fMax - rScaleZ.fMin ) * 100.f );
		if( nScaleRange > 0 )
			fRandom = rScaleZ.fMin +  ( (int)( rand()%nScaleRange ) / 100.f );
		else fRandom = rScaleZ.fMin;
		vScale.z = fRandom;
	}

	vPosition.y = GetHeight( vPosition.x, vPosition.z );

	CTEtWorldProp *pProp = (CTEtWorldProp *)AllocProp( GetPropClassID( szPropName ) );
	pProp->Initialize( this, szPropName, vPosition, vRotate, vScale );
	int nValue = AddPropCreateUniqueCount();
	pProp->SetCreateUniqueID( nValue );

	// 땅에 얼마나 박힐건지
	SAABox Box( &EtVector3( 0.f, 0.f, 0.f ), &EtVector3( 0.f, 0.f, 0.f ) );
	pProp->GetBoundingBox( Box );
	float fHeightPos = Box.Max.y - Box.Min.y;
	vPosition.y -= ( fHeightPos * ( CGlobalValue::GetInstance().m_fHeightDigPer / 100.f ) );
	pProp->SetPosition( vPosition );
	pProp->UpdateMatrixEx();


	InsertProp( pProp );

	if( pElement ) {
		((CActionElementBrushProp*)pElement)->AddProp( pProp );
	}
}

CEtWorldProp *CTEtWorldSector::CloneProp( CEtWorldProp *pProp, CActionElement *pElement )
{
	CTEtWorldProp *pClone = (CTEtWorldProp *)AllocProp( pProp->GetClassID() );
	pClone->Initialize( this, pProp->GetPropName(), *pProp->GetPosition(), *pProp->GetRotation(), *pProp->GetScale() );

	pClone->Show( false );

	pClone->CopyCustomParameter( (CTEtWorldProp*)pProp );
	/*
	if( pProp->GetAdditionalPropInfoSize() > 0 )
		memcpy( pClone->GetAdditionalPropInfo(), pProp->GetAdditionalPropInfo(), pProp->GetAdditionalPropInfoSize() );
	*/
	EtVector3 vPos = *pProp->GetPosition();
	vPos.x += 10.f;
	pClone->SetPosition( vPos );
	pClone->SetRotation( *pProp->GetRotation() );
	pClone->SetScale( *pProp->GetScale() );
	pClone->UpdateMatrixEx();
	pClone->Show( true );

	InsertProp( pClone );

	if( pElement ) {
		((CActionElementBrushProp*)pElement)->AddProp( pClone );
	}

	return pClone;
}

bool CTEtWorldSector::EnableLightmap( bool bEnable )
{
	if( bEnable == true ) {
		int nBlurSize = 1;
		if( CEnviControl::GetInstance().GetActiveElement() ) {
			nBlurSize = (int)CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetLightmapBlurSize();
		}
		m_Handle->BakeLightMap( -1, 128, 128, ( float )nBlurSize );
	}
	else {
		m_Handle->DeleteLightMapInfluenceBuffer();
	}
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	Option.bUseTerrainLightMap = bEnable;
	SetGraphicOption( Option );

	return true;
}

void CTEtWorldSector::SaveLightmap()
{
	int i, nStart, nEnd;
	m_Handle->CalcSelectCount( -1, nStart, nEnd );
	char szTemp[_MAX_PATH] = { 0, };
	for( i = nStart; i < nEnd; i++ )
	{
		EtTextureHandle hTexture = m_Handle->GetLightMap( i );
		if( hTexture )
		{
			sprintf_s( szTemp, "%s\\_LM", GetWorkingPath() );
			if( !PathFileExists( szTemp ) ) _mkdir( szTemp );

			sprintf_s( szTemp, "%s\\_LM\\_LM%d.dds", GetWorkingPath(), i );
			D3DXSaveSurfaceToFile( szTemp, D3DXIFF_DDS, hTexture->GetSurfaceLevel(), NULL, NULL );
		}
	}
}

void CTEtWorldSector::InsertControl( const char *szControlName )
{
	if( GetControlFromName( szControlName ) ) return;
	CTEtWorldEventControl *pControl = (CTEtWorldEventControl *)AllocControl();
	pControl->SetName( szControlName );
	m_pVecAreaControl.push_back( pControl );
}

CEtWorldEventControl *CTEtWorldSector::GetControlFromName( const char *szControlName )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		if( strcmp( szControlName, ((CTEtWorldEventControl*)m_pVecAreaControl[i])->GetName() ) == NULL ) return m_pVecAreaControl[i];
	}
	return NULL;
}

void CTEtWorldSector::AddArea( const char *szControlName, const char *szName, CActionElement *pElement )
{
	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( !pControl ) return;

	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	EtVector3 vMin, vMax;
	vMin.x = min( m_vPickPrevPos.x, m_vPickPos.x );
	vMin.z = min( m_vPickPrevPos.z, m_vPickPos.z );
	vMax.x = max( m_vPickPrevPos.x, m_vPickPos.x );
	vMax.z = max( m_vPickPrevPos.z, m_vPickPos.z );
	vMin.y = vMax.y = 0.f;

	vMin -= vOffset;
	vMax -= vOffset;

	if( vMax.x - vMin.x < 100.f || vMax.z - vMin.z < 100.f ) return;

	CEtWorldEventArea *pArea = pControl->InsertArea( vMin, vMax, szName );
	if( pArea ) {
		((CTEtWorldEventArea*)pArea)->InitializeCustomRender();
		((CActionElementAddRemoveEventArea*)pElement)->SetEventArea( pArea );
	}
}

CEtWorldEventArea *CTEtWorldSector::CopyArea( CEtWorldEventArea *pOrigArea, CActionElement *pElement )
{
	CEtWorldEventControl *pControl = pOrigArea->GetControl();
	std::string szName = pOrigArea->GetName();
	szName += "_Copy";

	EtVector3 vMin, vMax;
	vMin = *pOrigArea->GetMin();
	vMax = *pOrigArea->GetMax();

	CEtWorldEventArea *pArea = pControl->InsertArea( vMin, vMax, szName.c_str() );
	if( pArea ) {
		pArea->SetRotate( pOrigArea->GetRotate() );
		((CTEtWorldEventArea*)pArea)->InitializeCustomRender();
		((CActionElementAddRemoveEventArea*)pElement)->SetEventArea( pArea );
		return pArea;
	}
	return NULL;
}

CEtWorldEventControl *CTEtWorldSector::AllocControl()
{
	CTEtWorldEventControl *pControl = new CTEtWorldEventControl( this );
	return pControl;
}

void CTEtWorldSector::RemoveArea( const char *szControlName, const char *szName, CActionElement *pElement )
{
	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( !pControl ) return;

	CEtWorldEventArea *pArea = pControl->GetAreaFromName( szName );
	if( pArea ) ((CActionElementAddRemoveEventArea*)pElement)->SetEventArea( pArea );
	pControl->RemoveArea( szName ); 
}

void CTEtWorldSector::RemoveArea( const char *szControlName, int nUniqueID, CActionElement *pElement )
{
	CEtWorldEventControl *pControl = GetControlFromName( szControlName );
	if( !pControl ) return;

	CEtWorldEventArea *pArea = pControl->GetAreaFromCreateUniqueID( nUniqueID );
	if( pArea ) ((CActionElementAddRemoveEventArea*)pElement)->SetEventArea( pArea );
	pControl->RemoveAreaFromCreateUniqueID( nUniqueID ); 
}

CEtWorldSound *CTEtWorldSector::AllocSound()
{
	return new CTEtWorldSound( this );
}

bool CTEtWorldSector::AddSound( const char *szName )
{
	if( !m_pSoundInfo ) return false;

	float fRange = EtVec3Length( &( m_vPickPos - m_vPickPrevPos ) );
	if( fRange < 100.f ) return false;
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)m_pSoundInfo->AllocSoundEnvi();
	pEnvi->SetName( szName );
	EtVector3 vPos;
	vPos = m_vPickPrevPos;
	vPos.y = GetHeight( m_vPickPrevPos.x, m_vPickPrevPos.z );
	vPos -= vOffset;
	pEnvi->SetPosition( vPos );
	pEnvi->SetRange( fRange );
	m_pSoundInfo->InsertSoundEnvi( pEnvi );
	return true;
}

void CTEtWorldSector::RemoveSound( const char *szName )
{
	CEtWorldSoundEnvi *pEnvi = m_pSoundInfo->GetEnviFromName( szName );
	if( pEnvi ) m_pSoundInfo->DeleteSoundEnvi( pEnvi );
}

void CTEtWorldSector::DrawSound()
{
	DWORD dwColor;
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;
	MatrixEx Cross;

	for( DWORD i=0; i<m_pSoundInfo->GetEnviCount(); i++ ) {
		CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)m_pSoundInfo->GetEnviFromIndex(i);

		if( CGlobalValue::GetInstance().m_pSelectSound == pEnvi ) {
			dwColor = 0xFF4040FF;
			pEnvi->ShowAxis( true );
			float fLength = EtVec3Length( &( *pEnvi->GetPosition() - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );
			pEnvi->SetAxisScale( fLength / 600.f );

		}
		else {
			dwColor = 0xFFFFFF00;
			pEnvi->ShowAxis( false );
		}

		Cross.m_vPosition = *pEnvi->GetPosition() + vOffset;
		pEnvi->UpdateAxis( Cross );
		DrawSound( *pEnvi->GetPosition(), vOffset, pEnvi->GetRange(), pEnvi->GetRollOff(), dwColor );
	}
}

void CTEtWorldSector::DrawSound( EtVector3 &vPosition, EtVector3 &vOffset, float fRange, float fRollOff, DWORD dwColor )
{
	EtVector3 vPos[8];
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	float fCurAngle = 0.f;
	vPos[0].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * fRange );
	vPos[0].z = vPosition.z + ( sin( EtToRadian( fCurAngle ) ) * fRange );
	if( vPos[0].x < 0.f || vPos[0].x >= fWidth || vPos[0].z < 0.f || vPos[0].z >= fHeight ) vPos[0].y = 0.f;
	vPos[0].y = GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

	vPos[2].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * fRange );
	vPos[2].y = vPosition.y + ( sin( EtToRadian( fCurAngle ) ) * fRange );
	vPos[2].z = vPosition.z;

	vPos[4].z = vPosition.z + ( cos( EtToRadian( fCurAngle ) ) * fRange );
	vPos[4].y = vPosition.y + ( sin( EtToRadian( fCurAngle ) ) * fRange );
	vPos[4].x = vPosition.x;

	vPos[6].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * ( fRange * fRollOff ) );
	vPos[6].z = vPosition.z + ( sin( EtToRadian( fCurAngle ) ) * ( fRange * fRollOff ) );
	vPos[6].y = GetHeight( vPos[6].x, vPos[6].z ) + 30.f;
	if( vPos[6].x < 0.f || vPos[6].x >= fWidth || vPos[6].z < 0.f || vPos[6].z >= fHeight ) vPos[6].y = 0.f;
	vPos[6].y = GetHeight( vPos[6].x, vPos[6].z ) + 30.f;



	for( DWORD i=0;; i++ ) {
		vPos[1].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * fRange );
		vPos[1].z = vPosition.z + ( sin( EtToRadian( fCurAngle ) ) * fRange );
		if( vPos[1].x < 0.f || vPos[1].x >= fWidth || vPos[1].z < 0.f || vPos[1].z >= fHeight ) vPos[1].y = 0.f;

		vPos[1].y = GetHeight( vPos[1].x, vPos[1].z ) + 30.f;
		EternityEngine::DrawLine3D( ( vPos[0] + vOffset ), ( vPos[1] + vOffset ), dwColor );

		vPos[3].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * fRange );
		vPos[3].y = vPosition.y + ( sin( EtToRadian( fCurAngle ) ) * fRange );
		vPos[3].z = vPosition.z;
		EternityEngine::DrawLine3D( ( vPos[2] + vOffset ), ( vPos[3] + vOffset ), dwColor );

		vPos[5].z = vPosition.z + ( cos( EtToRadian( fCurAngle ) ) * fRange );
		vPos[5].y = vPosition.y + ( sin( EtToRadian( fCurAngle ) ) * fRange );
		vPos[5].x = vPosition.x;
		EternityEngine::DrawLine3D( ( vPos[4] + vOffset ), ( vPos[5] + vOffset ), dwColor );


		vPos[7].x = vPosition.x + ( cos( EtToRadian( fCurAngle ) ) * ( fRange * fRollOff ) );
		vPos[7].z = vPosition.z + ( sin( EtToRadian( fCurAngle ) ) * ( fRange * fRollOff ) );
		if( vPos[7].x < 0.f || vPos[7].x >= fWidth || vPos[7].z < 0.f || vPos[7].z >= fHeight ) vPos[7].y = 0.f;

		vPos[7].y = GetHeight( vPos[7].x, vPos[7].z ) + 30.f;
		EternityEngine::DrawLine3D( ( vPos[6] + vOffset ), ( vPos[7] + vOffset ), dwColor );

		vPos[0] = vPos[1];
		vPos[2] = vPos[3];
		vPos[4] = vPos[5];
		vPos[6] = vPos[7];

		if( fCurAngle >= 360.f ) break;
		fCurAngle += 5.f;
	}
}

void CTEtWorldSector::ShowProp( bool bShow )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {

		if (bShow && CTEtWorldSector::FindFilteringPropName(m_pVecPropList[i]->GetPropName()))
		{
			CTEtWorldLightProp* pLightProp = dynamic_cast<CTEtWorldLightProp*>(m_pVecPropList[i]);
			if (pLightProp)
				pLightProp->ShowProp(false);
			else
				((CTEtWorldProp*)m_pVecPropList[i])->Show( false );
			
			continue;
		}

		if( CheckIgnoreProp( m_pVecPropList[i]->GetPropName() ) )
		{
			((CTEtWorldProp*)m_pVecPropList[i])->Show( false );
			continue;
		}

		((CTEtWorldProp*)m_pVecPropList[i])->Show( bShow );
	}
}

void CTEtWorldSector::ShowPropCollision( bool bShow )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		EtAniObjectHandle hHandle = ((CTEtWorldProp*)m_pVecPropList[i])->GetObjectHandle();
		if( !hHandle ) continue;

		hHandle->ShowCollisionPrimitive( bShow );
	}
}

void CTEtWorldSector::ShowWater( bool bShow )
{
	m_bShowWater = bShow;
}

void CTEtWorldSector::ShowAttribute( bool bShow )
{
	m_bShowAttribute = bShow;
}

void CTEtWorldSector::ShowNavigation( bool bShow )
{
	m_bShowNavigation = bShow;
}

void CTEtWorldSector::DrawAttribute()
{
	for( DWORD i=0; i<m_pVecAttributePrimitive.size(); i++ ) {
		EternityEngine::DrawTriangle3D( m_pVecAttributePrimitive[i], m_dwVecAttributePrimitiveCount[i] );
	}
}

void CTEtWorldSector::ShowEventArea( bool bShow )
{
	m_bShowEventArea = bShow;
}

void CTEtWorldSector::CheckDiagonalAttribute( CRect rcRect )
{
	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileHeightCount() * GetTileSize() ) / m_nAttributeBlockSize;

	bool bCheckFlag[4] = { false, };
	char cEdgeAttr[4] = { 0, };

	int nCheckBlockWidth[4] = { 0, };
	int nCheckBlockHeight[4] = { 0, };

	for( int j=rcRect.top; j<=rcRect.bottom; j++ ) 
	{
		for( int i=rcRect.left; i<=rcRect.right; i++ ) 
		{
			char cCurAttr = *( m_pAttribute + ( j * nWidthCount ) + i );
			if( cCurAttr >> 4 != 0 ) 
			{
				*( m_pAttribute + ( j * nWidthCount ) + i ) = 0;
				cCurAttr = 0;
			}
			if( cCurAttr != 0 ) continue;

			nCheckBlockWidth[0] = i, nCheckBlockHeight[0] = j-1;
			nCheckBlockWidth[1] = i+1, nCheckBlockHeight[1] = j;
			nCheckBlockWidth[2] = i, nCheckBlockHeight[2] = j+1;
			nCheckBlockWidth[3] = i-1, nCheckBlockHeight[3] = j;

			for( int k=0; k<4; k++ ) {
				if( nCheckBlockWidth[k] == -1 || nCheckBlockHeight[k] == -1 || nCheckBlockWidth[k] == nWidthCount || nCheckBlockHeight[k] == nHeightCount ) 
				{
					bCheckFlag[k] = false;
				}
				else 
				{
					char cAttr = *( m_pAttribute + ( nCheckBlockHeight[k] * nWidthCount ) + nCheckBlockWidth[k] );
					if( cAttr != 0 && ( cAttr >> 4 ) == 0 ) 
					{
						cEdgeAttr[k] = cAttr;
						bCheckFlag[k] = true;
					}
					else bCheckFlag[k] = false;
				}
			}

			char cResultAttr = 0;

			if( bCheckFlag[0] == false && bCheckFlag[1] == true && bCheckFlag[2] == true && bCheckFlag[3] == false && cEdgeAttr[1] == cEdgeAttr[2] ) cResultAttr = 0x10 | cEdgeAttr[1];
			else if( bCheckFlag[0] == true && bCheckFlag[1] == true && bCheckFlag[2] == false && bCheckFlag[3] == false && cEdgeAttr[0] == cEdgeAttr[1] ) cResultAttr = 0x20 | cEdgeAttr[0];
			else if( bCheckFlag[0] == false && bCheckFlag[1] == false && bCheckFlag[2] == true && bCheckFlag[3] == true && cEdgeAttr[2] == cEdgeAttr[3] ) cResultAttr = 0x80 | cEdgeAttr[2];
			else if( bCheckFlag[0] == true && bCheckFlag[1] == false && bCheckFlag[2] == false && bCheckFlag[3] == true && cEdgeAttr[0] == cEdgeAttr[3] ) cResultAttr = 0x40 | cEdgeAttr[3];

			if( cResultAttr != 0 ) *( m_pAttribute + ( j * nWidthCount ) + i ) |= cResultAttr;
		}
	}
}

void CTEtWorldSector::UpdateAttribute( CRect &rcRect, bool bCheckDiagonal )
{
	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileHeightCount() * GetTileSize() ) / m_nAttributeBlockSize;

	int nBlockCountX = nWidthCount / m_nDrawAttributeCount;
	int nBlockCountY = nHeightCount / m_nDrawAttributeCount;

	if( nWidthCount % m_nDrawAttributeCount > 0 ) nBlockCountX += 1;
	if( nHeightCount % m_nDrawAttributeCount > 0 ) nBlockCountY += 1;

	if( bCheckDiagonal ) {
		CRect rcTemp = rcRect;
		rcTemp.InflateRect( 2, 2, 2, 2 );
		if( rcTemp.left < 0 ) rcTemp.left = 0;
		if( rcTemp.left >= nWidthCount ) rcTemp.left = nWidthCount - 1;
		if( rcTemp.right < 0 ) rcTemp.right = 0;
		if( rcTemp.right >= nWidthCount ) rcTemp.right = nWidthCount - 1;
		if( rcTemp.top < 0 ) rcTemp.top = 0;
		if( rcTemp.top >= nHeightCount ) rcTemp.top = nHeightCount - 1;
		if( rcTemp.bottom < 0 ) rcTemp.bottom = 0;
		if( rcTemp.bottom >= nHeightCount ) rcTemp.bottom = nHeightCount - 1;

		CheckDiagonalAttribute( rcTemp );
	}

	for( int m=0; m<nBlockCountY; m++ ) {
		for( int n=0; n<nBlockCountX; n++ ) {
			if( n < ( rcRect.left / m_nDrawAttributeCount ) || n > ( rcRect.right / m_nDrawAttributeCount ) ) continue;
			if( m < ( rcRect.top / m_nDrawAttributeCount ) || m > ( rcRect.bottom / m_nDrawAttributeCount ) ) continue;

			int nPrimitiveX = m_nDrawAttributeCount;
			int nPrimitiveY = m_nDrawAttributeCount;
			if( n == nBlockCountX - 1 && nWidthCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveX = nWidthCount % m_nDrawAttributeCount;
			}
			if( m == nBlockCountY - 1 && nHeightCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveY = nHeightCount % m_nDrawAttributeCount;
			}
			int nPosX = ( n * m_nDrawAttributeCount );
			int nPosY = ( m * m_nDrawAttributeCount );
			SPrimitiveDraw3D *pBuffer = m_pVecAttributePrimitive[ m * nBlockCountX + n ];
			for( int j=0; j<nPrimitiveY; j++ ) {
				for( int i=0; i<nPrimitiveX; i++ ) {
					DWORD dwColor = 0;
					char cAttr = m_pAttribute[ (j+nPosY) * nWidthCount + (i+nPosX) ];
					if( cAttr & 0x01 ) dwColor = 0x55EE2222;
					if( cAttr & 0x02 ) dwColor = 0x552222EE;
					if( cAttr & 0x04 ) dwColor = 0x5522EE22;
					if( cAttr & 0x08 ) dwColor = 0x55EEEE11;
					if( ( cAttr & 0xf ) == 0xf ) dwColor = 0x55ff00ff;

					pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = dwColor;

					if( (cAttr & 0x10) || (cAttr & 0x40) ) {
						bool *pTriangleType = m_bVecAttributePrimitiveType[ m * nBlockCountX + n ];
						if( pTriangleType[ j * nPrimitiveX + i ] == false ) {
							pTriangleType[ j * nPrimitiveX + i ] = true;

							EtVector3 vTemp = pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position = vTemp;
						}
						if( cAttr & 0x10 ) {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = 0;
						}
						else {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = 0;
						}

					}
					else if( (cAttr & 0x20) || (cAttr & 0x80) ) {
						bool *pTriangleType = m_bVecAttributePrimitiveType[ m * nBlockCountX + n ];
						if( pTriangleType[ j * nPrimitiveX + i ] == true ) {
							pTriangleType[ j * nPrimitiveX + i ] = false;

							EtVector3 vTemp = pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position = vTemp;
						}

						if( cAttr & 0x20 ) {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = 0;
						}
						else {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = 0;
						}
					}
				}
			}
		}
	}

}

void CTEtWorldSector::ApplyAttribute( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, char cAttr, bool bMousePos )
{
	if( !m_Handle ) return;
	EtVector3 vPickPos;
	if( bMousePos == true ) {
		if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return;

		float fCurAngle = 0.f;
		EtVector3 vPos[2];
		EtVector3 vOffset;
		float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
		float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
		vOffset.x = m_Offset.x - ( fWidth / 2.f );
		vOffset.z = m_Offset.z - ( fHeight / 2.f );
		vOffset.y = 0.f;
		vPickPos -= vOffset;
	}
	else {
		vPickPos.x = nX * m_fTileSize;
		vPickPos.z = nY * m_fTileSize;
		vPickPos.y = GetHeight( vPickPos.x, vPickPos.z );
	}

	CRect rcRect;
	std::vector<short *> pVecPtrList;
	int nAverage = 0;
	int nSize = (int)( fRadian / m_nAttributeBlockSize );
	fRadian /= 2.f;
	if( fRadian < m_nAttributeBlockSize ) {
		nSize = 0;
	}

	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;

	CRect rcEditRect = CRect( 0, 0, (int)nWidthCount, (int)nHeightCount );
	if( CGlobalValue::GetInstance().m_bLockSideHeight == true ) {
		rcEditRect.DeflateRect( 1, 1, 1, 1 );
	}

	int nSizeX, nSizeY;
	int nCountX, nCountY;
	m_Handle->GetBlockSize( nSizeX, nSizeY ); 
	m_Handle->GetBlockCount( nCountX, nCountY );

	for( int i=0; i<=nSize; i++ ) {
		float fY = vPickPos.z - fRadian + ( i * m_nAttributeBlockSize );
		int nY = (int)( fY / m_nAttributeBlockSize );

		for( int j=0; j<=nSize; j++ ) {
			float fX = vPickPos.x - fRadian + ( j * m_nAttributeBlockSize );
			int nX = (int)( fX / m_nAttributeBlockSize );

			if( nSize == 0 ) {
				fX = vPickPos.x;
				fY = vPickPos.z;
				nX = (int)( fX / m_nAttributeBlockSize );
				nY = (int)( fY / m_nAttributeBlockSize );
			}

			if( i == 0 ) rcRect.top = nY;
			if( i == nSize ) rcRect.bottom = nY;
			if( j == 0 ) rcRect.left = nX;
			if( j == nSize ) rcRect.right = nX;
			if( fX < 0.f || fY < 0.f ) continue;
			if( nX < rcEditRect.left || nX >= rcEditRect.right ) continue;
			if( nY < rcEditRect.top || nY >= rcEditRect.bottom ) continue;

			float fLength = EtVec2Length( &( EtVector2( fX, fY ) - EtVector2( vPickPos.x, vPickPos.z ) ) );
			if( fLength > fRadian ) continue;

			int nTileOffset = (int)( ( nY * m_nAttributeBlockSize ) / m_fTileSize ) * GetTileWidthCount() + (int)( ( nX * m_nAttributeBlockSize ) / m_fTileSize );
			// 높이체크
			if( m_pHeight[nTileOffset] < rHeight.nMin ||
				m_pHeight[nTileOffset] > rHeight.nMax ) continue;

			// 각 체크
			
			EtVector3 vNormal = GetHeightNormal( (int)( ( nX * m_nAttributeBlockSize ) / m_fTileSize ), (int)( ( nY * m_nAttributeBlockSize ) / m_fTileSize ) );
			float fDegree = EtToDegree( acos( EtVec3Dot( &vNormal, &EtVector3( 0.f, 1.f, 0.f ) ) ) );
			if( fDegree < rRadian.fMin || fDegree > rRadian.fMax ) continue;

			int nOffset = nY * nWidthCount + nX;
//			m_pAttribute[nOffset] = ( nModifyType == 0 ) ? true : false;
			m_pAttribute[nOffset] = cAttr;
		}
	}

	UpdateAttribute( rcRect, CGlobalValue::GetInstance().m_bAutoDiagonal );

	m_rcLastUpdate.left = ( m_rcLastUpdate.left == -1 ) ? rcRect.left : ( ( m_rcLastUpdate.left > rcRect.left ) ? rcRect.left : m_rcLastUpdate.left );
	m_rcLastUpdate.top = ( m_rcLastUpdate.top == -1 ) ? rcRect.top : ( ( m_rcLastUpdate.top > rcRect.top ) ? rcRect.top : m_rcLastUpdate.top );
	m_rcLastUpdate.right = ( m_rcLastUpdate.right == -1 ) ? rcRect.right : ( ( m_rcLastUpdate.right < rcRect.right ) ? rcRect.right : m_rcLastUpdate.right );
	m_rcLastUpdate.bottom = ( m_rcLastUpdate.bottom == -1 ) ? rcRect.bottom : ( ( m_rcLastUpdate.bottom < rcRect.bottom ) ? rcRect.bottom : m_rcLastUpdate.bottom );

	if( m_rcLastUpdate.left < 0 ) m_rcLastUpdate.left = 0;
	else if( m_rcLastUpdate.left >= nWidthCount ) m_rcLastUpdate.left = nWidthCount - 1;

	if( m_rcLastUpdate.top < 0 ) m_rcLastUpdate.top = 0;
	else if( m_rcLastUpdate.top >= nHeightCount ) m_rcLastUpdate.top = nHeightCount - 1;

	if( m_rcLastUpdate.right < 0 ) m_rcLastUpdate.right = 0;
	else if( m_rcLastUpdate.right >= nWidthCount ) m_rcLastUpdate.right = nWidthCount - 1;

	if( m_rcLastUpdate.bottom < 0 ) m_rcLastUpdate.bottom = 0;
	else if( m_rcLastUpdate.bottom >= nHeightCount ) m_rcLastUpdate.bottom = nHeightCount - 1;

}

void CTEtWorldSector::BuildPropAttribute()
{
	int i, j;
	const float fMinSegmentLength = 50.0f;
	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;

	DNVector(CEtWorldProp *) vecPickProp;
	EtVector3 vOrigin;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOrigin.x = m_Offset.x - ( fWidth / 2.f );
	vOrigin.z = m_Offset.z - ( fHeight / 2.f );
	vOrigin.y = 0.f;
	vOrigin.x += m_nAttributeBlockSize * 0.5f;
	vOrigin.z += m_nAttributeBlockSize * 0.5f;
	for( i = 0; i < nHeightCount; i++ )
	{
		for( j = 0; j < nWidthCount; j++ )
		{
/*			if( ( m_pAttribute[ i * nWidthCount + j ] == 0xf ) || ( m_pAttribute[ i * nWidthCount + j ] == 0x8 ) )
			{
				m_pAttribute[ i * nWidthCount + j ] = 0;
			}*/
			if( m_pAttribute[ i * nWidthCount + j ] )
			{
				continue;
			}

			SSegment Segment;
			SSphere Sphere;
			Segment.vOrigin = vOrigin + EtVector3( ( float )j * m_nAttributeBlockSize, 0.0f, ( float )i * m_nAttributeBlockSize );
			Segment.vOrigin.y = GetHeight( ( j + 0.5f ) * m_nAttributeBlockSize, ( i + 0.5f ) * m_nAttributeBlockSize );
			Segment.vDirection = EtVector3( 0.0f, fMinSegmentLength, 0.0f );
			Sphere.Center = Segment.vOrigin + Segment.vDirection * 0.5f;
			Sphere.fRadius = fMinSegmentLength * 0.5f;
			vecPickProp.clear();
			m_pPropOctree->Pick( Sphere, vecPickProp, false );
			for( int k = 0; k < ( int )vecPickProp.size(); k++ )
			{
				SAABox BoundingBox;
				vecPickProp[ k ]->GetBoundingBox( BoundingBox );
				if( ( !BoundingBox.IsInside( Segment.vOrigin ) ) && ( !BoundingBox.IsInside( Segment.vOrigin + Segment.vDirection ) ) )
				{
					continue;
				}
				if( vecPickProp[ k ]->GetObjectHandle()->TestSegmentCollision( Segment ) )
				{
					/*
					int nClassID = vecPickProp[ k ]->GetClassID();
//					if( ( nClassID == CTEtWorldProp::Action ) || ( nClassID == CTEtWorldProp::Broken ) )
					if( ( nClassID == 1 ) || ( nClassID == 2 ) )
					{
						// 게이트 프랍이나 부서지는 프랍은 이벤트 처리 보류한다
//						m_pAttribute[ i * nWidthCount + j ] = 0xf;
					}
					else
					{
						m_pAttribute[ i * nWidthCount + j ] = 8;
					}
					*/
					if( ((CTEtWorldProp*)vecPickProp[k])->IsIncludeNavMesh() )
						m_pAttribute[ i * nWidthCount + j ] = 8;
					break;
				}
			}
		}
	}
	UpdateAttribute( CRect( 0, 0, nWidthCount, nHeightCount ), false );
}

void CTEtWorldSector::AddPropNavMesh()
{
	int i, j;

	CGlobalValue::GetInstance().UpdateInitDesc( "Generation Prop NaviMesh.." );
	int nSize = (int)m_pVecPropList.size();
	for( i = 0; i < ( int )m_pVecPropList.size(); i++ )
	{
		CGlobalValue::GetInstance().UpdateProgress( nSize, i );
		if( ( ( CTEtWorldProp * )m_pVecPropList[ i ] )->IsIgnoreBuildNavMesh() ) continue;
		NavigationMesh *pFindNavMesh = ( ( CTEtWorldProp * )m_pVecPropList[ i ] )->GetNavMesh();
		int nCount = pFindNavMesh->GetEntranceCount() / 2;
		// 출구 설정 안된 네비도 Add 가능하게..
/*		if( nCount <= 0 )
		{
			continue;
		}*/

		NavigationMesh *pNavMesh = new NavigationMesh();
		pFindNavMesh->CopyFrom( pNavMesh );

		std::vector< int > &vecEntrance = pNavMesh->GetEntrance();
		for( j = 0; j < nCount; j++ )
		{
			NavigationCell *pNavCell = pNavMesh->Cell( vecEntrance[ j * 2 ] );
			if( pNavCell == NULL )
			{
				continue;
			}
			FindSuitableNavCell( pNavCell, vecEntrance[ j * 2 + 1 ] );
		}
		int nPrevCellCount = m_pNavMesh->TotalCells();
		m_pNavMesh->AddCellCount( pNavMesh->TotalCells() );
		for( j = 0; j < pNavMesh->TotalCells(); j++ )
		{
			NavigationCell *pCell = pNavMesh->Cell( j );
			m_pNavMesh->AddCell( pCell->Vertex( 0 ), pCell->Vertex( 1 ), pCell->Vertex( 2 ), nPrevCellCount + j );
			m_pNavMesh->Cell( nPrevCellCount + j )->SetType( pCell->GetType() );
			m_pNavMesh->Cell( nPrevCellCount + j )->SetNavType( ( (CTEtWorldProp*)m_pVecPropList[ i ] )->GetNavType() );
		}
		m_pNavMesh->LinkCells();
		SAFE_DELETE( pNavMesh );
	}
}

bool CTEtWorldSector::FindSuitableNavCell( NavigationCell *pNavCell, int nEdgeIndex )
{
	int i;
	bool bResult = false;
	float fSuitableDot = -FLT_MAX;
	NavigationCell *pSuitableCell;
	int nSuitableEdgeIndex;
	EtVector3 vWallMidPoint;

	pNavCell->GetWallMidPoint( nEdgeIndex, vWallMidPoint );
	NavigationCell *pClosestCell = m_pNavMesh->FindClosestCell( vWallMidPoint );
	if( pClosestCell == NULL )
	{
		return bResult;
	}
	if( FindSuitableEdge( pNavCell, nEdgeIndex, pClosestCell, fSuitableDot, nSuitableEdgeIndex ) )
	{
		pSuitableCell = pClosestCell;
	}
	for( i = 0; i < 3; i++ )
	{
		if( pClosestCell->Link( i ) == NULL )
		{
			continue;
		}
		if( FindSuitableEdge( pNavCell, nEdgeIndex, pClosestCell->Link( i ), fSuitableDot, nSuitableEdgeIndex ) )
		{
			pSuitableCell = pClosestCell->Link( i );
		}
	}
	if( fSuitableDot != -FLT_MAX )
	{
		// pSuitableCell Split 처리
		// pSuitableCell의 edge가 pNavCell의 edge보다 길이가 1.3배 이상 길면 2개의 삼각형으로 분리한다.
		float pNavCellLength = pNavCell->Length( nEdgeIndex );
		float pSuitableCellLength = pSuitableCell->Length( nSuitableEdgeIndex );
		if( pSuitableCellLength > pNavCellLength * 1.3f )
		{
			EtVector3 vSplitPoint;
			if( EtVec3Length( &( pSuitableCell->Vertex( nSuitableEdgeIndex ) - vWallMidPoint ) ) < EtVec3Length( &( pSuitableCell->Vertex( ( nSuitableEdgeIndex + 1 ) % 3 ) - vWallMidPoint ) ) )
			{
				vSplitPoint = pSuitableCell->Vertex( ( nSuitableEdgeIndex + 1 ) % 3 ) - pSuitableCell->Vertex( nSuitableEdgeIndex );
				EtVec3Normalize( &vSplitPoint, &vSplitPoint );
				vSplitPoint *= pNavCellLength;
				vSplitPoint += pSuitableCell->Vertex( nSuitableEdgeIndex );
				m_pNavMesh->SplitCell( pSuitableCell, nSuitableEdgeIndex, vSplitPoint );
			}
			else
			{
				vSplitPoint = pSuitableCell->Vertex( nSuitableEdgeIndex ) - pSuitableCell->Vertex( ( nSuitableEdgeIndex + 1 ) % 3 );
				EtVec3Normalize( &vSplitPoint, &vSplitPoint );
				vSplitPoint *= pNavCellLength;
				vSplitPoint += pSuitableCell->Vertex( ( nSuitableEdgeIndex + 1 ) % 3 );
				pSuitableCell = m_pNavMesh->SplitCell( pSuitableCell, nSuitableEdgeIndex, vSplitPoint );
			}
		}

		EtVector3 vVertex[ 3 ];
		pNavCell->GetVertex( vVertex );
		vVertex[ nEdgeIndex ] = pSuitableCell->Vertex( ( nSuitableEdgeIndex + 1 ) % 3 );
		vVertex[ ( nEdgeIndex + 1 ) % 3 ] = pSuitableCell->Vertex( nSuitableEdgeIndex );

		NavigationCell *pLinkCell;
		int j;
		EtVector3 vLinkVertex[ 3 ];
		for( i = 0; i < 3; i++ )
		{
			pLinkCell = pNavCell->Link( i );
			if( pLinkCell )
			{
				for( j = 0; j < 3; j++ )
				{
					if( pLinkCell->Link( j ) == pNavCell )
					{
						pLinkCell->GetVertex( vLinkVertex );
						vLinkVertex[ j ] = vVertex[ ( i + 1 ) % 3 ];
						vLinkVertex[ ( j + 1 ) % 3 ] = vVertex[ i ];
						pLinkCell->Initialize( vLinkVertex[ 0 ], vLinkVertex[ 1 ], vLinkVertex[ 2 ] );
						break;
					}
				}
			}
		}
		pNavCell->Initialize( vVertex[ 0 ], vVertex[ 1 ], vVertex[ 2 ] );
	}

	return bResult;
}

bool CTEtWorldSector::FindSuitableEdge( NavigationCell *pSourCell, int nSourEdgeIndex, NavigationCell *pClosestCell, float &fSuitableDot, int &nSuitableEdgeIndex )
{
	int i;
	bool bResult = false;
	SSegment SourSeg, DestSeg;
	EtVector3 vWallMidPoint, vSourDir, vDestDir;

	SourSeg.vOrigin = pSourCell->Vertex( nSourEdgeIndex );
	SourSeg.vDirection = pSourCell->Vertex( ( nSourEdgeIndex + 1 ) % 3 ) - pSourCell->Vertex( nSourEdgeIndex );
	EtVec3Normalize( &vSourDir, &SourSeg.vDirection );
	for( i = 0; i < 3; i++ )
	{
		if( pClosestCell->Link( i ) )
		{
			continue;
		}
		DestSeg.vOrigin = pClosestCell->Vertex( i % 3 );
		DestSeg.vDirection = pClosestCell->Vertex( ( i + 1 ) % 3 ) - pClosestCell->Vertex( i % 3 );
		EtVec3Normalize( &vDestDir, &DestSeg.vDirection );
		float fDot = EtVec3Dot( &vSourDir, &vDestDir );
		if( fDot >= 0.0f )
		{
			continue;
		}
		fDot = fabs( fDot );
		if( fDot > fSuitableDot )
		{
			fSuitableDot = fDot;
			nSuitableEdgeIndex = i;
			bResult = true;
		}
	}

	return bResult;
}

int CTEtWorldSector::FindNearestAttribute( EtVector3 vStart, EtVector3 vDir )
{
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nX, nZ;

	vStart.x -= m_Offset.x - ( fWidth / 2.f );
	vStart.z -= m_Offset.z - ( fHeight / 2.f );
	while( 1 )
	{
		nX = ( int )( vStart.x / m_nAttributeBlockSize );
		nZ = ( int )( vStart.z / m_nAttributeBlockSize );
		if( ( nX < 0 ) || ( nX >= nWidthCount ) )
		{
			return 0;
		}
		if( ( nZ < 0 ) || ( nZ >= nHeightCount ) )
		{
			return 0;
		}
		if( m_pAttribute[ nZ * nWidthCount + nX ] )
		{
			return m_pAttribute[ nZ * nWidthCount + nX ] & 0xf;
		}
		float fModX, fModZ;
		if( vDir.x == 0.0f )
		{
			fModX = FLT_MAX;
		}
		else
		{
			fModX = fabs( ( ( ( int )( vStart.x / m_nAttributeBlockSize ) + 1 ) * m_nAttributeBlockSize - vStart.x ) / vDir.x );
		}
		if( vDir.z == 0.0f )
		{
			fModZ = FLT_MAX;
		}
		else
		{
			fModZ = fabs( ( ( ( int )( vStart.z / m_nAttributeBlockSize ) + 1 ) * m_nAttributeBlockSize - vStart.z ) / vDir.z );
		}
		if( fModX > fModZ )
		{
			vStart += fModZ * vDir;
		}
		else
		{
			vStart += fModX * vDir;
		}
	}
}

void CTEtWorldSector::BuildNavCellSideAttribute()
{
	int i, j;

	CGlobalValue::GetInstance().UpdateInitDesc( "Generation Terrain Attribute NaviMesh.." );
	int nSize = m_pNavMesh->TotalCells();
	for( i = 0; i < m_pNavMesh->TotalCells(); i++ )
	{
		NavigationCell *pCell = m_pNavMesh->Cell( i );
		for( j = 0; j < 3; j++ )
		{
			if( pCell->Link( j ) )	// 링크가 있으면 벽이 아니므로 통과.
			{
				continue;
			}
			if( pCell->GetType() == NavigationCell::CT_PROP )
			{
				continue;
			}
			EtVector3 vWallMidPoint = pCell->WallMidpoint( j );
			EtVector3 vWallNormal = pCell->Vertex( ( j + 1 ) % 3 ) - pCell->Vertex( j );
			vWallNormal.y = 0.0f;
			EtVec3Normalize( &vWallNormal, &vWallNormal );
			EtVec3Cross( &vWallNormal, &vWallNormal, &EtVector3( 0.0f, 1.0f, 0.0f ) );
			pCell->SetWallAttribute( j, FindNearestAttribute( vWallMidPoint, vWallNormal ) );
		}
		CGlobalValue::GetInstance().UpdateProgress( nSize, i );
	}
}

void CTEtWorldSector::GenerationNavigationMesh( bool bGeneration )
{
	int i, j;

	SAFE_DELETE( m_pNavCell );
	SAFE_DELETE( m_pNavMesh );

	if( !bGeneration ) return;
	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;

	int nVertex, nFace, nEvent;

	m_pNavCell = new CEtNavigationMeshGenerator;
	m_pNavCell->Initialize( nWidthCount, nHeightCount, m_pAttribute );
	m_pNavCell->Generate( nVertex, nFace );
	nEvent = 0;

	m_pNavMesh = new NavigationMesh;

	std::vector< int > vecFaceIndex;
	for( i = 0; i < m_pNavCell->getNumNavEvent(); i++ )
	{
		int nFaceCount;
		int *pFaceBegin = m_pNavCell->getEventFaceBegin( i, nFaceCount );
		for( j = 0; j < nFaceCount; j++ )
		{
			vecFaceIndex.push_back( pFaceBegin[ j * 3 ] );
			vecFaceIndex.push_back( pFaceBegin[ j * 3 + 1 ] );
			vecFaceIndex.push_back( pFaceBegin[ j * 3 + 2 ] );
		}
	}

	m_pNavMesh->CreateNaviCells( nFace + ( int )vecFaceIndex.size() / 3 );

	EtVector3 vPos[3], vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;
	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;
	for( i=0; i<m_pNavCell->getNumNavFaceNormal() * 3; i+=3 ) {
		memcpy( &vPos[0].x, &m_pNavCell->getVertexBegin()[m_pNavCell->getFaceBegin()[i]].x, sizeof(float) * 3 );
		memcpy( &vPos[1].x, &m_pNavCell->getVertexBegin()[m_pNavCell->getFaceBegin()[i+1]].x, sizeof(float) * 3 );
		memcpy( &vPos[2].x, &m_pNavCell->getVertexBegin()[m_pNavCell->getFaceBegin()[i+2]].x, sizeof(float) * 3 );

		for( int j=0; j<3; j++ ) {
			vPos[j].y = GetHeight( vPos[j].x, vPos[j].z );
			vPos[j] += vOffset;
		}
		m_pNavMesh->AddCell( vPos[0], vPos[1], vPos[2], i / 3 );
	}

	for( i = 0; i < ( int )vecFaceIndex.size(); i += 3 )
	{
		memcpy( &vPos[0].x, &m_pNavCell->getVertexBegin()[ vecFaceIndex[ i ] ].x, sizeof(float) * 3 );
		memcpy( &vPos[1].x, &m_pNavCell->getVertexBegin()[ vecFaceIndex[ i + 1 ] ].x, sizeof(float) * 3 );
		memcpy( &vPos[2].x, &m_pNavCell->getVertexBegin()[ vecFaceIndex[ i + 2 ] ].x, sizeof(float) * 3 );

		for( int j=0; j<3; j++ ) {
			vPos[j].y = GetHeight( vPos[j].x, vPos[j].z );
			vPos[j] += vOffset;
		}
		m_pNavMesh->AddCell( vPos[0], vPos[1], vPos[2], m_pNavCell->getNumNavFaceNormal() + i / 3 );
		m_pNavMesh->Cell( m_pNavCell->getNumNavFaceNormal() + i / 3 )->SetType( NavigationCell::CT_EVENT );
	}

	m_pNavMesh->LinkCells();
	m_pNavMesh->RemoveAcuteCell();
	AddPropNavMesh();
	BuildNavCellSideAttribute();
}

void CTEtWorldSector::DrawNavigationMesh()
{
	if( !m_pNavMesh ) return;

	EtVector3 vPos[3];
	for( int i = 0; i < m_pNavMesh->TotalCells(); i++ )
	{
		NavigationCell *pCell = m_pNavMesh->Cell( i );
		memcpy( &vPos[0].x, &pCell->Vertex( 0 ), sizeof(float) * 3 );
		memcpy( &vPos[1].x, &pCell->Vertex( 1 ), sizeof(float) * 3 );
		memcpy( &vPos[2].x, &pCell->Vertex( 2 ), sizeof(float) * 3 );

		vPos[0].y += 20.0f;
		vPos[1].y += 20.0f;
		vPos[2].y += 20.0f;

		DWORD dwColor;
		dwColor = 0xffffffff;
		if( pCell->GetType() == NavigationCell::CT_PROP )
		{
			dwColor = 0xffff0000;
		}
		else if( pCell->GetType() == NavigationCell::CT_EVENT )
		{
			dwColor = 0xffff00ff;
		}

		int j;
		for( j = 0; j < 3; j++ )
		{
/*			int nWallAttribute;
			nWallAttribute = pCell->GetWallAttribute( j );
			switch( nWallAttribute & 0xf )
			{
				case 0x1: 
					dwColor = 0xffEE2222;
					break;
				case 0x2:
					dwColor = 0xff2222EE;
					break;
				case 0x4:
					dwColor = 0xff22EE22;
					break;
				case 0x8:
					dwColor = 0xffEEEE11;
					break;
				default:
					dwColor = 0xffffffff;
					break;
			}*/
			EternityEngine::DrawLine3D( vPos[ j ], vPos[ ( j + 1 ) % 3 ], dwColor );
		}
	}
}

int CTEtWorldSector::GetNavigationFaceCount()
{
	if( !IsGenerationNavigationMesh() ) return -1;
	return m_pNavCell->getNumNavFaceNormal();
}

int CTEtWorldSector::GetPropClassID( const char *szFileName )
{
	if( strcmp( szFileName, "Light.skn" ) == NULL ) return 4;
	if( s_pPropSOX == NULL ) return 0;
	int nIndex = s_pPropSOX->GetItemIDFromField( "_Name", szFileName );
	if( nIndex == -1 ) return 0;
	return s_pPropSOX->GetFieldFromLablePtr( nIndex, "_ClassID" )->GetInteger();
}

void CTEtWorldSector::SetIgnorePropFolderList( std::vector<std::string> & vecIgnorePropFolder )
{
	m_vecIgnorePropFolder.clear();
	for( int i = 0; i < (int)vecIgnorePropFolder.size(); ++i )
		m_vecIgnorePropFolder.push_back( vecIgnorePropFolder[i] );
}

int CTEtWorldSector::GetPropIndexFromPtr( CEtWorldProp *pProp )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		if( m_pVecPropList[i] == pProp ) return i;
	}
	return -1;
}

CEtTrigger *CTEtWorldSector::AllocTrigger()
{
	return new CTEtTrigger( this );
}

CString CTEtWorldSector::GetUniqueCategoryName()
{
	CString szTemp;

	int nCount = 0;
	while(1) {
		szTemp.Format( "New Category - %03d", nCount );
		if( ((CTEtTrigger*)GetTrigger())->IsExistCategory( szTemp ) == false ) break;
		nCount++;
	}
	return szTemp;
}

CString CTEtWorldSector::GetUniqueTriggerName()
{
	CString szTemp;

	int nCount = 0;
	while(1) {
		szTemp.Format( "New Trigger - %03d", nCount );
		if( ((CTEtTrigger*)GetTrigger())->GetTriggerFromName( szTemp ) == NULL ) break;
		nCount++;
	}
	return szTemp;
}


CString CTEtWorldSector::GetEventControlNameFromCreateUniqueID( int nValue )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		for( DWORD j=0; j<m_pVecAreaControl[i]->GetAreaCount(); j++ ) {
			if( m_pVecAreaControl[i]->GetAreaFromIndex(j)->GetCreateUniqueID() == nValue ) return ((CTEtWorldEventControl*)m_pVecAreaControl[i])->GetName();
		}
	}
	return CString("");
}

bool CTEtWorldSector::MakeMinimap( int nWidth, int nHeight, const char *szFileName, const char *szEnviName, bool bRenderTerrain, bool bRenderProp, bool bRenderAttribute )
{
	CString szActiveName;
	if( CEnviControl::GetInstance().GetActiveElement() )
		szActiveName = CEnviControl::GetInstance().GetActiveElement()->GetInfo().GetName();
	CEnviControl::GetInstance().ActivateElement( szEnviName );
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();

	float fSizeX = m_pParentGrid->GetGridWidth() * 100.f;
	float fSizeY = m_pParentGrid->GetGridHeight() * 100.f;

	EtCameraHandle CamHandle;
	SCameraInfo CamInfo;
	CamInfo.fWidth = (float)nWidth;
	CamInfo.fHeight = (float)nHeight;
	CamInfo.fViewWidth = fSizeX;
	CamInfo.fViewHeight = fSizeY;
	CamInfo.Type = CT_ORTHOGONAL;
	CamInfo.Target = CT_RENDERTARGET;
	CamInfo.fFar = 1000000.f;
	CamInfo.fFogFar = 1000000.f;
	CamInfo.fFogNear = 1000000.f;
	CamInfo.FogColor = pElement->GetInfo().GetFogColor();
	CamHandle = EternityEngine::CreateCamera( &CamInfo );
	MatrixEx Cross;

	EtVector3 vCross;
	Cross.RotatePitch( 90.f );
	Cross.m_vPosition = *GetOffset();
	Cross.m_vPosition.y = min( fSizeX, fSizeY ) * 1.2f;

	CamHandle->Update( Cross );
	if( CRenderBase::s_bRenderMainScene == true ) {
		SAFE_RELEASE_SPTR( CamHandle );
		if( szActiveName ) CEnviControl::GetInstance().ActivateElement( szActiveName );
		return false;
	}
	if( bRenderProp ) {
		for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
			CamHandle->AddRenderSkin( m_pVecPropList[i]->GetPropName(), *((CTEtWorldProp*)m_pVecPropList[i])->GetMatEx() );
		}
	}
	if( bRenderTerrain ) {
		CamHandle->Render();
	}
	else CamHandle->RenderSkinList();

	D3DXIMAGE_FILEFORMAT fmt;
	char szExt[256] = { 0, };
	_GetExt( szExt, _countof(szExt), szFileName );

	if( _stricmp( szExt, "bmp" ) == NULL ) fmt = D3DXIFF_BMP;
	else if( _stricmp( szExt, "tga" ) == NULL ) fmt = D3DXIFF_TGA;
	else if( _stricmp( szExt, "jpg" ) == NULL ) fmt = D3DXIFF_JPG;
	else if( _stricmp( szExt, "png" ) == NULL ) fmt = D3DXIFF_PNG;
	else if( _stricmp( szExt, "dds" ) == NULL ) fmt = D3DXIFF_DDS;

	EtTextureHandle TextureHandle = CamHandle->GetRenderTargetTexture();
	LPD3DXBUFFER pBuffer = NULL;

	TextureHandle->ChangeFormat( FMT_A8R8G8B8, USAGE_DYNAMIC, POOL_DEFAULT );

	bool bLoadAttribute = false;
	if( !( m_nSectorRWLevel & SRL_ATTRIBUTE ) ) {
		LoadSector( SRL_ATTRIBUTE );
//		AttributeInfoFile( IFT_READ );
		bLoadAttribute = true;
	}

	int nStride;
	DWORD *pPtr = (DWORD*)TextureHandle->Lock( nStride, true );

	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;

	float fAttrX = nWidthCount / (float)nWidth;
	float fAttrY = nHeightCount / (float)nHeight;
	char cAttr;
	for( int j=0; j<nHeight; j++ ) {
		for( int i=0; i<nWidth; i++ ) {
			cAttr = m_pAttribute[ (int)( (int)((nHeight-j)*fAttrY) * nWidthCount ) + (int)(i*fAttrX) ];
			*pPtr &= 0x00ffffff;
			if( cAttr == 0 ) *pPtr |= 0xff000000;
			else *pPtr |= 0x44000000;
			pPtr++;
		}
	}

	TextureHandle->Unlock();

	if( bLoadAttribute ) {
		FreeSector( SRL_ATTRIBUTE );
	}
	EtBaseTexture *pEtTexture = TextureHandle->GetTexturePtr();
	D3DXSaveTextureToFile( szFileName, fmt, pEtTexture, NULL );

	SAFE_RELEASE_SPTR( CamHandle );
	if( szActiveName ) CEnviControl::GetInstance().ActivateElement( szActiveName );

	return true;
}

CEtWorldWater *CTEtWorldSector::AllocWater()
{
	return new CTEtWorldWater( this );
}

CEtWorldDecal *CTEtWorldSector::AllocDecal()
{
	return new CTEtWorldDecal( this );
}

CEtWorldDecal *CTEtWorldSector::AddDecal( int nX, int nY, float fRadius, float fRotate, const char *szTexName, EtColor vColor, float fAlpha )
{
	if( !m_Handle ) return NULL;
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return NULL;

	EtVector3 Position, Direction;
	CRenderBase::GetInstance().GetCameraHandle()->CalcPositionAndDir( nX, nY, Position, Direction );
	
	float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Direction );
	float fAngle = fRotate;

	CTEtWorldDecal *pDecal = (CTEtWorldDecal *)AllocDecal();
	if( pDecal->Initialize( EtVec3toVec2( vPickPos ), fRadius / 2.f, fAngle, vColor, fAlpha, szTexName ) == false ) {
		SAFE_DELETE( pDecal );
		return NULL;
	}
	
	InsertDecal( pDecal );
	return pDecal;
}

void CTEtWorldSector::UpdateDecal( CEtWorldDecal *pDecal )
{
	SCircle Circle;
	pDecal->GetBoundingCircle( Circle );

	pDecal->SetCurQuadtreeNode( m_pDecalQuadtree->Update( pDecal, Circle, pDecal->GetCurQuadtreeNode() ) );
}

int CTEtWorldSector::ScanDecal( int nX, int nY, float fRadius, std::vector<CEtWorldDecal *> &pVecResult )
{
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return 0;
	m_vPickPos = vPickPos;

	CEtWorldSector::ScanDecal( EtVec3toVec2( vPickPos ), fRadius, &pVecResult );
	return (int)pVecResult.size();
}

void CTEtWorldSector::SetDecalTexture( const char *szFileName )
{
	char szBuf[_MAX_PATH];
	_GetFullFileName( szBuf, _countof(szBuf), szFileName );
	m_szDecalTextureName = szBuf;
}

const char *CTEtWorldSector::GetDecalTextureName()
{
	return m_szDecalTextureName.GetBuffer();
}

CTEtWorldWaterRiver *CTEtWorldSector::ApplyRiver( int nX, int nY )
{
	EtVector3 vPickPos;
	if( m_Handle->Pick( nX, nY, vPickPos, ( CRenderBase::IsActive() && CRenderBase::GetInstance().GetCameraHandle() ) ? CRenderBase::GetInstance().GetCameraHandle()->GetMyItemIndex() : 0 ) == false ) return NULL;

	for( DWORD i=0; i<CGlobalValue::GetInstance().m_pVecSelectPointList.size(); i++ ) {
		CGlobalValue::GetInstance().m_pVecSelectPointList[i]->bSelect = false;
		CGlobalValue::GetInstance().m_pVecSelectPointList[i]->pAxis->Show( false );
	}

//	vPickPos.y += 100.f;

	CTEtWorldWaterRiver *pRiver = ((CTEtWorldWater*)m_pWater)->AddRiver( vPickPos );

	CTEtWorldWaterRiver::RiverPoint *pPoint = pRiver->InsertPoint( vPickPos );
	CGlobalValue::GetInstance().m_pVecSelectPointList.clear();
	pPoint->bSelect = true;
	pPoint->pAxis->Show( true );
	CGlobalValue::GetInstance().m_pVecSelectPointList.push_back( pPoint );
	CPointAxisRenderObject::m_nSelectAxis = 3;
	CGlobalValue::GetInstance().m_nSelectAxis = 3;
	CGlobalValue::GetInstance().m_bControlAxis = true;

	CGlobalValue::GetInstance().m_pVecSelectRiverList.clear();
	CGlobalValue::GetInstance().m_pVecSelectRiverList.push_back( pRiver );

	return pRiver;

}

CTEtWorldWaterRiver::RiverPoint *CTEtWorldSector::AddRiverPoint( CTEtWorldWaterRiver *pRiver, CTEtWorldWaterRiver::RiverPoint *pClonePoint )
{
	CTEtWorldWaterRiver::RiverPoint *pPoint = pRiver->InsertPoint( pClonePoint->vPos, &pClonePoint->vPos );
	pPoint->bSelect = true;
	pPoint->pAxis->Show( true );
	return pPoint;
}

void CTEtWorldSector::DeleteRiverPoint( CTEtWorldWaterRiver::RiverPoint *pPoint )
{
	((CTEtWorldWater*)m_pWater)->RemoveRiverPoint( pPoint );
}

bool CTEtWorldSector::IsReadOnlyFile( const char *szFileName )
{
	if( PathFileExists( szFileName ) == FALSE ) return false;
	DWORD dwAttr = GetFileAttributes( szFileName );
	if( dwAttr & FILE_ATTRIBUTE_READONLY ) return true;
	return false;
}

bool CTEtWorldSector::ExportTerrainToMaxScript( const char *szFileName )
{
	FILE *fp;
	char szTemp[256] = { 0, };
	char szName[256] = { 0, };
	char szPath[256] = { 0, };
	char szExt[32] = { 0, };
	_GetExt( szExt, _countof(szExt), szFileName );
	_GetFileName( szName, _countof(szName), szFileName );
	_GetPath( szPath, _countof(szPath), szFileName );
	sprintf_s( szTemp, "%s.msd", szName, szExt );

	fopen_s( &fp, szFileName, "wt" );
	if( fp == NULL ) return false;

	fprintf_s( fp, "fp = openfile \"%s\" mode:\"rb\"\n", szTemp );
	fprintf_s( fp, "HeightXSize = %d\n", GetTileWidthCount() - 1 );
	fprintf_s( fp, "HeightYSize = %d\n", GetTileHeightCount() - 1 );
	fprintf_s( fp, "PlaneWidth = %d\n", (int)GetTileSize() * ( GetTileWidthCount() -1 ) );
	fprintf_s( fp, "PlaneHeight = %d\n", (int)GetTileSize() * ( GetTileHeightCount() - 1 ) );
	fprintf_s( fp, "Plane length:PlaneHeight width:PlaneWidth pos:[ 0, 0, 0 ] isSelected:on lengthsegs:HeightYSize widthsegs:HeightXSize\n" );
	fprintf_s( fp, "convertTo $ TriMeshGeometry\n" );

//	sprintf_s( szTemp, "%s.tga", szName, szExt );
//	fprintf_s( fp, "$.Material = standard diffuseMap:(bitmap(\"%s\")) showInViewport:true\n\n", szTemp );

	int nCount = GetTileWidthCount() * GetTileHeightCount();

	fprintf_s( fp, "for i = 1 to %d do\n", nCount );
	fprintf_s( fp, "(\n", nCount );
	fprintf_s( fp, "	Height = readValue fp\n" );
	fprintf_s( fp, "	$.verts[i].pos.z = Height\n" );
	fprintf_s( fp, ")\n", nCount );
	fprintf_s( fp, "close fp\n", nCount );

	fclose(fp);

	sprintf_s( szTemp, "%s%s.msd", szPath, szName );
	fopen_s( &fp, szTemp, "wt" );
	if( fp == NULL ) return false;

	for( int i=0; i<nCount; i++ ) {
		fprintf_s( fp, "%.2f\n", m_pHeight[i] * GetHeightMultiply() );
	}

	fclose(fp);

	sprintf_s( szTemp, "%s%s.tga", szPath, szName );
	MakeMinimap( 512, 512, szTemp, "Edit Set", true, false, false );
	return true;
}

bool CTEtWorldSector::ExportPropInfoToMaxScript( const char *szFileName, const char *szMaxFolder )
{
	FILE *fp;
	char szTemp[256] = { 0, };
	char szTemp3[256] = { 0, };
	char szName[256] = { 0, };
	char szPath[256] = { 0, };
	char szExt[32] = { 0, };
	_GetExt( szExt, _countof(szExt), szFileName );
	_GetFileName( szName, _countof(szName), szFileName );
	_GetPath( szPath, _countof(szPath), szFileName );

	sprintf_s( szTemp3, "%s.msd", szName, szExt );

	sprintf_s( szTemp, "%s%s.msd", szPath, szName );
	fopen_s( &fp, szTemp, "wt" );
	if( fp == NULL ) return false;

	int nCount = 0;
	char szTemp2[256] = { 0, };
	CTEtWorldProp *pProp;
	for( int i=0; i<(int)GetPropCount(); i++ ) {
		pProp = (CTEtWorldProp*)m_pVecPropList[i];
		_GetFileName( szName, _countof(szName), pProp->GetPropName() );
		sprintf_s( szTemp2, "%s.max", szName );
		memset( szTemp, 0, sizeof(szTemp) );
		if( FindFileInDirectory( szMaxFolder, szTemp2, szTemp ) == 0 ) continue;

		fprintf_s( fp, "%s|%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", szTemp, pProp->GetMatEx()->m_vPosition.x, pProp->GetMatEx()->m_vPosition.y, pProp->GetMatEx()->m_vPosition.z, pProp->GetRotation()->x, pProp->GetRotation()->y, pProp->GetRotation()->z, pProp->GetScale()->x, pProp->GetScale()->y, pProp->GetScale()->z );
		nCount++;
	}
	fclose(fp);

	fopen_s( &fp, szFileName, "wt" );
	if( fp == NULL ) return false;

	fprintf_s( fp, "fp = openfile \"%s\" mode:\"rb\"\n", szTemp3 );
	fprintf_s( fp, "for i = 1 to %d do\n", nCount );
	fprintf_s( fp, "(\n" );
	fprintf_s( fp, "	PropName = readDelimitedString  fp \"|\"\n" );
	fprintf_s( fp, "	PropX = readValue fp\n" );
	fprintf_s( fp, "	PropY = readValue fp\n" );
	fprintf_s( fp, "	PropZ = readValue fp\n" );
	fprintf_s( fp, "	PropRotateX = readValue fp\n" );
	fprintf_s( fp, "	PropRotateY = readValue fp\n" );
	fprintf_s( fp, "	PropRotateZ = readValue fp\n" );
	fprintf_s( fp, "	PropScaleX = readValue fp\n" );
	fprintf_s( fp, "	PropScaleY = readValue fp\n" );
	fprintf_s( fp, "	PropScaleZ = readValue fp\n" );
	fprintf_s( fp, "	Ret = mergeMAXFile PropName #select #mergeDups #useMergedMtlDups\n" );
	fprintf_s( fp, "	if Ret == true then\n" );
	fprintf_s( fp, "	(\n" );
	fprintf_s( fp, "		rotate $ ( angleaxis PropRotateX [ 1, 0, 0 ] )\n" );
	fprintf_s( fp, "		rotate $ ( angleaxis PropRotateZ [ 0, 1, 0 ] )\n" );
	fprintf_s( fp, "		rotate $ ( angleaxis PropRotateY [ 0, 0, 1 ] )\n" );
	fprintf_s( fp, "		if $ == selection then\n" );
	fprintf_s( fp, "		(\n" );
	fprintf_s( fp, "			for j = 1 to $.count do\n" );
	fprintf_s( fp, "			(\n" );
	fprintf_s( fp, "				if isProperty selection[ j ] #name == true then\n" );
	fprintf_s( fp, "				(\n" );
	fprintf_s( fp, "					if findString selection[ j ].name \"Box\" == 1 then\n" );
	fprintf_s( fp, "					(\n" );
	fprintf_s( fp, "						delete selection[ j ]\n" );
	fprintf_s( fp, "						j--\n" );
	fprintf_s( fp, "						continue\n" );
	fprintf_s( fp, "					)\n" );
	fprintf_s( fp, "					if findString selection[ j ].name \"Capsule\" == 1 then\n" );
	fprintf_s( fp, "					(\n" );
	fprintf_s( fp, "						delete selection[ j ]\n" );
	fprintf_s( fp, "						j--\n" );
	fprintf_s( fp, "						continue\n" );
	fprintf_s( fp, "					)\n" );
	fprintf_s( fp, "					if findString selection[ j ].name \"Shpere\" == 1 then\n" );
	fprintf_s( fp, "					(\n" );
	fprintf_s( fp, "						delete selection[ j ]\n" );
	fprintf_s( fp, "						j--\n" );
	fprintf_s( fp, "						continue\n" );
	fprintf_s( fp, "					)\n" );
	fprintf_s( fp, "					if findString selection[ j ].name \"@\" == 1 then\n" );
	fprintf_s( fp, "					(\n" );
	fprintf_s( fp, "						delete selection[ j ]\n" );
	fprintf_s( fp, "						j--\n" );
	fprintf_s( fp, "						continue\n" );
	fprintf_s( fp, "					)\n" );
	fprintf_s( fp, "				)\n" );
	fprintf_s( fp, "				if isProperty selection[ j ] #pos == true then\n" );
	fprintf_s( fp, "				(\n" );
	fprintf_s( fp, "					selection[ j ].pos.x = PropX\n" );
	fprintf_s( fp, "					selection[ j ].pos.y = PropZ\n" );
	fprintf_s( fp, "					selection[ j ].pos.z = PropY\n" );
	fprintf_s( fp, "				)\n" );
	fprintf_s( fp, "				if isProperty selection[ j ] #scale == true then\n" );
	fprintf_s( fp, "				(\n" );
	fprintf_s( fp, "					selection[ j ].scale.x = selection[ j ].scale.x * PropScaleX\n" );
	fprintf_s( fp, "					selection[ j ].scale.y = selection[ j ].scale.y * PropScaleZ\n" );
	fprintf_s( fp, "					selection[ j ].scale.z = selection[ j ].scale.z * PropScaleY\n" );
	fprintf_s( fp, "				)\n" );
	fprintf_s( fp, "			)\n" );
	fprintf_s( fp, "		)\n" );
	fprintf_s( fp, "		else\n" );
	fprintf_s( fp, "		(\n" );
	fprintf_s( fp, "			if isProperty $ #pos == true then\n" );
	fprintf_s( fp, "			(\n" );
	fprintf_s( fp, "				$.pos.x = PropX\n" );
	fprintf_s( fp, "				$.pos.y = PropZ\n" );
	fprintf_s( fp, "				$.pos.z = PropY\n" );
	fprintf_s( fp, "			)\n" );
	fprintf_s( fp, "			if isProperty $ #scale == true then\n" );
	fprintf_s( fp, "			(\n" );
	fprintf_s( fp, "				$.scale.x = $.scale.x*PropScaleX\n" );
	fprintf_s( fp, "				$.scale.y = $.scale.x*PropScaleY\n" );
	fprintf_s( fp, "				$.scale.z = $.scale.x*PropScaleZ\n" );
	fprintf_s( fp, "			)\n" );
	fprintf_s( fp, "		)\n" );
	fprintf_s( fp, "	)\n" );		
	fprintf_s( fp, ")\n" );
	fprintf_s( fp, "close fp\n" );

	fclose(fp);


	return true;
}

void CTEtWorldSector::CalcSectorSize( float &fHeightCenter, float &fSize )
{
	float fMaxSize = 0.f;
	float fWidthSize = (float)( max( GetParentGrid()->GetGridWidth() * 100.f, GetParentGrid()->GetGridHeight() * 100.f ) );
	float fHeightTemp[2] = { 0.f, 0.f };


	int nCount = GetTileWidthCount() * GetTileHeightCount();
	int nHeightSize[2] = { 0, 0 };
	for( int i=0; i<nCount; i++ ) {
		if( i == 0 ) {
			nHeightSize[0] = nHeightSize[1] = m_pHeight[i];
		}
		if( nHeightSize[0] > m_pHeight[i] ) nHeightSize[0] = m_pHeight[i];
		if( nHeightSize[1] < m_pHeight[i] ) nHeightSize[1] = m_pHeight[i];
	}

	fHeightTemp[0] = ( nHeightSize[0] * m_fHeightMultiply );
	fHeightTemp[1] = ( nHeightSize[1] * m_fHeightMultiply );

	bool bLoadPropInfo = false;
	if( !( m_nSectorRWLevel & SRL_PROP ) ) {
		bLoadPropInfo = true;
		LoadSector( SRL_PROP );
	}

	SAABox Box;
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		CEtWorldProp *pProp = m_pVecPropList[i];
		pProp->GetBoundingBox( Box );

		if( fHeightTemp[0] > Box.Min.y ) fHeightTemp[0] = Box.Min.y;
		if( fHeightTemp[1] < Box.Max.y ) fHeightTemp[1] = Box.Max.y;
	}
	fHeightTemp[0] -= 1000.f;
	fHeightTemp[1] += 1000.f;

	if( bLoadPropInfo ) {
		FreeSector( SRL_PROP );
	}

	fSize = max( ( fHeightTemp[1] - fHeightTemp[0] ), fWidthSize );
	fHeightCenter = fHeightTemp[0] + ( ( fHeightTemp[1] - fHeightTemp[0] ) / 2.f );
}


bool CTEtWorldSector::LoadSectorSize( const char *szSectorPath )
{
	return false;
}

void CTEtWorldSector::LoadNavMesh()
{
	int i;

	for( i = 0; i < ( int )GetPropCount(); i++ )
	{
		CTEtWorldProp *pProp = ( CTEtWorldProp * )m_pVecPropList[ i ];
		if( pProp )
		{
			pProp->LoadNavMesh();
		}
	}

}


//////////////////////////////////////////////////////////////////////////
CTEtWorldSector::PROP_NAME_LIST CTEtWorldSector::ms_PropFilteringList;
bool CTEtWorldSector::ms_IsFiltering = false;
bool CTEtWorldSector::FindFilteringPropName(const char* szPropName)
{
	PROP_NAME_LIST::iterator iter = ms_PropFilteringList.begin();
	for (; iter != ms_PropFilteringList.end(); ++iter)
	{
		//같은 녀석이 이미 있으면 추가 안됨..
		if (0 == _strcmpi(szPropName, (*iter).c_str()))
			return true;
	}

	return false;
}

void CTEtWorldSector::AddPropNameForFiltering(const char* szPropName)
{
	if (NULL == szPropName || strlen(szPropName) < 1)
		return;

	if (FindFilteringPropName(szPropName))
		return;

	ms_PropFilteringList.push_back(szPropName);
}

void CTEtWorldSector::RemovePropNameForFiltering(const char* szPropName)
{
	PROP_NAME_LIST::iterator iter = ms_PropFilteringList.begin();
	for (; iter != ms_PropFilteringList.end(); )
	{
		//같은 녀석을 찾아서 지움
		if (0 == _strcmpi(szPropName, (*iter).c_str()))
		{
			iter = ms_PropFilteringList.erase(iter);
			continue;
		}
		
		++iter;
	}
}

void CTEtWorldSector::InitFilteringNames()
{
	ms_PropFilteringList.clear();
}
//////////////////////////////////////////////////////////////////////////