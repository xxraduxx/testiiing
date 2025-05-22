// PreLogoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PreLogoDlg.h"
#include <mmsystem.h>


// CPreLogoDlg dialog

IMPLEMENT_DYNAMIC(CPreLogoDlg, CDialog)

CPreLogoDlg::CPreLogoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreLogoDlg::IDD, pParent)
{
	m_dwFinishTime = 0;
	m_pdcBitmap = NULL;
	m_pmemDC = NULL;
}

CPreLogoDlg::~CPreLogoDlg()
{
	DeleteMemDC();
}

void CPreLogoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPreLogoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPreLogoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPreLogoDlg::OnBnClickedCancel)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CPreLogoDlg message handlers

void CPreLogoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CPreLogoDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CPreLogoDlg::UpdateDescription( const char *szStr )
{
	char szTemp[256] = { 0, };
	strcpy_s( szTemp, szStr );
	int nSize = (int)strlen(szStr);
	if( nSize < 256 ) {
		for( int i=nSize; i<256; i++ ) {
			szTemp[i] = ' ';
		}
	}
	SetDlgItemText( IDC_STATIC_DESC, szTemp );
}

void CPreLogoDlg::Finish()
{
	m_dwFinishTime = timeGetTime();
}

BOOL CPreLogoDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPreLogoDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call __super::OnPaint() for painting messages
}

void CPreLogoDlg::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect;
	GetClientRect( &rcRect );

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->LoadBitmap( IDB_BITMAP10 );
//	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CPreLogoDlg::DeleteMemDC()
{
	if( m_pdcBitmap ) {
		m_pdcBitmap->DeleteObject();
		SAFE_DELETE( m_pdcBitmap );
	}
	if( m_pmemDC ) {
		m_pmemDC->DeleteDC();
		SAFE_DELETE( m_pmemDC );
	}
}

BOOL CPreLogoDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CreateMemDC();

	CBitmap *pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );
	CRect rcRect;
	GetClientRect( &rcRect );

	pDC->FillSolidRect( &rcRect, GetSysColor(COLOR_ACTIVEBORDER) );
	pDC->BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height(), m_pmemDC, 0, 0, SRCCOPY );
	m_pmemDC->SelectObject( pOldBitmap );

	return FALSE;

}
