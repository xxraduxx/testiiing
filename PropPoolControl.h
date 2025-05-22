#pragma once

#include "Singleton.h"
class CPropPoolStage {
public:
	CPropPoolStage();
	~CPropPoolStage();

	struct PropPoolStruct {
		CString szPropName;
		EtVector2 vOffset;
	};

protected:
	CString m_szStageName;
	std::vector<PropPoolStruct> m_VecList;

public:
	bool LoadStage( CString szFullName );
	bool SaveStage( CString szFullName = CString("") );
	CString GetStageName() { return m_szStageName; }
	void SetStageName( CString szName ) { m_szStageName = szName; }

	void AddProp( CString szName, EtVector2 vOffset );
	void RemoveProp( DWORD dwIndex );
	void Empty();
	bool IsEmpty() { return ( m_VecList.size() == 0 ) ? true : false; }

	DWORD GetPropCount();
	CString GetPropName( DWORD dwIndex );
	EtVector2 GetPropOffset( DWORD dwIndex );
	void SetPropOffset( DWORD dwIndex, EtVector2 vOffset );

};

class CPropPoolControl : public CSingleton<CPropPoolControl > {
public:
	CPropPoolControl();
	~CPropPoolControl();

protected:
	std::vector<CPropPoolStage *> m_pVecList;
	CPropPoolStage *m_pNoneStage;

	int m_nCurrentStageIndex;

public:
	void LoadStage();
	void SaveStage();
	DWORD GetStageCount();
	CString GetStageName( DWORD dwIndex );

	void SetCurrentStage( CString szStageName );
	CPropPoolStage *GetCurrentStage();
};

extern CPropPoolControl g_PropPoolControl;;