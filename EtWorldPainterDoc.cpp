// EtWorldPainterDoc.cpp : CEtWorldPainterDoc 클래스의 구현
//

#include "stdafx.h"
#include "EtWorldPainter.h"

#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "ConnectSettingDlg.h"
#include "CreateProjectDlg.h"
#include "FileServer.h"
#include "DummyView.h"
#include "UserMessage.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "SelectConnectNationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEtWorldPainterDoc

IMPLEMENT_DYNCREATE(CEtWorldPainterDoc, CDocument)

BEGIN_MESSAGE_MAP(CEtWorldPainterDoc, CDocument)
	ON_COMMAND(ID_PROJECT_CONNECT, &CEtWorldPainterDoc::OnProjectConnect)
	ON_COMMAND(ID_PROJECT_SAVE, &CEtWorldPainterDoc::OnProjectSave)
	ON_COMMAND(ID_PROJECT_SETTING, &CEtWorldPainterDoc::OnProjectSetting)
	ON_COMMAND(ID_PROJECT_DISCONNECT, &CEtWorldPainterDoc::OnProjectDisconnect)
END_MESSAGE_MAP()


// CEtWorldPainterDoc 생성/소멸

CEtWorldPainterDoc::CEtWorldPainterDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.
}

CEtWorldPainterDoc::~CEtWorldPainterDoc()
{
}

BOOL CEtWorldPainterDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CEtWorldPainterDoc serialization

void CEtWorldPainterDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CEtWorldPainterDoc 진단

#ifdef _DEBUG
void CEtWorldPainterDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEtWorldPainterDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CEtWorldPainterDoc 명령

void CEtWorldPainterDoc::OnProjectConnect()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( /*strlen( CFileServer::GetInstance().GetWorkingFolder() ) == 0 */strlen( CFileServer::GetInstance().GetUseProfile() ) == 0 || CFileServer::GetInstance().GetProfile( CFileServer::GetInstance().GetUseProfile() ) == NULL ||
		strlen( CFileServer::GetInstance().GetUserName() ) == 0 ) {

		MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("작업 경로 혹은 사용자 이름이 등록되어 있지 않습니다.\n설정을 확인하세요."), _T("Error"), MB_OK );
		CConnectSettingDlg Dlg;
		if( Dlg.DoModal() == IDOK ) {
			if( CFileServer::GetInstance().Connect() == false )
				MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("연결 실패."), _T("Error"), MB_OK );
		}
	}
	else {
		if( CFileServer::GetInstance().GetNationCount() > 0 ) {
			CSelectConnectNationDlg Dlg;
			if( Dlg.DoModal() != IDOK ) return;
		}

		if( CFileServer::GetInstance().Connect() == false ) {
			MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("연결 실패."), _T("Error"), MB_OK );
		}
	}
	if( CFileServer::GetInstance().IsConnect() == true ) {
		CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
		if( pView != NULL ) {
			pView->EnableTabView( -1, false );
			pView->EnableTabView( CGlobalValue::GRID, true );
		}
		pView->GetDummyView()->SendMessage( UM_SELECT_CHANGE_OPEN_VIEW );
	}
}

void CEtWorldPainterDoc::OnProjectSave()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CGlobalValue::GetInstance().m_bAutoCheckIn = false;
	if( CGlobalValue::GetInstance().Save() == false ) {
		MessageBox( ((CMainFrame*)AfxGetMainWnd())->m_hWnd, _T("세이브 에러!!"), _T("Error"), MB_OK );
	}
}

void CEtWorldPainterDoc::OnProjectSetting()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CConnectSettingDlg Dlg;
	if( Dlg.DoModal() == IDOK ) {
		// 설정으로 들어와서 User 명이 Administrator 라면 프로젝트 생성 메뉴를 띄어준다.
		if( Dlg.IsCreateEmptyProject() && _stricmp( CFileServer::GetInstance().GetUserName(), "administrator" ) == NULL ) {
			CCreateProjectDlg Dlg;
			Dlg.DoModal();
		}

	}
}

void CEtWorldPainterDoc::OnProjectDisconnect()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	CGlobalValue::GetInstance().CheckSaveMessage( CGlobalValue::GetInstance().m_pParentView->m_hWnd );
	CGlobalValue::GetInstance().SetModify( -1, false );

	CEtWorldPainterView *pView = (CEtWorldPainterView *)CGlobalValue::GetInstance().m_pParentView;
	if( pView != NULL ) {
		pView->SelectTabView( CGlobalValue::GRID );
		pView->EnableTabView( -1, false );
	}
	CGlobalValue::GetInstance().m_SelectGrid = CGlobalValue::GetInstance().m_CheckOutGrid = -1;

	CFileServer::GetInstance().Disconnect();

	pView->Refresh();
	int nCount = sizeof(g_PaneList) / sizeof(PaneCreateStruct);
	CWnd *pWnd;
	for( int i=0; i<nCount; i++ ) {
		pWnd = GetPaneWnd( g_PaneList[i].nPaneID );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	};

	CGlobalValue::GetInstance().RefreshRender();
//	CGlobalValue::GetInstance().m_pParentView->SendMessage( UM_REFRESH_VIEW, 1 );
}
