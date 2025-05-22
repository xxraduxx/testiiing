// ImagePrevDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ImagePrevDlg.h"
#include <process.h>


// CImagePrevDlg
HANDLE CImagePrevDlg::s_hThreadHandle = NULL;
int CImagePrevDlg::s_nThreadStatus = 0;

IMPLEMENT_DYNAMIC(CImagePrevDlg, CFileDialog)

CImagePrevDlg::CImagePrevDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_IMAGEPREVDLG);


	memset( m_pdcBitmap, 0, sizeof(m_pdcBitmap) );
	memset( m_pmemDC, 0, sizeof(m_pmemDC) );
	m_nViewIndex = 0;
	m_nChannelOption = 0;
	m_bSwapBit = false;
	m_nCenter = m_nMin = m_nMax = 0;

	s_nThreadStatus = 0;
	m_dwThreadIndex = 0;
	m_bExportMode = false;
}

CImagePrevDlg::~CImagePrevDlg()
{
	for( int i=0; i<5; i++ ) {
		SAFE_DELETE( m_pmemDC[i] );
		SAFE_DELETE( m_pdcBitmap[i] );
	}
}


BEGIN_MESSAGE_MAP(CImagePrevDlg, CFileDialog)
	ON_WM_MENUSELECT()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, &CImagePrevDlg::OnBnClickedColorRGB)
	ON_BN_CLICKED(IDC_BUTTON3, &CImagePrevDlg::OnBnClickedColorR)
	ON_BN_CLICKED(IDC_BUTTON9, &CImagePrevDlg::OnBnClickedColorG)
	ON_BN_CLICKED(IDC_BUTTON10, &CImagePrevDlg::OnBnClickedColorB)
	ON_BN_CLICKED(IDC_BUTTON11, &CImagePrevDlg::OnBnClickedColorA)
	ON_BN_CLICKED(IDC_RADIO1, &CImagePrevDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CImagePrevDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CImagePrevDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CImagePrevDlg::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &CImagePrevDlg::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &CImagePrevDlg::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO7, &CImagePrevDlg::OnBnClickedRadio7)
	ON_BN_CLICKED(IDC_RADIO8, &CImagePrevDlg::OnBnClickedRadio8)
	ON_BN_CLICKED(IDC_RADIO9, &CImagePrevDlg::OnBnClickedRadio9)
	ON_BN_CLICKED(IDC_RADIO10, &CImagePrevDlg::OnBnClickedRadio10)
	ON_BN_CLICKED(IDC_CHECK1, &CImagePrevDlg::OnBnClickedCheck1)
	ON_EN_CHANGE(IDC_EDIT2, &CImagePrevDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CImagePrevDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT7, &CImagePrevDlg::OnEnChangeEdit7)
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CImagePrevDlg 메시지 처리기입니다.



BOOL CImagePrevDlg::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	CButton *pButton = (CButton *)GetDlgItem( IDC_BUTTON2 );
	((CButton *)GetDlgItem( IDC_BUTTON2 ))->SetButtonStyle( BS_RADIOBUTTON );
	((CButton *)GetDlgItem( IDC_BUTTON3 ))->SetButtonStyle( BS_RADIOBUTTON );
	((CButton *)GetDlgItem( IDC_BUTTON9 ))->SetButtonStyle( BS_RADIOBUTTON );
	((CButton *)GetDlgItem( IDC_BUTTON10 ))->SetButtonStyle( BS_RADIOBUTTON );
	((CButton *)GetDlgItem( IDC_BUTTON11 ))->SetButtonStyle( BS_RADIOBUTTON );
	pButton->SetCheck( BST_CHECKED );

	((CButton *)GetDlgItem( IDC_RADIO1 ))->SetCheck(1);
	((CButton *)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER1 ))->SetRange( -32767, 32767, TRUE );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER1 ))->SetPos( 0 );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER2 ))->SetRange( 0, 32767, TRUE );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER2 ))->SetPos( 0 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER3 ))->SetRange( -32767, 0, TRUE );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER3 ))->SetPos( 0 );

	DWORD nOffsetMax = 32767;
	DWORD nOffsetMin = -32767;
	char szMaxStr[16] = { 0, };
	char szMinStr[16] = { 0, };
	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "GroundOffsetMax", nOffsetMax );
	GetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "GroundOffsetMin", nOffsetMin );
	((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( CString("0") );
	_itoa_s( (int)nOffsetMax, szMaxStr, 10 );
	_itoa_s( (int)nOffsetMin, szMinStr, 10 );
	((CEdit *)GetDlgItem( IDC_EDIT3 ))->SetWindowText( szMaxStr );
	((CEdit *)GetDlgItem( IDC_EDIT7 ))->SetWindowText( szMinStr );

	if( m_bExportMode ) {
		GetDlgItem( IDC_RADIO1 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO4 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO5 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO6 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO7 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO8 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO9 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO10 )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK1 )->EnableWindow( FALSE );

		GetDlgItem( IDC_SLIDER2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_SLIDER3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT7 )->EnableWindow( FALSE );

		GetDlgItem( IDC_BUTTON2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON9 )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON10 )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON11 )->EnableWindow( FALSE );

		GetDlgItem( IDC_STATIC_PREV )->EnableWindow( FALSE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CImagePrevDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CFileDialog::OnMenuSelect(nItemID, nFlags, hSysMenu);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

BOOL CImagePrevDlg::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	/*
	if( nID == 0 ) {
		if( nCode == 5176742 ) RefreshPreview();
	}
	*/

	return CFileDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CImagePrevDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFileDialog::OnCommand(wParam, lParam);
}

UINT __stdcall CImagePrevDlg::BeginThread( void *pParam )
{
	CImagePrevDlg *pThis = (CImagePrevDlg *)pParam;
	if( CImagePrevDlg::s_hThreadHandle == NULL ) {
		pThis->s_nThreadStatus = -1;
		_endthreadex( 0 );
		return 0;
	}

	if( CImagePrevDlg::s_nThreadStatus == 0 ) {
		CImagePrevDlg::s_nThreadStatus = 1;
		TCHAR szPath[MAX_PATH];
		_stprintf_s( szPath, "%s", pThis->GetPathName().GetString() );

		for( int i=0; i<5; i++ ) {
			SAFE_DELETE( pThis->m_pmemDC[i] );
			SAFE_DELETE( pThis->m_pdcBitmap[i] );
		}

		LPDIRECT3DTEXTURE9 pTexture;
		D3DXIMAGE_INFO Info;
		D3DLOCKED_RECT rc;
		D3DSURFACE_DESC sd;
		HRESULT hResult;
		CRect rcRect = pThis->GetPrevRect();

		hResult = D3DXCreateTextureFromFileEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), szPath, rcRect.Width(), rcRect.Height(), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, &Info, NULL, &pTexture );
		if( FAILED( hResult ) ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}

		pTexture->GetLevelDesc( 0, &sd );
		pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

		CDC *pDC = pThis->GetDC();
		for( int i=0; i<5; i++ ) {
			pThis->m_pmemDC[i] = new CDC;
			pThis->m_pdcBitmap[i] = new CBitmap;

			pThis->m_pmemDC[i]->CreateCompatibleDC( pDC );
			pThis->m_pdcBitmap[i]->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
			pThis->m_pmemDC[i]->SelectObject( pThis->m_pdcBitmap[i] );
		}
		pThis->ReleaseDC( pDC );


		DWORD *pSource;
		DWORD *pRowSource = (DWORD*)rc.pBits;
		DWORD dwColor;
		BYTE r,g,b,a;

		for( int j=0; j<(int)sd.Height; j++ ) {
			pSource = pRowSource;
			for( int k=0; k<(int)sd.Width; k++ ) {
				if( CImagePrevDlg::s_hThreadHandle == NULL ) {
					pTexture->UnlockRect(0);
					pTexture->Release();

					pThis->s_nThreadStatus = -1;
					_endthreadex( 0 );
					return 0;
				}

				dwColor = *pSource;
				r = (BYTE)( ( dwColor << 8 ) >> 24 );
				g = (BYTE)( ( dwColor << 16) >> 24 );
				b = (BYTE)( ( dwColor << 24 ) >> 24 );
				a = (BYTE)( dwColor >> 24 );

				pThis->m_pmemDC[0]->SetPixel( k, j, RGB(r,g,b) );
				pThis->m_pmemDC[1]->SetPixel( k, j, RGB(r,r,r) );
				pThis->m_pmemDC[2]->SetPixel( k, j, RGB(g,g,g) );
				pThis->m_pmemDC[3]->SetPixel( k, j, RGB(b,b,b) );
				pThis->m_pmemDC[4]->SetPixel( k, j, RGB(a,a,a) );
				pSource++;
			}
			pRowSource += rc.Pitch / 4;
		}


		pTexture->UnlockRect(0);
		pTexture->Release();

		pThis->Invalidate();
	}
	pThis->s_nThreadStatus = -1;

	_endthreadex( 0 );
	return 0;
}

void CImagePrevDlg::RefreshPreview()
{
	if( s_hThreadHandle ) {
		CloseHandle( s_hThreadHandle );

		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;

		while(1) {
			Sleep(1);
			if( s_nThreadStatus == -1 ) break;
		}
	}

	s_nThreadStatus = 0;
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
	while(1) {
		if( s_nThreadStatus == 1 || s_nThreadStatus == -1 ) break;
		Sleep(1);
	}
}

void CImagePrevDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFileDialog::OnPaint()을(를) 호출하지 마십시오.

	CRect rcRect = GetPrevRect();

	if( m_pmemDC ) {
		dc.BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height(), m_pmemDC[m_nViewIndex], 0, 0, SRCCOPY );
	}
	rcRect.InflateRect( 1, 1, 1, 1 );
	dc.Draw3dRect( &rcRect, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ) );
}


CRect CImagePrevDlg::GetPrevRect()
{
	CWnd *pWnd = GetDlgItem( IDC_STATIC_PREV );
	CRect rcRect, rcTemp[2];
	CPoint pt;
	pWnd->GetClientRect( &rcRect );

	GetClientRect( &rcTemp[0] );
	ClientToScreen( &rcTemp[0] );

	pWnd->GetClientRect( &rcTemp[1] );
	pWnd->ClientToScreen( &rcTemp[1] );

	pt.x = rcTemp[1].left - rcTemp[0].left;
	pt.y = rcTemp[1].top - rcTemp[0].top;

	rcTemp[0] = CRect( pt.x, pt.y, pt.x + rcRect.Width(), pt.y + rcRect.Height() );
	rcTemp[0].DeflateRect( 5, 19, 5, 5 );

	return rcTemp[0];
}
void CImagePrevDlg::OnBnClickedColorRGB()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nViewIndex = 0;
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_BUTTON3 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON9 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON10 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON11 ))->SetCheck( 0 );
	Invalidate();
}

void CImagePrevDlg::OnBnClickedColorR()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nViewIndex = 1;
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON3 ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_BUTTON9 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON10 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON11 ))->SetCheck( 0 );
	Invalidate();
}

void CImagePrevDlg::OnBnClickedColorG()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nViewIndex = 2;
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON3 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON9 ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_BUTTON10 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON11 ))->SetCheck( 0 );
	Invalidate();
}

void CImagePrevDlg::OnBnClickedColorB()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nViewIndex = 3;
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON3 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON9 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON10 ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_BUTTON11 ))->SetCheck( 0 );
	Invalidate();
}

void CImagePrevDlg::OnBnClickedColorA()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nViewIndex = 4;
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON3 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON9 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON10 ))->SetCheck( 0 );
	((CButton*)GetDlgItem( IDC_BUTTON11 ))->SetCheck( 1 );
	Invalidate();
}

void CImagePrevDlg::OnBnClickedRadio1()
{
	m_nChannelOption = 0;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio2()
{
	m_nChannelOption = 1;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio3()
{
	m_nChannelOption = 2;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio4()
{
	m_nChannelOption = 3;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio5()
{
	m_nChannelOption = 4;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( TRUE );
}
void CImagePrevDlg::OnBnClickedRadio6()
{
	m_nChannelOption = 5;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( TRUE );
}
void CImagePrevDlg::OnBnClickedRadio7()
{
	m_nChannelOption = 6;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( TRUE );
}

void CImagePrevDlg::OnBnClickedRadio8()
{
	m_nChannelOption = 7;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio9()
{
	m_nChannelOption = 8;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}
void CImagePrevDlg::OnBnClickedRadio10()
{
	m_nChannelOption = 9;
	((CButton*)GetDlgItem( IDC_CHECK1 ))->EnableWindow( FALSE );
}


void CImagePrevDlg::OnBnClickedCheck1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( ((CButton*)GetDlgItem( IDC_CHECK1 ))->GetCheck() == 1 ) m_bSwapBit = true;
	else m_bSwapBit = false;
}

void CImagePrevDlg::OnEnChangeEdit2()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szStr;
	int nValue;
	CEdit *pEdit = (CEdit *)GetDlgItem( IDC_EDIT2 );
	pEdit->GetWindowText( szStr );
	nValue = atoi( szStr );
	if( nValue < -32767 ) {
		nValue = -32767;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}
	if( nValue > 32767 ) {
		nValue = 32767;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos( -nValue );
}

void CImagePrevDlg::OnEnChangeEdit3()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szStr;
	int nValue;
	CEdit *pEdit = (CEdit *)GetDlgItem( IDC_EDIT3 );
	pEdit->GetWindowText( szStr );
	nValue = atoi( szStr );
	if( nValue < 0 ) {
		nValue = 0;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}
	if( nValue > 32767 ) {
		nValue = 32767;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER2 ))->SetPos( 32767 - nValue );

	CString szTemp;
	((CEdit *)GetDlgItem( IDC_EDIT2 ))->GetWindowText( szTemp );

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetRangeMin( -nValue, TRUE );
	if( atoi( szTemp ) > nValue ) {
		((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos( -nValue );
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( szStr );
	}

}

void CImagePrevDlg::OnEnChangeEdit7()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString szStr;
	int nValue;
	CEdit *pEdit = (CEdit *)GetDlgItem( IDC_EDIT7 );
	pEdit->GetWindowText( szStr );
	nValue = atoi( szStr );
	if( nValue < -32767 ) {
		nValue = -32767;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}
	if( nValue > 0 ) {
		nValue = 0;
		szStr.Format( "%d", nValue );
		pEdit->SetWindowText( szStr );
	}

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER3 ))->SetPos( -( 32767 + nValue ) );

	CString szTemp;
	((CEdit *)GetDlgItem( IDC_EDIT2 ))->GetWindowText( szTemp );

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetRangeMax( -nValue, TRUE );
	if( atoi( szTemp ) < nValue ) {
		((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos( -nValue );
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( szStr );
	}

}

void CImagePrevDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CString szStr;
	int nValue = pScrollBar->GetScrollPos();

	if( pScrollBar->m_hWnd == GetDlgItem( IDC_SLIDER1 )->m_hWnd ) {
		szStr.Format( "%d", -((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->GetPos() );
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( szStr );
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->UpdateData( TRUE );
	}
	else if( pScrollBar->m_hWnd == GetDlgItem( IDC_SLIDER2 )->m_hWnd ) {
		szStr.Format( "%d", 32767 - ((CSliderCtrl*)GetDlgItem( IDC_SLIDER2 ))->GetPos() );
		((CEdit *)GetDlgItem( IDC_EDIT3 ))->SetWindowText( szStr );
		((CEdit *)GetDlgItem( IDC_EDIT3 ))->UpdateData( TRUE );

		int nTemp = atoi(szStr);
		CString szTemp;
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->GetWindowText( szTemp );

		((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetRangeMin( -nTemp, TRUE );
		if( atoi( szTemp ) > nTemp ) {
			((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos( -nTemp );
			((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( szStr );
		}

	}
	else if( pScrollBar->m_hWnd == GetDlgItem( IDC_SLIDER3 )->m_hWnd ) {
		szStr.Format( "%d", -( 32767 + ((CSliderCtrl*)GetDlgItem( IDC_SLIDER3 ))->GetPos() ) );
		((CEdit *)GetDlgItem( IDC_EDIT7 ))->SetWindowText( szStr );
		((CEdit *)GetDlgItem( IDC_EDIT7 ))->UpdateData( TRUE );

		int nTemp = atoi(szStr);
		CString szTemp;
		((CEdit *)GetDlgItem( IDC_EDIT2 ))->GetWindowText( szTemp );

		((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetRangeMax( -nTemp, TRUE );
		if( atoi( szTemp ) < nTemp ) {
			((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos( -nTemp );
			((CEdit *)GetDlgItem( IDC_EDIT2 ))->SetWindowText( szStr );
		}

	}

	CFileDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CImagePrevDlg::OnFileNameChange()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CFileDialog::OnFileNameChange();

	RefreshPreview();
}

BOOL CImagePrevDlg::OnFileNameOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	BOOL bResult = CFileDialog::OnFileNameOK();

	CString szStr;
	((CEdit *)GetDlgItem( IDC_EDIT2 ))->UpdateData( FALSE );
	((CEdit *)GetDlgItem( IDC_EDIT2 ))->GetWindowText( szStr );
	m_nCenter = atoi( szStr );

	((CEdit *)GetDlgItem( IDC_EDIT3 ))->UpdateData( FALSE );
	((CEdit *)GetDlgItem( IDC_EDIT3 ))->GetWindowText( szStr );
	m_nMax = atoi( szStr );

	((CEdit *)GetDlgItem( IDC_EDIT7 ))->UpdateData( FALSE );
	((CEdit *)GetDlgItem( IDC_EDIT7 ))->GetWindowText( szStr );
	m_nMin = atoi( szStr );

	return bResult;
}

void CImagePrevDlg::OnOK()
{
	CFileDialog::OnOK();
}

void CImagePrevDlg::OnDestroy()
{
	if( s_hThreadHandle ) {
		CloseHandle( s_hThreadHandle );

		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
		while(1) {
			Sleep(1);
			if( s_nThreadStatus == -1 ) break;
		}
	}

	CString szOffsetMin;
	CString szOffsetMax;
	((CEdit *)GetDlgItem( IDC_EDIT3 ))->GetWindowText( szOffsetMin );
	((CEdit *)GetDlgItem( IDC_EDIT7 ))->GetWindowText( szOffsetMax );

	SetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "GroundOffsetMax", atoi(szOffsetMin) );
	SetRegistryNumber( HKEY_CURRENT_USER, REG_SUBKEY, "GroundOffsetMin", atoi(szOffsetMax) );
	CFileDialog::OnDestroy();
}
