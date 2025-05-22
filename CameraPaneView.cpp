// CameraPaneView.cpp : ���� �����Դϴ�.
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


// CCameraPaneView �����Դϴ�.

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


// CCameraPaneView �޽��� ó�����Դϴ�.

void CCameraPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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
