#include "StdAfx.h"
#include "TEtTriggerElement.h"
#include "ScriptControl.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "SundriesFuncEx.h"
#include "TEtTriggerObject.h"
#include "TEtWorldEventArea.h"
#include "TEtTrigger.h"

CTEtTriggerElement::CTEtTriggerElement( CEtTriggerObject *pObject )
: CEtTriggerElement( pObject, NULL )
{
	m_hItem = NULL;
}

CTEtTriggerElement::~CTEtTriggerElement()
{
	FreeParam();
}

void CTEtTriggerElement::SetFileName( const char *szFileName )
{
	FreeParam();
	m_szScriptName = szFileName;
	m_szCustomScript.clear();

	AllocParam();
}

void CTEtTriggerElement::SetCustomScript( const char *szString )
{
	FreeParam();
	m_szScriptName.clear();
	m_szCustomScript = szString;
}

std::string CTEtTriggerElement::MakeString()
{
	std::string szStr;

	if( m_ScriptType == CustomScript ) {
		szStr = "Custom Script";
		return szStr;
	}

	CScriptControl::DefineScriptStruct *pStruct = GetScriptStruct();
	if( pStruct == NULL ) return szStr;

	int nParamIndex = 0;
	for( DWORD j=0; j<pStruct->VecStringList.size(); j++ ) {
		if( pStruct->VecStringList[j].Type != -1 ) {
			szStr += "[";

			szStr += MakeString( nParamIndex );

			szStr += "]";
			nParamIndex++;
		}
		else {
			szStr += pStruct->VecStringList[j].szStr;
		}
	}
	return szStr;
}

std::string CTEtTriggerElement::MakeString( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecParamList.size() ) return "";
	CUnionValueProperty *pVariable = m_pVecParamList[dwIndex];

	CScriptControl::DefineScriptStruct *pStruct = GetScriptStruct();
	if( pStruct == NULL ) return "";
	
	char szBuf[256] = { 0, };
	switch( pStruct->GetParamStruct(dwIndex)->Type ) {
		case CEtTrigger::Integer:
			sprintf_s( szBuf, "%d", pVariable->GetVariableInt() );
			break;
		case CEtTrigger::Float:
			sprintf_s( szBuf, "%.2f", pVariable->GetVariableFloat() );
			break;
		case CEtTrigger::String:
			sprintf_s( szBuf, "%s", pVariable->GetVariableString() );
			break;
		case CEtTrigger::Operator:
			switch( pVariable->GetVariableInt() ) {
				case 0: sprintf_s( szBuf, "==" ); break;
				case 1: sprintf_s( szBuf, "!=" ); break;
				case 2: sprintf_s( szBuf, ">" ); break;
				case 3: sprintf_s( szBuf, "<" ); break;
				case 4: sprintf_s( szBuf, ">=" ); break;
				case 5: sprintf_s( szBuf, "<=" ); break;
			}
			break;
		case CEtTrigger::Position:
			sprintf_s( szBuf, "%.2f; %.2f; %.2f", pVariable->GetVariableVector3Ptr()->x, pVariable->GetVariableVector3Ptr()->y, pVariable->GetVariableVector3Ptr()->z );
			break;
		case CEtTrigger::Prop:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				CEtWorldProp *pProp = pSector->GetPropFromCreateUniqueID( pVariable->GetVariableInt() );
				if( pProp ) sprintf_s( szBuf, "%s (%d)", pProp->GetPropName(), pVariable->GetVariableInt() );
				else sprintf_s( szBuf, "Unknown (%d)", pVariable->GetVariableInt() );
			}
			break;
		case CEtTrigger::EventArea:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				CString szControlName, szAreaName;
				CEtWorldEventArea *pArea = pSector->GetEventAreaFromCreateUniqueID( pVariable->GetVariableInt() );
				if( pArea ) {
					szAreaName = pArea->GetName();
					szControlName = pSector->GetEventControlNameFromCreateUniqueID( pVariable->GetVariableInt() );
					sprintf_s( szBuf, "%s - %s(%d)", szControlName, szAreaName, pVariable->GetVariableInt() );
				}
				else {
					sprintf_s( szBuf, "Unknown (%d)", pVariable->GetVariableInt() );
				}
			}
			break;
		case CEtTrigger::Value:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
				if( pTrigger == NULL ) break;

				CEtTrigger::DefineValue *pValue = pTrigger->GetDefineValueFromUniqueID( pVariable->GetVariableInt() );
				if( pValue ) {
					sprintf_s( szBuf, "%s (%d)", pValue->szName.c_str(), pValue->nUniqueID );
				}
				else {
					sprintf_s( szBuf, "NoName (%d)", pVariable->GetVariableInt() );
				}

			}
			break;
	}
	return szBuf;
}

CScriptControl::DefineScriptStruct *CTEtTriggerElement::GetScriptStruct()
{
	switch( m_Type ) {
		case Condition: return CScriptControl::GetInstance().GetDefineConditionStructFromName( m_szScriptName.c_str() ); break;
		case Action: return CScriptControl::GetInstance().GetDefineActionStructFromName( m_szScriptName.c_str() ); break;
		case Event: return CScriptControl::GetInstance().GetDefineEventStructFromName( m_szScriptName.c_str() ); break;
	}
	return NULL;
}

void CTEtTriggerElement::AllocParam()
{
	CScriptControl::DefineScriptStruct *pStruct = GetScriptStruct();
	if( pStruct == NULL ) return;

	DWORD dwCount = pStruct->GetParamCount();

	for( DWORD i=0; i<dwCount; i++ ) {
		CUnionValueProperty *pVariable = NULL;
		CScriptControl::ScriptStringStruct *pParam = pStruct->GetParamStruct(i);
		switch( pParam->Type ) {
			case CEtTrigger::Integer: pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
			case CEtTrigger::Float:	pVariable = new CUnionValueProperty( CUnionValueProperty::Float ); break;
			case CEtTrigger::String: pVariable = new CUnionValueProperty( CUnionValueProperty::String ); break;
			case CEtTrigger::Operator: pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
			case CEtTrigger::Position: pVariable = new CUnionValueProperty( CUnionValueProperty::Vector3 ); break;
			case CEtTrigger::Prop: pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
			case CEtTrigger::EventArea: pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
			case CEtTrigger::Value: pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
		}
		if( pVariable == NULL ) {
			assert(0);
			continue;
		}
		m_pVecParamList.push_back( pVariable );
		m_bVecModifyFlag.push_back( false );
	}
}

void CTEtTriggerElement::FreeParam()
{
	SAFE_DELETE_PVEC( m_pVecParamList );
	SAFE_DELETE_VEC( m_bVecModifyFlag );
}

CUnionValueProperty *CTEtTriggerElement::GetParamVariable( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecParamList.size() ) return NULL;
	return m_pVecParamList[dwIndex];
}

void CTEtTriggerElement::SetParamVariable( DWORD dwIndex, CUnionValueProperty *pVariable )
{
	if( dwIndex < 0 || dwIndex >= m_pVecParamList.size() ) return;
	m_bVecModifyFlag[dwIndex] = true;
	*m_pVecParamList[dwIndex] = *pVariable;
}

CTEtTriggerElement & CTEtTriggerElement::operator = ( CTEtTriggerElement &e )
{
	m_Type = e.m_Type;
	m_ScriptType = e.m_ScriptType;
	m_hItem = e.m_hItem;
	m_OperatorType = e.m_OperatorType;
	m_nOperatorIndex = e.m_nOperatorIndex;
	memcpy( m_nOperatorValue, e.m_nOperatorValue, sizeof(m_nOperatorValue) );
	if( e.m_ScriptType == ScriptFile ) SetFileName( e.GetFileName() );
	else SetCustomScript( e.GetCustomScript() );
	if( m_pVecParamList.empty() ) {
		for( DWORD i=0; i<e.m_pVecParamList.size(); i++ ) {
			CUnionValueProperty *pVariable = new CUnionValueProperty( e.m_pVecParamList[i]->GetType() );
			*pVariable = *e.m_pVecParamList[i];
			m_pVecParamList.push_back( pVariable );
			m_bVecModifyFlag.push_back( e.m_bVecModifyFlag[i] );
		}
	}
	else {
		for( DWORD i=0; i<e.m_pVecParamList.size(); i++ ) {
			*m_pVecParamList[i] = *e.m_pVecParamList[i];
			m_bVecModifyFlag[i] = e.m_bVecModifyFlag[i];
		}
	}
	return *this;
}

void CTEtTriggerElement::Save( FILE *fp )
{
	fwrite( &m_Type, sizeof(int), 1, fp );
	fwrite( &m_ScriptType, sizeof(int), 1, fp );
	fwrite( &m_OperatorType, sizeof(int), 1, fp );
	fwrite( &m_nOperatorIndex, sizeof(int), 1, fp );
	fwrite( m_nOperatorValue, sizeof(m_nOperatorValue), 1, fp );

	CString szStr;
	switch( m_ScriptType ) {
		case ScriptFile:
			{
				szStr = m_szScriptName.c_str();
				WriteCString( &szStr, fp );

				// Save Parameter;
				CScriptControl::DefineScriptStruct *pStruct = GetScriptStruct();

				DWORD dwCount = pStruct->GetParamCount();
				fwrite( &dwCount, sizeof(DWORD), 1, fp );

				
				for( DWORD i=0; i<pStruct->GetParamCount(); i++ ) {
					CScriptControl::ScriptStringStruct *pSS = pStruct->GetParamStruct(i);
					CUnionValueProperty *pVariable = m_pVecParamList[i];
					fwrite( &pSS->Type, sizeof(int), 1, fp );

					int nSize = 0;
					void *pPtr = NULL;

					switch( pSS->Type ) {
						case CEtTrigger::Integer:
						case CEtTrigger::Operator:
						case CEtTrigger::Prop:
						case CEtTrigger::EventArea: 
						case CEtTrigger::Value:
							{
								nSize = sizeof(int);
								fwrite( &nSize, sizeof(int), 1, fp );

								int nValue = pVariable->GetVariableInt();
								fwrite( &nValue, nSize, 1, fp );
							}
							break;
						case CEtTrigger::Float:
							{
								nSize = sizeof(float);
								fwrite( &nSize, sizeof(int), 1, fp );

								float fValue = pVariable->GetVariableFloat();
								fwrite( &fValue, nSize, 1, fp );
							}
							break;
						case CEtTrigger::String:
							WriteCString( &CString( pVariable->GetVariableString() ), fp );
							break;
						case CEtTrigger::Position:
							nSize = sizeof(EtVector3);
							fwrite( &nSize, sizeof(int), 1, fp );
							fwrite( pVariable->GetVariableVector3Ptr(), nSize, 1, fp );
							break;
					}
				}
			}
			break;
		case CustomScript:
			szStr = m_szCustomScript.c_str();
			WriteCString( &szStr, fp );
			break;
	}
}

bool CTEtTriggerElement::Load( CStream *pStream )
{
	bool bResult = CEtTriggerElement::Load( pStream );
	if( bResult == false ) return false;

	FreeParam();
	if( m_ScriptType == ScriptFile ) {
		CScriptControl::DefineScriptStruct *pStruct = GetScriptStruct();
		if( pStruct == NULL ) return false;

		AllocParam();

		if( pStruct->GetParamCount() != m_VecParamList.size() ) {
			CString szStr;
			szStr.Format( "%s : %s 파라매터 갯수가 틀립니다.\n File : %d, Current Script : %d", m_pTriggerObj->GetTriggerName(), m_szScriptName.c_str(), m_VecParamList.size(), pStruct->GetParamCount() );
			MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, szStr, "에러", MB_OK );
		}


		for( DWORD i=0; i<pStruct->GetParamCount(); i++ ) {
			if( i < 0 || i >= m_VecParamList.size() ) continue;

			CUnionValueProperty *pVariable = m_pVecParamList[i];
			switch( pStruct->GetParamStruct(i)->Type ) {
				case CEtTrigger::Integer: 
				case CEtTrigger::Prop:
				case CEtTrigger::Operator:
				case CEtTrigger::EventArea: 
				case CEtTrigger::Value:
					pVariable->SetVariable( m_VecParamList[i].nValue );
					break;
				case CEtTrigger::Float:
					pVariable->SetVariable( m_VecParamList[i].fValue ); 
					break;
				case CEtTrigger::String:
					pVariable->SetVariable( m_VecParamList[i].szValue ); 
					break;
				case CEtTrigger::Position:
					pVariable->SetVariable( *m_VecParamList[i].vValue ); 
					break;
			}
			m_bVecModifyFlag[i] = true;
		}
	}

	ReleaseParamList();
	return true;
}