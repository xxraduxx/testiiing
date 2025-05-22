// LayerHeightView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "LayerHeightView.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "UserMessage.h"
#include "FileServer.h"
#include "ImagePrevDlg.h"


// CLayerHeightView

IMPLEMENT_DYNCREATE(CLayerHeightView, CFormView)

CLayerHeightView::CLayerHeightView()
	: CFormView(CLayerHeightView::IDD)
{
	m_pdcBitmap = NULL;
	m_pmemDC = NULL;
	m_bLButtonDown = false;
	m_bDrawPoint = false;
	m_pCurAction = NULL;

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
}

CLayerHeightView::~CLayerHeightView()
{
	SAFE_DELETE( m_pContextMenu );
	SAFE_DELETE( m_pCurAction );
	DeleteMemDC();
}

void CLayerHeightView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PREV, m_PrevStatic);
}

BEGIN_MESSAGE_MAP(CLayerHeightView, CFormView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LAYERHEIGHT_IMPORTTEXTURE, &CLayerHeightView::OnLayerheightImporttexture)
	ON_COMMAND(ID_LAYERHEIGHT_EXPORTTEXTURE, &CLayerHeightView::OnLayerheightExporttexture)
	ON_WM_CREATE()
	ON_COMMAND(ID_RELOAD, &CLayerHeightView::OnReload)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CLayerHeightView 진단입니다.

#ifdef _DEBUG
void CLayerHeightView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CLayerHeightView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLayerHeightView 메시지 처리기입니다.

void CLayerHeightView::UpdateHeightPreview()
{
	Invalidate();
}

void CLayerHeightView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;


	CreateMemDC();

	CBitmap *pOldBitmap;
	CRect rcRect;
	m_PrevStatic.GetClientRect( &rcRect );

	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );

	// Draw memDC
	m_pmemDC->Draw3dRect( &rcRect, RGB( 100, 100, 100 ), RGB( 255, 255, 255 ) );
	rcRect.DeflateRect( 1, 1, 1, 1 );
	short *pHeight = pSector->GetHeightPtr();

	float fUnit = 128 / 32767.f;
	for( int i=0; i<rcRect.Height(); i++ ) {
		for( int j=0; j<rcRect.Width(); j++ ) {
			int nX = (int)( pSector->GetTileWidthCount() / (float)rcRect.Width() * j );
			int nY = (int)( pSector->GetTileHeightCount() / (float)rcRect.Height() * i );
			short nHeight = pHeight[ nY * pSector->GetTileWidthCount() + nX ];
			int nValue = 128 + (int)( pHeight[ ( nY * pSector->GetTileWidthCount() ) + nX ] * fUnit );
			if( nValue > 255 ) nValue = 255;
			else if( nValue < 0 ) nValue = 0;

			m_pmemDC->SetPixel( 1+j, 1+(rcRect.Height()-i), RGB( (BYTE)nValue, (BYTE)nValue, (BYTE)nValue ) );
		}
	}
	rcRect.InflateRect( 1, 1, 1, 1 );

	if( m_bDrawPoint == true && CGlobalValue::GetInstance().IsCheckOutMe() ) {
		CRect rcCircle;
		float fSize = (float)( CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize() ) / 100.f;
		float fWidth = rcRect.Width() / (float)CTEtWorld::GetInstance().GetGridWidth() * fSize;
		float fHeight = rcRect.Height() / (float)CTEtWorld::GetInstance().GetGridHeight() * fSize;

		rcCircle.left = m_MousePos.x - (int)( fWidth / 2.f );
		rcCircle.top = m_MousePos.y - (int)( fWidth / 2.f );
		rcCircle.right = m_MousePos.x + (int)( fHeight / 2.f );
		rcCircle.bottom = m_MousePos.y + (int)( fHeight / 2.f );
		m_pmemDC->Ellipse( &rcCircle );
	}

	// Blt
	CRect rcClientRect, rcStaticRect;
	GetClientRect( &rcClientRect );
	ClientToScreen( &rcClientRect );
	m_PrevStatic.GetClientRect( &rcStaticRect );
	m_PrevStatic.ClientToScreen( &rcStaticRect );

	dc.BitBlt( rcStaticRect.left - rcClientRect.left, rcStaticRect.top - rcClientRect.top, rcRect.Width(), rcRect.Height() ,
		m_pmemDC, rcRect.left, rcRect.top, SRCCOPY );
	m_pmemDC->SelectObject( pOldBitmap );
}


void CLayerHeightView::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect;
	m_PrevStatic.GetClientRect( &rcRect );

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CLayerHeightView::DeleteMemDC()
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

void CLayerHeightView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_PrevStatic.ShowWindow( SW_HIDE );
	Invalidate();
}

void CLayerHeightView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CFormView::OnLButtonDown(nFlags, point);

	m_bLButtonDown = true;
	ProcessMouseRect( point );
	if( m_bDrawPoint == true ) {
		ApplyHeight();
	}
	else FinishApplyHeight();

	Invalidate();

}

void CLayerHeightView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_bLButtonDown = false;
	m_bDrawPoint = false;
	FinishApplyHeight();

	Invalidate();

	CFormView::OnLButtonUp(nFlags, point);
}

void CLayerHeightView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CFormView::OnMouseMove(nFlags, point);

	if( m_bLButtonDown == true ) {
		ProcessMouseRect( point );
		if( m_bDrawPoint == true ) {
			ApplyHeight();
		}
		Invalidate();
	}

}

void CLayerHeightView::ApplyHeight()
{
	if( CGlobalValue::GetInstance().IsCheckOutMe() == false ) return;
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;


	CRect rcClientRect, rcStaticRect, rcRect;
	GetClientRect( &rcClientRect );
	ClientToScreen( &rcClientRect );

	m_PrevStatic.GetClientRect( &rcStaticRect );
	m_PrevStatic.ClientToScreen( &rcStaticRect );

	rcRect.left = rcStaticRect.left - rcClientRect.left;
	rcRect.top = rcStaticRect.top - rcClientRect.top;
	rcRect.right = rcRect.left + rcStaticRect.Width();
	rcRect.bottom = rcRect.top + rcStaticRect.Height();
	rcRect.DeflateRect( 1, 1, 1, 1 );

	int nX = (int)( ( pSector->GetTileWidthCount() / (float)rcRect.Width() ) * m_MousePos.x );
	int nY = (int)( (float)pSector->GetTileHeightCount() - ( ( pSector->GetTileHeightCount() / (float)rcRect.Height() ) * m_MousePos.y ) );

	int nModifyType = CGlobalValue::GetInstance().m_nBrushType;
	if( nModifyType == 0 && GetAKState( VK_LCONTROL ) < 0 ) nModifyType = -1;
	else if( nModifyType == 2 && GetAKState( VK_LCONTROL ) < 0 ) nModifyType = -2;

	if( m_pCurAction == NULL ) {
		m_pCurAction = new CActionElementBrushHeight( pSector );
		m_pCurAction->SetDesc( "Edit Height - Preview" );
		((CActionElementBrushHeight*)m_pCurAction)->BeginModify();
	}

	pSector->ApplyHeight( nX, nY, CGlobalValue::GetInstance().m_nBrushDiameter * pSector->GetTileSize(), CGlobalValue::GetInstance().m_fBrushHardness, CGlobalValue::GetInstance().m_fBrushStrong * 2500.f, CGlobalValue::GetInstance().m_rHeight, CGlobalValue::GetInstance().m_rRadian, nModifyType, false );

	CGlobalValue::GetInstance().RefreshRender();
	CGlobalValue::GetInstance().SetModify();
}

void CLayerHeightView::FinishApplyHeight()
{
	if( CGlobalValue::GetInstance().IsCheckOutMe() == false ) return;
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	if( m_pCurAction ) {
		if( ((CActionElementBrushHeight*)m_pCurAction)->FinishModify() == true ) {
			((CActionElementBrushHeight*)m_pCurAction)->AddAction();
			m_pCurAction = NULL;
		}
		else SAFE_DELETE( m_pCurAction );

	}

	pSector->ResetUpdateInfo();	

	CGlobalValue::GetInstance().RefreshRender();
}

void CLayerHeightView::ProcessMouseRect( CPoint point )
{
	CRect rcClientRect, rcStaticRect, rcRect;
	GetClientRect( &rcClientRect );
	ClientToScreen( &rcClientRect );

	m_PrevStatic.GetClientRect( &rcStaticRect );
	m_PrevStatic.ClientToScreen( &rcStaticRect );

	rcRect.left = rcStaticRect.left - rcClientRect.left;
	rcRect.top = rcStaticRect.top - rcClientRect.top;
	rcRect.right = rcRect.left + rcStaticRect.Width();
	rcRect.bottom = rcRect.top + rcStaticRect.Height();
	rcRect.DeflateRect( 1, 1, 1, 1 );

	if( point.x < rcRect.left || point.x > rcRect.right || point.y < rcRect.top || point.y > rcRect.bottom ) {
		m_bDrawPoint = false;
	}
	else {
		m_bDrawPoint = true;
	}
	point.x -= rcRect.left;
	point.y -= rcRect.top;

	m_MousePos = point;
}

BOOL CLayerHeightView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rcRectStatic, rcRect, rcClientRect, rcStaticRect;

	GetClientRect( &rcClientRect );
	ClientToScreen( &rcClientRect );

	m_PrevStatic.GetClientRect( &rcStaticRect );
	m_PrevStatic.ClientToScreen( &rcStaticRect );

	rcRectStatic.left = rcStaticRect.left - rcClientRect.left;
	rcRectStatic.top = rcStaticRect.top - rcClientRect.top;
	rcRectStatic.right = rcRectStatic.left + rcStaticRect.Width();
	rcRectStatic.bottom = rcRectStatic.top + rcStaticRect.Height();


	GetClientRect( &rcRect );
	rcRect.right = rcRectStatic.left;
	pDC->FillSolidRect( &rcRect, GetSysColor(COLOR_ACTIVEBORDER) );

	GetClientRect( &rcRect );
	rcRect.left = rcRectStatic.right;
	pDC->FillSolidRect( &rcRect, GetSysColor(COLOR_ACTIVEBORDER) );

	GetClientRect( &rcRect );
	rcRect.bottom = rcRectStatic.top;
	pDC->FillSolidRect( &rcRect, GetSysColor(COLOR_ACTIVEBORDER) );

	GetClientRect( &rcRect );
	rcRect.top = rcRectStatic.bottom;
	pDC->FillSolidRect( &rcRect, GetSysColor(COLOR_ACTIVEBORDER) );

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CLayerHeightView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( CFileServer::GetInstance().IsConnect() == false ) return;
	if( CGlobalValue::GetInstance().IsCheckOutMe() == false ) return;
	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(2);

	pSubMenu->EnableMenuItem( ID_LAYERHEIGHT_IMPORTTEXTURE, MF_BYCOMMAND | MF_ENABLED );
	pSubMenu->EnableMenuItem( ID_LAYERHEIGHT_EXPORTTEXTURE, MF_BYCOMMAND | MF_ENABLED );

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );

}

void CLayerHeightView::OnLayerheightImporttexture()
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CString szFileList;
	TCHAR szFilter[] = _T( "All Support File(*.dds;*.tga;*.jpg;*.bmp)|*.dds;*.tga;*.jpg;*.bmp|DirectX Texture File (*.dds)|*.dds|Bitmap File (*.bmp)|*.bmp|Tga File (*.tga)|*.tga|Jpeg Texture File (*.jpg)|*.jpg|Png Texture File (*.png)|*.png|All Files (*.*)|*.*||" );

	CImagePrevDlg dlg(TRUE, _T("dds;tga;jpg;bmp;png"), _T("*.dds;*.tga;*.jpg;*.bmp;*.png"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	if( dlg.DoModal() != IDOK ) return;

	ImportHeight( dlg.GetChannelOption(), dlg.GetPathName(), dlg.IsSwapBit(), dlg.GetCenter(), dlg.GetMin(), dlg.GetMax(), pSector );
	Invalidate();

	pSector->ResetUpdateInfo();
	CGlobalValue::GetInstance().RefreshRender();
	CGlobalValue::GetInstance().SetModify();
}

void CLayerHeightView::ImportHeight( int nChannelOption, LPCSTR szPathName, bool bSwapBit, int nCenter, int nMin, int nMax, CTEtWorldSector *pSector )
{
	LPDIRECT3DTEXTURE9 pTexture;
	D3DXIMAGE_INFO Info;
	D3DLOCKED_RECT rc;
	D3DSURFACE_DESC sd;
	HRESULT hResult;

	D3DXGetImageInfoFromFile( szPathName, &Info );
	if( nChannelOption < 7 ) 
		hResult = D3DXCreateTextureFromFileEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), szPathName, Info.Width, Info.Height, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_FILTER_TRIANGLE, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );
	else 
		hResult = D3DXCreateTextureFromFileEx( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), szPathName, Info.Width, Info.Height, 1, 0, D3DFMT_A16B16G16R16, D3DPOOL_SYSTEMMEM, D3DX_FILTER_TRIANGLE, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );
	if( FAILED( hResult ) ) {
		MessageBox( "에러", "못읽어왔삼", MB_OK );
		return;
	}

	float fMinRatio = 1 / 32767.f * (float)-nMin;
	float fMaxRatio = 1 / 32767.f * (float)nMax;

	pTexture->GetLevelDesc( 0, &sd );
	pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

	DWORD *pSource;
	DWORD *pRowSource = (DWORD*)rc.pBits;
	DWORD dwColor;
	short *pHeight = new short[ Info.Width * Info.Height ];
	BYTE r,g,b,a;
	BYTE Sample[2];
	int nTemp;
	for( int j=0; j<(int)sd.Height; j++ ) {
		pSource = pRowSource;
		for( int k=0; k<(int)sd.Width; k++ ) {

			dwColor = *pSource;
			if( nChannelOption < 7 ) {
				r = (BYTE)( ( dwColor << 8 ) >> 24 );
				g = (BYTE)( ( dwColor << 16) >> 24 );
				b = (BYTE)( ( dwColor << 24 ) >> 24 );
				a = (BYTE)( dwColor >> 24 );

				switch( nChannelOption ) {
					case 0:	Sample[0] = r;	break;
					case 1:	Sample[0] = g;	break;
					case 2:	Sample[0] = b;	break;
					case 3:	Sample[0] = a;	break;
					case 4:	Sample[0] = r, Sample[1] = g;	break;
					case 5: Sample[0] = g, Sample[1] = b;	break;
					case 6: Sample[0] = r, Sample[1] = b;	break;
				}
				if( nChannelOption < 4 ) nTemp = -32767 + ( Sample[0] * 256 );
				else {
					if( bSwapBit ) _Swap( Sample[0], Sample[1] );
					nTemp = -32767 + ( ( (int)Sample[0]  << 8 ) + Sample[1] );
				}
				if( nTemp > 0 ) nTemp = (int)( nTemp * fMaxRatio );
				else if( nTemp < 0 ) nTemp = (int)( nTemp * fMinRatio );

				pHeight[ (sd.Height-j-1) * (int)sd.Width + k ] = (short)nTemp;
			}
			else {
				pSource++;
				DWORD dwColor2 = *pSource;

				switch( nChannelOption ) {
					case 7:	nTemp = ( (int)( dwColor << 16 ) >> 16 ); break;
					case 8: nTemp = (int)( (dwColor >> 16) ); break;
					case 9: nTemp = ( (int)( dwColor2 << 16 ) >> 16 ); break;
				}

				pHeight[ (sd.Height-j-1) * (int)sd.Width + k ] = -32768 + nTemp;
			}

			pSource++;
		}
		pRowSource += rc.Pitch / 4;
	}

	pTexture->UnlockRect(0);
	SAFE_RELEASE( pTexture );

	pSector->ApplyHeight( CRect( 0, 0, pSector->GetTileWidthCount() , pSector->GetTileHeightCount() ), Info.Width, Info.Height, pHeight, 1.f );
	SAFE_DELETEA( pHeight );

#ifdef _TOOLCOMPILE
	m_nChannelOption = nChannelOption;
	m_szPathName = szPathName;
	m_bSwapBit = bSwapBit;
	m_nCenter = nCenter;
	m_nMin = nMin;
	m_nMax = nMax;
#endif
}

void CLayerHeightView::OnLayerheightExporttexture()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CString szFileList;
	TCHAR szFilter[] = _T( "All Support File(*.dds;*.tga;*.jpg;*.bmp)|*.dds;*.tga;*.jpg;*.bmp|DirectX Texture File (*.dds)|*.dds|Bitmap File (*.bmp)|*.bmp|Tga File (*.tga)|*.tga|Jpeg Texture File (*.jpg)|*.jpg|Png Texture File (*.png)|*.png|All Files (*.*)|*.*||" );

	CImagePrevDlg dlg(FALSE, _T("dds;tga;jpg;bmp;png"), _T("*.dds;*.tga;*.jpg;*.bmp;*.png"), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	dlg.SetExportMode();
	if( dlg.DoModal() != IDOK ) return;

	short *pHeight = pSector->GetHeightPtr();
	int nWidth = pSector->GetTileWidthCount();
	int nHeight = pSector->GetTileHeightCount();
	LPDIRECT3DTEXTURE9 pTexture;
	HRESULT hr = D3DXCreateTexture( (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr(), nWidth, nHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTexture );
	if( FAILED(hr) ) {
		MessageBox( "텍스쳐 버퍼를 생성할 수 없습니다.", "에러", MB_OK );
		return;
	}

	D3DSURFACE_DESC sd;
	D3DLOCKED_RECT rc;
	pTexture->GetLevelDesc( 0, &sd );
	pTexture->LockRect( 0, &rc, NULL, D3DLOCK_READONLY );

	DWORD *pSource;
	DWORD *pRowSource = (DWORD*)rc.pBits + ( ( rc.Pitch / 4 ) * ( sd.Height - 1 ) );
	short nValue;
	BYTE Color;
	for( int j=0; j<nHeight; j++ ) {
		pSource = pRowSource;
		for( int i=0; i<nWidth; i++ ) {
			nValue = pHeight[j * nHeight + i];
			Color = (BYTE)( ( nValue + 32767 ) / 256 );
			*pSource = D3DCOLOR_ARGB( 255, Color, Color, Color );

			pSource++;
		}
		pRowSource -= rc.Pitch / 4;
	}

	pTexture->UnlockRect( 0 );
	D3DXIMAGE_FILEFORMAT fmt;

	if( _stricmp( dlg.GetFileExt(), "bmp" ) == NULL ) fmt = D3DXIFF_BMP;
	else if( _stricmp( dlg.GetFileExt(), "tga" ) == NULL ) fmt = D3DXIFF_TGA;
	else if( _stricmp( dlg.GetFileExt(), "jpg" ) == NULL ) fmt = D3DXIFF_JPG;
	else if( _stricmp( dlg.GetFileExt(), "png" ) == NULL ) fmt = D3DXIFF_PNG;
	else if( _stricmp( dlg.GetFileExt(), "dds" ) == NULL ) fmt = D3DXIFF_DDS;

	hr = D3DXSaveTextureToFile( dlg.GetPathName(), fmt, pTexture, NULL );
	if( FAILED(hr) ) {
		MessageBox( "저장 실패하였습니다!!", "에러", MB_OK );
	}
	SAFE_RELEASE( pTexture );
}

int CLayerHeightView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_wndToolBar.CreateToolBar(WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this);
	m_wndToolBar.LoadToolBar(IDR_RELOAD);

	m_wndToolBar.GetControls()->SetControlType( 0, xtpControlButton );

	return 0;
}

void CLayerHeightView::OnReload()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	bool bReload = false;
#ifdef _TOOLCOMPILE
	if( !m_szPathName.empty() ) bReload = true;
#endif
	if( !bReload ) return;

#ifdef _TOOLCOMPILE
	ImportHeight( m_nChannelOption, m_szPathName.c_str(), m_bSwapBit, m_nCenter, m_nMin, m_nMax, pSector );
#endif
	Invalidate();

	pSector->ResetUpdateInfo();
	CGlobalValue::GetInstance().RefreshRender();
}

void CLayerHeightView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_wndToolBar.GetSafeHwnd() )
	{
		CSize sz = m_wndToolBar.CalcDockingLayout(cx, LM_HORZDOCK | LM_HORZ | LM_COMMIT);

		m_wndToolBar.MoveWindow(0, 0, cx, sz.cy);
		m_wndToolBar.Invalidate(FALSE);
	}
}
