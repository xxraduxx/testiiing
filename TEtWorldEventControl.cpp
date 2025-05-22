#include "StdAfx.h"
#include "TEtWorldEventControl.h"
#include "PropertyGridCreator.h"
#include "TEtWorldEventArea.h"
#include "SundriesFuncEx.h"
#include "EventSignalManager.h"
#include "EventSignalItem.h"
#include "TEtWorldSector.h"
#include <map>
using namespace std;

CTEtWorldEventControl::CTEtWorldEventControl( CEtWorldSector *pSector )
: CEtWorldEventControl( pSector )
{
}

CTEtWorldEventControl::~CTEtWorldEventControl()
{
}

bool CTEtWorldEventControl::Initialize()
{
	return CEtWorldEventControl::Initialize();
}

CEtWorldEventArea *CTEtWorldEventControl::AllocArea()
{
	CTEtWorldEventArea *pArea = new CTEtWorldEventArea( this, GetName() );
	
	int nIndex = ((CTEtWorldSector*)m_pSector)->AddEventAreaCreateUniqueCount();
	pArea->SetCreateUniqueID( nIndex );
	return pArea;
}

bool CTEtWorldEventControl::SaveControl( FILE *fp )
{
	/*
	FILE *fp;
	fopen_s( &fp, szFileName, "wb" );
	if( fp == NULL ) return false;

	int nCount = (int)GetAreaCount();
	fwrite( &nCount, sizeof(int), 1, fp );

	CString szName;
	char *pDummy = new char[CEtWorldEventArea::s_nDummySize];
	memset( pDummy, 0, CEtWorldEventArea::s_nDummySize );

	float fValue;
	int nValue;
	for( int i=0; i<nCount; i++ ) {
		CTEtWorldEventArea *pArea = (CTEtWorldEventArea *)GetAreaFromIndex(i);
		szName = pArea->GetName();
		WriteCString( &szName, fp );
		fwrite( pArea->GetMin(), sizeof(EtVector3), 1, fp );
		fwrite( pArea->GetMax(), sizeof(EtVector3), 1, fp );
		fValue = pArea->GetRotate();
		fwrite( &fValue, sizeof(float), 1, fp );
		nValue = pArea->GetCreateUniqueID();
		fwrite( &nValue, sizeof(int), 1, fp );
		fwrite( pDummy, CEtWorldEventArea::s_nDummySize, 1, fp );

		pArea->Save( fp );
	}
	SAFE_DELETEA( pDummy );
	fclose(fp);
	*/
	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( CString(GetName()) );
	if( pItem == NULL ) return false;
	int nUniqueID = pItem->GetUniqueIndex();

	fwrite( &nUniqueID, sizeof(int), 1, fp );
	int nCount = (int)GetAreaCount();
	fwrite( &nCount, sizeof(int), 1, fp );

	CString szName;
	char *pDummy = new char[CEtWorldEventArea::s_nDummySize];
	memset( pDummy, 0, CEtWorldEventArea::s_nDummySize );

	float fValue;
	int nValue;
	for( int i=0; i<nCount; i++ ) {
		CTEtWorldEventArea *pArea = (CTEtWorldEventArea *)GetAreaFromIndex(i);
		szName = pArea->GetName();
		WriteCString( &szName, fp );
		fwrite( pArea->GetMin(), sizeof(EtVector3), 1, fp );
		fwrite( pArea->GetMax(), sizeof(EtVector3), 1, fp );
		fValue = pArea->GetRotate();
		fwrite( &fValue, sizeof(float), 1, fp );
		nValue = pArea->GetCreateUniqueID();
		fwrite( &nValue, sizeof(int), 1, fp );
		fwrite( pDummy, CEtWorldEventArea::s_nDummySize, 1, fp );

		pArea->Save( fp );
	}
	SAFE_DELETEA( pDummy );


	return true;
}

bool CTEtWorldEventControl::LoadControl( FILE *fp )
{
	/*
	FILE *fp;
	fopen_s( &fp, szFileName, "rb" );
	if( fp == NULL ) return false;

	int nCount;
	fread( &nCount, sizeof(int), 1, fp );

	CString szName;
	EtVector3 vMin, vMax;
	float fRotate;
	int nValue;
	for( int i=0; i<nCount; i++ ) {
		CTEtWorldEventArea *pArea = (CTEtWorldEventArea *)AllocArea();
		ReadCString( &szName, fp );
		fread( &vMin, sizeof(EtVector3), 1, fp );
		fread( &vMax, sizeof(EtVector3), 1, fp );
		fread( &fRotate, sizeof(float), 1, fp );
		pArea->SetRotate( fRotate );
		fread( &nValue, sizeof(int), 1, fp );
		pArea->SetCreateUniqueID( nValue );
		fseek( fp, CEtWorldEventArea::s_nDummySize, SEEK_CUR );

		pArea->Load( fp );

		pArea->SetName( szName );
		pArea->SetMin( vMin );
		pArea->SetMax( vMax );
		m_pVecList.push_back( pArea );
//		InsertArea( vMin, vMax, szName );
	}
	fclose(fp);
	*/
	fread(&m_nUniqueID, sizeof(int), 1, fp);



	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromUniqueIndex( m_nUniqueID );
	if( pItem ) m_szName = pItem->GetName();

	int nCount;
	fread( &nCount, sizeof(int), 1, fp );

	char buff[128];
	wsprintf(buff, "m_nUniqueID = %d count = %d ", m_nUniqueID ,nCount);
	//MessageBoxA(NULL, buff, "CONTROL LOAD", MB_OK);

	CString szName;
	EtVector3 vMin, vMax;
	float fRotate;
	int nValue;
	for( int i=0; i<nCount; i++ ) {
		CTEtWorldEventArea *pArea = (CTEtWorldEventArea *)AllocArea();
		ReadCString( &szName, fp );
		fread( &vMin, sizeof(EtVector3), 1, fp );
		fread( &vMax, sizeof(EtVector3), 1, fp );
		fread( &fRotate, sizeof(float), 1, fp );
		pArea->SetRotate( fRotate );
		fread( &nValue, sizeof(int), 1, fp );
		pArea->SetCreateUniqueID( nValue );
		fseek( fp, CEtWorldEventArea::s_nDummySize, SEEK_CUR );

		pArea->Load( fp );

		pArea->SetName( szName );
		pArea->SetMin( vMin );
		pArea->SetMax( vMax );

		pArea->UpdatePropertyPositionInfo();
		pArea->UpdatePropertyRotationInfo();

		m_pVecList.push_back( pArea );
		m_nMapCreateUniqueIDSearch.insert( make_pair( pArea->GetCreateUniqueID(), pArea ) );
		((CTEtWorldEventArea*)pArea)->InitializeCustomRender();
	}
	if( pItem == NULL ) return false;

	return true;
}


CEtWorldEventArea *CTEtWorldEventControl::InsertArea( EtVector3 &vMin, EtVector3 &vMax, const char *szName, int nUniqueID )
{
	CTEtWorldEventArea *pArea = new CTEtWorldEventArea( this, GetName() );
	pArea->SetCreateUniqueID( nUniqueID );
	pArea->SetName( szName );
	pArea->SetMin( vMin );
	pArea->SetMax( vMax );

	pArea->UpdatePropertyPositionInfo();
	pArea->UpdatePropertyRotationInfo();

	m_pVecList.push_back( pArea );
	m_nMapCreateUniqueIDSearch.insert( make_pair( pArea->GetCreateUniqueID(), pArea ) );
	return pArea;
}