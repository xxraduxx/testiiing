// CreateEventAreaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreateEventAreaDlg.h"
#include "UnionValueProperty.h"

// CCreateEventAreaDlg dialog

IMPLEMENT_DYNAMIC(CCreateEventAreaDlg, CDialog)

CCreateEventAreaDlg::CCreateEventAreaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateEventAreaDlg::IDD, pParent)
{
	m_pCurrentView = NULL;
	m_CurrentViewType = CEventEditWizardCommon::UNKNOWN_TYPE;
}

CCreateEventAreaDlg::~CCreateEventAreaDlg()
{
}

void CCreateEventAreaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BACK, m_BackButton);
	DDX_Control(pDX, IDC_NEXT, m_NextButton);
	DDX_Control(pDX, IDOK, m_FinishButton);
}


BEGIN_MESSAGE_MAP(CCreateEventAreaDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_NEXT, &CCreateEventAreaDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_BACK, &CCreateEventAreaDlg::OnBnClickedBack)
	ON_BN_CLICKED(IDOK, &CCreateEventAreaDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCreateEventAreaDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCreateEventAreaDlg message handlers

BOOL CCreateEventAreaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CEventEditWizardCommon::s_nStartType = -1;
	for( int i=0; ; i++ ) {
		if( g_SignalEventEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		CFrameWnd * pFrame = new CFrameWnd;

		CCreateContext context;
		context.m_pNewViewClass = g_SignalEventEditWizardStructDefine[i].pRuntimeClass;
		context.m_pCurrentDoc = NULL;

		DWORD dwStyle;
		dwStyle = WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		pFrame->Create( NULL, NULL, dwStyle, CRect(0, 0, 0, 0), this, 0, 0, &context );
		pFrame->ModifyStyleEx( WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_OVERLAPPEDWINDOW, 0 );
		pFrame->ModifyStyle( WS_BORDER | WS_THICKFRAME, 0 );
		pFrame->SendMessageToDescendants( WM_INITIALUPDATE, 0, 0, TRUE, TRUE );	

		CWnd *pChild = pFrame->GetWindow(GW_CHILD);
		if( pChild ) {
			pChild->ModifyStyle( DS_3DLOOK, 0 );
			pChild->ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );
		}

		pFrame->ShowWindow( SW_HIDE );
		g_SignalEventEditWizardStructDefine[i].pThis = pFrame;
	}
	SetCurrentView( m_CurrentViewType );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateEventAreaDlg::SetCurrentView( CEventEditWizardCommon::VIEW_TYPE Type, bool bPreview )
{
	//	int nCount = sizeof(g_SignalEventEditWizardStructDefine) / sizeof(SignalWizardStruct);
	for( int i=0; ; i++ ) {
		if( g_SignalEventEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		if( g_SignalEventEditWizardStructDefine[i].ViewType == Type ) {
			CEventEditWizardCommon::VIEW_TYPE PrevViewType = m_CurrentViewType;
			m_CurrentViewType = Type;
			if( g_SignalEventEditWizardStructDefine[i].pThis ) {

				m_pCurrentView = g_SignalEventEditWizardStructDefine[i].pThis;
				CRect rcRect;
				GetClientRect( &rcRect );
				m_pCurrentView->MoveWindow( &CRect( 188, 0, rcRect.Width(), rcRect.Height() - 43 ) );
				m_pCurrentView->ShowWindow( SW_SHOW );

				CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
				CEventEditWizardCommon *pWizard = dynamic_cast<CEventEditWizardCommon *>(pWnd);

				if( !bPreview ) pWizard->SetPrevViewType( PrevViewType );

				m_NextButton.EnableWindow( pWizard->IsVisivleNextButton() );
				m_BackButton.EnableWindow( pWizard->IsVisivlePrevButton() );
				m_FinishButton.EnableWindow( pWizard->IsVisivleFinishButton() );
				if( pWizard->IsFinishOrCancel() == true )
					m_FinishButton.SetWindowText( "Finish" );
				else m_FinishButton.SetWindowText( "Cancel" );

				pWnd->SetFocus();

				pWizard->PreProcess();

			}
		}
		else {
			if( g_SignalEventEditWizardStructDefine[i].pThis ) 
				g_SignalEventEditWizardStructDefine[i].pThis->ShowWindow( SW_HIDE );
		}
	}
}

void CCreateEventAreaDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_pCurrentView ) {
		m_pCurrentView->MoveWindow( &CRect( 188, 0, cx, cy - 43 ) );
	}
}

void CCreateEventAreaDlg::OnBnClickedNext()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CEventEditWizardCommon *pWizard = dynamic_cast<CEventEditWizardCommon *>(pWnd);

	CEventEditWizardCommon::VIEW_TYPE ViewType = pWizard->GetNextView();

	if( ViewType == CEventEditWizardCommon::UNKNOWN_TYPE ) return;
	if( pWizard->PostProcess() == false ) {
		pWnd->SetFocus();
		return;
	}
	SetCurrentView( ViewType );

}

void CCreateEventAreaDlg::OnBnClickedBack()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CEventEditWizardCommon *pWizard = dynamic_cast<CEventEditWizardCommon *>(pWnd);

	CEventEditWizardCommon::VIEW_TYPE ViewType = pWizard->GetPrevView();

	if( ViewType == CEventEditWizardCommon::UNKNOWN_TYPE ) return;
	SetCurrentView( ViewType, true );
}

void CCreateEventAreaDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szStr;
	m_FinishButton.GetWindowText( szStr );
	if( _stricmp( szStr, "Cancel" ) == NULL ) {
		OnBnClickedCancel();
		return;
	}
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CEventEditWizardCommon *pWizard = dynamic_cast<CEventEditWizardCommon *>(pWnd);

	if( pWizard->PostProcess() == false ) {
		pWnd->SetFocus();
		return;
	}

	OnOK();
}

void CCreateEventAreaDlg::OnBnClickedCancel()
{
	switch( CEventEditWizardCommon::s_nStartType ) {
		case 0:
		case 1:
			SAFE_DELETE( CEventEditWizardCommon::s_pCurrentSignalItem );
			break;
		case 2:
			break;
		case 3:
		case 4:
			CEventEditWizardCommon::s_pCurrentSignalItem = NULL;
			SAFE_DELETE( CEventEditWizardCommon::s_pCurrentPropertyVariable );
			break;
		case 5:
			CEventEditWizardCommon::s_pCurrentSignalItem = NULL;
			break;
	}
	OnCancel();
}


void CCreateEventAreaDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//	int nCount = sizeof(g_SignalEventEditWizardStructDefine) / sizeof(SignalWizardStruct);
	for( int i=0;; i++ ) {
		if( g_SignalEventEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		g_SignalEventEditWizardStructDefine[i].pThis->DestroyWindow();
		g_SignalEventEditWizardStructDefine[i].pThis = NULL;
	}
}

BOOL CCreateEventAreaDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return CDialog::OnEraseBkgnd(pDC);
}

