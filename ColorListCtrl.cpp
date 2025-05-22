// ColorListCtrl.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ColorListCtrl.h"


// CColorListCtrl

IMPLEMENT_DYNAMIC(CColorListCtrl, CXTListCtrl)

CColorListCtrl::CColorListCtrl()
{
	m_nColorColumnIndex = 1;
}

CColorListCtrl::~CColorListCtrl()
{
}


BEGIN_MESSAGE_MAP(CColorListCtrl, CXTListCtrl)
END_MESSAGE_MAP()



// CColorListCtrl 메시지 처리기입니다.



void CColorListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// TODO:  지정된 항목을 그리는 코드를 추가합니다.
	if ( (lpDrawItemStruct->itemAction & ODA_DRAWENTIRE) ||
		((!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT)))
		)
	{
		SetAutoFont( true );
		int nColumnCount = GetColumnCount();
		CRect *Rect = new CRect[nColumnCount];

		int nWidthAdd = 0;
		for( int i=0; i<nColumnCount; i++ ) {
			Rect[i].SetRect( lpDrawItemStruct->rcItem.left + nWidthAdd, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.left + nWidthAdd + GetColumnWidth(i), lpDrawItemStruct->rcItem.bottom );
			nWidthAdd += GetColumnWidth(i);
		}

		for( int i=0; i<nColumnCount; i++ ) {
			if( i == m_nColorColumnIndex ) {
				int BGColor=0;
				int BGSideColor=0;
				int SelColor=0;
				int SelSideColor=0;
				CString szStr = GetItemText( lpDrawItemStruct->itemID, m_nColorColumnIndex );
				sscanf_s( szStr.GetBuffer(), "%d|%d|%d|%d", &BGColor, &BGSideColor, &SelColor, &SelSideColor );

				Rect[i].left += 2, Rect[i].right -= 2; Rect[i].bottom -= 1;

				pDC->Draw3dRect( &Rect[i], BGSideColor, BGSideColor );
				Rect[i].DeflateRect( 1, 1, 1, 1 );
				pDC->FillSolidRect( &Rect[i], BGColor );
			}
			else {
				Rect[i].left += 2, Rect[i].top -= 1; Rect[i].right -= 2;

				DrawText( pDC, GetItemText( lpDrawItemStruct->itemID, i ), RGB( 0, 0, 0 ), RGB(247, 243, 233), Rect[i], DT_LEFT | DT_VCENTER );
			}
		}
		SAFE_DELETEA( Rect );
	}


	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{	
		int nColumnCount = GetColumnCount();
		CRect *Rect = new CRect[nColumnCount];

		int nWidthAdd = 0;
		for( int i=0; i<nColumnCount; i++ ) {
			Rect[i].SetRect( lpDrawItemStruct->rcItem.left + nWidthAdd, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.left + nWidthAdd + GetColumnWidth(i), lpDrawItemStruct->rcItem.bottom );
			nWidthAdd += GetColumnWidth(i);
		}

		for( int i=0; i<nColumnCount; i++ ) {
			if( i == m_nColorColumnIndex ) {
				int BGColor=0;
				int BGSideColor=0;
				int SelColor=0;
				int SelSideColor=0;
				CString szStr = GetItemText( lpDrawItemStruct->itemID, m_nColorColumnIndex );
				sscanf_s( szStr.GetBuffer(), "%d|%d|%d|%d", &BGColor, &BGSideColor, &SelColor, &SelSideColor );

				Rect[i].left += 2, Rect[i].right -= 2; Rect[i].bottom -= 1;

				pDC->Draw3dRect( &Rect[i], SelSideColor, SelSideColor );
				Rect[i].DeflateRect( 1, 1, 1, 1 );
				pDC->FillSolidRect( &Rect[i], SelColor );
			}
			else {
				Rect[i].left += 2, Rect[i].top -= 1; Rect[i].right -= 2;

				/*
				CHeaderCtrl* pHeaderCtrl = _xtGetHeaderCtrl();
				HDITEM *hd_item = new HDITEM;
				pHeaderCtrl->GetItem(i, hd_item);

				int nFormat; 
				if( hd_item->lParam & LVCFMT_LEFT ) nFormat = DT_LEFT | DT_VCENTER;
				else if( hd_item->lParam & LVCFMT_RIGHT ) nFormat = DT_RIGHT | DT_VCENTER;
				else if( hd_item->lParam & LVCFMT_CENTER ) nFormat = DT_CENTER | DT_VCENTER;
				nFormat = DT_CENTER | DT_VCENTER;
				*/

				DrawText( pDC, GetItemText( lpDrawItemStruct->itemID, i ), RGB( 255, 255, 255 ), RGB(106, 36, 10), Rect[i], DT_LEFT | DT_VCENTER );
//				delete hd_item;
			}
		}
		SAFE_DELETEA( Rect );
	}
}

void CColorListCtrl::DrawText( CDC *pDC, CString &szString, DWORD dwCol, DWORD dwBg, RECT Rect, UINT nDT )
{
	pDC->FillSolidRect( &Rect, dwBg );
	pDC->SetTextColor( dwCol );

	pDC->DrawText( szString, &Rect, nDT );
}

void CColorListCtrl::DrawFrameBorder(CDC* pDC, LPRECT rect)
{
	pDC->Draw3dRect(rect, RGB(220,220,220), RGB(0,0,0));
}