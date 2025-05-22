// PropEdit_SelectEditType.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropEdit_SelectEditType.h"


// CPropEdit_SelectEditType

IMPLEMENT_DYNCREATE(CPropEdit_SelectEditType, CFormView)

CPropEdit_SelectEditType::CPropEdit_SelectEditType()
	: CFormView(CPropEdit_SelectEditType::IDD)
{
	m_nEditType = 0;
}

CPropEdit_SelectEditType::~CPropEdit_SelectEditType()
{
}

void CPropEdit_SelectEditType::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropEdit_SelectEditType, CFormView)
	ON_BN_CLICKED(IDC_RADIO1, &CPropEdit_SelectEditType::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPropEdit_SelectEditType::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CPropEdit_SelectEditType::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CPropEdit_SelectEditType::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &CPropEdit_SelectEditType::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &CPropEdit_SelectEditType::OnBnClickedRadio6)
END_MESSAGE_MAP()


// CPropEdit_SelectEditType diagnostics

#ifdef _DEBUG
void CPropEdit_SelectEditType::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEdit_SelectEditType::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropEdit_SelectEditType message handlers

CPropEditWizardCommon::VIEW_TYPE CPropEdit_SelectEditType::GetNextView()
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

bool CPropEdit_SelectEditType::IsVisivleNextButton()
{
	return true;
}

void CPropEdit_SelectEditType::PreProcess()
{
}

bool CPropEdit_SelectEditType::PostProcess()
{
	switch( m_nEditType ) {
		case 0: // Create
			{
				s_pCurrentSignalItem = new CPropSignalItem(-1);
			}
			break;
		case 1: // Modify
			{
				s_pCurrentSignalItem = new CPropSignalItem(-1);
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

void CPropEdit_SelectEditType::OnBnClickedRadio1()
{
	m_nEditType = 0;
}

void CPropEdit_SelectEditType::OnBnClickedRadio2()
{
	m_nEditType = 1;
}

void CPropEdit_SelectEditType::OnBnClickedRadio3()
{
	m_nEditType = 2;
}

void CPropEdit_SelectEditType::OnBnClickedRadio4()
{
	m_nEditType = 3;
}

void CPropEdit_SelectEditType::OnBnClickedRadio5()
{
	m_nEditType = 4;
}

void CPropEdit_SelectEditType::OnBnClickedRadio6()
{
	m_nEditType = 5;
}

void CPropEdit_SelectEditType::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	CheckRadioButton( IDC_RADIO1, IDC_RADIO6, IDC_RADIO1 );
}
