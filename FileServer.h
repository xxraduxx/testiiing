#pragma once

#include "Singleton.h"
#include "EventSignalManager.h"
#include "PropSignalManager.h"

class CEtWorldGrid;
class CPluginManager;
#include "DNTableFile.h"
class CFileServer : public CSingleton<CFileServer> {
public:
	CFileServer();
	~CFileServer();

	enum ServerTypeEnum {
		NetworkAccess,
		SVNAccess,
	};

	struct LogStruct {
		char szIP[16];
		SectorIndex CheckOutGridIndex;
		char szCheckOutGridName[64];
	};

	struct WorkingUserStruct { 
		LogStruct LogStruct;
		CString szUserName;
	};
	std::map<CString,CString> m_szMapUseTextureList;
	std::map<CString,CString> m_szMapUseSkinList;
	std::map<CString,CString> m_szMapUseSoundList;

	struct NationStruct {
		CString szNationStr;
		CString szNationPath;
		CString szNationResPath;
	};

	struct ProfileStruct {
		CString szProfileName;
		CString szLocalPath;
		CString szResPath;
		CString szLoginPath;
		std::vector<NationStruct> VecNation;
	};
	std::vector<ProfileStruct> m_VecProfileList;

public:
	bool Initialize();
	void SetWorkingFolder( char *szStr );
	char *GetWorkingFolder();
	char *GetLocalWorkingFolder();
	void SetUserName( char *szStr );
	char *GetUserName();
	void SetUseProfile( char *szStr );
	char *GetUseProfile();
	char *GetLoginFolder();

	void SetPassword( char *szStr );
	char *GetPassword();

	ServerTypeEnum GetServerType();
	void SetServerType( ServerTypeEnum Type );
	bool IsCanServerType( ServerTypeEnum Type );

	bool Connect();
	void Disconnect();
	bool IsConnect() { return m_bConnected; }
	bool IsCanConnectSetting();

	bool CheckOut();
	bool CheckIn();
	bool IsCanCheckOut( const char *szGridName, SectorIndex Index );
	bool ApplyNewFiles( const char *szLocalPath );
	bool ApplyDeleteFiles( const char *szLocalPath );

	bool CreateDefaultFile();
	bool CreateGridDefaultFile( const char *szGridName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight );

	bool IsUpdateLoginList() { return m_bUpdateLoginInfo; }
	void UpdateLoginList();
	void UpdateUseTextureList( CEtWorldGrid *pGrid = NULL );
	void UpdateUseSkinList( CEtWorldGrid *pGrid = NULL );
	void UpdateUseSoundList( CEtWorldGrid *pGrid = NULL );

	bool IsUseTexture( CString szName, CString *szResult = NULL );
	bool IsUseSkin( CString szName, CString *szResult = NULL );
	bool IsUseSound( CString szName, CString *szResult = NULL );

	std::vector<WorkingUserStruct> *GetLoginUserList() { return &m_vecLoginList; }

	void RefreshProfile();
	void SaveProfile();
	DWORD GetProfileCount();
	ProfileStruct *GetProfile( DWORD dwIndex );
	ProfileStruct *GetProfile( const char *szProfileName );
	bool AddProfile( const char *szProfileName );
	bool RemoveProfile( const char *szProfileName );
	bool IsExistProfile( const char *szProfileName );
	bool IsNeedLogin();

	void AddResourcePath();
	void RemoveResourcePath();

	void AddNationResourcePath();
	void RemoveNationResourcePath();

	void LockNation( CString szNation );
	CString GetLockNation() { return m_szLockNation; }
	void UnlockNation();
	bool IsLockNation() { return ( m_szLockNation.IsEmpty() ) ? false : true; }
	bool IsExistNationSector( char *szGridName, SectorIndex Index );
	void GetNationSectorNationName( char *szGridName, SectorIndex Index, std::vector<CString> &VecNationNameList );

	int GetNationCount();
	CString GetNationName( int nIndex );
	CString GetNationResourceFolder( int nIndex );
	CString GetNationWorkingFolder( int nIndex );

	DNTableFileFormat *LoadSox( const char *szFileName, bool bGenerationInverseFindMap );
	void FindExtFileList( const char *szFileName, std::vector<CFileNameString> &szVecList );

protected:
	bool m_bConnected;
	CString m_szWorkingFolder;
	CString m_szUserName;
	CString m_szProfile;
	CString m_szLockNation;
	CString m_szLoginFolder;
	std::vector<WorkingUserStruct> m_vecLoginList;
	FILE *m_pStream; // Log ÆÄÀÏ
	CEventSignalManager *m_pEventSignalMng;
	CPropSignalManager *m_pPropSignalMng;
	ServerTypeEnum m_ServerType;

	CString m_szPass;

	bool m_bUpdateLoginInfo;
	static HANDLE s_hThreadHandle;
	unsigned m_dwThreadIndex;
	CSyncLock m_SyncLock;
	static HANDLE s_hThreadEndRequest;

	CPluginManager *m_pPluginMng;
public:
	static bool s_bExitThread;

protected:
	bool CreateLoginFile();
	void DeleteLoginFile();
	void GetLocalIP( std::string &szIP );

	void BeginLoginInfoThread();
	void EndLoginInfoThread();
	static UINT __stdcall BeginThread( void *pParam );

	void CFileServer::GetDepthPath( const char *szPath, char *szBuf, int nDepth );
	int CFileServer::GetDepthPathCount( const char *szPath );

	void CopyResourceToFile( int nResourceID, const char *szOutputFilePath );
};