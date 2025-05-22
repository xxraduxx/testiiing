// BrushPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "BrushPaneView.h"
#include "UserMessage.h"
#include "TexturePrevDlg.h"

int s_nBrushDiameterDef[] = { 5, 10, 20, 40 };
float s_fBrushHardnessDef[] = { 0.5f, 0.5f, 0.3f, 0.3f };

// CBrushPaneView

IMPLEMENT_DYNCREATE(CBrushPaneView, CFormView)

CBrushPaneView::CBrushPaneView()
	: CFormView(CBrushPaneView::IDD)
{
	m_bActivate = false;
}

CBrushPaneView::~CBrushPaneView()
{
}

void CBrushPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_BrushDef1);
	DDX_Control(pDX, IDC_BUTTON2, m_BrushDef2);
	DDX_Control(pDX, IDC_BUTTON3, m_BrushDef3);
	DDX_Control(pDX, IDC_BUTTON4, m_BrushDef4);
	DDX_Control(pDX, IDC_BUTTON5, m_BrushDef5);
	DDX_Control(pDX, IDC_BUTTON6, m_BrushDef6);
	DDX_Control(pDX, IDC_BUTTON7, m_BrushDef7);
	DDX_Control(pDX, IDC_BUTTON8, m_BrushDef8);
}

BEGIN_MESSAGE_MAP(CBrushPaneView, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CBrushPaneView::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CBrushPaneView::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CBrushPaneView::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CBrushPaneView::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CBrushPaneView::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CBrushPaneView::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CBrushPaneView::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CBrushPaneView::OnBnClickedButton8)
	ON_MESSAGE( UM_NUMMINISLIDER_UPDATE, OnUpdateNumMiniSlider )
	ON_MESSAGE( UM_BRUSHPANE_UPDATE_USE_BUTTON, OnUpdateUseButton )
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
END_MESSAGE_MAP()


// CBrushPaneView 진단입니다.

#ifdef _DEBUG
void CBrushPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBrushPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBrushPaneView 메시지 처리기입니다.

void CBrushPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	// 아구찮아.. 그냥 줄줄이..
	m_BrushDef1.SetChecked( TRUE );
	m_BrushDef1.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef2.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef3.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef4.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef5.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef6.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef7.SetXButtonStyle( BS_XT_XPFLAT );
	m_BrushDef8.SetXButtonStyle( BS_XT_XPFLAT );

	m_BrushDef1.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF1 );
	m_BrushDef2.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF2 );
	m_BrushDef3.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF3 );
	m_BrushDef4.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF4 );
	m_BrushDef5.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF5 );
	m_BrushDef6.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF6 );
	m_BrushDef7.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF7 );
	m_BrushDef8.SetBitmap( CSize( 28, 28 ), IDB_BRUSHDEF8 );


	CUSTOM_Control( IDC_DIAMETER, m_DiameterCtrl );
	CUSTOM_Control( IDC_HARDNESS, m_HardnessCtrl );
	CUSTOM_Control( IDC_STRONG, m_StrongCtrl );
	CUSTOM_Control( IDC_RADIAN, m_RadianCtrl );
	CUSTOM_Control( IDC_HEIGHT, m_HeightCtrl );

	m_DiameterCtrl.SetRange( 1, 200 );
	m_HardnessCtrl.SetRange( 1.f, 100.f );
	m_StrongCtrl.SetRange( 0.01f, 100.f );
	m_RadianCtrl.SetRange( 0.f, 90.f );
	m_HeightCtrl.SetRange( -32767, 32767 );

	m_DiameterCtrl.SetPos( s_nBrushDiameterDef[0] );
	m_HardnessCtrl.SetPos( s_fBrushHardnessDef[0] * 100.f );
	m_StrongCtrl.SetPos( CGlobalValue::GetInstance().m_fBrushStrong * 100.f );

	m_RadianCtrl.SetPos( frange( 0.f, 90.f ) );
	m_HeightCtrl.SetPos( irange( -32767, 32767 ) );

}

void CBrushPaneView::OnBnClickedButton1()
{
	CGlobalValue::GetInstance().m_nBrushType = 0;

	m_BrushDef1.SetChecked( TRUE );
	m_BrushDef2.SetChecked( FALSE );
	m_BrushDef3.SetChecked( FALSE );
	m_BrushDef4.SetChecked( FALSE );
}

void CBrushPaneView::OnBnClickedButton2()
{
	CGlobalValue::GetInstance().m_nBrushType = 1;

	m_BrushDef1.SetChecked( FALSE );
	m_BrushDef2.SetChecked( TRUE );
	m_BrushDef3.SetChecked( FALSE );
	m_BrushDef4.SetChecked( FALSE );
}

void CBrushPaneView::OnBnClickedButton3()
{
	CGlobalValue::GetInstance().m_nBrushType = 2;

	m_BrushDef1.SetChecked( FALSE );
	m_BrushDef2.SetChecked( FALSE );
	m_BrushDef3.SetChecked( TRUE );
	m_BrushDef4.SetChecked( FALSE );
}

void CBrushPaneView::OnBnClickedButton4()
{
	// 여기서 TGA 같은 파일 읽어서 스템프 식으로 찍히게 하장.. 나중에 ㅡㅡ;
	TCHAR szFilter[] = _T( "All Support File(*.dds;*.tga;*.jpg;*.bmp)|*.dds;*.tga;*.jpg;*.bmp| \
						   DirectX Texture File (*.dds)|*.dds| \
						   Bitmap File (*.bmp)|*.bmp| \
						   Tga File (*.tga)|*.tga| \
						   jpeg Texture File (*.jpg)|*.jpg| \
						   All Files (*.*)|*.*||" );

	CTexturePrevDlg Dlg(TRUE, _T("dds;tga;jpg;bmp"), _T("*.dds;*.tga;*.jpg;*.bmp"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	if( Dlg.DoModal() != IDOK ) return;
	//////////

	CGlobalValue::GetInstance().m_nBrushType = 3;

	m_BrushDef1.SetChecked( FALSE );
	m_BrushDef2.SetChecked( FALSE );
	m_BrushDef3.SetChecked( FALSE );
	m_BrushDef4.SetChecked( TRUE );
}

void CBrushPaneView::OnBnClickedButton5()
{
	m_DiameterCtrl.SetPos( s_nBrushDiameterDef[0] );
	m_HardnessCtrl.SetPos( s_fBrushHardnessDef[0] * 100.f );
	CGlobalValue::GetInstance().m_nBrushDiameter = s_nBrushDiameterDef[0];
	CGlobalValue::GetInstance().m_fBrushHardness = s_fBrushHardnessDef[0];
}

void CBrushPaneView::OnBnClickedButton6()
{
	m_DiameterCtrl.SetPos( s_nBrushDiameterDef[1] );
	m_HardnessCtrl.SetPos( s_fBrushHardnessDef[1] * 100.f );
	CGlobalValue::GetInstance().m_nBrushDiameter = s_nBrushDiameterDef[1];
	CGlobalValue::GetInstance().m_fBrushHardness = s_fBrushHardnessDef[1];
}

void CBrushPaneView::OnBnClickedButton7()
{
	m_DiameterCtrl.SetPos( s_nBrushDiameterDef[2] );
	m_HardnessCtrl.SetPos( s_fBrushHardnessDef[2] * 100.f );
	CGlobalValue::GetInstance().m_nBrushDiameter = s_nBrushDiameterDef[2];
	CGlobalValue::GetInstance().m_fBrushHardness = s_fBrushHardnessDef[2];
}

void CBrushPaneView::OnBnClickedButton8()
{
	m_DiameterCtrl.SetPos( s_nBrushDiameterDef[3] );
	m_HardnessCtrl.SetPos( s_fBrushHardnessDef[3] * 100.f );
	CGlobalValue::GetInstance().m_nBrushDiameter = s_nBrushDiameterDef[3];
	CGlobalValue::GetInstance().m_fBrushHardness = s_fBrushHardnessDef[3];
}


LRESULT CBrushPaneView::OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam )
{
	HWND hWnd = (HWND)wParam;
	if( hWnd == m_DiameterCtrl.m_hWnd ) {
		int nPos = (int)lParam;
		CGlobalValue::GetInstance().m_nBrushDiameter = nPos;
	}
	else if( hWnd == m_HardnessCtrl.m_hWnd ) {
		float fPos = *(float *)lParam;
		CGlobalValue::GetInstance().m_fBrushHardness = fPos * 0.01f;
	}
	else if( hWnd == m_StrongCtrl.m_hWnd ) {
		float fPos = *(float *)lParam;
		CGlobalValue::GetInstance().m_fBrushStrong = fPos * 0.01f;
	}
	else if( hWnd == m_RadianCtrl.m_hWnd ) {
		frange range = *(frange*)lParam;
		CGlobalValue::GetInstance().m_rRadian = range;
	}
	else if( hWnd == m_HeightCtrl.m_hWnd ) {
		irange range = *(irange*)lParam;
		CGlobalValue::GetInstance().m_rHeight = range;
	}
	return S_OK;
}

BOOL CBrushPaneView::OnEraseBkgnd(CDC* pDC)
{
	return CFormView::OnEraseBkgnd(pDC);
}


LRESULT CBrushPaneView::OnUpdateUseButton( WPARAM wParam, LPARAM lParam )
{
	switch( (int)wParam ) {
		case 0:	// Terrain Brush
			m_BrushDef1.EnableWindow( TRUE );
			m_BrushDef2.EnableWindow( TRUE );
			m_BrushDef3.EnableWindow( TRUE );
			m_BrushDef4.EnableWindow( TRUE );
			m_StrongCtrl.EnableWindow( TRUE );
			break;
		case 1:	// Prop
			m_BrushDef1.EnableWindow( TRUE );
			m_BrushDef2.EnableWindow( FALSE );
			m_BrushDef3.EnableWindow( FALSE );
			m_BrushDef4.EnableWindow( TRUE );
			m_StrongCtrl.EnableWindow( FALSE );
			break;
	}
	return S_OK;
}

LRESULT CBrushPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_DiameterCtrl.SetPos( CGlobalValue::GetInstance().m_nBrushDiameter );
	m_HardnessCtrl.SetPos( CGlobalValue::GetInstance().m_fBrushHardness );
	m_StrongCtrl.SetPos( CGlobalValue::GetInstance().m_fBrushStrong * 100.f );

	m_RadianCtrl.SetPos( CGlobalValue::GetInstance().m_rRadian );
	m_HeightCtrl.SetPos( CGlobalValue::GetInstance().m_rHeight );

	return S_OK;
}
