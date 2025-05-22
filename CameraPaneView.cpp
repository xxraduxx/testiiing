// CameraPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "CameraPaneView.h"
#include "GlobalValue.h"
#include "EnviControl.h"


// CCameraPaneView

IMPLEMENT_DYNCREATE(CCameraPaneView, CFormView)

CCameraPaneView::CCameraPaneView()
	: CFormView(CCameraPaneView::IDD)
{
	m_bActivate = false;
}

CCameraPaneView::~CCameraPaneView()
{
}

void CCameraPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCameraPaneView, CFormView)
	ON_MESSAGE( UM_NUMMINISLIDER_UPDATE, OnUpdateNumMiniSlider )
END_MESSAGE_MAP()


// CCameraPaneView 진단입니다.

#ifdef _DEBUG
void CCameraPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CCameraPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCameraPaneView 메시지 처리기입니다.

void CCameraPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CUSTOM_Control( IDC_SLIDER_NEAR, m_NearCtrl );
	CUSTOM_Control( IDC_SLIDER_FAR, m_FarCtrl );
	CUSTOM_Control( IDC_SLIDER_FOV, m_FovCtrl );
	CUSTOM_Control( IDC_SLIDER_CAMSPEED, m_CamSpeedCtrl );

	m_NearCtrl.SetRange( 1, 10000 );
	m_FarCtrl.SetRange( 1000, 500000 );
	m_FovCtrl.SetRange( 1.f, 179.f );
	m_CamSpeedCtrl.SetRange( 0.01f, 20.f );

	m_NearCtrl.SetPos( 100 );
	m_FarCtrl.SetPos( 200000 );
	m_FovCtrl.SetPos( 45.f );
	m_CamSpeedCtrl.SetPos( CGlobalValue::GetInstance().m_fCamSpeed );
}

LRESULT CCameraPaneView::OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam )
{
	HWND hWnd = (HWND)wParam;
	if( hWnd == m_NearCtrl.m_hWnd ) {
		int nValue = (int)lParam;
		CGlobalValue::GetInstance().m_fCamNear = (float)nValue;
		CEnviControl::GetInstance().RefreshElement();
	}
	else if( hWnd == m_FarCtrl.m_hWnd ) {
		int nValue = (int)lParam;
		CGlobalValue::GetInstance().m_fCamFar = (float)nValue;
		CEnviControl::GetInstance().RefreshElement();
	}
	else if( hWnd == m_FovCtrl.m_hWnd ) {
		float fValue = *(float *)lParam;
		fValue = fValue / 180.f * ET_PI;
		CGlobalValue::GetInstance().m_fCamFOV = fValue;
		CEnviControl::GetInstance().RefreshElement();
	}
	else if( hWnd == m_CamSpeedCtrl ) {
		float fValue = *(float *)lParam;
		CGlobalValue::GetInstance().m_fCamSpeed = fValue;
	}

	return S_OK;
}
