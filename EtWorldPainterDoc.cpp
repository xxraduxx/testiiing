// EtWorldPainterDoc.cpp : CEtWorldPainterDoc Ŭ������ ����
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


// CEtWorldPainterDoc ����/�Ҹ�

CEtWorldPainterDoc::CEtWorldPainterDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.
}

CEtWorldPainterDoc::~CEtWorldPainterDoc()
{
}

BOOL CEtWorldPainterDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ ���ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.

	return TRUE;
}




// CEtWorldPainterDoc serialization

void CEtWorldPainterDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}


// CEtWorldPainterDoc ����

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


// CEtWorldPainterDoc ���

void CEtWorldPainterDoc::OnProjectConnect()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	if( /*strlen( CFileServer::GetInstance().GetWorkingFolder() ) == 0 */strlen( CFileServer::GetInstance().GetUseProfile() ) == 0 || CFileServer::GetInstance().GetProfile( CFileServer::GetInstance().GetUseProfile() ) == NULL ||
		strlen( CFileServer::GetInstance().GetUserName() ) == 0 ) {

		MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("�۾� ��� Ȥ�� ����� �̸��� ��ϵǾ� ���� �ʽ��ϴ�.\n������ Ȯ���ϼ���."), _T("Error"), MB_OK );
		CConnectSettingDlg Dlg;
		if( Dlg.DoModal() == IDOK ) {
			if( CFileServer::GetInstance().Connect() == false )
				MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("���� ����."), _T("Error"), MB_OK );
		}
	}
	else {
		if( CFileServer::GetInstance().GetNationCount() > 0 ) {
			CSelectConnectNationDlg Dlg;
			if( Dlg.DoModal() != IDOK ) return;
		}

		if( CFileServer::GetInstance().Connect() == false ) {
			MessageBox( CGlobalValue::GetInstance().m_pParentView->m_hWnd, _T("���� ����."), _T("Error"), MB_OK );
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
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CGlobalValue::GetInstance().m_bAutoCheckIn = false;
	if( CGlobalValue::GetInstance().Save() == false ) {
		MessageBox( ((CMainFrame*)AfxGetMainWnd())->m_hWnd, _T("���̺� ����!!"), _T("Error"), MB_OK );
	}
}

void CEtWorldPainterDoc::OnProjectSetting()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CConnectSettingDlg Dlg;
	if( Dlg.DoModal() == IDOK ) {
		// �������� ���ͼ� User ���� Administrator ��� ������Ʈ ���� �޴��� ����ش�.
		if( Dlg.IsCreateEmptyProject() && _stricmp( CFileServer::GetInstance().GetUserName(), "administrator" ) == NULL ) {
			CCreateProjectDlg Dlg;
			Dlg.DoModal();
		}

	}
}

void CEtWorldPainterDoc::OnProjectDisconnect()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.

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
