// CreatePropPropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CreatePropPropertyDlg.h"
#include "UnionValueProperty.h"


// CCreatePropPropertyDlg dialog

IMPLEMENT_DYNAMIC(CCreatePropPropertyDlg, CDialog)

CCreatePropPropertyDlg::CCreatePropPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreatePropPropertyDlg::IDD, pParent)
{
	m_pCurrentView = NULL;
	m_CurrentViewType = CPropEditWizardCommon::UNKNOWN_TYPE;

}

CCreatePropPropertyDlg::~CCreatePropPropertyDlg()
{
}

void CCreatePropPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BACK, m_BackButton);
	DDX_Control(pDX, IDC_NEXT, m_NextButton);
	DDX_Control(pDX, IDOK, m_FinishButton);
}


BEGIN_MESSAGE_MAP(CCreatePropPropertyDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_NEXT, &CCreatePropPropertyDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_BACK, &CCreatePropPropertyDlg::OnBnClickedBack)
	ON_BN_CLICKED(IDOK, &CCreatePropPropertyDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCreatePropPropertyDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCreatePropPropertyDlg message handlers

BOOL CCreatePropPropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CPropEditWizardCommon::s_nStartType = -1;
	for( int i=0; ; i++ ) {
		if( g_SignalPropEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		CFrameWnd * pFrame = new CFrameWnd;

		CCreateContext context;
		context.m_pNewViewClass = g_SignalPropEditWizardStructDefine[i].pRuntimeClass;
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
		g_SignalPropEditWizardStructDefine[i].pThis = pFrame;
	}
	SetCurrentView( m_CurrentViewType );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCreatePropPropertyDlg::SetCurrentView( CPropEditWizardCommon::VIEW_TYPE Type, bool bPreview )
{
	//	int nCount = sizeof(g_SignalPropEditWizardStructDefine) / sizeof(SignalWizardStruct);
	for( int i=0; ; i++ ) {
		if( g_SignalPropEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		if( g_SignalPropEditWizardStructDefine[i].ViewType == Type ) {
			CPropEditWizardCommon::VIEW_TYPE PrevViewType = m_CurrentViewType;
			m_CurrentViewType = Type;
			if( g_SignalPropEditWizardStructDefine[i].pThis ) {

				m_pCurrentView = g_SignalPropEditWizardStructDefine[i].pThis;
				CRect rcRect;
				GetClientRect( &rcRect );
				m_pCurrentView->MoveWindow( &CRect( 188, 0, rcRect.Width(), rcRect.Height() - 43 ) );
				m_pCurrentView->ShowWindow( SW_SHOW );

				CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
				CPropEditWizardCommon *pWizard = dynamic_cast<CPropEditWizardCommon *>(pWnd);

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
			if( g_SignalPropEditWizardStructDefine[i].pThis ) 
				g_SignalPropEditWizardStructDefine[i].pThis->ShowWindow( SW_HIDE );
		}
	}
}

void CCreatePropPropertyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_pCurrentView ) {
		m_pCurrentView->MoveWindow( &CRect( 188, 0, cx, cy - 43 ) );
	}
}

void CCreatePropPropertyDlg::OnBnClickedNext()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CPropEditWizardCommon *pWizard = dynamic_cast<CPropEditWizardCommon *>(pWnd);

	CPropEditWizardCommon::VIEW_TYPE ViewType = pWizard->GetNextView();

	if( ViewType == CPropEditWizardCommon::UNKNOWN_TYPE ) return;
	if( pWizard->PostProcess() == false ) {
		pWnd->SetFocus();
		return;
	}
	SetCurrentView( ViewType );

}

void CCreatePropPropertyDlg::OnBnClickedBack()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CPropEditWizardCommon *pWizard = dynamic_cast<CPropEditWizardCommon *>(pWnd);

	CPropEditWizardCommon::VIEW_TYPE ViewType = pWizard->GetPrevView();

	if( ViewType == CPropEditWizardCommon::UNKNOWN_TYPE ) return;
	SetCurrentView( ViewType, true );
}

void CCreatePropPropertyDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szStr;
	m_FinishButton.GetWindowText( szStr );
	if( _stricmp( szStr, "Cancel" ) == NULL ) {
		OnBnClickedCancel();
		return;
	}
	CWnd *pWnd = ((CFrameWnd*)m_pCurrentView)->GetWindow(GW_CHILD);
	CPropEditWizardCommon *pWizard = dynamic_cast<CPropEditWizardCommon *>(pWnd);

	if( pWizard->PostProcess() == false ) {
		pWnd->SetFocus();
		return;
	}

	OnOK();
}

void CCreatePropPropertyDlg::OnBnClickedCancel()
{
	switch( CPropEditWizardCommon::s_nStartType ) {
		case 0:
		case 1:
			SAFE_DELETE( CPropEditWizardCommon::s_pCurrentSignalItem );
			break;
		case 2:
			break;
		case 3:
		case 4:
			CPropEditWizardCommon::s_pCurrentSignalItem = NULL;
			SAFE_DELETE( CPropEditWizardCommon::s_pCurrentPropertyVariable );
			break;
		case 5:
			CPropEditWizardCommon::s_pCurrentSignalItem = NULL;
			break;
	}
	OnCancel();
}


void CCreatePropPropertyDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//	int nCount = sizeof(g_SignalPropEditWizardStructDefine) / sizeof(SignalWizardStruct);
	for( int i=0;; i++ ) {
		if( g_SignalPropEditWizardStructDefine[i].pRuntimeClass == NULL ) break;
		g_SignalPropEditWizardStructDefine[i].pThis->DestroyWindow();
		g_SignalPropEditWizardStructDefine[i].pThis = NULL;
	}
}

BOOL CCreatePropPropertyDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return CDialog::OnEraseBkgnd(pDC);
}

