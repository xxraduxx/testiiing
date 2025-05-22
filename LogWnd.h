#pragma once


struct LogLevel
{
	// 0 번이 제일 크리티컬한 로그 숫자가 커질수록 평범한(?) 로그,
	enum Enum
	{
		None = -1,	// -1 이면 로그 찍지 않음
		Error = 0,
		Warning,
		Info,
		Debug
	};
};

namespace LogWnd
{
	const DWORD LOG_TARGET_WINDOW =	0x01;
	const DWORD LOG_TARGET_FILE =	0x02;
	const DWORD LOG_TARGET_ALL = (LOG_TARGET_WINDOW|LOG_TARGET_FILE);

	const DWORD LOG_WINDOW_CX = 500;
	const DWORD LOG_WINDOW_CY = 800;

	typedef void (CALLBACK* PCMD_PROC)(const TCHAR* szCmd);

	bool CreateLog(bool bAttachDate = false, DWORD nLogTarget = LOG_TARGET_WINDOW|LOG_TARGET_FILE, const TCHAR* szFileName = NULL);
	void Log( int nLogLevel, const TCHAR* fmt, ... );
	void TraceLog( const TCHAR* fmt, ... );
	void SetCommandCallBack(PCMD_PROC pCallBack);
	bool DestroyLog();

	#define FUNC_LOG()	LogWnd::Log( 7, __FUNCTIONW__ )
};

