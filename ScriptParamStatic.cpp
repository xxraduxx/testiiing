// ScriptParamStatic.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "ScriptParamStatic.h"
#include "ParamInputDlg.h"
#include "TEtTrigger.h"
#include "TEtTriggerObject.h"
#include "TEtTriggerElement.h"

// CScriptParamStatic

IMPLEMENT_DYNAMIC(CScriptParamStatic, CStatic)

CScriptParamStatic::CScriptParamStatic()
{
	m_bCreateFont = false;
	_bMouseInControl = false;

	_hHandCursor = ::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND));
	_hArrowCursor = ::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));
	m_pElement = NULL;
	m_pStruct = NULL;
}

CScriptParamStatic::~CScriptParamStatic()
{
}


BEGIN_MESSAGE_MAP(CScriptParamStatic, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE( UM_PARAMINPUT_PICK, OnParamPick )
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CScriptParamStatic message handlers


void CScriptParamStatic::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if( InCaptionRange(point) && m_pElement ) {
		int nParamIndex = GetCaptionIndex(point);
		CPoint p = point;
		ClientToScreen( &p );
		GetParent()->SendMessage( UM_SCRIPTSTATIC_CLICK, nParamIndex, (LPARAM)&p );
	}
	CStatic::OnLButtonDown(nFlags, point);
}

void CScriptParamStatic::OnPaint() 
{	
	CreateFont();

	CPaintDC dc(this);
	CFont *pOldFont = dc.GetCurrentFont();
	dc.SetBkMode( TRANSPARENT );

	int nColorIndex = WINVER >= 0x0501 ? COLOR_MENUBAR : COLOR_ACTIVEBORDER;
	CRect rcRect;
	GetClientRect( &rcRect );
	dc.FillSolidRect( &rcRect, (COLORREF)GetSysColor(nColorIndex) );
	std::string szStr;
	if( m_pStruct ) {
		int nOffset = 0;
		int nX = 0;
		for( DWORD i=0; i<m_pStruct->VecStringList.size(); i++ ) {
			CScriptControl::ScriptStringStruct *pStruct = &m_pStruct->VecStringList[i];
			if( pStruct->Type == (CEtTrigger::ParamTypeEnum)-1 ) {
				dc.SelectObject(&m_Font[0]);
				dc.SetTextColor(RGB(0,0,0));

				szStr = pStruct->szStr;
			}
			else {
				dc.SelectObject(&m_Font[1]);
				dc.SetTextColor(RGB(0,0,255));

				if( m_pElement->IsModifyParam( nOffset ) ) {
					szStr = m_pElement->MakeString(nOffset);
				}
				else {
					szStr = pStruct->szStr;
				}
				nOffset++;
			}
			dc.TextOut(nX, 0, szStr.c_str());

			CSize size = dc.GetTextExtent( szStr.c_str() );
			nX += size.cx;
		}
	}
	dc.SelectObject(pOldFont);
}

void CScriptParamStatic::OnDestroy() 
{
	CStatic::OnDestroy();
	for( int i=0; i<2; i++ ) m_Font[i].DeleteObject();
}

void CScriptParamStatic::PreSubclassWindow() 
{
	ModifyStyle(0, SS_NOTIFY, TRUE);
	CStatic::PreSubclassWindow();
}

LRESULT CScriptParamStatic::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	_bMouseInControl = false;
	::SetCursor(_hArrowCursor);	
	return 0;
}

void CScriptParamStatic::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( _bMouseInControl == false ) {
		//Track the mouse leave event
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = GetSafeHwnd();
		tme.dwFlags = TME_LEAVE;        
		_TrackMouseEvent(&tme);
		_bMouseInControl = true;
	}
	else {
		::SetCursor((InCaptionRange(point))?_hHandCursor:_hArrowCursor);		
	}
	CStatic::OnMouseMove(nFlags, point);
}


void CScriptParamStatic::CreateFont()
{
	if( m_bCreateFont ) return;
	CFont* pFontParent = GetParent()->GetFont();	
	if ( pFontParent ) {
		LOGFONT lf;
		pFontParent->GetObject(sizeof(lf), &lf);
		lf.lfUnderline = FALSE;
		m_Font[0].CreateFontIndirect(&lf);
		lf.lfUnderline = TRUE;
		m_Font[1].CreateFontIndirect(&lf);
		m_bCreateFont = true;
	}
}

bool CScriptParamStatic::InCaptionRange(CPoint &point)
{
	if( GetCaptionIndex( point ) == -1 ) return false;
	return true;
}

int CScriptParamStatic::GetCaptionIndex(CPoint &point)
{
	if( m_pStruct ) {
		std::vector<int> nVecOffset;

		CClientDC dc(this);
		CFont *pOldFont = dc.GetCurrentFont();
		int nOffset = 0;
		int nX = 0;
		int nYOrder = 0;
		for( DWORD i=0; i<m_pStruct->VecStringList.size(); i++ ) {
			CScriptControl::ScriptStringStruct *pStruct = &m_pStruct->VecStringList[i];
			if( pStruct->Type == (CEtTrigger::ParamTypeEnum)-1 ) {
				dc.SelectObject(&m_Font[0]);
				CSize size = dc.GetTextExtent( pStruct->szStr.c_str() );
				nX += size.cx;
			}
			else {
				dc.SelectObject(&m_Font[1]);

				CSize size;
				if( m_pElement->IsModifyParam( nOffset ) ) {
					size = dc.GetTextExtent( m_pElement->MakeString( nOffset ).c_str() );
//					size = dc.GetTextExtent( *m_pElement->GetParamVariable(nOffset)->GetBindStr() );
				}
				else size = dc.GetTextExtent( pStruct->szStr.c_str() );

				nVecOffset.push_back( nX );
				nX += size.cx;
				nVecOffset.push_back( nX );

				if( nYOrder == 0 ) nYOrder = size.cy;
				nOffset++;
			}
		}
		dc.SelectObject( pOldFont );


		bool bCheck = false;
		for( DWORD i=0; i<nVecOffset.size(); i+=2 ) {
			if( (( point.x >= nVecOffset[i] )&&( point.x < nVecOffset[i+1] ) && ( point.y >= 0 )&&( point.y < nYOrder )) ) {
				return i / 2;
			}
		}
	}
	return -1;
}


void CScriptParamStatic::SetScriptStruct( CTEtTriggerElement *pElement )
{
	m_pElement = pElement;
	if( m_pElement == NULL ) {
		m_pStruct = NULL;
		return;
	}
	switch( m_pElement->GetType() ) {
		case CEtTriggerElement::Condition: m_pStruct = CScriptControl::GetInstance().GetDefineConditionStructFromName( m_pElement->GetFileName() ); break;
		case CEtTriggerElement::Action: m_pStruct = CScriptControl::GetInstance().GetDefineActionStructFromName( m_pElement->GetFileName() ); break;
		case CEtTriggerElement::Event: m_pStruct = CScriptControl::GetInstance().GetDefineEventStructFromName( m_pElement->GetFileName() ); break;
	}
}

int CScriptParamStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

LRESULT CScriptParamStatic::OnParamPick( WPARAM wParam, LPARAM lParam )
{
	ShowWindow( SW_HIDE );
	return S_OK;
}