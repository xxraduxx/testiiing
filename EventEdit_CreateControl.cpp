// CEventEdit_CreateControl.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventEdit_CreateControl.h"
#include "EventSignalManager.h"


// CEventEdit_CreateControl

IMPLEMENT_DYNCREATE(CEventEdit_CreateControl, CFormView)

CEventEdit_CreateControl::CEventEdit_CreateControl()
	: CFormView(CEventEdit_CreateControl::IDD)
{

}

CEventEdit_CreateControl::~CEventEdit_CreateControl()
{
}

void CEventEdit_CreateControl::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_BackColorBtn);
	DDX_Control(pDX, IDC_BUTTON2, m_BackSideColorBtn);
	DDX_Control(pDX, IDC_BUTTON9, m_SelectColorBtn);
	DDX_Control(pDX, IDC_BUTTON10, m_SelectSideColorBtn);
	DDX_XTColorPicker(pDX, IDC_BUTTON1, m_BackgroundColor);
	DDX_XTColorPicker(pDX, IDC_BUTTON2, m_BackgroundSideColor);
	DDX_XTColorPicker(pDX, IDC_BUTTON9, m_SelectColor);
	DDX_XTColorPicker(pDX, IDC_BUTTON10, m_SelectSideColor);
	DDX_Text(pDX, IDC_EDIT1, m_szSignalName);
	DDV_MaxChars(pDX, m_szSignalName, 32);
}

BEGIN_MESSAGE_MAP(CEventEdit_CreateControl, CFormView)
	ON_CPN_XT_SELENDOK(IDC_BUTTON1, OnSelEndOkBackColor)
	ON_EN_CHANGE(IDC_EDIT1, &CEventEdit_CreateControl::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CEventEdit_CreateControl diagnostics

#ifdef _DEBUG
void CEventEdit_CreateControl::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventEdit_CreateControl::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventEdit_CreateControl message handlers

void CEventEdit_CreateControl::OnSelEndOkBackColor()
{
}
void CEventEdit_CreateControl::OnEnChangeEdit1()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// __super::OnInitDialog() 함수를 재지정하고  마스크에 OR 연산하여 설정된
	// ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출해야만
	// 해당 알림 메시지를 보냅니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
	Invalidate( FALSE );
}


void CEventEdit_CreateControl::PreProcess()
{
	m_szSignalName = s_pCurrentSignalItem->GetName();
	m_BackgroundColor = s_pCurrentSignalItem->GetBackgroundColor();
	m_BackgroundSideColor = s_pCurrentSignalItem->GetBackgroundSideColor();
	m_SelectColor = s_pCurrentSignalItem->GetSelectColor();
	m_SelectSideColor = s_pCurrentSignalItem->GetSelectSideColor();
	UpdateData( FALSE );
}

bool CEventEdit_CreateControl::PostProcess()
{
	UpdateData();

	if( m_szSignalName.IsEmpty() ) {
		MessageBox( "사용할 이벤트 컨트롤 이름을 입력해 주세요.", "에러", MB_OK );
		return false;
	}
	bool bImpossibleChar = false;
	for( int i=0; i<m_szSignalName.GetLength(); i++ ) {
		if( m_szSignalName[i] < 'A' || m_szSignalName[i] > 'z' ) {
			bImpossibleChar = true;
			break;
		}
	}
	if( bImpossibleChar ) {
		MessageBox( "이름에 사용 불가능한 문자가 있습니다.", "에러", MB_OK );
		return false;
	}


	if( CEventSignalManager::GetInstance().IsExistSignalItem( m_szSignalName, ( s_nStartType == 1 ) ? s_szCurrentSignalName : "" ) == true ) {
		MessageBox( "중복되는 이름이 있습니다.", "에러", MB_OK );
		return false;
	}

	s_pCurrentSignalItem->SetName( m_szSignalName );
	s_pCurrentSignalItem->SetBackgroundColor( m_BackgroundColor );
	s_pCurrentSignalItem->SetBackgroundSideColor( m_BackgroundSideColor );
	s_pCurrentSignalItem->SetSelectColor( m_SelectColor );
	s_pCurrentSignalItem->SetSelectSideColor( m_SelectSideColor );

	return true;
}
