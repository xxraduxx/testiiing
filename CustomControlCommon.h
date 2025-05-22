#pragma once


void DDX_CustomControl( CDataExchange *pDX, int nID, CWnd &control );

#define CUSTOM_Control( nID, control ) \
{ \
	CRect rcCurRect, rcChildRect, rcRect; \
	GetWindowRect( &rcCurRect );	\
	GetDlgItem( nID )->GetWindowRect( &rcChildRect );	\
	GetDlgItem( nID )->GetClientRect( &rcRect );	\
	rcRect.left += rcChildRect.left - rcCurRect.left;	\
	rcRect.right += rcChildRect.left - rcCurRect.left ;	\
	rcRect.top += rcChildRect.top - rcCurRect.top;	\
	rcRect.bottom += rcChildRect.top - rcCurRect.top;	\
	CString szStr;	\
	GetDlgItem( nID )->GetWindowText( szStr ); \
	control.Create( NULL, szStr, WS_VISIBLE | WS_CHILD, rcRect, this, nID ); \
} 
