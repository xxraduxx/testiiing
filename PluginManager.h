#pragma once

#include "Singleton.h"
#include "PluginCommon.h"
#include "EtResourceMng.h"
class CWnd;
class CEtEngine;
class CEtWorldEventArea;
class CUnionValueProperty;
class CEtResourceMng;
class CPluginManager : public CSingleton<CPluginManager> {
public:
	CPluginManager();
	~CPluginManager();

protected:
	struct DllStruct {
		std::string szFileName;
		PluginTypeEnum Type;
		HMODULE hModule;

//		int (*pGetPluginType)();
		int (*pGetPluginParam)( int );
		HINSTANCE (*pRegResource)( CEtResourceMng * );

		DllStruct() {
			hModule = NULL;
			Type = (PluginTypeEnum)-1;
		};

		~DllStruct() {
			FreeLibrary( hModule );
		};
	};

	std::vector<DllStruct *> m_pVecDllList;

public:
	bool Initialize( const char *szPath );
	void Finalize();

	void RegisterResource();

	int GetDllStructCount( PluginTypeEnum Type, int nParamType );
	DllStruct *GetDllStruct( PluginTypeEnum Type, int nParamType, int nIndex );

	template< typename T1 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};

	template< typename T1, typename T2 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1, T2 arg2 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1, T2 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1, arg2 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};
	template< typename T1, typename T2, typename T3 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1, T2 arg2, T3 arg3 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1, T2, T3 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1, arg2, arg3 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};
	template< typename T1, typename T2, typename T3, typename T4 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1, T2 arg2, T3 arg3, T4 arg4 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1, T2, T3, T4 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1, arg2, arg3, arg4 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};
	template< typename T1, typename T2, typename T3, typename T4, typename T5 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1, T2, T3, T4, T5 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1, arg2, arg3, arg4, arg5 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};
	template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
	bool CallDll( PluginTypeEnum Type, int nParamType, const char *szFuncName, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6 ) {
		int nCount = GetDllStructCount( Type, nParamType );
		bool bResult = false;
		for( int i=0; i<nCount; i++ ) {
			DllStruct *pStruct = GetDllStruct( Type, nParamType, i );
			if( pStruct == NULL ) continue;

			bool (*pFunc)( T1, T2, T3, T4, T5, T6 );
			*( FARPROC * )&pFunc = GetProcAddress( pStruct->hModule, szFuncName );
			if( pFunc == NULL ) continue;

			HINSTANCE hInstance = NULL;
			if( pStruct->pRegResource ) hInstance = pStruct->pRegResource( &CEtResourceMng::GetInstance() );
			if( pFunc( arg1, arg2, arg3, arg4, arg5, arg6 ) ) bResult = true;
			if( hInstance ) AfxSetResourceHandle( AfxGetInstanceHandle() );

		}
		return bResult;
	};

};