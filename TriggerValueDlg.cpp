// TriggerValueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerValueDlg.h"
#include "TEtTrigger.h"


// CTriggerValueDlg dialog

IMPLEMENT_DYNAMIC(CTriggerValueDlg, CDialog)

CTriggerValueDlg::CTriggerValueDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerValueDlg::IDD, pParent)
{

}

CTriggerValueDlg::~CTriggerValueDlg()
{
}

void CTriggerValueDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
	DDX_Control(pDX, IDC_EDIT1, m_Name);
	DDX_Control(pDX, IDC_EDIT4, m_DefaultValue);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTriggerValueDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CTriggerValueDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CTriggerValueDlg message handlers

BOOL CTriggerValueDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEtTrigger::DefineValue *pValue = GetValue();
	switch( pValue->ParamType ) {
		case CEtTrigger::Integer: 
			{
				m_Combo.SetCurSel(0);
				char szStr[256] = { 0, };
				_itoa_s( pValue->nValue, szStr, 10 );
				m_DefaultValue.SetWindowText( szStr );
			}
			break;
		case CEtTrigger::Float: 
			{
				m_Combo.SetCurSel(1); 
				char szStr[256] = { 0, };
				int nDecimal;
				int nSign;
				_fcvt_s( szStr, sizeof(szStr), pValue->fValue, 2, &nDecimal, &nSign );
				m_DefaultValue.SetWindowText( szStr );
			}
			break;
		case CEtTrigger::String: 
			m_Combo.SetCurSel(2); 
			m_DefaultValue.SetWindowText( pValue->szValue );
			break;
	}
	m_Name.SetWindowText( pValue->szName.c_str() );

	return TRUE;
}

void CTriggerValueDlg::OnCbnSelchangeCombo1()
{
	CEtTrigger::DefineValue *pValue = GetValue();
	CString szStr;

	m_Combo.GetLBText( m_Combo.GetCurSel(), szStr );
	if( szStr == "Integer" ) {
		m_DefaultValue.SetWindowText( "0" );
		pValue->ParamType = CEtTrigger::Integer;
	}
	else if( szStr == "Float" ) {
		m_DefaultValue.SetWindowText( "0.0" );
		pValue->ParamType = CEtTrigger::Float;
	}
	else if( szStr == "String" ) {
		m_DefaultValue.SetWindowText( "None" );
		pValue->ParamType = CEtTrigger::String;
	}
}

void CTriggerValueDlg::OnOK()
{
	CEtTrigger::DefineValue *pValue = GetValue();

	CString szStr;

	if( pValue->ParamType == CEtTrigger::String ) {
		SAFE_DELETEA( pValue->szValue );
	}

	m_Combo.GetLBText( m_Combo.GetCurSel(), szStr );
	if( szStr == "Integer" ) {
		pValue->ParamType = CEtTrigger::Integer;
	}
	else if( szStr == "Float" ) {
		pValue->ParamType = CEtTrigger::Float;
	}
	else if( szStr == "String" ) {
		pValue->ParamType = CEtTrigger::String;
	}


	m_Name.GetWindowText( szStr );
	pValue->szName = szStr;

	m_DefaultValue.GetWindowText( szStr );

	switch( pValue->ParamType ) {
		case CEtTrigger::Integer:
			pValue->nValue = atoi( szStr );
			break;
		case CEtTrigger::Float:
			pValue->fValue = (float)atof( szStr );
			break;
		case CEtTrigger::String:
			{
				int nSize = szStr.GetLength();
				pValue->szValue = new char[nSize + 1];
				memset( pValue->szValue, 0, nSize + 1 );
				memcpy( pValue->szValue, szStr.GetBuffer(), nSize );
			}
			break;
	}
	CDialog::OnOK();
}

void CTriggerValueDlg::OnCancel()
{
	CDialog::OnCancel();
}
