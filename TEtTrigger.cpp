#include "StdAfx.h"
#include "TEtTrigger.h"
#include "TEtTriggerObject.h"
#include "SundriesFuncEx.h"

CTEtTrigger::CTEtTrigger( CEtWorldSector *pSector )
: CEtTrigger( pSector )
{
}

CTEtTrigger::~CTEtTrigger()
{
}

bool CTEtTrigger::Load( const char *szFileName )
{
	bool bResult = CEtTrigger::Load( szFileName );
	
#ifdef PRE_FIX_62845
	if( CEtTrigger::IsDetectLuaError() )
	{
		CEtTrigger::ShowErrLuaName();
	}
#endif

	if( bResult == false ) return false;

	return true;
}

bool CTEtTrigger::Save( const char *szFileName )
{
	FILE *fp;
	fopen_s( &fp, szFileName, "wb" );
	if( fp == NULL ) return false;

	DWORD dwCount = GetTriggerCount();
	fwrite( &dwCount, sizeof(DWORD), 1, fp );

	for( DWORD i=0; i<dwCount; i++ ) {
		CTEtTriggerObject *pObject = (CTEtTriggerObject *)GetTriggerFromIndex(i);
		if( pObject->Save( fp ) == false ) {
			fclose(fp);
			return false;
		}
	}

	fclose( fp );
	return true;
}

bool CTEtTrigger::SaveDefine( const char *szFileName )
{
	FILE *fp;
	fopen_s( &fp, szFileName, "wb" );
	if( fp == NULL ) return false;

	DWORD dwCount = GetDefineValueCount();
	fwrite( &dwCount, sizeof(DWORD), 1, fp );

	CString szStr;
	for( DWORD i=0; i<dwCount; i++ ) {
		DefineValue *pValue = GetDefineValueFromIndex(i);

		fwrite( &pValue->nUniqueID, sizeof(int), 1, fp );

		szStr = pValue->szName.c_str();
		WriteCString( &szStr, fp );
		fwrite( &pValue->ParamType, sizeof(int), 1, fp );

		switch( pValue->ParamType ) {
			case CEtTrigger::Integer:
				fwrite( &pValue->nValue, sizeof(int), 1, fp );
				break;
			case CEtTrigger::Float:
				fwrite( &pValue->fValue, sizeof(float), 1, fp );
				break;
			case CEtTrigger::String:
				{
					szStr = pValue->szValue;
					WriteCString( &szStr, fp );
				}
				break;
		}
	}

	fclose( fp );
	return true;
}

CEtTriggerObject *CTEtTrigger::AllocTriggerObject()
{
	return new CTEtTriggerObject( this );
}

bool CTEtTrigger::IsExistCategory( const char *szName )
{
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( strcmp( m_pVecTriggerList[i]->GetCategory(), szName ) == NULL ) return true;
	}
	return false;
}

void CTEtTrigger::InsertTrigger( const char *szCategory, const char *szTriggerName )
{
	CTEtTriggerObject *pTrigger = (CTEtTriggerObject *)AllocTriggerObject();
	pTrigger->SetCategory( szCategory );
	pTrigger->SetTriggerName( szTriggerName );
	m_pVecTriggerList.push_back( pTrigger );
}

void CTEtTrigger::InsertTrigger( CEtTriggerObject *pObject )
{
	m_pVecTriggerList.push_back( pObject );
}

void CTEtTrigger::RemoveTrigger( const char *szCategory )
{
	std::vector<std::string> szVecTriggerList;
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( strcmp( m_pVecTriggerList[i]->GetCategory(), szCategory ) == NULL ) {
			szVecTriggerList.push_back( m_pVecTriggerList[i]->GetTriggerName() );
		}
	}
	for( DWORD i=0; i<szVecTriggerList.size(); i++ ) {
		RemoveTrigger( szCategory, szVecTriggerList[i].c_str() );
	}
}

void CTEtTrigger::RemoveTrigger( const char *szCategory, const char *szTriggerName )
{
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( strcmp( m_pVecTriggerList[i]->GetCategory(), szCategory ) == NULL &&
			strcmp( m_pVecTriggerList[i]->GetTriggerName(), szTriggerName ) == NULL ) {
			SAFE_DELETE( m_pVecTriggerList[i] );
			m_pVecTriggerList.erase( m_pVecTriggerList.begin() + i );
			return;
		}
	}
}

void CTEtTrigger::ChangeCategoryName( const char *szOldName, const char *szNewName )
{
	std::vector<DWORD> dwVecIndex;
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( strcmp( m_pVecTriggerList[i]->GetCategory(), szOldName ) == NULL ) {
			dwVecIndex.push_back( i );
		}
	}
	for( DWORD i=0; i<dwVecIndex.size(); i++ ) {
		((CTEtTriggerObject*)GetTriggerFromIndex( dwVecIndex[i] ))->SetCategory( szNewName );
	}
}

void CTEtTrigger::ChangeTriggerOrder( CTEtTriggerObject *pObject, CTEtTriggerObject *pTarget )
{
	bool bValid = false;
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( m_pVecTriggerList[i] == pObject ) {
			m_pVecTriggerList.erase( m_pVecTriggerList.begin() + i );
			bValid = true;
			break;
		}
	}
	if( !bValid ) return;
	if( pTarget == NULL ) {
		m_pVecTriggerList.insert( m_pVecTriggerList.begin(), pObject );
	}
	else {
		for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
			if( m_pVecTriggerList[i] == pTarget ) {
				m_pVecTriggerList.insert( m_pVecTriggerList.begin() + i + 1, pObject );
				break;
			}
		}
	}
}

void CTEtTrigger::ChangeTriggerOrder( CTEtTriggerObject *pObject, CString szCategory )
{
	if( _stricmp( pObject->GetCategory(), szCategory ) == NULL ) {
		int nCount = 0;
		for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
			if( _stricmp( m_pVecTriggerList[i]->GetCategory(), szCategory ) == NULL ) {
				nCount++;
			}
		}
		if( nCount == 1 ) return;
	}

	bool bValid = false;
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( m_pVecTriggerList[i] == pObject ) {
			m_pVecTriggerList.erase( m_pVecTriggerList.begin() + i );
			bValid = true;
			break;
		}
	}
	if( !bValid ) return;

	bValid = false;
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( _stricmp( m_pVecTriggerList[i]->GetCategory(), szCategory ) == NULL ) {
			m_pVecTriggerList.insert( m_pVecTriggerList.begin() + i, pObject );
			bValid = true;
			break;
		}
	}
	if( bValid == false ) {
		m_pVecTriggerList.push_back( pObject );
	}
}

DWORD CTEtTrigger::GetDefineValueCount()
{
	return (DWORD)m_pVecDefineValueList.size();
}

CEtTrigger::DefineValue *CTEtTrigger::GetDefineValueFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecDefineValueList.size() ) return NULL;

	return m_pVecDefineValueList[dwIndex];
}

CEtTrigger::DefineValue *CTEtTrigger::GetDefineValueFromName( const char *szValueName )
{
	for( DWORD i=0; m_pVecDefineValueList.size(); i++ ) {
		if( strcmp( m_pVecDefineValueList[i]->szName.c_str(), szValueName ) == NULL ) return m_pVecDefineValueList[i];
	}
	return NULL;
}

CEtTrigger::DefineValue *CTEtTrigger::GetDefineValueFromUniqueID( int nUniqueID )
{
	for( DWORD i=0; i<m_pVecDefineValueList.size(); i++ ) {
		if( m_pVecDefineValueList[i]->nUniqueID == nUniqueID ) {
			return m_pVecDefineValueList[i];
		}
	}
	return NULL;
}

CEtTrigger::DefineValue *CTEtTrigger::AddDefineValue( const char *szValueName )
{
	int nUniqueID = GetDefineValueUniqueID();
	if( nUniqueID == -1 ) return NULL;

	DefineValue *pStruct = new DefineValue;
	pStruct->nUniqueID = GetDefineValueUniqueID();
	pStruct->szName = szValueName;
	m_pVecDefineValueList.push_back( pStruct );
	return pStruct;
}

void CTEtTrigger::DeleteDefineValueFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecDefineValueList.size() ) return;

	SAFE_DELETE( m_pVecDefineValueList[dwIndex] );
	m_pVecDefineValueList.erase( m_pVecDefineValueList.begin() + dwIndex );
}

void CTEtTrigger::DeleteDefineValueFromName( const char *szValueName )
{
	for( DWORD i=0; i<m_pVecDefineValueList.size(); i++ ) {
		if( strcmp( m_pVecDefineValueList[i]->szName.c_str(), szValueName ) == NULL ) {
			SAFE_DELETE( m_pVecDefineValueList[i] );
			m_pVecDefineValueList.erase( m_pVecDefineValueList.begin() + i );
			return;
		}
	}
}

int CTEtTrigger::GetDefineValueUniqueID()
{
	for( int i=0; i<256; i++ ) {
		bool bValid = true;
		for( DWORD j=0; j<m_pVecDefineValueList.size(); j++ ) {
			if( m_pVecDefineValueList[j]->nUniqueID == i ) {
				bValid = false;
				break;
			}
		}
		if( bValid ) return i;
	}
	return -1;
}

void CTEtTrigger::ChangeCategoryOrder( CString szCategory, CString szTarget )
{
	std::vector<CEtTriggerObject *> pVecList;

	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( _stricmp( m_pVecTriggerList[i]->GetCategory(), szCategory ) == NULL ) {
			pVecList.push_back( m_pVecTriggerList[i] );
			m_pVecTriggerList.erase( m_pVecTriggerList.begin() + i );
			i--;
		}
	}
	if( pVecList.empty() ) return;

	if( szTarget.IsEmpty() ) {
		for( int i=(int)pVecList.size() - 1; i>=0; i-- ) {
			m_pVecTriggerList.insert( m_pVecTriggerList.begin(), pVecList[i] );
		}
	}
	else {
		for( int i=(int)m_pVecTriggerList.size() - 1; i>=0; i-- ) {
			if( _stricmp( m_pVecTriggerList[i]->GetCategory(), szTarget ) == NULL ) {
				for( int j=(int)pVecList.size() - 1; j>=0; j-- ) {
					m_pVecTriggerList.insert( m_pVecTriggerList.begin() + i + 1, pVecList[j] );
				}
				break;
			}
		}
	}
}

void CTEtTrigger::SortCategory( std::vector<CString> &szVecList )
{
	std::vector<CEtTriggerObject *> pVecList = m_pVecTriggerList;

	m_pVecTriggerList.clear();
	for( DWORD i=0; i<szVecList.size(); i++ ) {
		for( DWORD j=0; j<pVecList.size(); j++ ) {
			if( _stricmp( szVecList[i], pVecList[j]->GetCategory() ) == NULL ) {
				m_pVecTriggerList.push_back( pVecList[j] );
				pVecList.erase( pVecList.begin() + j );
				j--;
			}
		}
	}
}