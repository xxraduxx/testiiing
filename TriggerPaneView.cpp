// TriggerPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "UserMessage.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "FileServer.h"
#include "TEtTrigger.h"
#include "TEtTriggerObject.h"
#include "TEtTriggerElement.h"


// CTriggerPaneView

IMPLEMENT_DYNCREATE(CTriggerPaneView, CFormView)

CTriggerPaneView::CTriggerPaneView()
	: CFormView(CTriggerPaneView::IDD)
{
	m_bActivate = false;

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_BITMAP3 );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 5, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );

	m_pDragImage = NULL;
	m_bDragging = false;
	m_hDrag = 0;
	m_hDrop = 0;
	m_bClipObjectCut = false;
	m_bCopyDrag = false;
}

CTriggerPaneView::~CTriggerPaneView()
{
	SAFE_DELETE( m_pContextMenu );
}

void CTriggerPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
}

BEGIN_MESSAGE_MAP(CTriggerPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_TRIGGERLIST_ADDCATEGORY, &CTriggerPaneView::OnTriggerlistAddcategory)
	ON_COMMAND(ID_TRIGGERLIST_ADDTRIGGER, &CTriggerPaneView::OnTriggerlistAddtrigger)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, &CTriggerPaneView::OnTvnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, &CTriggerPaneView::OnTvnEndlabeleditTree1)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CTriggerPaneView::OnTvnSelchangedTree1)
	ON_MESSAGE( UM_ENABLE_PANE_VIEW, OnEnablePane )
	ON_COMMAND(ID_TRIGGERLIST_DELETE, &CTriggerPaneView::OnTriggerlistDelete)
	ON_WM_KEYDOWN()
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE1, &CTriggerPaneView::OnTvnBegindragTree1)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_TRIGGERLIST_COPY, &CTriggerPaneView::OnTriggerlistCopy)
	ON_COMMAND(ID_TRIGGERLIST_CUT, &CTriggerPaneView::OnTriggerlistCut)
	ON_COMMAND(ID_TRIGGERLIST_PASTE, &CTriggerPaneView::OnTriggerlistPaste)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE1, &CTriggerPaneView::OnTvnKeydownTree1)
END_MESSAGE_MAP()


// CTriggerPaneView diagnostics

#ifdef _DEBUG
void CTriggerPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTriggerPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTriggerPaneView message handlers

void CTriggerPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_TreeCtrl ) {
		m_TreeCtrl.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

void CTriggerPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	SendMessage( UM_REFRESH_PANE_VIEW );
}

BOOL CTriggerPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

namespace TempStruct {
	struct CategoryStruct {
		CString szStr;
		HTREEITEM hti;
	};

};
LRESULT CTriggerPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( !m_TreeCtrl ) return S_OK;
	m_TreeCtrl.DeleteAllItems();
	m_TreeCtrl.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_TreeCtrl.EnableMultiSelect( true );
	if( wParam == 0 ) m_pVecClipObjectList.clear();

	m_hRootTree = m_TreeCtrl.InsertItem( "Trigger", 0, 1 );

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return S_OK;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return S_OK;

using namespace TempStruct;
	std::vector<CategoryStruct> VecCategory;

	for( DWORD i=0; i<pTrigger->GetTriggerCount(); i++ ) {
		CTEtTriggerObject *pObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromIndex(i);
		int nIndex = -1;
		for( DWORD j=0; j<VecCategory.size(); j++ ) {
			if( strcmp( VecCategory[j].szStr, pObject->GetCategory() ) == NULL ) {
				nIndex = j;
				break;
			}
		}
		if( nIndex == -1 ) {
			CategoryStruct Struct;
			Struct.szStr = pObject->GetCategory();
			Struct.hti = m_TreeCtrl.InsertItem( Struct.szStr, 0, 1, m_hRootTree );
			VecCategory.push_back( Struct );
		}
	}
	m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
	for( DWORD i=0; i<pTrigger->GetTriggerCount(); i++ ) {
		CTEtTriggerObject *pObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromIndex(i);

		int nIndex = -1;
		for( DWORD j=0; j<VecCategory.size(); j++ ) {
			if( strcmp( VecCategory[j].szStr, pObject->GetCategory() ) == NULL ) {
				nIndex = j;
				break;
			}
		}
		if( nIndex == -1 ) assert(0);
		m_TreeCtrl.InsertItem( pObject->GetTriggerName(), 2, 2, VecCategory[nIndex].hti );
//		m_TreeCtrl.Expand( VecCategory[nIndex].hti, TVE_EXPAND );
	}
	return S_OK;
}

void CTriggerPaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
	if( CFileServer::GetInstance().IsConnect() == false ) return;
	if( CGlobalValue::GetInstance().m_nPickType != -1 ) return;
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(5);

	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_ADDCATEGORY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_ADDTRIGGER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_PASTE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERLIST_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
		CalcSelectItem();
		if( m_SelectItemList.GetCount() == 1 ) {
			HTREEITEM hti = m_SelectItemList.GetHead();
			CString szName = m_TreeCtrl.GetItemText( hti );

			if( hti == m_hRootTree ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERLIST_ADDCATEGORY, MF_BYCOMMAND | MF_ENABLED );
			}
			else {
				if( pTrigger->GetTriggerFromName( szName ) == NULL ) {
					pSubMenu->EnableMenuItem( ID_TRIGGERLIST_ADDTRIGGER, MF_BYCOMMAND | MF_ENABLED );
				}
				if( !m_pVecClipObjectList.empty() ) pSubMenu->EnableMenuItem( ID_TRIGGERLIST_PASTE, MF_BYCOMMAND | MF_ENABLED );
			}
		}
		if( m_SelectItemList.GetCount() >= 1 ) {
			bool bValid = true;
			POSITION p = m_SelectItemList.GetHeadPosition();
			HTREEITEM hti;
			while(p) {
				hti = m_SelectItemList.GetNext(p);
				if( hti == m_hRootTree ) {
					bValid = false;
					break;
				}
			}
			if( bValid ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERLIST_DELETE, MF_BYCOMMAND | MF_ENABLED );
				pSubMenu->EnableMenuItem( ID_TRIGGERLIST_COPY, MF_BYCOMMAND | MF_ENABLED );
				pSubMenu->EnableMenuItem( ID_TRIGGERLIST_CUT, MF_BYCOMMAND | MF_ENABLED );
			}
		}
	}

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void CTriggerPaneView::OnTriggerlistAddcategory()
{
	// TODO: Add your command handler code here
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	CString szName;
	int nCount = 0;
	while(1) {
		szName.Format( "New Category - %03d", nCount );
		HTREEITEM hti = m_TreeCtrl.FindItem( szName, TRUE, TRUE, TRUE, m_hRootTree );
		if( hti == NULL ) break;
		nCount++;
	}

	HTREEITEM hti = m_TreeCtrl.InsertItem( szName, 0, 1, m_hRootTree );

	m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
	m_TreeCtrl.SelectItems( hti, hti, TRUE );

	CGlobalValue::GetInstance().SetModify();
}

void CTriggerPaneView::OnTriggerlistAddtrigger()
{
	// TODO: Add your command handler code here
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	CString szName = pSector->GetUniqueTriggerName();
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	HTREEITEM hParent = m_SelectItemList.GetHead();
	CString szCategory = m_TreeCtrl.GetItemText( hParent );

	pTrigger->InsertTrigger( szCategory, szName );
	HTREEITEM hti = m_TreeCtrl.InsertItem( szName, 2, 2, hParent );

	m_TreeCtrl.Expand( hParent, TVE_EXPAND );
	m_TreeCtrl.SelectItems( hti, hti, TRUE );

	CGlobalValue::GetInstance().SetModify();
}

void CTriggerPaneView::CalcSelectItem()
{
	m_SelectItemList.RemoveAll();
	m_TreeCtrl.GetSelectedList( m_SelectItemList );
}

void CTriggerPaneView::OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_szPrevLabelString = pTVDispInfo->item.pszText;
}

void CTriggerPaneView::OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if( m_hRootTree == pTVDispInfo->item.hItem ) return;
	if( pTVDispInfo->item.pszText == NULL ) return;

	if( !CGlobalValue::GetInstance().IsCheckOutMe() ) return;
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( m_szPrevLabelString );
	if( pObject ) {
		CEtTriggerObject *pNewObject = pTrigger->GetTriggerFromName( pTVDispInfo->item.pszText );
		if( pNewObject ) return;
		((CTEtTriggerObject*)pObject)->SetTriggerName( pTVDispInfo->item.pszText );
		m_TreeCtrl.SetItemText( pTVDispInfo->item.hItem, pTVDispInfo->item.pszText );
		CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 0, (LPARAM)pObject );
		pWnd = GetPaneWnd( TRIGGER_ATTR_PANE );
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pObject, 0 );
	}
	else {
		HTREEITEM hti = m_TreeCtrl.FindItem( pTVDispInfo->item.pszText, TRUE, TRUE, TRUE );
		if( hti ) {
			if( hti != pTVDispInfo->item.hItem ) return;
		}
		pTrigger->ChangeCategoryName( m_szPrevLabelString, pTVDispInfo->item.pszText );
		m_TreeCtrl.SetItemText( pTVDispInfo->item.hItem, pTVDispInfo->item.pszText );
	}
	CGlobalValue::GetInstance().SetModify();
}

void CTriggerPaneView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	CWnd *pWndAttr = GetPaneWnd( TRIGGER_ATTR_PANE );

	CalcSelectItem();
	if( m_SelectItemList.GetCount() == 1 ) {
		HTREEITEM hti = m_SelectItemList.GetHead();
		CString szName = m_TreeCtrl.GetItemText( hti );

		if( m_hRootTree == hti ) {
			if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );
			if( pWndAttr ) pWndAttr->SendMessage( UM_REFRESH_PANE_VIEW );
		}
		else {
			CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szName );
			if( pObject ) {
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 0, (LPARAM)pObject );
				if( pWndAttr ) pWndAttr->SendMessage( UM_REFRESH_PANE_VIEW, (WPARAM)pObject );
			}
		}
	}
	else {
		if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1 );
		if( pWndAttr ) pWndAttr->SendMessage( UM_REFRESH_PANE_VIEW );
	}
}

LRESULT CTriggerPaneView::OnEnablePane( WPARAM wParam, LPARAM lParam )
{
	m_TreeCtrl.EnableWindow( (BOOL)wParam );
	return S_OK;
}

void CTriggerPaneView::OnTriggerlistDelete()
{
	// TODO: Add your command handler code here
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	for( DWORD i=0; i<m_pVecClipObjectList.size(); i++ ) {
		HTREEITEM hti = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
		}
	}
	m_pVecClipObjectList.clear();

	POSITION p = m_SelectItemList.GetHeadPosition();
	std::vector<HTREEITEM> hVecList;
	HTREEITEM hti;
	CString szStr, szCategory;
	std::vector<HTREEITEM> hVecCategory;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree ) continue;
		hVecList.push_back( hti );
	}
	for( DWORD i=0; i<hVecList.size(); i++ ) {
		szStr = m_TreeCtrl.GetItemText( hVecList[i] );
		CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szStr );
		if( pObject ) {
			HTREEITEM hParent = m_TreeCtrl.GetParentItem( hVecList[i] );
			szCategory = m_TreeCtrl.GetItemText( hParent );
			pTrigger->RemoveTrigger( szCategory, szStr );
			m_TreeCtrl.DeleteItem( hVecList[i] );
		}
		else {
			pTrigger->RemoveTrigger( szStr );
			if( std::find( hVecCategory.begin(), hVecCategory.end(), hVecList[i] ) == hVecCategory.end() ) {
				hVecCategory.push_back( hVecList[i] );
			}
		}
	}
	for( DWORD i=0; i<hVecCategory.size(); i++ ) {
		m_TreeCtrl.DeleteItem( hVecCategory[i] );
	}

	CGlobalValue::GetInstance().SetModify();

	CWnd *pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	CWnd *pWndAttr = GetPaneWnd( TRIGGER_ATTR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, -1, -1 );
	if( pWndAttr ) pWndAttr->SendMessage( UM_REFRESH_PANE_VIEW );
}

void CTriggerPaneView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTriggerPaneView::OnTvnBegindragTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if( m_TreeCtrl.GetParentItem(pNMTreeView->itemNew.hItem) == NULL ) return; 

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	m_hDrag = pNMTreeView->itemNew.hItem;
	if( m_hDrag == m_hRootTree ) return;
//	CString szObjectName = m_TreeCtrl.GetItemText( m_hDrag );
//	CTEtTriggerObject *pElement = (CTEtTriggerObject *)pTrigger->GetTriggerFromName( szObjectName.GetBuffer() );
//	if( !pElement ) return;

	// get the image list for dragging
	m_pDragImage = m_TreeCtrl.CreateDragImage( m_hDrag );

	m_bDragging = true;
	m_pDragImage->BeginDrag( 0, CPoint(-15,0) );

	POINT pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	m_pDragImage->DragEnter( NULL, pt );
	SetCapture();
	if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) m_bCopyDrag = true;
	else m_bCopyDrag = false;

	*pResult = 0;
}

void CTriggerPaneView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	HTREEITEM	hitem;
	UINT		flags;

	if( m_bDragging ) {
		POINT pt = point;
		ClientToScreen( &pt );
		CImageList::DragMove(pt);


		if( (hitem = m_TreeCtrl.HitTest(point, &flags) ) != NULL ) {
			CImageList::DragShowNolock(FALSE);

			SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
			if( Index == -1 ) return;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
			if( pSector == NULL ) return;
			CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
			if( pTrigger == NULL ) return;

			CString szDragObject = m_TreeCtrl.GetItemText( m_hDrag );
			CTEtTriggerObject *pObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromName( szDragObject );

			bool bValid = true;
			if( pObject ) {
				if( m_hRootTree == hitem ) bValid = false;
			}
			else {
				CString szDrop = m_TreeCtrl.GetItemText( hitem );
				pObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromName( szDrop );
				if( pObject ) bValid = false;
			}

			if( bValid ) {
				m_TreeCtrl.SelectDropTarget(hitem);
				m_hDrop = hitem;
			}
			else {
				m_TreeCtrl.SelectDropTarget(NULL);
				m_hDrop = NULL;
			}

			CImageList::DragShowNolock( TRUE );
		}
	}

	CFormView::OnMouseMove(nFlags, point);
}

void CTriggerPaneView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if( m_bDragging ) {
		m_bDragging = false;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();

		if( m_pDragImage != NULL ) { 
			delete m_pDragImage; 
			m_pDragImage = NULL; 
		} 

		m_TreeCtrl.SelectDropTarget(NULL);

		if( m_hDrag != m_hDrop ) {
			HTREEITEM hitem;
			POINT pt2 = point;

			if( ((hitem = m_TreeCtrl.HitTest(pt2, &nFlags)) == NULL)  ) return ;

			SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
			if( Index == -1 ) return;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
			if( pSector == NULL ) return;
			CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
			if( pTrigger == NULL ) return;

			CString szDrop = m_TreeCtrl.GetItemText( hitem );
			CString szDrag = m_TreeCtrl.GetItemText( m_hDrag );
			CTEtTriggerObject *pObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromName( szDrop );
			CTEtTriggerObject *pDragObject = (CTEtTriggerObject *)pTrigger->GetTriggerFromName( szDrag );

			if( pDragObject ) {
				if( m_hRootTree == hitem ) return;
			}
			else {
				if( pObject ) return;
			}

			CGlobalValue::GetInstance().SetModify();
			if( pDragObject == NULL ) {
				if( m_hDrop == m_hRootTree ) {
					pTrigger->ChangeCategoryOrder( szDrag, "" );
				}
				else {
					pTrigger->ChangeCategoryOrder( szDrag, szDrop );
				}
				SendMessage( UM_REFRESH_PANE_VIEW, 1 );
//				return;
			}
			else {
				if( m_bCopyDrag ) {
					if( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) m_bCopyDrag = false;
				}

				if( m_bCopyDrag ) {
					CTEtTriggerObject *pCopy = (CTEtTriggerObject *)pTrigger->AllocTriggerObject();
					*pCopy = *pDragObject;
					CString szCopy;
					int nCount = 0;
					while(1) {
						szCopy.Format( "Copy to %s %d", pCopy->GetTriggerName(), nCount );
						if( pTrigger->GetTriggerFromName( szCopy ) == NULL ) break;
						nCount++;
					}
					pCopy->SetTriggerName( szCopy );

					pDragObject = pCopy;
					pTrigger->InsertTrigger( pCopy );
				}
				if( pObject == NULL ) {
					if( _stricmp( szDrop, pDragObject->GetCategory() ) == NULL ) {
						pTrigger->ChangeTriggerOrder( pDragObject, szDrop );
					}
					else {
						pDragObject->SetCategory( szDrop );
						pTrigger->ChangeTriggerOrder( pDragObject, szDrop );
					}
					if( !m_bCopyDrag ) m_TreeCtrl.DeleteItem( m_hDrag );
					m_TreeCtrl.InsertItem( pDragObject->GetTriggerName(), 2, 2, hitem, TVI_FIRST );
				}
				else {
					if( _stricmp( pObject->GetCategory(), pDragObject->GetCategory() ) != NULL ) {
						pDragObject->SetCategory( pObject->GetCategory() );
					}
					pTrigger->ChangeTriggerOrder( pDragObject, pObject );


					HTREEITEM hCategory = m_TreeCtrl.FindItem( pObject->GetCategory(), TRUE, TRUE, TRUE, m_hRootTree );
//					HTREEITEM hti = m_TreeCtrl.FindItem( pObject->GetTriggerName(), TRUE, TRUE, TRUE, hCategory );
					if( !m_bCopyDrag ) m_TreeCtrl.DeleteItem( m_hDrag );
					m_TreeCtrl.InsertItem( pDragObject->GetTriggerName(), 2, 2, hCategory, hitem );
				}
			}
		}
	}
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;
	HTREEITEM hCategory = m_TreeCtrl.GetChildItem( m_hRootTree );
	std::vector<CString> szVecCategoryList;
	while( hCategory != NULL ) {
		CString szName = m_TreeCtrl.GetItemText( hCategory );
		hCategory = m_TreeCtrl.GetNextItem( hCategory );
		CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szName );
		if( pObject == NULL ) {
			szVecCategoryList.push_back( szName );
		}
		else {
			bool bExist = false;
			for( DWORD i=0; i<szVecCategoryList.size(); i++ ) {
				if( strcmp( szVecCategoryList[i].GetBuffer(), pObject->GetCategory() ) == NULL ) {
					bExist = true;
					break;
				}
			}
			if( !bExist ) {
				szVecCategoryList.push_back( pObject->GetCategory() );
			}
		}
	}
	pTrigger->SortCategory( szVecCategoryList );
	m_bCopyDrag = false;

	OutputDebug( "------------------------------------------------\n" );
	for( DWORD i=0; i<pTrigger->GetTriggerCount(); i++ ) {
		OutputDebug( "%d - %s ( %s )\n", i, pTrigger->GetTriggerFromIndex(i)->GetTriggerName(), pTrigger->GetTriggerFromIndex(i)->GetCategory() );
	}

	CFormView::OnLButtonUp(nFlags, point);
}

void CTriggerPaneView::OnTriggerlistCopy()
{
	for( DWORD i=0; i<m_pVecClipObjectList.size(); i++ ) {
		HTREEITEM hti = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
		}
	}
	m_pVecClipObjectList.clear();

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr, szCategory;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree ) continue;
		szStr = m_TreeCtrl.GetItemText( hti );
		CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szStr );
		if( pObject ) {
			if( std::find( m_pVecClipObjectList.begin(), m_pVecClipObjectList.end(), pObject ) == m_pVecClipObjectList.end() ) {
				m_pVecClipObjectList.push_back( pObject );
			}
		}
		else {
			for( DWORD i=0; i<pTrigger->GetTriggerCount(); i++ ) {
				if( _stricmp( pTrigger->GetTriggerFromIndex(i)->GetCategory(), szStr ) == NULL ) {
					if( std::find( m_pVecClipObjectList.begin(), m_pVecClipObjectList.end(), pTrigger->GetTriggerFromIndex(i) ) == m_pVecClipObjectList.end() ) {
						m_pVecClipObjectList.push_back( pTrigger->GetTriggerFromIndex(i) );
					}
				}
			}
		}
	}
	m_bClipObjectCut = false;
}

void CTriggerPaneView::OnTriggerlistCut()
{
	for( DWORD i=0; i<m_pVecClipObjectList.size(); i++ ) {
		HTREEITEM hti = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
		}
	}
	m_pVecClipObjectList.clear();

	// TODO: Add your command handler code here
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr, szCategory;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree ) continue;
		szStr = m_TreeCtrl.GetItemText( hti );
		CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szStr );
		if( pObject ) {
			if( std::find( m_pVecClipObjectList.begin(), m_pVecClipObjectList.end(), pObject ) == m_pVecClipObjectList.end() ) {
				m_TreeCtrl.SetItemState( hti, TVIS_CUT, TVIS_CUT );
				m_pVecClipObjectList.push_back( pObject );
			}
		}
		else {
			for( DWORD i=0; i<pTrigger->GetTriggerCount(); i++ ) {
				if( _stricmp( pTrigger->GetTriggerFromIndex(i)->GetCategory(), szStr ) == NULL ) {
					if( std::find( m_pVecClipObjectList.begin(), m_pVecClipObjectList.end(), pTrigger->GetTriggerFromIndex(i) ) == m_pVecClipObjectList.end() ) {
						HTREEITEM hChild = m_TreeCtrl.FindItem( pTrigger->GetTriggerFromIndex(i)->GetTriggerName(), TRUE, TRUE, TRUE, hti );
						if( hChild ) {
							m_TreeCtrl.SetItemState( hChild, TVIS_CUT, TVIS_CUT );
							m_pVecClipObjectList.push_back( pTrigger->GetTriggerFromIndex(i) );
						}
					}
				}
			}
		}
	}
	m_bClipObjectCut = true;
}

void CTriggerPaneView::OnTriggerlistPaste()
{
	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree ) return;
		szStr = m_TreeCtrl.GetItemText( hti );
		break;
	}

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;
	CTEtTrigger *pTrigger = (CTEtTrigger *)pSector->GetTrigger();
	if( pTrigger == NULL ) return;


	// TODO: Add your command handler code here
	if( m_bClipObjectCut ) {
		for( DWORD i=0; i<m_pVecClipObjectList.size(); i++ ) {
			HTREEITEM hTemp = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
			if( hTemp ) {
				m_TreeCtrl.SetItemState( hTemp, 0, TVIS_CUT );
			}
		}

		HTREEITEM hParent;
		HTREEITEM hOrder;
		for( int i=(int)m_pVecClipObjectList.size()-1; i>=0; i-- ) {
			if( pTrigger->GetTriggerFromName( szStr ) ) {
				hParent = m_TreeCtrl.GetParentItem( hti );
				hOrder = hti;

				CString szCategory = m_TreeCtrl.GetItemText( hParent );
				((CTEtTriggerObject*)m_pVecClipObjectList[i])->SetCategory( szCategory );
				pTrigger->ChangeTriggerOrder( (CTEtTriggerObject*)m_pVecClipObjectList[i], (CTEtTriggerObject*)pTrigger->GetTriggerFromName( szStr ) );
			}
			else {
				hParent = hti;
				hOrder = TVI_FIRST;
				((CTEtTriggerObject*)m_pVecClipObjectList[i])->SetCategory( szStr );
				pTrigger->ChangeTriggerOrder( (CTEtTriggerObject*)m_pVecClipObjectList[i], szStr );
			}
			HTREEITEM hClip = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
			m_TreeCtrl.DeleteItem( hClip );
			m_TreeCtrl.InsertItem( ((CTEtTriggerObject*)m_pVecClipObjectList[i])->GetTriggerName(), 2, 2, hParent, hOrder );

		}
		m_pVecClipObjectList.clear();
	}
	else {
		for( int i=(int)m_pVecClipObjectList.size()-1; i>=0; i-- ) {
			CTEtTriggerObject *pCopy = (CTEtTriggerObject *)pTrigger->AllocTriggerObject();
			*pCopy = *(CTEtTriggerObject*)m_pVecClipObjectList[i];
			CString szCopy;
			int nCount = 0;
			while(1) {
				szCopy.Format( "Copy to %s %d", pCopy->GetTriggerName(), nCount );
				if( pTrigger->GetTriggerFromName( szCopy ) == NULL ) break;
				nCount++;
			}
			pCopy->SetTriggerName( szCopy );
			pTrigger->InsertTrigger( pCopy );
			m_pVecClipObjectList[i] = pCopy;

			HTREEITEM hParent;
			HTREEITEM hOrder;
			if( pTrigger->GetTriggerFromName( szStr ) ) {
				hParent = m_TreeCtrl.GetParentItem( hti );
				hOrder = hti;

				CString szCategory = m_TreeCtrl.GetItemText( hParent );
				((CTEtTriggerObject*)m_pVecClipObjectList[i])->SetCategory( szCategory );
				pTrigger->ChangeTriggerOrder( (CTEtTriggerObject*)m_pVecClipObjectList[i], (CTEtTriggerObject*)pTrigger->GetTriggerFromName( szStr ) );
			}
			else {
				hParent = hti;
				hOrder = TVI_FIRST;
				((CTEtTriggerObject*)m_pVecClipObjectList[i])->SetCategory( szStr );
				pTrigger->ChangeTriggerOrder( (CTEtTriggerObject*)m_pVecClipObjectList[i], szStr );
			}
			m_TreeCtrl.InsertItem( ((CTEtTriggerObject*)m_pVecClipObjectList[i])->GetTriggerName(), 2, 2, hParent, hOrder );

		}
	}

	HTREEITEM hCategory = m_TreeCtrl.GetChildItem( m_hRootTree );
	std::vector<CString> szVecCategoryList;
	while( hCategory != NULL ) {
		CString szName = m_TreeCtrl.GetItemText( hCategory );
		hCategory = m_TreeCtrl.GetNextItem( hCategory );
		CEtTriggerObject *pObject = pTrigger->GetTriggerFromName( szName );
		if( pObject == NULL ) {
			szVecCategoryList.push_back( szName );
		}
		else {
			bool bExist = false;
			for( DWORD i=0; i<szVecCategoryList.size(); i++ ) {
				if( strcmp( szVecCategoryList[i].GetBuffer(), pObject->GetCategory() ) == NULL ) {
					bExist = true;
					break;
				}
			}
			if( !bExist ) {
				szVecCategoryList.push_back( pObject->GetCategory() );
			}
		}
	}
	pTrigger->SortCategory( szVecCategoryList );
}

void CTriggerPaneView::OnTvnKeydownTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	if( pTVKeyDown->wVKey == VK_DELETE ) {
		OnTriggerlistDelete();
	}
	else {
		if( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) {
			if( pTVKeyDown->wVKey == 0x43 ) { // Copy
				CalcSelectItem();
				OnTriggerlistCopy();
			}
			if( pTVKeyDown->wVKey == 0x58 ) { // Cut
				CalcSelectItem();
				OnTriggerlistCut();
			}
			if( pTVKeyDown->wVKey == 0x56 ) { // Paste
				CalcSelectItem();
				OnTriggerlistPaste();
			}
		}
	}
	*pResult = 0;
}

BOOL CTriggerPaneView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			for( DWORD i=0; i<m_pVecClipObjectList.size(); i++ ) {
				HTREEITEM hti = m_TreeCtrl.FindItem( m_pVecClipObjectList[i]->GetTriggerName(), TRUE, TRUE, TRUE );
				if( hti ) {
					m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
				}
			}
			m_pVecClipObjectList.clear();
		}
	}

	return CFormView::PreTranslateMessage(pMsg);
}
