// SoundListTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundListTreeCtrl.h"
#include "UserMessage.h"



// SoundListTreeCtrl

IMPLEMENT_DYNAMIC(SoundListTreeCtrl, CXTTreeCtrl)

SoundListTreeCtrl::SoundListTreeCtrl()
{

}

SoundListTreeCtrl::~SoundListTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(SoundListTreeCtrl, CXTTreeCtrl)
	ON_MESSAGE( UM_SOUNDPANE_DROPITEM, OnDropItem )
	ON_MESSAGE( UM_SOUNDPANE_DROPMOVEITEM, OnDropMoveItem )
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// SoundListTreeCtrl message handlers



LRESULT SoundListTreeCtrl::OnDropItem( WPARAM wParam, LPARAM lParam )
{
	return GetParent()->SendMessage( UM_SOUNDPANE_DROPITEM, wParam, lParam );
}

LRESULT SoundListTreeCtrl::OnDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	return GetParent()->SendMessage( UM_SOUNDPANE_DROPMOVEITEM, wParam, lParam );
}

void SoundListTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch( nChar ) {
		case VK_F2:
		{
			CTypedPtrList<CPtrList, HTREEITEM> SelectItemList;
			GetSelectedList( SelectItemList );

			if( SelectItemList.GetCount() == 1 ) {
				HTREEITEM hti = SelectItemList.GetHead();
				HTREEITEM hParent = GetParentItem( hti );
				if( !hParent ) break;
				if( GetItemText( hParent ) != "Environment" ) break;

				EditLabel( hti);
			}
		}
		break;
	}

	CXTTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
