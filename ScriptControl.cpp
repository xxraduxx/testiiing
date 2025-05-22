#include "StdAfx.h"
#include "ScriptControl.h"
#include "EtResourceMng.h"
#include "FileServer.h"

char *CScriptControl::s_szParamTypeString[CEtTrigger::ParamTypeEnum_Amount] = {
	"Integer",
	"Float",
	"String",
	"Operator",
	"Position",
	"Prop",
	"EventArea",
	"Value",
};

CScriptControl g_ScriptControl;

CScriptControl::CScriptControl()
{
}

CScriptControl::~CScriptControl()
{
	SAFE_DELETE_PVEC( m_pVecDefineEventList );
	SAFE_DELETE_PVEC( m_pVecDefineConditionList );
	SAFE_DELETE_PVEC( m_pVecDefineActionList );
}


bool DescriptionCompare( CScriptControl::DefineScriptStruct *pStruct1, CScriptControl::DefineScriptStruct *pStruct2 )
{
	std::string szStr1;
	std::string szStr2;
	for( DWORD i=0; i<pStruct1->VecStringList.size(); i++ ) {
		if( pStruct1->VecStringList[i].Type != -1 ) szStr1 += "[";
		szStr1 += pStruct1->VecStringList[i].szStr;
		if( pStruct1->VecStringList[i].Type != -1 ) szStr1 += "]";
	}

	for( DWORD i=0; i<pStruct2->VecStringList.size(); i++ ) {
		if( pStruct2->VecStringList[i].Type != -1 ) szStr2 += "[";
		szStr2 += pStruct2->VecStringList[i].szStr;
		if( pStruct2->VecStringList[i].Type != -1 ) szStr2 += "]";
	}

	return ( _stricmp( szStr1.c_str(), szStr2.c_str() ) >= 0 ) ? false : true;
}

void CScriptControl::RefreshDefineScript()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Register Script.." );

	SAFE_DELETE_PVEC( m_pVecDefineEventList );
	SAFE_DELETE_PVEC( m_pVecDefineConditionList );
	SAFE_DELETE_PVEC( m_pVecDefineActionList );

	char szWorkingFolder[_MAX_PATH];
	std::vector<std::string> szVecResult;

	// 컨디션 먼저 구하궁~
	sprintf_s( szWorkingFolder, "%s\\Resource\\Trigger\\Condition", CFileServer::GetInstance().GetLocalWorkingFolder() );
	FindFileListInDirectory( szWorkingFolder, "*.lua", szVecResult, false, true );

	for( DWORD i=0; i<szVecResult.size(); i++ ) {

		DefineScriptStruct *pStruct = new DefineScriptStruct;
		pStruct->szFileName = szVecResult[i];
		if( ParseScriptFromFile( szVecResult[i].c_str(), pStruct->VecStringList ) == false ) {
			SAFE_DELETE( pStruct );
			continue;
		}

		m_pVecDefineConditionList.push_back( pStruct );
	}
	std::sort( m_pVecDefineConditionList.begin(), m_pVecDefineConditionList.end(), DescriptionCompare );


	// 액션 구하구
	szVecResult.clear();
	sprintf_s( szWorkingFolder, "%s\\Resource\\Trigger\\Action", CFileServer::GetInstance().GetLocalWorkingFolder() );
	FindFileListInDirectory( szWorkingFolder, "*.lua", szVecResult, false, true );

	for( DWORD i=0; i<szVecResult.size(); i++ ) {

		DefineScriptStruct *pStruct = new DefineScriptStruct;
		pStruct->szFileName = szVecResult[i];
		if( ParseScriptFromFile( szVecResult[i].c_str(), pStruct->VecStringList ) == false ) {
			SAFE_DELETE( pStruct );
			continue;
		}

		m_pVecDefineActionList.push_back( pStruct );
	}
	std::sort( m_pVecDefineActionList.begin(), m_pVecDefineActionList.end(), DescriptionCompare );

	// 이벤트 구하고
	szVecResult.clear();
	sprintf_s( szWorkingFolder, "%s\\Resource\\Trigger\\Event", CFileServer::GetInstance().GetLocalWorkingFolder() );
	FindFileListInDirectory( szWorkingFolder, "*.lua", szVecResult, false, true );

	for( DWORD i=0; i<szVecResult.size(); i++ ) {
		DefineScriptStruct *pStruct = new DefineScriptStruct;
		pStruct->szFileName = szVecResult[i];
		if( ParseScriptFromFile( szVecResult[i].c_str(), pStruct->VecStringList ) == false ) {
			SAFE_DELETE( pStruct );
			continue;
		}

		m_pVecDefineEventList.push_back( pStruct );
	}
	std::sort( m_pVecDefineEventList.begin(), m_pVecDefineEventList.end(), DescriptionCompare );
	
}

bool CScriptControl::ParseScriptFromFile( const char *szFileName, std::vector<ScriptStringStruct> &VecResult )
{
	lua_State *pState = lua_open();
	luaL_openlibs(pState); 

	if( lua_tinker::dofile( pState, CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str() ) != 0 ) {
		lua_close( pState );
		return false;
	}

	char *szStr = lua_tinker::get<char *>( pState, "g_Description" );
	if( szStr == NULL ) {
		OutputDebug( "[ %s ] 문법 오류!! : g_Description 이 선언되어있지 않아요!!", szFileName );
		lua_close( pState );
		return false;
	}
	lua_tinker::table ParamType = lua_tinker::get<lua_tinker::table>( pState, "g_ParamType" );
	if( ParamType.m_obj == NULL ) {
		OutputDebug( "[ %s ] 문법 오류!! : g_ParamType 이 선언되어있지 않아요!!", szFileName );
		lua_close( pState );
		return false;
	}

	std::vector<std::string> szVecParamStr;
	for( int j=0; ; j++ ) {
		char *pParam = ParamType.get<char *>(j + 1);
		if( pParam == NULL ) break;
		szVecParamStr.push_back( pParam );
	}
	int nLength = (int)strlen( szStr );
	// 파라메터와 실제 타입 갯수가 일치하는지 체쿠!!
	int nParamDefCount = -1;
	for( int j=0; ; j++ ) {
		const char *pResult = _GetSubStrByCount( j, szStr, '%' );
		if( pResult == NULL ) break;
		nParamDefCount++;
	}
	if( (int)szVecParamStr.size() != nParamDefCount ) {
		OutputDebug( "[ %s ] 문법 오류!! : 파라메터 갯수가 일치하지 않아요!!", szFileName );
		lua_close( pState );
		return false;
	}

	char szTemp[256];
	int nOffset = 0;
	int nParamCount = 0;

	bool bValid = true;
	ScriptStringStruct sss;
	for( int j=0; j<=nLength; j++ ) {
		if( szStr[j] == '%' ) {

			if( j - nOffset > 0 ) {
				memset( szTemp, 0, sizeof(szTemp) );
				memcpy( szTemp, szStr + nOffset, j - nOffset );
				sss.szStr = szTemp;
				sss.Type = (CEtTrigger::ParamTypeEnum)-1;
				VecResult.push_back( sss );
			}
			sss.szStr = szVecParamStr[nParamCount];
			CEtTrigger::ParamTypeEnum Type = GetParamTypeFromString( szVecParamStr[nParamCount].c_str() );
			if( Type == (CEtTrigger::ParamTypeEnum)-1 ) {
				bValid = false;
				break;
			}
			sss.Type = Type;
			VecResult.push_back( sss );
			nParamCount++;
			nOffset = j + 1;
		}
		else if( j == nLength ) {
			memset( szTemp, 0, sizeof(szTemp) );
			memcpy( szTemp, szStr + nOffset, j - nOffset );
			sss.szStr = szTemp;
			sss.Type = (CEtTrigger::ParamTypeEnum)-1;
			VecResult.push_back( sss );
		}
	}
	if( bValid == false ) {
		VecResult.clear();
		OutputDebug( "[ %s ] 문법 오류!! : 파라메터 타입이 정의되지 않은 타입입니다", szFileName );
		return false;
	}
	lua_close( pState );
	return true;
}

CEtTrigger::ParamTypeEnum CScriptControl::GetParamTypeFromString( const char *szStr )
{
	for( int i=0; i<CEtTrigger::ParamTypeEnum_Amount; i++ ) {
		if( strcmp( szStr, s_szParamTypeString[i] ) == NULL ) {
			return (CEtTrigger::ParamTypeEnum)i;
		}
	}
	return (CEtTrigger::ParamTypeEnum)-1;
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineEventStructFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecDefineEventList.size() ) return NULL;
	return m_pVecDefineEventList[dwIndex];
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineConditionStructFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecDefineConditionList.size() ) return NULL;
	return m_pVecDefineConditionList[dwIndex];
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineActionStructFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecDefineActionList.size() ) return NULL;
	return m_pVecDefineActionList[dwIndex];
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineEventStructFromName( const char *szFileName )
{
	for( DWORD i=0; i<m_pVecDefineEventList.size(); i++ ) {
		if( _stricmp( m_pVecDefineEventList[i]->szFileName.c_str(), szFileName ) == NULL ) {
			return m_pVecDefineEventList[i];
		}
	}
	return NULL;
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineConditionStructFromName( const char *szFileName )
{
	for( DWORD i=0; i<m_pVecDefineConditionList.size(); i++ ) {
		if( _stricmp( m_pVecDefineConditionList[i]->szFileName.c_str(), szFileName ) == NULL ) {
			return m_pVecDefineConditionList[i];
		}
	}
	return NULL;
}

CScriptControl::DefineScriptStruct *CScriptControl::GetDefineActionStructFromName( const char *szFileName )
{
	for( DWORD i=0; i<m_pVecDefineActionList.size(); i++ ) {
		if( _stricmp( m_pVecDefineActionList[i]->szFileName.c_str(), szFileName ) == NULL ) {
			return m_pVecDefineActionList[i];
		}
	}
	return NULL;
}
