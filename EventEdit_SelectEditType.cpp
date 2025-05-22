// EventEdit_SelectEditType.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EventEdit_SelectEditType.h"


// CEventEdit_SelectEditType

IMPLEMENT_DYNCREATE(CEventEdit_SelectEditType, CFormView)

CEventEdit_SelectEditType::CEventEdit_SelectEditType()
	: CFormView(CEventEdit_SelectEditType::IDD)
{
	m_nEditType = 0;
}

CEventEdit_SelectEditType::~CEventEdit_SelectEditType()
{
}

void CEventEdit_SelectEditType::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEventEdit_SelectEditType, CFormView)
	ON_BN_CLICKED(IDC_RADIO1, &CEventEdit_SelectEditType::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CEventEdit_SelectEditType::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CEventEdit_SelectEditType::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CEventEdit_SelectEditType::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &CEventEdit_SelectEditType::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &CEventEdit_SelectEditType::OnBnClickedRadio6)
END_MESSAGE_MAP()


// CEventEdit_SelectEditType diagnostics

#ifdef _DEBUG
void CEventEdit_SelectEditType::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEventEdit_SelectEditType::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEventEdit_SelectEditType message handlers

CEventEditWizardCommon::VIEW_TYPE CEventEdit_SelectEditType::GetNextView()
{
	s_nStartType = m_nEditType;
	switch( m_nEditType ) {
		case 0: return CREATE_SIGNAL;
		case 1:	return SELECT_SIGNAL;
		case 2: return SELECT_SIGNAL;
		case 3:	return SELECT_SIGNAL;
		case 4:	return SELECT_SIGNAL;
		case 5:	return SELECT_SIGNAL;
	}
	return UNKNOWN_TYPE;
}

bool CEventEdit_SelectEditType::IsVisivleNextButton()
{
	return true;
}

void CEventEdit_SelectEditType::PreProcess()
{
}

bool CEventEdit_SelectEditType::PostProcess()
{
	switch( m_nEditType ) {
		case 0: // Create
			{
				s_pCurrentSignalItem = new CEventSignalItem(-1);
				s_pCurrentSignalItem->SetBackgroundColor( RGB(rand()%256,rand()%256,rand()%256) );
				s_pCurrentSignalItem->SetBackgroundSideColor( RGB(rand()%256,rand()%256,rand()%256) );
				s_pCurrentSignalItem->SetSelectColor( RGB(rand()%256,rand()%256,rand()%256) );
				s_pCurrentSignalItem->SetSelectSideColor( RGB(rand()%256,rand()%256,rand()%256) );
			}
			break;
		case 1: // Modify
			{
				s_pCurrentSignalItem = new CEventSignalItem(-1);
			}
			break;
		case 2:	// Delete
			break;
		case 3:	// Create Parameter
			break;
		case 4:	// Modify Parameter
			break;
		case 5:	// Delete Parameter
			break;
	}
	return true;
}

void CEventEdit_SelectEditType::OnBnClickedRadio1()
{
	m_nEditType = 0;
}

void CEventEdit_SelectEditType::OnBnClickedRadio2()
{
	m_nEditType = 1;
}

void CEventEdit_SelectEditType::OnBnClickedRadio3()
{
	m_nEditType = 2;
}

void CEventEdit_SelectEditType::OnBnClickedRadio4()
{
	m_nEditType = 3;
}

void CEventEdit_SelectEditType::OnBnClickedRadio5()
{
	m_nEditType = 4;
}

void CEventEdit_SelectEditType::OnBnClickedRadio6()
{
	m_nEditType = 5;
}

void CEventEdit_SelectEditType::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	CheckRadioButton( IDC_RADIO1, IDC_RADIO6, IDC_RADIO1 );
}
