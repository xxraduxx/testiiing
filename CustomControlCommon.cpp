#include "StdAfx.h"
#include "CustomControlCommon.h"

void DDX_CustomControl( CDataExchange *pDX, int nID, CWnd &control )
{
	CRect rcRect;
	pDX->m_pDlgWnd->GetDlgItem( nID )->GetClientRect( &rcRect );

	control.Create( NULL, NULL, WS_VISIBLE, rcRect, pDX->m_pDlgWnd, 0xffff );
}