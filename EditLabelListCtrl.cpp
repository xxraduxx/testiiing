// EditLavelListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EditLabelListCtrl.h"


// CEditLabelListCtrl

IMPLEMENT_DYNAMIC(CEditLabelListCtrl, CXTListCtrl)

CEditLabelListCtrl::CEditLabelListCtrl()
{

}

CEditLabelListCtrl::~CEditLabelListCtrl()
{
}


BEGIN_MESSAGE_MAP(CEditLabelListCtrl, CXTListCtrl)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// CEditLabelListCtrl message handlers




void CEditLabelListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch( nChar )
	{
	case VK_F2:
		{
			// To Use F2 as hot Key to get EditCtrl on the ListView it must have 
			// the Style LVS_EDITLABELS
			ASSERT( GetStyle() & LVS_EDITLABELS );

			// don't do an Edit Label when the multiple Items are selected
			if( GetSelectedCount() == 1 )
			{
				UINT nListSelectedItem = GetSelectedItem();
				VERIFY( EditLabel( nListSelectedItem ) != NULL );
			}
			else
				CXTListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
		}
		break;

	default:
		CXTListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
		break;
	}

}

// this Function Returns the first Selected Item In the List
UINT CEditLabelListCtrl::GetSelectedItem()
{
	// this Function Valid Only when a Single Item Is Selected
	ASSERT( GetSelectedCount( ) == 1 );

	UINT nNoOfItems = GetItemCount( );

	UINT nListItem;
	for( nListItem = 0; nListItem < nNoOfItems; nListItem++ )
		if( GetItemState( nListItem, LVIS_SELECTED )  )
			break;

	ASSERT( nListItem < nNoOfItems );
	return nListItem;
}
