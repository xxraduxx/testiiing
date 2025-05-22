// GridDescriptionPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "GridDescriptionPaneView.h"
#include "UserMessage.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"


// CGridDescriptionPaneView

IMPLEMENT_DYNCREATE(CGridDescriptionPaneView, CFormView)

CGridDescriptionPaneView::CGridDescriptionPaneView()
	: CFormView(CGridDescriptionPaneView::IDD)
{
	m_bActivate = false;
	m_pGrid = NULL;
}

CGridDescriptionPaneView::~CGridDescriptionPaneView()
{
}

void CGridDescriptionPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_RichEdit);
}

BEGIN_MESSAGE_MAP(CGridDescriptionPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_ERASEBKGND()
	ON_EN_CHANGE(IDC_RICHEDIT21, &CGridDescriptionPaneView::OnEnChangeRichedit21)
	ON_EN_UPDATE(IDC_RICHEDIT21, &CGridDescriptionPaneView::OnEnUpdateRichedit21)
END_MESSAGE_MAP()


// CGridDescriptionPaneView diagnostics

#ifdef _DEBUG
void CGridDescriptionPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CGridDescriptionPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CGridDescriptionPaneView message handlers

void CGridDescriptionPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_RichEdit.EnableWindow( FALSE );
}

void CGridDescriptionPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_RichEdit ) {
		CRect rcRect;
		GetClientRect( &rcRect );
		m_RichEdit.MoveWindow( &rcRect );
	}
}

LRESULT CGridDescriptionPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_RichEdit.SetWindowText( "" );

	CString szGridName = CGlobalValue::GetInstance().m_szSelectGrid;
	m_pGrid = (CTEtWorldGrid*)CTEtWorld::GetInstance().FindGrid( szGridName );
	if( !m_pGrid ) {
		m_RichEdit.EnableWindow( FALSE );
		return S_OK;
	}
	m_RichEdit.SetWindowText( m_pGrid->GetDescription() );

	szGridName = CGlobalValue::GetInstance().m_szCheckOutGrid;
	CTEtWorldGrid *pGrid = (CTEtWorldGrid*)CTEtWorld::GetInstance().FindGrid( szGridName );
	m_RichEdit.EnableWindow( ( pGrid ) ? TRUE : FALSE );

	Invalidate();
	return S_OK;
}

BOOL CGridDescriptionPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return FALSE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CGridDescriptionPaneView::OnEnChangeRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CGridDescriptionPaneView::OnEnUpdateRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	CString szStr;
	m_RichEdit.GetWindowText( szStr );

	if( m_pGrid ) m_pGrid->SetDescription( szStr );
}
