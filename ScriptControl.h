#pragma once

#include "Singleton.h"
#include "EtTrigger.h"
class CScriptControl : public CSingleton<CScriptControl>
{
public:
	CScriptControl();
	virtual ~CScriptControl();

	static char *s_szParamTypeString[CEtTrigger::ParamTypeEnum_Amount];

	struct ScriptStringStruct {
		std::string szStr;
		CEtTrigger::ParamTypeEnum Type;
	};
	struct DefineScriptStruct {
		std::string szFileName;
		std::vector<ScriptStringStruct> VecStringList;

		ScriptStringStruct *GetParamStruct( DWORD dwIndex ) {
			int nOffset = 0;
			for( DWORD i=0; i<VecStringList.size(); i++ ) {
				if( VecStringList[i].Type != (CEtTrigger::ParamTypeEnum)-1 ) {
					if( (int)dwIndex == nOffset ) return &VecStringList[i];
					nOffset++;
				}
			}
			return NULL;
		}
		DWORD GetParamCount() {
			DWORD dwCount = 0;
			for( DWORD i=0; i<VecStringList.size(); i++ ) {
				if( VecStringList[i].Type != (CEtTrigger::ParamTypeEnum)-1 ) dwCount++;
			}
			return dwCount;
		}
	};

protected:
	std::vector<DefineScriptStruct *> m_pVecDefineEventList;
	std::vector<DefineScriptStruct *> m_pVecDefineConditionList;
	std::vector<DefineScriptStruct *> m_pVecDefineActionList;

protected:
	CEtTrigger::ParamTypeEnum GetParamTypeFromString( const char *szStr );

public:
	void RefreshDefineScript();
	bool ParseScriptFromFile( const char *szFileName, std::vector<ScriptStringStruct> &VecResult );

	DWORD GetDefineEventCount() { return (DWORD)m_pVecDefineEventList.size(); }
	DWORD GetDefineConditionCount() { return (DWORD)m_pVecDefineConditionList.size(); }
	DWORD GetDefineActionCount() { return (DWORD)m_pVecDefineActionList.size(); }

	DefineScriptStruct *GetDefineEventStructFromIndex( DWORD dwIndex );
	DefineScriptStruct *GetDefineConditionStructFromIndex( DWORD dwIndex );
	DefineScriptStruct *GetDefineActionStructFromIndex( DWORD dwIndex );

	DefineScriptStruct *GetDefineEventStructFromName( const char *szFileName );
	DefineScriptStruct *GetDefineConditionStructFromName( const char *szFileName );
	DefineScriptStruct *GetDefineActionStructFromName( const char *szFileName );

};

extern CScriptControl g_ScriptControl;