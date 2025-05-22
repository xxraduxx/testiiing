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
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// __super::OnInitDialog() �Լ��� �������ϰ�  ����ũ�� OR �����Ͽ� ������
	// ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ���ؾ߸�
	// �ش� �˸� �޽����� �����ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
		MessageBox( "����� �̺�Ʈ ��Ʈ�� �̸��� �Է��� �ּ���.", "����", MB_OK );
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
		MessageBox( "�̸��� ��� �Ұ����� ���ڰ� �ֽ��ϴ�.", "����", MB_OK );
		return false;
	}


	if( CEventSignalManager::GetInstance().IsExistSignalItem( m_szSignalName, ( s_nStartType == 1 ) ? s_szCurrentSignalName : "" ) == true ) {
		MessageBox( "�ߺ��Ǵ� �̸��� �ֽ��ϴ�.", "����", MB_OK );
		return false;
	}

	s_pCurrentSignalItem->SetName( m_szSignalName );
	s_pCurrentSignalItem->SetBackgroundColor( m_BackgroundColor );
	s_pCurrentSignalItem->SetBackgroundSideColor( m_BackgroundSideColor );
	s_pCurrentSignalItem->SetSelectColor( m_SelectColor );
	s_pCurrentSignalItem->SetSelectSideColor( m_SelectSideColor );

	return true;
}
