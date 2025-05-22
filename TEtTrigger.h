#pragma once

#include "EtTrigger.h"

class CTEtTriggerObject;
class CTEtTrigger : public CEtTrigger
{
public:
	CTEtTrigger( CEtWorldSector *pSector );
	virtual ~CTEtTrigger();

	enum PickTypeEnum {
		Prop,
		EventArea,
		Position,
	};
protected:

public:
	virtual bool Load( const char *szFileName );

	bool Save( const char *szFileName );
	bool SaveDefine( const char *szFileName );
	virtual CEtTriggerObject *AllocTriggerObject();

	bool IsExistCategory( const char *szName );
	void InsertTrigger( const char *szCategory, const char *szTriggerName );
	void ChangeCategoryName( const char *szOldName, const char *szNewName );
	void RemoveTrigger( const char *szCategory );
	void RemoveTrigger( const char *szCategory, const char *szTriggerName );
	void ChangeTriggerOrder( CTEtTriggerObject *pObject, CTEtTriggerObject *pTarget );
	void ChangeTriggerOrder( CTEtTriggerObject *pObject, CString szCategory );
	void ChangeCategoryOrder( CString szCategory, CString szTarget );
	void SortCategory( std::vector<CString> &szVecList );
	void InsertTrigger( CEtTriggerObject *pObject );

	DWORD GetDefineValueCount();
	DefineValue *GetDefineValueFromIndex( DWORD dwIndex );
	DefineValue *GetDefineValueFromName( const char *szValueName );
	DefineValue *GetDefineValueFromUniqueID( int nUniqueID );
	DefineValue *AddDefineValue( const char *szValueName );
	void DeleteDefineValueFromIndex( DWORD dwIndex );
	void DeleteDefineValueFromName( const char *szValueName );
	int GetDefineValueUniqueID();
};
