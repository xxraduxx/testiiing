// EtWorldPainter.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "MainFrm.h"

#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "PreLogoDlg.h"
#include <process.h>
#include <MMSystem.h>

#include "../../Common/BugReporter/BugReporter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEtWorldPainterApp

BEGIN_MESSAGE_MAP(CEtWorldPainterApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CEtWorldPainterApp::OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
//	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CEtWorldPainterApp 생성

CEtWorldPainterApp::CEtWorldPainterApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
//	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF  ); // | _CRTDBG_CHECK_CRT_DF<-요거는 왠만하면 풀지마라..

//	_CrtSetBreakAlloc(258477);
}


// 유일한 CEtWorldPainterApp 개체입니다.

CEtWorldPainterApp theApp;


MMRESULT g_PreLogoTimerID;
void CALLBACK mmTimerProc(UINT id,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	if( CPreLogoDlg::IsActive() ) {
		CPreLogoDlg::GetInstance().ShowWindow( SW_HIDE );
		timeKillEvent( g_PreLogoTimerID );
	}
}

// CEtWorldPainterApp 초기화

BOOL CEtWorldPainterApp::InitInstance()
{
//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	BugReporter::ReportingServerInfo info;
	info.szServerURL = "http://192.168.0.20/bugreport/bugprocess.aspx";
	gs_BugReporter.SetReportingServerInfo(info);
	gs_BugReporter.Enable();

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다. 
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CPreLogoDlg::CreateInstance();
	CPreLogoDlg::GetInstance().Create( IDD_PRELOGO );
	CPreLogoDlg::GetInstance().ShowWindow( SW_SHOW );
	CPreLogoDlg::GetInstance().UpdateDescription( "Starting EtWorldPainter.." );

	CWinApp::InitInstance();

	// 만약 0xC015000F 크래시 난다면 아래거 주석풀어서 해결하면 될거다.
	//afxAmbientActCtx = FALSE;

	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("EyedentityGames"));
	LoadStdProfileSettings(4);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.
	// 응용 프로그램의 문서 템플릿을 등록합니다. 문서 템플릿은
	//  문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CEtWorldPainterDoc),
		RUNTIME_CLASS(CMainFrame),       // 주 SDI 프레임 창입니다.
		RUNTIME_CLASS(CEtWorldPainterView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// DDE Execute 열기를 활성화합니다.
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// 명령줄에 지정된 명령을 디스패치합니다.
	// 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 창 하나만 초기화되었으므로 이를 표시하고 업데이트합니다.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	//  SDI 응용 프로그램에서는 ProcessShellCommand 후에 이러한 호출이 발생해야 합니다.
	// 끌어서 놓기에 대한 열기를 활성화합니다.
	m_pMainWnd->DragAcceptFiles();

	g_PreLogoTimerID = timeSetEvent( 2500, 1, mmTimerProc, 0, TIME_PERIODIC );

	return TRUE;
}



// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CEtWorldPainterApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CEtWorldPainterApp 메시지 처리기


BOOL CEtWorldPainterApp::OnIdle(LONG lCount)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	pView->Refresh();

	CWinApp::OnIdle(0);
	return TRUE;

//	return CWinApp::OnIdle(lCount);
}
