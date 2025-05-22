// TriggerSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerSelectDlg.h"
#include "ScriptControl.h"
#include "TEtTriggerElement.h"
#include "ParamInputDlg.h"
#include "InputDevice.h"


// CTriggerSelectDlg dialog

IMPLEMENT_DYNAMIC(CTriggerSelectDlg, CXTResizeDialog)

CTriggerSelectDlg::CTriggerSelectDlg(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CTriggerSelectDlg::IDD, pParent)
{
	m_pElement = NULL;
	m_bModify = false;
	m_nPrevComboIndex = 0;
	m_nPrevCheck = 0;
}

CTriggerSelectDlg::~CTriggerSelectDlg()
{
	CInputDevice::GetInstance().ResetAllInput();
}

void CTriggerSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
	DDX_Control(pDX, IDC_RICHEDIT21, m_CustomScript);
	DDX_Control(pDX, IDC_DESC_STATIC, m_ParamDesc);
}


BEGIN_MESSAGE_MAP(CTriggerSelectDlg, CXTResizeDialog)
	ON_BN_CLICKED(IDC_CHECK1, &CTriggerSelectDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CTriggerSelectDlg::OnBnClickedCheck2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CTriggerSelectDlg::OnCbnSelchangeCombo1)
	ON_MESSAGE( UM_SCRIPTSTATIC_CLICK, OnClickParam )
	ON_BN_CLICKED(IDOK, &CTriggerSelectDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_RICHEDIT21, &CTriggerSelectDlg::OnEnChangeRichedit21)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTriggerSelectDlg message handlers

void CTriggerSelectDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	CButton *pButton = (CButton *)GetDlgItem( IDC_CHECK2 );
	pButton->SetCheck( FALSE );

	pButton = (CButton *)GetDlgItem( IDC_CHECK1 );
	pButton->SetCheck( TRUE );

	m_Combo.EnableWindow( TRUE );
	m_CustomScript.EnableWindow( FALSE );

	m_ParamDesc.SetScriptStruct( m_pElement );
	m_ParamDesc.Invalidate();
	m_nPrevCheck = 0;

	m_pElement->SetScriptType( CEtTriggerElement::ScriptFile );
}

void CTriggerSelectDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
	CButton *pButton = (CButton *)GetDlgItem( IDC_CHECK1 );
	pButton->SetCheck( FALSE );

	pButton = (CButton *)GetDlgItem( IDC_CHECK2 );
	pButton->SetCheck( TRUE );

	m_ParamDesc.SetScriptStruct( NULL );
	m_ParamDesc.Invalidate();
	m_Combo.EnableWindow( FALSE );
	m_CustomScript.EnableWindow( TRUE );

	m_nPrevCheck = 1;
	m_pElement->SetScriptType( CEtTriggerElement::CustomScript );
}

BOOL CTriggerSelectDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CXTResizeDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CTriggerSelectDlg::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	SetResize( IDC_STATIC1, SZ_TOP_LEFT, SZ_TOP_RIGHT );
	SetResize( IDC_STATIC2, SZ_TOP_LEFT, SZ_BOTTOM_RIGHT );
	SetResize( IDC_COMBO1, SZ_TOP_LEFT, SZ_TOP_RIGHT );
	SetResize( IDC_CHECK1, SZ_TOP_LEFT, SZ_TOP_RIGHT );
	SetResize( IDC_CHECK2, SZ_TOP_LEFT, SZ_TOP_RIGHT );
	SetResize( IDC_DESC_STATIC, SZ_TOP_LEFT, SZ_TOP_RIGHT );
	SetResize( IDC_RICHEDIT21, SZ_TOP_LEFT, SZ_BOTTOM_RIGHT );

	SetResize( IDOK, SZ_BOTTOM_RIGHT, SZ_BOTTOM_RIGHT );
	SetResize( IDCANCEL, SZ_BOTTOM_RIGHT, SZ_BOTTOM_RIGHT );


	// TODO:  Add extra initialization here
	BOOL bValid = ( m_pElement->GetScriptType() == CEtTriggerElement::ScriptFile ) ? TRUE : FALSE;
	m_nPrevCheck = !bValid;
	CButton *pButton = (CButton *)GetDlgItem( IDC_CHECK1 );
	pButton->SetCheck( bValid );
	pButton = (CButton *)GetDlgItem( IDC_CHECK2 );
	pButton->SetCheck( !bValid );

	m_Combo.EnableWindow( bValid );
	m_CustomScript.EnableWindow( !bValid );

	m_CustomScript.SetFont( GetParent()->GetFont() );

	PARAFORMAT pf ; 
	pf.cbSize = sizeof(PARAFORMAT); 
	pf.dwMask = PFM_TABSTOPS ; 
	pf.cTabCount = MAX_TAB_STOPS;
	for( int itab = 0 ; itab < pf.cTabCount ; itab++ ) 
		pf.rgxTabs[itab]= ((itab+1)*100*3);

	m_CustomScript.SetParaFormat( pf ); 

	int nSelectCombo = -1;
	switch( m_pElement->GetType() ) {
		case 0:
			{
				DWORD dwCount = CScriptControl::GetInstance().GetDefineConditionCount();
				std::string szString;
				for( DWORD i=0; i<dwCount; i++ ) {
					szString.clear();
					CScriptControl::DefineScriptStruct *pStruct = CScriptControl::GetInstance().GetDefineConditionStructFromIndex(i);
					for( DWORD j=0; j<pStruct->VecStringList.size(); j++ ) {
						if( pStruct->VecStringList[j].Type != -1 ) szString += "[";
						szString += pStruct->VecStringList[j].szStr;
						if( pStruct->VecStringList[j].Type != -1 ) szString += "]";
					}
					m_Combo.InsertString( m_Combo.GetCount(), szString.c_str() );
					if( m_pElement && strlen( m_pElement->GetFileName() ) > 0 && nSelectCombo == -1 ) {
						if( _stricmp( m_pElement->GetFileName(), pStruct->szFileName.c_str() ) == NULL ) {
							nSelectCombo = i;
						}
					}
				}
			}
			break;
		case 1:
			{
				DWORD dwCount = CScriptControl::GetInstance().GetDefineActionCount();
				std::string szString;
				for( DWORD i=0; i<dwCount; i++ ) {
					szString.clear();
					CScriptControl::DefineScriptStruct *pStruct = CScriptControl::GetInstance().GetDefineActionStructFromIndex(i);
					for( DWORD j=0; j<pStruct->VecStringList.size(); j++ ) {
						if( pStruct->VecStringList[j].Type != -1 ) szString += "[";
						szString += pStruct->VecStringList[j].szStr;
						if( pStruct->VecStringList[j].Type != -1 ) szString += "]";
					}
					m_Combo.InsertString( m_Combo.GetCount(), szString.c_str() );
					if( m_pElement && strlen( m_pElement->GetFileName() ) > 0 && nSelectCombo == -1 ) {
						if( _stricmp( m_pElement->GetFileName(), pStruct->szFileName.c_str() ) == NULL ) {
							nSelectCombo = i;
						}
					}
				}
			}
			break;
		case 2:
			{
				DWORD dwCount = CScriptControl::GetInstance().GetDefineEventCount();
				std::string szString;
				for( DWORD i=0; i<dwCount; i++ ) {
					szString.clear();
					CScriptControl::DefineScriptStruct *pStruct = CScriptControl::GetInstance().GetDefineEventStructFromIndex(i);
					for( DWORD j=0; j<pStruct->VecStringList.size(); j++ ) {
						if( pStruct->VecStringList[j].Type != -1 ) szString += "[";
						szString += pStruct->VecStringList[j].szStr;
						if( pStruct->VecStringList[j].Type != -1 ) szString += "]";
					}
					m_Combo.InsertString( m_Combo.GetCount(), szString.c_str() );
					if( m_pElement && strlen( m_pElement->GetFileName() ) > 0 && nSelectCombo == -1 ) {
						if( _stricmp( m_pElement->GetFileName(), pStruct->szFileName.c_str() ) == NULL ) {
							nSelectCombo = i;
						}
					}
				}
			}
			break;
	}
	m_bModify = true;
	if( nSelectCombo == -1 ) {
		m_bModify = false;
		nSelectCombo = 0;
	}

	m_Combo.SetCurSel(nSelectCombo);
	m_nPrevComboIndex = -1;
	OnCbnSelchangeCombo1();

	long mask = m_CustomScript.GetEventMask();
	m_CustomScript.SetEventMask(mask |= ENM_CHANGE );

	LPTSTR sKeywords = "for,for,else,main,struct,enum,switch,auto,"
		"template,explicit,this,bool,extern,thread,break,false,"
		"throw,case,namespace,true,catch,new,try,float,noreturn,"
		"char,operator,typedef,class,friend,private,const,goto,"
		"protected,typename,if,public,union,continue,inline,"
		"unsigned,using,directive,default,int,return,delete,short"
		"signed,virtual,sizeof,void,do,static,double,long,while";
	LPTSTR sDirectives = "#define,#elif,#else,#endif,#error,#ifdef,"
		"#ifndef,#import,#include,#line,#pragma,#undef";
	LPTSTR sPragmas = "comment,optimize,auto_inline,once,warning,"
		"component,pack,function,intrinsic,setlocale,hdrstop,message,end";

	m_sc.ClearKeywordList();
	m_sc.AddKeyword(sKeywords,RGB(0,0,255),0);
	m_sc.AddKeyword(sDirectives,RGB(0,0,255),1);
	m_sc.AddKeyword(sPragmas,RGB(0,0, 255),2);

	m_CustomScript.SetWindowText( m_pElement->GetCustomScript() );
//	m_sc.Colorize( 0, -1, &m_CustomScript );
	for( int i=0; i<m_CustomScript.GetLineCount(); i++ ) {
		int nLength = m_CustomScript.LineLength(i);
		int nStart = m_CustomScript.LineIndex(i);

		m_sc.Colorize( nStart, nStart + nLength, &m_CustomScript );
	}

	LoadPlacement(_T("CTriggerSelectDlg"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTriggerSelectDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	if( m_nPrevComboIndex == m_Combo.GetCurSel() ) return;
	if( m_pElement->GetScriptType() == CEtTriggerElement::ScriptFile ) {
		CScriptControl::DefineScriptStruct *pStruct = NULL;
		switch( m_pElement->GetType() ) {
			case 0: pStruct = CScriptControl::GetInstance().GetDefineConditionStructFromIndex( m_Combo.GetCurSel() ); break;
			case 1: pStruct = CScriptControl::GetInstance().GetDefineActionStructFromIndex( m_Combo.GetCurSel() ); break;
			case 2: pStruct = CScriptControl::GetInstance().GetDefineEventStructFromIndex( m_Combo.GetCurSel() ); break;
		}
		if( !m_bModify && m_pElement ) {
			if( pStruct ) {
				m_pElement->SetFileName( pStruct->szFileName.c_str() );
			}
		}
	}
	m_bModify = false;
	m_ParamDesc.SetScriptStruct( m_pElement );
	m_ParamDesc.Invalidate();
	m_nPrevComboIndex = m_Combo.GetCurSel();
}


void CTriggerSelectDlg::SetTriggerElement( CTEtTriggerElement *pElement )
{
	m_pElement = pElement;
}

LRESULT CTriggerSelectDlg::OnClickParam( WPARAM wParam, LPARAM lParam )
{
	int nParamIndex = (int)wParam;
	CPoint p = *(CPoint*)lParam;

	CScriptControl::DefineScriptStruct *pStruct = NULL;
	switch( m_pElement->GetType() ) {
		case 0: pStruct = CScriptControl::GetInstance().GetDefineConditionStructFromIndex( m_Combo.GetCurSel() ); break;
		case 1: pStruct = CScriptControl::GetInstance().GetDefineActionStructFromIndex( m_Combo.GetCurSel() ); break;
		case 2: pStruct = CScriptControl::GetInstance().GetDefineEventStructFromIndex( m_Combo.GetCurSel() ); break;
	}
	if( pStruct == NULL ) return S_OK;

	CParamInputDlg Dlg;
	CEtTrigger::ParamTypeEnum Type = pStruct->GetParamStruct(nParamIndex)->Type;
	Dlg.SetInitFlag( Type, p, m_pElement->GetParamVariable(nParamIndex) );
	if( Dlg.DoModal() == IDOK ) {
		if( CGlobalValue::GetInstance().m_nPickType == -1 ) {
			CUnionValueProperty *pVariable = Dlg.GetVariable();
			if( pVariable == NULL ) {
				assert(0);
				return S_OK;
			}
			m_pElement->SetParamVariable( nParamIndex, pVariable );
			m_ParamDesc.Invalidate();
		}
		else {
			CGlobalValue::GetInstance().m_nPickParamIndex = nParamIndex;
			CGlobalValue::GetInstance().m_pPickStandbyElement = m_pElement;
			CDialog::OnOK();
		}
	}
	return S_OK;
}
void CTriggerSelectDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if( m_nPrevCheck == 0 ) {
		for( DWORD i=0; i<m_pElement->GetParamVariableCount(); i++ ) {
			if( m_pElement->IsModifyParam(i) == false ) {
				MessageBox( "설정하지 않은 파라메터가 있습니다.", "다 채워!!", MB_OK );
				return;
			}
		}
	}
	else {
		CString szStr;
		m_CustomScript.GetWindowText( szStr );
		m_pElement->SetCustomScript( szStr );
	}
	CGlobalValue::GetInstance().SetModify();

	CDialog::OnOK();
}

void CTriggerSelectDlg::OnEnChangeRichedit21()
{
	CHARRANGE cr;
	m_CustomScript.GetSel(cr);

	int len = m_CustomScript.LineLength();
	int start = m_CustomScript.LineIndex();

	//call the colorizer
	m_sc.Colorize( start, start + len, &m_CustomScript );

	m_CustomScript.SetSel(cr);
}

BOOL CTriggerSelectDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB)) {
		int nPos = LOWORD(m_CustomScript.CharFromPos(m_CustomScript.GetCaretPos()));
		m_CustomScript.SetSel(nPos, nPos);
		m_CustomScript.ReplaceSel("\t", TRUE);
		return TRUE;
	}

	return CXTResizeDialog::PreTranslateMessage(pMsg);
}

void CTriggerSelectDlg::OnDestroy()
{
	CXTResizeDialog::OnDestroy();

	// TODO: Add your message handler code here
	SavePlacement(_T("CTriggerSelectDlg"));
}