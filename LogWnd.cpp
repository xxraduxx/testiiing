#include "stdafx.h"
#include "LogWnd.h"
#include <time.h>

#pragma warning(disable:4311)
#pragma warning(disable:4312)

namespace LogWnd {


PCMD_PROC g_pCallBack = NULL;
#define CMD_EDIT 100

class __LogWnd
{
public:
	__LogWnd( bool bAttachDate = false, DWORD nTarget = LOG_TARGET_WINDOW, const TCHAR* szFilename = NULL, const TCHAR* szPath = NULL );
	virtual ~__LogWnd();

	int Log( int nLogLevel, const TCHAR* szLog );

private:
	void CreateLogWindow();

	bool			m_bAttachDate;
	unsigned int	m_nTarget;
	TCHAR			m_szFilename[8192];
	HWND			m_hwnd;
	HWND			m_hwndList;
	static HWND		m_hwndEdit;
	static WNDPROC  m_OldEditProc;

private:
	static LRESULT CALLBACK __LogWnd::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK __LogWnd::EditSubWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

HWND		__LogWnd::m_hwndEdit= NULL;
WNDPROC		__LogWnd::m_OldEditProc= NULL;

__LogWnd* g_pLog = NULL;

//-------------------------------------------------------------------------------------------------------------
__LogWnd::__LogWnd( bool bAttachDate, DWORD nTarget, const TCHAR* szFilename, const TCHAR* szPath )
{
	m_hwndList = NULL;
	m_bAttachDate = bAttachDate;
	m_nTarget = nTarget;
	ZeroMemory( m_szFilename, sizeof(TCHAR) * 1024 );

	if( nTarget & LOG_TARGET_FILE )
	{
		if( szFilename == NULL )
		{
			struct tm now;
			time_t systemTime;

			time(&systemTime); // 현재 시각을 초 단위로 얻기
			localtime_s(&now, &systemTime); // 초 단위의 시간을 분리하여 구조체에 넣기

			// 현재 실행 모듈을 알아낸다.
			TCHAR szCurFileName[MAX_PATH] = {0,};
			::GetModuleFileName (NULL, szCurFileName, MAX_PATH );
			TCHAR __szDrive[MAX_PATH] = { 0, };
			TCHAR __szDir[MAX_PATH] = { 0, };
			TCHAR __szFileName[MAX_PATH] = { 0, };
			TCHAR __szExt[MAX_PATH] = { 0, };

			_tsplitpath( szCurFileName, __szDrive, __szDir, __szFileName, __szExt );

			_sntprintf_s( m_szFilename, 4000, _T("%s%s\\Log\\%s_%04d%02d%02d_%02d%02d%02d_Log.txt"), __szDrive, __szDir, __szFileName, 
						1900+now.tm_year, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec );
		}
		else
			_tcscpy_s( m_szFilename, szFilename );
	}
	else
		m_szFilename[0] = NULL;

	if( nTarget & LOG_TARGET_WINDOW )
		CreateLogWindow();
	else
		m_hwnd = NULL;

	Log( LogLevel::Info, _T("Logging Start..") );
}

__LogWnd::~__LogWnd()
{
	DestroyWindow( m_hwnd );
}

void __LogWnd::CreateLogWindow()
{
	int			x, y, cx, cy;
	WNDCLASS	wc;
	RECT		rc;

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC)__LogWnd::WndProc;
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= GetModuleHandle( NULL );
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= (HBRUSH)GetStockObject( WHITE_BRUSH) ;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("LogWindow");

	RegisterClass( &wc );

	cx = LOG_WINDOW_CX;
	cy = LOG_WINDOW_CY;
	x = GetSystemMetrics( SM_CXSCREEN ) - cx;
	y = 0;

	// 현재 실행 모듈을 알아낸다.
	TCHAR szCurFileName[MAX_PATH];
	::GetModuleFileName( NULL, szCurFileName, MAX_PATH );

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	_tsplitpath_s( szCurFileName, drive, dir, fname, ext ); 

	m_hwnd = CreateWindow( _T("LogWindow"), fname, WS_OVERLAPPED | WS_CAPTION, x, y, cx, cy, NULL, NULL, GetModuleHandle( NULL ), NULL );
	DWORD nError = GetLastError();
	GetClientRect( m_hwnd, &rc );

	m_hwndList = CreateWindow(  _T("LISTBOX"), _T(""), WS_CHILD | WS_VSCROLL | WS_HSCROLL, 0, 0, rc.right, rc.bottom-30, m_hwnd, NULL, GetModuleHandle( NULL ), NULL );
	m_hwndEdit = CreateWindow( _T("edit"), _T(""), WS_CHILD|WS_BORDER|ES_NOHIDESEL , 0, rc.bottom-30, rc.right, 30, m_hwnd, (HMENU)CMD_EDIT, GetModuleHandle( NULL ), NULL );

	ShowWindow( m_hwnd, SW_SHOW );
	ShowWindow( m_hwndList, SW_SHOW );
	ShowWindow( m_hwndEdit, SW_SHOW );

	m_OldEditProc = (WNDPROC)SetWindowLong( m_hwndEdit, GWL_WNDPROC, LONG((WNDPROC)__LogWnd::EditSubWndProc) );
}

int __LogWnd::Log( int nLogLevel, const TCHAR* szLog )
{
	struct tm now;
	time_t systemTime;

	time( &systemTime );				// 현재 시각을 초 단위로 얻기
	localtime_s( &now, &systemTime );	// 초 단위의 시간을 분리하여 구조체에 넣기

	TCHAR szTotal[2048] = _T("");
	ZeroMemory(szTotal, sizeof(TCHAR) * 2048);

	TCHAR szErrorLevel[128] = {0,};
	switch( nLogLevel )
	{
	case LogLevel::Error:
		_tcscpy( szErrorLevel, _T("error") );
		break;
	case LogLevel::Warning:
		_tcscpy( szErrorLevel, _T("warning") );
	    break;
	case LogLevel::Info:
		_tcscpy( szErrorLevel, _T("info") );
		break;
	case LogLevel::Debug:
		_tcscpy( szErrorLevel, _T("debug") );
		break;
	}

	if( m_bAttachDate )
	{
		if( m_nTarget & LOG_TARGET_WINDOW )
		{
			_sntprintf_s(szTotal, 2048, _T("%04d%02d%02d%02d%02d%02d  [%s]    %s"),
				1900+now.tm_year, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, szErrorLevel, szLog );
		}
		else
		{
			_sntprintf_s(szTotal, 2048, _T("%04d%02d%02d%02d%02d%02d  [%s]\t%s"),
				1900+now.tm_year, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, szErrorLevel, szLog );
		}
	}
	else
		_sntprintf_s(szTotal, 2048, _T("%s"), szLog );
		//_sntprintf_s(szTotal, 2048, _T("[Lv:%d]=>%s"), nLogLevel, szLog );

	
	// Log File에 출력할 경우
	if( m_nTarget & LOG_TARGET_FILE )
	{
		//char szTemp[2048] = { 0, };
		//WideCharToMultiByte( CP_ACP, 0, szTotal, -1, szTemp , 2048, NULL, NULL );

		FILE*	fp = NULL;
		errno_t error = _tfopen_s( &fp, m_szFilename, _T("a+") );
		if( fp )
		{
			fprintf_s( fp, "%s\n", szTotal);
			fclose( fp );
		}
	}

	static const int nEnterLineLen = 65;
	// Log Window에 출력할 경우 
	if( m_nTarget & LOG_TARGET_WINDOW )
	{
		WCHAR* pwszChar;
		pwszChar = new WCHAR[1024];
		memset(pwszChar, 0, sizeof(pwszChar));

		MultiByteToWideChar(CP_ACP, 0, szTotal, -1, pwszChar, 2048 );
		std::wstring szLogMsg = pwszChar;

		while( szLogMsg.size() > 0 )
		{
			std::wstring szTemp;
			szTemp = szLogMsg.substr( 0, min(nEnterLineLen, szLogMsg.size()) );
			szLogMsg.erase( 0, min(nEnterLineLen, szLogMsg.size()) );
			SendMessage( m_hwndList, LB_ADDSTRING, 0, (LPARAM) szTemp.c_str() );
			LRESULT n = SendMessage( m_hwndList, LB_GETCOUNT, 0, 0L ) - 1;
			SendMessage( m_hwndList, LB_SETCURSEL, (WPARAM)n, 0L );
		}

		delete pwszChar;
	}

	return 1;
}

LRESULT CALLBACK __LogWnd::EditSubWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_CHAR:
		{
			if( wParam == VK_RETURN )
				return 1;
		}
		break;
	case WM_KEYDOWN:
		{
			if( wParam == VK_RETURN )
			{
				static TCHAR s_CmdBuff[2048] = _T("");
				GetWindowText( m_hwndEdit, s_CmdBuff, 2048-1 );

				if( g_pCallBack )
					g_pCallBack(s_CmdBuff);

				SetWindowText( m_hwndEdit, _T("") );
				return 0;
			}
		}
		break;
	}

	return CallWindowProc( m_OldEditProc,hwnd,uMsg,wParam,lParam );
}

LRESULT CALLBACK __LogWnd::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_ACTIVATE:
		{
			SetFocus( m_hwndEdit );
		}
		break;

	case WM_KEYDOWN:
		{
			if( wParam == VK_RETURN )
				return 0;
		}
		break;
	case WM_DESTROY:
		{
			SetWindowLong( m_hwndEdit, GWL_WNDPROC, (LONG)m_OldEditProc );
		}
		break;
	}

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

//-------------------------------------------------------------------------------------------------------------
bool CreateLog( bool bAttachDate, DWORD nLogTarget, const TCHAR* szFileName )
{
	if( g_pLog )
		return false;

	g_pLog = new __LogWnd( bAttachDate, nLogTarget, szFileName );

	return true;
}

void SetCommandCallBack( PCMD_PROC pCallBack )
{
	g_pCallBack = pCallBack;
}

void Log( int nLogLevel, const TCHAR* fmt, ... )
{
	if( nLogLevel < 0 || g_pLog == NULL )
		return;

	static TCHAR gs_Buffer[2048] = { _T('0'), };

	ZeroMemory( gs_Buffer, sizeof(TCHAR) * 2048 );
	va_list args;
	va_start( args, fmt );
	_vsntprintf_s( gs_Buffer, 2048-1, fmt, args );
	va_end( args );
	gs_Buffer[2048-1] = 0;

	g_pLog->Log( nLogLevel, gs_Buffer );
}

void TraceLog( const TCHAR* fmt, ... )
{
	if( g_pLog == NULL )
		return;

	static TCHAR gs_Buffer[2048] = { _T('0'), };

	ZeroMemory( gs_Buffer, sizeof(TCHAR) * 2048 );
	va_list args;
	va_start( args, fmt );
	_vsntprintf_s( gs_Buffer, 2048-1, fmt, args );
	va_end( args );
	gs_Buffer[2048-1] = 0;

	CString str;
	str = gs_Buffer;
	str += _T('\n');

	TRACE( str.GetBuffer() );
	g_pLog->Log( LogLevel::Debug, gs_Buffer );
}

bool DestroyLog()
{
	if( g_pLog )
	{
		SAFE_DELETE( g_pLog );
		g_pLog = NULL;
		return true;
	}

	return false;
}

}; // namespace LogWnd;