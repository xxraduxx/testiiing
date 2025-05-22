// TriggerOperatorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerOperatorDlg.h"


// CTriggerOperatorDlg dialog

IMPLEMENT_DYNAMIC(CTriggerOperatorDlg, CDialog)

CTriggerOperatorDlg::CTriggerOperatorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerOperatorDlg::IDD, pParent)
{

}

CTriggerOperatorDlg::~CTriggerOperatorDlg()
{
}

void CTriggerOperatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTriggerOperatorDlg, CDialog)
	ON_WM_CREATE()
	ON_MESSAGE( UM_NUMMINISLIDER_UPDATE, OnUpdateNumMiniSlider )
END_MESSAGE_MAP()


// CTriggerOperatorDlg message handlers

BOOL CTriggerOperatorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CUSTOM_Control( IDC_STATIC_SLIDER, m_RandomPercent );
	m_RandomPercent.SetRange( 1, 100 );
	m_RandomPercent.SetPos( m_nRandomValue );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTriggerOperatorDlg::SetValue( int nIndex, int nValue )
{
	m_nRandomValue = nValue;
}

int CTriggerOperatorDlg::GetValue( int nIndex )
{
	return m_nRandomValue;
}

int CTriggerOperatorDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

LRESULT CTriggerOperatorDlg::OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam )
{
	m_nRandomValue = (int)lParam;
	return S_OK;
}