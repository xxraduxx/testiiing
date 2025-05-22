#pragma once

class CUnionValueProperty;
class CPropSignalItem {
public:
	CPropSignalItem( int nUniqueIndex );
	virtual ~CPropSignalItem();


protected:
	CString m_szName;
	int m_nUniqueIndex;
	int m_nParamUniqueIndex;

	//	irange m_Range;
	std::vector<CUnionValueProperty *> m_pVecList;
	std::vector<int> m_nVecUniqueList;

public:
	void SetName( CString &szStr ) { m_szName = szStr; }
	CString GetName() { return m_szName; }

	int GetUniqueIndex() { return m_nUniqueIndex; }
	void SetUniqueIndex( int nValue ) { m_nUniqueIndex = nValue; }

	bool IsExistParameter( CString &szStr, CString &szOriginalName = CString("") );

	bool AddParameter( CUnionValueProperty *pVariable, bool bAllowUnique );
	void RemoveParameter( CString &szStr );
	DWORD GetParameterCount() { return (DWORD)m_pVecList.size(); }
	CUnionValueProperty *GetParameter( DWORD dwIndex ) { 
		if( dwIndex < 0 || dwIndex >= m_pVecList.size() ) return NULL;
		return m_pVecList[dwIndex]; 
	}
	CUnionValueProperty *GetParameterFromName( CString &szName );
	int GetParameterUniqueIndex( DWORD dwIndex ) { return m_nVecUniqueList[dwIndex]; }

	bool Save( FILE *fp );
	bool Load( FILE *fp );
	bool ExportHeader( FILE *fp, int& nUsingBuffCount, int &nUsing64BuffCount );

};