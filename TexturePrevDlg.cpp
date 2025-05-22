// TexturePrevDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TexturePrevDlg.h"
#include <process.h>
#include "RenderBase.h"


// CTexturePrevDlg
HANDLE CTexturePrevDlg::s_hThreadHandle = NULL;
int CTexturePrevDlg::s_nThreadStatus = 0;

IMPLEMENT_DYNAMIC(CTexturePrevDlg, CFileDialog)

CTexturePrevDlg::CTexturePrevDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_TEXTUREPREV_DLG);


	memset( m_pdcBitmap, 0, sizeof(m_pdcBitmap) );
	memset( m_pmemDC, 0, sizeof(m_pmemDC) );
	m_nViewIndex = 0;

	s_nThreadStatus = 0;
	m_dwThreadIndex = 0;
}

CTexturePrevDlg::~CTexturePrevDlg()
{
	for( int i=0; i<5; i++ ) {
		SAFE_DELETE( m_pmemDC[i] );
		SAFE_DELETE( m_pdcBitmap[i] );
	}
}


BEGIN_MESSAGE_MAP(CTexturePrevDlg, CFileDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, &CTexturePrevDlg::OnBnClickedColorRGB)
	ON_BN_CLICKED(IDC_BUTTON3, &CTexturePrevDlg::OnBnClickedColorR)
	ON_BN_CLICKED(IDC_BUTTON9, &CTexturePrevDlg::OnBnClickedColorG)
	ON_BN_CLICKED(IDC_BUTTON10, &CTexturePrevDlg::OnBnClickedColorB)
	ON_BN_CLICKED(IDC_BUTTON11, &CTexturePrevDlg::OnBnClickedColorA)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CTexturePrevDlg 메시지 처리기입니다.



BOOL CTexturePrevDlg::OnInitDialog()
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

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

UINT __stdcall CTexturePrevDlg::BeginThread( void *pParam )
{
	CTexturePrevDlg *pThis = (CTexturePrevDlg *)pParam;
	if( CTexturePrevDlg::s_hThreadHandle == NULL ) {
		pThis->s_nThreadStatus = -1;
		_endthreadex( 0 );
		return 0;
	}

	if( CTexturePrevDlg::s_nThreadStatus == 0 ) {
		CTexturePrevDlg::s_nThreadStatus = 1;
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
				if( CTexturePrevDlg::s_hThreadHandle == NULL ) {
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

void CTexturePrevDlg::RefreshPreview()
{
	if( s_hThreadHandle ) {
		WaitForSingleObject( s_hThreadHandle, 5000 );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;

		/*
		while(1) {
			Sleep(1);
			if( s_nThreadStatus == -1 ) break;
		}
		*/
	}

	s_nThreadStatus = 0;
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 65536, BeginThread, (void*)this, 0, &m_dwThreadIndex );
	while(1) {
		if( s_nThreadStatus == 1 || s_nThreadStatus == -1 ) break;
		Sleep(1);
	}
}

void CTexturePrevDlg::OnPaint()
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


CRect CTexturePrevDlg::GetPrevRect()
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
void CTexturePrevDlg::OnBnClickedColorRGB()
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

void CTexturePrevDlg::OnBnClickedColorR()
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

void CTexturePrevDlg::OnBnClickedColorG()
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

void CTexturePrevDlg::OnBnClickedColorB()
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

void CTexturePrevDlg::OnBnClickedColorA()
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


void CTexturePrevDlg::OnFileNameChange()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CFileDialog::OnFileNameChange();

	RefreshPreview();
}


BOOL CTexturePrevDlg::OnFileNameOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	BOOL bResult = CFileDialog::OnFileNameOK();

	return bResult;
}

void CTexturePrevDlg::OnDestroy()
{
	if( s_hThreadHandle ) {
		WaitForSingleObject( s_hThreadHandle, INFINITE );
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;

	}

	CFileDialog::OnDestroy();
}
