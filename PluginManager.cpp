#include "StdAfx.h"
#include "PluginManager.h"

CPluginManager::CPluginManager()
{
}

CPluginManager::~CPluginManager()
{
	Finalize();
}


bool CPluginManager::Initialize( const char *szPath )
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Plugins.." );
	std::vector<std::string> szVecList;
	FindFileListInDirectory( szPath, "*.dll", szVecList, false, true );

	HMODULE hModule;
	char szName[_MAX_PATH];
	int (*pGetPluginType)();

	for( DWORD i=0; i<szVecList.size(); i++ ) {
		sprintf_s( szName, "%s\\%s", szPath, szVecList[i].c_str() );
		hModule = LoadLibrary( szName );
		if( hModule == NULL ) continue;

		*( FARPROC * )&pGetPluginType = GetProcAddress( hModule, "GetPluginType" );
		if( pGetPluginType == NULL ) continue;
		int nType = pGetPluginType();

		switch( nType ) {
			case EVENTAREA_DLL: break;
			case ACTIONSIGNAL_DLL: continue;
		}
		DllStruct *pStruct = new DllStruct;

		pStruct->hModule = hModule;
		pStruct->szFileName = szVecList[i];
		pStruct->Type = (PluginTypeEnum)nType;
		*( FARPROC * )&pStruct->pGetPluginParam = GetProcAddress( hModule, "GetPluginParam" );
		*( FARPROC * )&pStruct->pRegResource = GetProcAddress( hModule, "RegResource" );

		m_pVecDllList.push_back( pStruct );

		CGlobalValue::GetInstance().UpdateInitDesc( "Register Plugins.. %s", szVecList[i].c_str() );
		CGlobalValue::GetInstance().UpdateProgress( (int)szVecList.size()-1, (int)i );
	}
	return true;
}

void CPluginManager::RegisterResource()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Register Plugin Resource.." );
	for( DWORD i=0; i<m_pVecDllList.size(); i++ ) {
		DllStruct *pStruct = m_pVecDllList[i];
		if( pStruct->pRegResource ) {
			HINSTANCE hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );
		}

		CGlobalValue::GetInstance().UpdateInitDesc( "Register Plugin Resource.. %s", pStruct->szFileName.c_str() );
		CGlobalValue::GetInstance().UpdateProgress( (int)m_pVecDllList.size()-1, (int)i );
	}
}

void CPluginManager::Finalize()
{
	SAFE_DELETE_PVEC( m_pVecDllList );
}

int CPluginManager::GetDllStructCount( PluginTypeEnum Type, int nParamType )
{
	int nCount = 0;
	for( DWORD i=0; i<m_pVecDllList.size(); i++ ) {
		if( m_pVecDllList[i]->Type == Type ) {
			if( m_pVecDllList[i]->pGetPluginParam(0) == nParamType ) nCount++;
		}
	}
	return nCount;
}

CPluginManager::DllStruct *CPluginManager::GetDllStruct( PluginTypeEnum Type, int nParamType, int nIndex )
{
	int nCount = 0;
	for( DWORD i=0; i<m_pVecDllList.size(); i++ ) {
		if( m_pVecDllList[i]->Type == Type ) {
			if( m_pVecDllList[i]->pGetPluginParam(0) == nParamType ) {
				if( nCount == nIndex )
					return m_pVecDllList[i];
				nCount++;
			}
		}
	}
	return NULL;
}