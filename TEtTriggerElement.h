#pragma once

#include "EtTriggerElement.h"
#include "ScriptControl.h"
#include "UnionValueProperty.h"

class CEtTriggerObject;
class CTEtTriggerElement : public CEtTriggerElement
{
public:
	CTEtTriggerElement( CEtTriggerObject *pObject );
	virtual ~CTEtTriggerElement();

protected:
	HTREEITEM m_hItem;
	std::vector<bool> m_bVecModifyFlag;
	std::vector<CUnionValueProperty *> m_pVecParamList;

protected:
	void AllocParam();
	void FreeParam();
	CScriptControl::DefineScriptStruct *GetScriptStruct();

public:
	void Save( FILE *fp );
	virtual bool Load( CStream *pStream );

	void SetType( TriggerElementTypeEnum Type ) { m_Type = Type; }
	void SetScriptType( ScriptTypeEnum Type ) { m_ScriptType = Type; }

	void SetFileName( const char *szFileName );
	void SetCustomScript( const char *szString );
	void SetTreeItem( HTREEITEM hti ) { m_hItem = hti; }

	HTREEITEM GetTreeItem() { return m_hItem; }
	std::string MakeString();
	std::string MakeString( DWORD dwIndex );

	CUnionValueProperty *GetParamVariable( DWORD dwIndex );
	DWORD GetParamVariableCount() { return (DWORD)m_pVecParamList.size(); }
	void SetParamVariable( DWORD dwIndex, CUnionValueProperty *pVariable );
	bool IsModifyParam( DWORD dwIndex ) { return m_bVecModifyFlag[dwIndex]; }
	void SetModifyParam( DWORD dwIndex ) { m_bVecModifyFlag[dwIndex] = true; }

	CTEtTriggerElement & operator = ( CTEtTriggerElement &e );
};