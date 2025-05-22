// TexturePrevDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SkinPrevDlg.h"
#include <process.h>
#include "RenderBase.h"


// CSkinPrevDlg
HANDLE CSkinPrevDlg::s_hThreadHandle = NULL;
int CSkinPrevDlg::s_nThreadStatus = 0;

IMPLEMENT_DYNAMIC(CSkinPrevDlg, CFileDialog)

CSkinPrevDlg::CSkinPrevDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		 DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SKINPREVDLG);


	m_pdcBitmap = NULL;
	m_pmemDC = NULL;

	s_nThreadStatus = 0;
	m_dwThreadIndex = 0;
}

CSkinPrevDlg::~CSkinPrevDlg()
{
	SAFE_DELETE( m_pmemDC );
	SAFE_DELETE( m_pdcBitmap );
}


BEGIN_MESSAGE_MAP(CSkinPrevDlg, CFileDialog)
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CSkinPrevDlg 메시지 처리기입니다.



BOOL CSkinPrevDlg::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

UINT __stdcall CSkinPrevDlg::BeginThread( void *pParam )
{
	CSkinPrevDlg *pThis = (CSkinPrevDlg *)pParam;
	if( CSkinPrevDlg::s_hThreadHandle == NULL ) {
		pThis->s_nThreadStatus = -1;
		_endthreadex( 0 );
		return 0;
	}

	if( CSkinPrevDlg::s_nThreadStatus == 0 ) {
		LPDIRECT3DTEXTURE9 pTexture;
		HRESULT hResult;
		D3DLOCKED_RECT rc;
		D3DSURFACE_DESC sd;
		CSkinPrevDlg::s_nThreadStatus = 1;
		TCHAR szPath[MAX_PATH];

		SAFE_DELETE( pThis->m_pmemDC );
		SAFE_DELETE( pThis->m_pdcBitmap );

		char szExt[64] = { 0, };
		_stprintf_s( szPath, "%s", pThis->GetPathName().GetString() );
		_GetExt( szExt, _countof(szExt), szPath );
		if( strcmp( szExt, "skn" ) != NULL ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}
		FILE *fp;
		fopen_s( &fp, szPath, "rb" );
		if( fp == NULL ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}
		fclose(fp);
		

		// 렌더 타겟 카메라 생성
		EtCameraHandle CamHandle;
		SCameraInfo CamInfo;
		EtMatrix matWorld;
		CRect rcRect = pThis->GetPrevRect();
		CamInfo.fWidth = (float)rcRect.Width();
		CamInfo.fHeight = (float)rcRect.Height();
		CamInfo.Target = CT_RENDERTARGET;
		CamHandle = EternityEngine::CreateCamera( &CamInfo );
		MatrixEx Cross;
		SAABox Box;

		EtMatrixIdentity( &matWorld );
		CRenderBase::GetInstance().Lock();
		CRenderBase::s_bThreadRenderTarget = true;

		EtSkinHandle hSkinHandle = CamHandle->AddRenderSkin( szPath, &matWorld );
		CRenderBase::s_bThreadRenderTarget = false;
		CRenderBase::GetInstance().Unlock();

		if( !hSkinHandle ) {
			pThis->s_nThreadStatus = -1;
			SAFE_RELEASE_SPTR( CamHandle );
			_endthreadex( 0 );
			return 0;
		}
		hSkinHandle->GetMeshHandle()->GetBoundingBox( Box );

		EtVector3 vCross;
		float fSizeX = Box.Max.x - Box.Min.x;
		float fSizeY = Box.Max.y - Box.Min.y;
		Cross.m_vPosition.z = -( max( fSizeX, fSizeY ) * 1.6f );

		Cross.m_vPosition.x = Box.Max.x - ( ( Box.Max.x - Box.Min.x ) / 2.f );
		Cross.m_vPosition.y = Box.Max.y - ( ( Box.Max.y - Box.Min.y ) / 2.f );

		CamHandle->Update( Cross );
		if( CRenderBase::s_bRenderMainScene == true ) {
			pThis->s_nThreadStatus = -1;
			SAFE_RELEASE_SPTR( CamHandle );
			_endthreadex( 0 );
			return 0;
		}
		CRenderBase::GetInstance().Lock();
		CRenderBase::s_bThreadRenderTarget = true;

		CamHandle->RenderSkinList();

		CRenderBase::s_bThreadRenderTarget = false;
		CRenderBase::GetInstance().Unlock();

		EtTextureHandle TextureHandle = CamHandle->GetRenderTargetTexture();
		LPD3DXBUFFER pBuffer = NULL;
		EtBaseTexture *pEtTexture = TextureHandle->GetTexturePtr();

		hResult = D3DXSaveTextureToFileInMemory( &pBuffer, D3DXIFF_BMP, pEtTexture, NULL );

		SAFE_RELEASE_SPTR( CamHandle );

		if( FAILED( hResult ) ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}

		hResult = D3DXCreateTextureFromFileInMemoryEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), rcRect.Width(), rcRect.Height(), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );

		SAFE_RELEASE( pBuffer );

		if( FAILED( hResult ) || !pTexture ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}

		pTexture->GetLevelDesc( 0, &sd );

		pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

		CDC *pDC = pThis->GetDC();
		pThis->m_pmemDC = new CDC;
		pThis->m_pdcBitmap = new CBitmap;

		pThis->m_pmemDC->CreateCompatibleDC( pDC );
		pThis->m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
		pThis->ReleaseDC( pDC );

		pThis->m_pmemDC->SelectObject( pThis->m_pdcBitmap );

		DWORD *pSource;
		DWORD *pRowSource = (DWORD*)rc.pBits;
		DWORD dwColor;
		BYTE r,g,b;

		for( int j=0; j<(int)sd.Height; j++ ) {
			pSource = pRowSource;
			for( int k=0; k<(int)sd.Width; k++ ) {
				if( CSkinPrevDlg::s_hThreadHandle == NULL ) {
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

				pThis->m_pmemDC->SetPixel( k, j, RGB(r,g,b) );
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

void CSkinPrevDlg::RefreshPreview()
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

void CSkinPrevDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFileDialog::OnPaint()을(를) 호출하지 마십시오.

	CRect rcRect = GetPrevRect();

	if( m_pmemDC ) {
		dc.BitBlt( rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height(), m_pmemDC, 0, 0, SRCCOPY );
	}
	rcRect.InflateRect( 1, 1, 1, 1 );
	dc.Draw3dRect( &rcRect, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ) );
}


CRect CSkinPrevDlg::GetPrevRect()
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
void CSkinPrevDlg::OnFileNameChange()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CFileDialog::OnFileNameChange();
	UpdateData( TRUE );

	RefreshPreview();
}


BOOL CSkinPrevDlg::OnFileNameOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	BOOL bResult = CFileDialog::OnFileNameOK();

	return bResult;
}

void CSkinPrevDlg::OnDestroy()
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

	CFileDialog::OnDestroy();
}
