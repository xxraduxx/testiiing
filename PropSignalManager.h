#pragma once

#include "Singleton.h"
class CPropSignalItem;
class CPropSignalManager : public CSingleton<CPropSignalManager> {
public:
	CPropSignalManager();
	virtual ~CPropSignalManager();

	static int s_nUniqueIndex;
protected:
	CString m_szMyName;
	CString m_szWorkingFolder;
	CString m_szFileName;
	std::vector<CPropSignalItem *> m_pVecList;

	void BackupFiles( CString szFileName );
	void ExportEnum( FILE *fp );

public:
	void GetHostName( CString &szHost );
	void GetLocalIP( CString &szIP );

	bool AddSignalItem( CPropSignalItem *pSignalItem );
	bool RemoveSignalItem( CString &szName );
	DWORD GetSignalItemCount();
	CPropSignalItem *GetSignalItem( DWORD dwIndex );
	CPropSignalItem *GetSignalItemFromName( CString &szSignalName );
	CPropSignalItem *GetSignalItemFromUniqueIndex( int nValue );

	bool IsExistSignalItem( CString &szName, CString &szOriginalName = CString("") );
	bool IsUseUniqueIndex( CString &szOriginalName, int nIndex );

	bool Save();
	bool Load();
	bool ExportHeader( CString szFileName );

	void ChangeWorkingFolder( CString szPath );
	CString GetWorkingFolder() { return m_szWorkingFolder; }

	bool IsCanCheckOut();
	bool CheckOut();
	bool CheckIn();

};