// ParamInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ParamInputDlg.h"
#include "GlobalValue.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "TEtWorldEventControl.h"
#include "TEtWorldEventArea.h"
#include "TEtTrigger.h"
#include "UserMessage.h"


// CParamInputDlg dialog

IMPLEMENT_DYNAMIC(CParamInputDlg, CDialog)

CParamInputDlg::CParamInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParamInputDlg::IDD, pParent)
{
	m_StartPos = CPoint( -1, -1 );
	m_Type = (CEtTrigger::ParamTypeEnum)-1;
	m_pVariable = NULL;
}

CParamInputDlg::~CParamInputDlg()
{
	SAFE_DELETE( m_pVariable );
}

void CParamInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParamInputDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED( 1001, &OnBnClickedPick )
	ON_BN_CLICKED( IDOK, &OnBnClickedOK )
	ON_BN_CLICKED( IDCANCEL, &OnBnClickedCancel )
END_MESSAGE_MAP()


// CParamInputDlg message handlers

BOOL CParamInputDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CParamInputDlg::SetInitFlag( CEtTrigger::ParamTypeEnum Type, CPoint &p, CUnionValueProperty *pDefaultVariable )
{
	m_Type = Type;
	m_StartPos = p;
	switch( Type ) {
		case CEtTrigger::Integer: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
		case CEtTrigger::Float: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Float ); break;
		case CEtTrigger::String: m_pVariable = new CUnionValueProperty( CUnionValueProperty::String ); break;
		case CEtTrigger::Operator: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
		case CEtTrigger::Position: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Vector3 ); break;
		case CEtTrigger::Prop: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
		case CEtTrigger::EventArea: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
		case CEtTrigger::Value: m_pVariable = new CUnionValueProperty( CUnionValueProperty::Integer ); break;
	}
	*m_pVariable = *pDefaultVariable;
}

BOOL CParamInputDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rcRect;
	GetClientRect( &rcRect );
	// TODO:  Add extra initialization here
	int nWidth = 150, nHeight = 24;
	
	switch( m_Type ) {
		case CEtTrigger::Integer: 
			{
				CRect rcControl = CRect( 50, 3, nWidth - 4, 21 );
				m_szDesc = "Integer";
				m_EditBox.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_RIGHT, rcControl, this, 1001 );
				m_EditBox.ShowWindow( SW_SHOW );
				m_EditBox.SetFont( GetParent()->GetFont() );
				m_EditBox.SetWindowText( *m_pVariable->GetBindStr() );
			}
			break;
		case CEtTrigger::Float: 
			{
				CRect rcControl = CRect( 50, 3, nWidth - 4, 21 );
				m_szDesc = "Float"; 
				m_EditBox.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_RIGHT, rcControl, this, 1001 );
				m_EditBox.ShowWindow( SW_SHOW );
				m_EditBox.SetFont( GetParent()->GetFont() );
				m_EditBox.SetWindowText( *m_pVariable->GetBindStr() );
			}
			break;
		case CEtTrigger::String: 
			{
				CRect rcControl = CRect( 50, 3, nWidth - 4, 21 );
				m_szDesc = "String"; 
				m_EditBox.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL | ES_AUTOHSCROLL, rcControl, this, 1001 );
				m_EditBox.ShowWindow( SW_SHOW );
				m_EditBox.SetFont( GetParent()->GetFont() );
				m_EditBox.SetWindowText( *m_pVariable->GetBindStr() );
			}
			break;
		case CEtTrigger::Operator: 
			{
				nHeight = 26;
				CRect rcControl = CRect( 50, 3, nWidth - 4, 200 );
				m_szDesc = "Operator"; 
				m_Combo.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | CBN_DROPDOWN | WS_VSCROLL, rcControl, this, 1001 );
				m_Combo.InsertString( m_Combo.GetCount(), "==" );
				m_Combo.InsertString( m_Combo.GetCount(), "!=" );
				m_Combo.InsertString( m_Combo.GetCount(), ">" );
				m_Combo.InsertString( m_Combo.GetCount(), "<" );
				m_Combo.InsertString( m_Combo.GetCount(), ">=" );
				m_Combo.InsertString( m_Combo.GetCount(), "<=" );

				m_Combo.SetCurSel( m_pVariable->GetVariableInt() );
				m_Combo.ShowWindow( SW_SHOW );
				m_Combo.SetFont( GetParent()->GetFont() );
			}
			break;
		case CEtTrigger::Position:
			{
				nWidth = 250;
				nHeight = 45;

				m_szDesc = "Position"; 
				CRect rcControl = CRect( 50, 3, nWidth - 4, 21 );
				m_Button.Create( "Pick", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rcControl, this, 1001 );
				m_Button.SetFont( GetParent()->GetFont() );

				rcControl = CRect( 50, 24, nWidth - 4, 42 );
				m_EditBox.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rcControl, this, 1002 );
				CString szStr;
				szStr.Format( "%.2f; %.2f; %.2f", m_pVariable->GetVariableVector3().x, m_pVariable->GetVariableVector3().y, m_pVariable->GetVariableVector3().z );
				m_EditBox.SetWindowText( szStr );
				m_EditBox.SetFont( GetParent()->GetFont() );

			}
			break; 
		case CEtTrigger::Prop: 
			{
				nWidth = 350;
				nHeight = 50;

				m_szDesc = "Prop"; 
				CRect rcControl = CRect( 50, 3, nWidth - 4, 21 );
				m_Button.Create( "Pick", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rcControl, this, 1001 );
				m_Button.SetFont( GetParent()->GetFont() );

				rcControl = CRect( 50, 24, nWidth - 4, 200 );
				m_Combo.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | CBN_DROPDOWN | WS_VSCROLL, rcControl, this, 1002 );

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				CString szTemp;
				for( DWORD i=0; i<pSector->GetPropCount(); i++ ) {
					CTEtWorldProp *pProp = (CTEtWorldProp*)pSector->GetPropFromIndex(i);
					szTemp.Format( "%s (%d)", pProp->GetPropName(), pProp->GetCreateUniqueID() );
					m_Combo.InsertString( m_Combo.GetCount(), szTemp );
				}

				m_Combo.SetCurSel(0);
				m_Combo.SetFont( GetParent()->GetFont() );
			}
			break;
		case CEtTrigger::EventArea: 
			{
				nWidth = 250;
				nHeight = 50;
				m_szDesc = "EventArea"; 
				CRect rcControl = CRect( 70, 3, nWidth - 4, 21 );
				m_Button.Create( "Pick", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rcControl, this, 1001 );
				m_Button.SetFont( GetParent()->GetFont() );

				rcControl = CRect( 70, 24, nWidth - 4, 200 );
				m_Combo.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | CBN_DROPDOWN | WS_VSCROLL, rcControl, this, 1002 );

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				CString szTemp;
				int nSelectIndex = -1;
				int nOffset = 0;
				for( DWORD i=0; i<pSector->GetControlCount(); i++ ) {
					CTEtWorldEventControl *pControl = (CTEtWorldEventControl *)pSector->GetControlFromIndex(i);
					for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
						CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(j);
						szTemp.Format( "%s - %s(%d)", pControl->GetName(), pArea->GetName(), pArea->GetCreateUniqueID() );
						m_Combo.InsertString( m_Combo.GetCount(), szTemp );
						if( pArea->GetCreateUniqueID() == m_pVariable->GetVariableInt() ) {
							nSelectIndex = nOffset;
						}
						nOffset++;
					}
				}
				if( nSelectIndex == -1 ) nSelectIndex = 0;
				m_Combo.SetCurSel(nSelectIndex);
				m_Combo.SetFont( GetParent()->GetFont() );

			}
			break;
		case CEtTrigger::Value:
			{
				nHeight = 26;
				nWidth = 250;
				CRect rcControl = CRect( 50, 3, nWidth - 4, 200 );
				m_szDesc = "Value"; 
				m_Combo.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | CBN_DROPDOWN | WS_VSCROLL, rcControl, this, 1001 );

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();

				CEtTrigger::DefineValue *pValue;
				for( DWORD i=0; i<pTrigger->GetDefineValueCount(); i++ ) {
					pValue = pTrigger->GetDefineValueFromIndex(i);
					m_Combo.InsertString( m_Combo.GetCount(), pValue->szName.c_str() );
					if( pValue->nUniqueID == m_pVariable->GetVariableInt() ) {
						m_Combo.SetCurSel( i );
					}
				}

				m_Combo.ShowWindow( SW_SHOW );
				m_Combo.SetFont( GetParent()->GetFont() );
			}
			break;
	}

	MoveWindow( m_StartPos.x, m_StartPos.y, nWidth, nHeight );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CParamInputDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	CRect rcRect;
	GetClientRect( &rcRect );
	dc.FillSolidRect( &rcRect, RGB( 255, 255, 220 ) );
	dc.Draw3dRect( &rcRect, RGB( 0, 0, 0 ), RGB( 0, 0, 0 ) );

	CFont *pFontParent = GetParent()->GetFont();
	CFont *pOldFont = dc.SelectObject( pFontParent );

	dc.SetBkMode( TRANSPARENT );
	dc.SetTextColor( RGB( 0, 0, 0 ) );
	dc.TextOut( 3, 4, m_szDesc );

	dc.SelectObject( pOldFont );
}


void CParamInputDlg::OnBnClickedPick()
{
	OutputDebug( "¹öÆ° ³î·È´Ù" );

	CGlobalValue::GetInstance().m_nPickType = m_Type;

	CDialog::OnOK();
}

void CParamInputDlg::OnBnClickedOK()
{
	OutputDebug( "¿ÀÄÉÀÌ" );
	CGlobalValue::GetInstance().m_nPickType = -1;

	switch( m_Type ) {
		case CEtTrigger::Integer:
			{
				CString szStr;
				m_EditBox.GetWindowText( szStr );
				m_pVariable->SetVariable( (int)atoi(szStr) );
			}
			break;
		case CEtTrigger::Float: 
			{
				CString szStr;
				m_EditBox.GetWindowText( szStr );
				m_pVariable->SetVariable( (float)atof(szStr) );
			}
			break;
		case CEtTrigger::String: 
			{
				CString szStr;
				m_EditBox.GetWindowText( szStr );
				m_pVariable->SetVariable( (char*)szStr.GetBuffer() );
			}
			break;
		case CEtTrigger::Operator:
			m_pVariable->SetVariable( (int)m_Combo.GetCurSel() );
			break;
		case CEtTrigger::Position:
			{
				CString szStr;
				m_EditBox.GetWindowText( szStr );
				EtVector3 vPos;
				sscanf_s( szStr, "%f; %f; %f", &vPos.x, &vPos.y, &vPos.z );

				m_pVariable->SetVariable( vPos );
			}
			break;
		case CEtTrigger::Prop:
			{
				/*
				CString szStr;
				m_EditBox.GetWindowText( szStr );
				EtVector3 vPos;
				sscanf_s( szStr, "%f; %f; %f", &vPos.x, &vPos.y, &vPos.z );
				*/
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				CEtWorldProp *pProp = pSector->GetPropFromIndex( m_Combo.GetCurSel() );
				m_pVariable->SetVariable( pProp->GetCreateUniqueID() );
			}
			break;
		case CEtTrigger::EventArea:
			{
				CString szStr;
				int nIndex = -1;
				char szTemp[2][256];
				m_Combo.GetLBText( m_Combo.GetCurSel(), szStr );
				sscanf_s( "%s - %s(%d)", szTemp[0], szTemp[1], &nIndex );
				m_pVariable->SetVariable( nIndex );
			}
			break;
		case CEtTrigger::Value:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();

				CString szStr;
				m_Combo.GetLBText( m_Combo.GetCurSel(), szStr );

				CEtTrigger::DefineValue *pValue;
				for( DWORD i=0; i<pTrigger->GetDefineValueCount(); i++ ) {
					pValue = pTrigger->GetDefineValueFromIndex(i);
					if( strcmp( szStr.GetBuffer(), pValue->szName.c_str() ) == NULL ) {
						m_pVariable->SetVariable( pValue->nUniqueID );
					}
				}
			}
			break;
	}

	CDialog::OnOK();
}

void CParamInputDlg::OnBnClickedCancel()
{
	OutputDebug( "ÄË¼ú" );
	CGlobalValue::GetInstance().m_nPickType = -1;
	CDialog::OnCancel();
}
