// CTriggerPropertiesPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "TriggerPropertiesPaneView.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldProp.h"
#include "UserMessage.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "TEtTrigger.h"
#include "TEtTriggerObject.h"
#include "FileServer.h"
#include "TriggerSelectDlg.h"
#include "TEtTriggerElement.h"
#include "EtWorldEventArea.h"
#include "TriggerOperatorDlg.h"

// CTriggerPropertiesPaneView

IMPLEMENT_DYNCREATE(CTriggerPropertiesPaneView, CFormView)

CTriggerPropertiesPaneView::CTriggerPropertiesPaneView()
	: CFormView(CTriggerPropertiesPaneView::IDD)
{
	m_bActivate = false;
	m_bEnable = false;
	m_pTriggerObj = NULL;

	CBitmap bitmap;
	bitmap.LoadBitmap( IDB_BITMAP3 );

	m_ImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 8, 1 );
	m_ImageList.Add( &bitmap, RGB(0,255,0) );

	m_pContextMenu = new CMenu;
	m_pContextMenu->LoadMenu( IDR_CONTEXTMENU );
	m_bLastAddElement = false;
	m_pOriginalElement = NULL;
	m_pDragImage = NULL;
	m_bDragging = false;
	m_hDrag = 0;
	m_hDrop = 0;

	m_bCopyDrag = false;
	m_bClipObjectCut = false;
}

CTriggerPropertiesPaneView::~CTriggerPropertiesPaneView()
{
	SAFE_DELETE( m_pContextMenu );
}

void CTriggerPropertiesPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_DescriptionCtrl);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
}

BEGIN_MESSAGE_MAP(CTriggerPropertiesPaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_COMMAND(ID_TRIGGERPROPERTIES_ADDACTION, &CTriggerPropertiesPaneView::OnTriggerpropertiesAddaction)
	ON_COMMAND(ID_TRIGGERPROPERTIES_ADDCONDITION, &CTriggerPropertiesPaneView::OnTriggerpropertiesAddcondition)
	ON_COMMAND(ID_TRIGGERPROPERTIES_ADDEVENT, &CTriggerPropertiesPaneView::OnTriggerpropertiesAddevent)
	ON_MESSAGE( UM_TRIGGERPROP_PANE_PICK_OBJECT, OnPickObject )
	ON_COMMAND(ID_TRIGGERPROPERTIES_PROPERTIES, &CTriggerPropertiesPaneView::OnTriggerpropertiesProperties)
	ON_MESSAGE( UM_ENABLE_PANE_VIEW, OnEnablePane )
	ON_COMMAND(ID_TRIGGERPROPERTIES_DELETE, &CTriggerPropertiesPaneView::OnTriggerpropertiesDelete)
	ON_COMMAND(ID_TRIGGERPROPERTIES_COPY, &CTriggerPropertiesPaneView::OnTriggerpropertiesCopy)
	ON_COMMAND(ID_TRIGGERPROPERTIES_CUT, &CTriggerPropertiesPaneView::OnTriggerpropertiesCut)
	ON_COMMAND(ID_TRIGGERPROPERTIES_PASTE, &CTriggerPropertiesPaneView::OnTriggerpropertiesPaste)
	ON_WM_KEYDOWN()
	ON_EN_CHANGE(IDC_RICHEDIT21, &CTriggerPropertiesPaneView::OnEnChangeRichedit21)
	ON_EN_UPDATE(IDC_RICHEDIT21, &CTriggerPropertiesPaneView::OnEnUpdateRichedit21)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE1, &CTriggerPropertiesPaneView::OnTvnBegindragTree1)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_ADDOPERATOR_OR, &CTriggerPropertiesPaneView::OnTriggerpropertiesAddOperator_OR)
	ON_COMMAND(ID_ADDOPERATOR_RANDOM, &CTriggerPropertiesPaneView::OnTriggerpropertiesAddOperator_Random)
END_MESSAGE_MAP()


// CTriggerPropertiesPaneView diagnostics

#ifdef _DEBUG
void CTriggerPropertiesPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CTriggerPropertiesPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTriggerPropertiesPaneView message handlers

void CTriggerPropertiesPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	SendMessage( UM_REFRESH_PANE_VIEW, -1 );
}

void CTriggerPropertiesPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_DescriptionCtrl ) {
		int nHeight = cy / 3;
		m_DescriptionCtrl.SetWindowPos( NULL, 0, 0, cx, nHeight, SWP_FRAMECHANGED );
	}
	if( m_TreeCtrl ) {
		int nHeight = cy / 3;
		m_TreeCtrl.SetWindowPos( NULL, 0, nHeight + 10, cx, cy - nHeight - 10, SWP_FRAMECHANGED );
	}
}

BOOL CTriggerPropertiesPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	if( m_bEnable ) {
		CRect rcRect, rcFillRect;
		GetClientRect( &rcRect );
		rcFillRect = rcRect;
		rcFillRect.top = rcRect.Height() / 3;
		rcFillRect.bottom = rcFillRect.top + 10;
		pDC->FillSolidRect( &rcFillRect, RGB( 250, 213, 14 ) );
	}
	else {
		return CFormView::OnEraseBkgnd(pDC);
	}
	return TRUE;
}

LRESULT CTriggerPropertiesPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	m_TreeCtrl.DeleteAllItems();
	m_TreeCtrl.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_TreeCtrl.EnableMultiSelect( true );
	SAFE_DELETE_VEC( m_VecOperatorList[0] );
	SAFE_DELETE_VEC( m_VecOperatorList[1] );

	m_pTriggerObj = NULL;
	if( (int)wParam < 0 ) {
		m_DescriptionCtrl.ShowWindow( SW_HIDE );
		m_TreeCtrl.ShowWindow( SW_HIDE );
		m_bEnable = false;

		if( CGlobalValue::GetInstance().m_nPickType != -1 ) {
			if( m_bLastAddElement == true ) {
				SAFE_DELETE( CGlobalValue::GetInstance().m_pPickStandbyElement );
			}
			CGlobalValue::GetInstance().m_pPickStandbyElement = NULL;
			CGlobalValue::GetInstance().m_nPickType = -1;
			CGlobalValue::GetInstance().m_nPickParamIndex = 0;
			SAFE_DELETE( m_pOriginalElement );
		}
		if( wParam == -2 || lParam == -1 ) {
			m_pVecClipElementList.clear();
			m_pClipObject = NULL;
			m_nClipElementType = -1;
			m_bClipObjectCut = false;
		}
	}
	else {
		if( lParam == NULL ) {
			Invalidate();
			return S_OK;
		}
		m_DescriptionCtrl.ShowWindow( SW_SHOW );
		m_TreeCtrl.ShowWindow( SW_SHOW );
		m_bEnable = true;

		m_pTriggerObj = (CTEtTriggerObject *)lParam;
		m_hRootTree = m_TreeCtrl.InsertItem( m_pTriggerObj->GetTriggerName(), 2, 2 );

		m_hEventTree = m_TreeCtrl.InsertItem( "Event", 2, 2, m_hRootTree );
		m_hConditionTree = m_TreeCtrl.InsertItem( "Condition", 3, 3, m_hRootTree );
		m_hActionTree = m_TreeCtrl.InsertItem( "Action", 4, 4, m_hRootTree );
		m_DescriptionCtrl.SetWindowText( m_pTriggerObj->GetDescription() );

		CTEtTriggerElement *pElement;

		for( DWORD i=0; i<m_pTriggerObj->GetEventCount(); i++ ) {
			pElement = (CTEtTriggerElement *)m_pTriggerObj->GetEventFromIndex(i);

			HTREEITEM hti = m_TreeCtrl.InsertItem( pElement->MakeString().c_str(), 5, 5, m_hEventTree );
			pElement->SetTreeItem( hti );
			if( m_bClipObjectCut && std::find( m_pVecClipElementList.begin(), m_pVecClipElementList.end(), pElement ) != m_pVecClipElementList.end() ) {
				m_TreeCtrl.SetItemState( hti, TVIS_CUT, TVIS_CUT );
			}
		}

		for( DWORD i=0; i<m_pTriggerObj->GetConditionCount(); i++ ) {
			pElement = (CTEtTriggerElement *)m_pTriggerObj->GetConditionFromIndex(i);

			OperatorTreeStruct *pStruct = InsertOperator( m_hConditionTree, TVI_LAST, 0, pElement->GetOperatorType(), pElement->GetOperatorValue(0), pElement );
			if( pStruct == NULL ) {
				HTREEITEM hti = m_TreeCtrl.InsertItem( pElement->MakeString().c_str(), 5, 5, m_hConditionTree );
				pElement->SetTreeItem( hti );
				if( m_bClipObjectCut && std::find( m_pVecClipElementList.begin(), m_pVecClipElementList.end(), pElement ) != m_pVecClipElementList.end() ) {
					m_TreeCtrl.SetItemState( hti, TVIS_CUT, TVIS_CUT );
				}
			}
			else {
				HTREEITEM hti = m_TreeCtrl.InsertItem( pElement->MakeString().c_str(), 5, 5, pStruct->hti, TVI_LAST );
				pElement->SetTreeItem( hti );
				if( pElement->GetOperatorIndex() != GetOperatorIndex( pStruct->hti, 0 ) ) {
					int nValue = pElement->GetOperatorIndex();
					int nChangeValue = GetOperatorIndex( pStruct->hti, 0 );
					for( DWORD j=i; j<m_pTriggerObj->GetConditionCount(); j++ ) {
						if( m_pTriggerObj->GetConditionFromIndex(j)->GetOperatorIndex() == nValue ) {
							m_pTriggerObj->GetConditionFromIndex(j)->SetOperatorIndex( nChangeValue );
						}
					}
				}
			}
		}

		for( DWORD i=0; i<m_pTriggerObj->GetActionCount(); i++ ) {
			pElement = (CTEtTriggerElement *)m_pTriggerObj->GetActionFromIndex(i);

			OperatorTreeStruct *pStruct = InsertOperator( m_hActionTree, TVI_LAST, 1, pElement->GetOperatorType(), pElement->GetOperatorValue(0), pElement );
			if( pStruct == NULL ) {
				HTREEITEM hti = m_TreeCtrl.InsertItem( pElement->MakeString().c_str(), 5, 5, m_hActionTree );
				pElement->SetTreeItem( hti );
				if( m_bClipObjectCut && std::find( m_pVecClipElementList.begin(), m_pVecClipElementList.end(), pElement ) != m_pVecClipElementList.end() ) {
					m_TreeCtrl.SetItemState( hti, TVIS_CUT, TVIS_CUT );
				}
			}
			else {
				HTREEITEM hti = m_TreeCtrl.InsertItem( pElement->MakeString().c_str(), 5, 5, pStruct->hti, TVI_LAST );
				pElement->SetTreeItem( hti );
				if( pElement->GetOperatorIndex() != GetOperatorIndex( pStruct->hti, 1 ) ) {
					int nValue = pElement->GetOperatorIndex();
					int nChangeValue = GetOperatorIndex( pStruct->hti, 1 );
					for( DWORD j=i; j<m_pTriggerObj->GetActionCount(); j++ ) {
						if( m_pTriggerObj->GetActionFromIndex(j)->GetOperatorIndex() == nValue ) {
							m_pTriggerObj->GetActionFromIndex(j)->SetOperatorIndex( nChangeValue );
						}
					}
				}
			}
		}

		m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
		m_TreeCtrl.Expand( m_hEventTree, TVE_EXPAND );
		m_TreeCtrl.Expand( m_hConditionTree, TVE_EXPAND );
		m_TreeCtrl.Expand( m_hActionTree, TVE_EXPAND );
		for( int i=0; i<2; i++ ) {
			for( DWORD j=0; j<m_VecOperatorList[i].size(); j++ ) {
				m_TreeCtrl.Expand( m_VecOperatorList[i][j].hti, TVE_EXPAND );
			}
		}
		m_TreeCtrl.SelectItems( m_hRootTree, m_hRootTree, TRUE );
	}
	Invalidate();
	return S_OK;
}

void CTriggerPropertiesPaneView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

	CMenu *pSubMenu = m_pContextMenu->GetSubMenu(6);

	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDEVENT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDCONDITION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDACTION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_PROPERTIES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_PASTE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_ADDOPERATOR_OR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	pSubMenu->EnableMenuItem( ID_ADDOPERATOR_RANDOM, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

	CalcSelectItem();
	if( m_SelectItemList.GetCount() == 1 ) {
		if( CGlobalValue::GetInstance().IsCheckOutMe() ) {
			HTREEITEM hti = m_SelectItemList.GetHead();

			if( hti == m_hEventTree ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDEVENT, MF_BYCOMMAND | MF_ENABLED );
			}
			if( hti == m_hConditionTree ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDCONDITION, MF_BYCOMMAND | MF_ENABLED );
				pSubMenu->EnableMenuItem( ID_ADDOPERATOR_OR, MF_BYCOMMAND | MF_ENABLED );
			}
			else if( hti == m_hActionTree ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_ADDACTION, MF_BYCOMMAND | MF_ENABLED );
				pSubMenu->EnableMenuItem( ID_ADDOPERATOR_RANDOM, MF_BYCOMMAND | MF_ENABLED );
			}
			else if( hti != m_hRootTree ){
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_PROPERTIES, MF_BYCOMMAND | MF_ENABLED );
			}
		}
	}
	if( m_SelectItemList.GetCount() >= 1 ) {
		bool bValid = true;
		POSITION p = m_SelectItemList.GetHeadPosition();
		HTREEITEM hti;
		int nElementType = -1;
		bool bSameElementType = true;
		bool bCanPaste = false;
		while(p) {
			hti = m_SelectItemList.GetNext(p);
			if( hti == m_hRootTree || hti == m_hConditionTree || hti == m_hActionTree || hti == m_hEventTree ) {
				if( m_nClipElementType != -1 && !m_pVecClipElementList.empty() ) {
					if( m_nClipElementType == 0 && hti == m_hConditionTree ) bCanPaste = true;
					else if( m_nClipElementType == 1 && hti == m_hActionTree ) bCanPaste = true;
					else if( m_nClipElementType == 2 && hti == m_hEventTree ) bCanPaste = true;
				}
				bValid = false;
				break;
			}
			CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
			if( !pElement ) continue;
			if( nElementType == -1 ) nElementType = (int)pElement->GetType();
			else {
				if( nElementType != (int)pElement->GetType() ) bSameElementType = false;
			}
		}
		if( bValid ) {
			pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_DELETE, MF_BYCOMMAND | MF_ENABLED );

			if( nElementType != -1 && bSameElementType ) {
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_COPY, MF_BYCOMMAND | MF_ENABLED );
				pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_CUT, MF_BYCOMMAND | MF_ENABLED );

				if( nElementType != -1 && m_nClipElementType == nElementType && !m_pVecClipElementList.empty() ) bCanPaste = true;
			}
		}
		if( bCanPaste ) pSubMenu->EnableMenuItem( ID_TRIGGERPROPERTIES_PASTE, MF_BYCOMMAND | MF_ENABLED );
	}
	else {
		if( m_SelectItemList.GetCount() == 0 ) return;
	}

	CPoint p = point;
	ScreenToClient( &p );
	CRect rcRect;
	m_TreeCtrl.GetWindowRect( &rcRect );
	ScreenToClient( &rcRect );

	if( p.x < rcRect.left || p.x > rcRect.right || p.y < rcRect.top || p.y > rcRect.bottom ) return;

	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void CTriggerPropertiesPaneView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CFormView::OnPaint() for painting messages
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesAddcondition()
{
	// TODO: Add your command handler code here
	AddElement( 0 );
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesAddaction()
{
	// TODO: Add your command handler code here
	AddElement( 1 );
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesAddevent()
{
	// TODO: Add your command handler code here
	AddElement( 2 );
}

bool CTriggerPropertiesPaneView::OpenElementDialog( CTEtTriggerElement *pElement, bool bAddElement )
{
	CTriggerSelectDlg Dlg;
	Dlg.SetTriggerElement( pElement );
	m_bLastAddElement = bAddElement;

	if( Dlg.DoModal() == IDOK ) {
		if( CGlobalValue::GetInstance().m_nPickType != -1 ) {
			CWnd *pWnd = NULL;
			pWnd = GetPaneWnd( TRIGGER_PANE );
			if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, FALSE );
			pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
			if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, FALSE );

			switch( CGlobalValue::GetInstance().m_nPickType ) {
				case CEtTrigger::Prop:
					pWnd = GetPaneWnd( EVENT_PANE );
					if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, FALSE );
					break;
				case CEtTrigger::EventArea:
					pWnd = GetPaneWnd( PROPLIST_PANE );
					if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, FALSE );
					break;
				case CEtTrigger::Position:
					break;
			}
		}
		else {
			CGlobalValue::GetInstance().SetModify();
			if( bAddElement == true ) {
				std::string szStr = pElement->MakeString();
				HTREEITEM hParent;
				switch( pElement->GetType() ) {
					case CEtTriggerElement::Condition: hParent = m_hConditionTree; break;
					case CEtTriggerElement::Action: hParent = m_hActionTree; break;
					case CEtTriggerElement::Event: hParent = m_hEventTree; break;
				}
				HTREEITEM hti = m_TreeCtrl.InsertItem( szStr.c_str(), 5, 5, hParent );
				pElement->SetTreeItem( hti );
				m_TreeCtrl.Expand( hParent, TVE_EXPAND );
				m_TreeCtrl.SelectItems( hti, hti, TRUE );

				switch( pElement->GetType() ) {
					case CEtTriggerElement::Condition: m_pTriggerObj->AddConditionElement( pElement ); break;
					case CEtTriggerElement::Action: m_pTriggerObj->AddActionElement( pElement ); break;
					case CEtTriggerElement::Event: m_pTriggerObj->AddEventElement( pElement ); break;
				}
			}
			else {
				std::string szStr = pElement->MakeString();
				HTREEITEM hti = pElement->GetTreeItem();
				m_TreeCtrl.SetItemText( hti, szStr.c_str() );

			}
		}
		return true;
	}
	return false;
}

void CTriggerPropertiesPaneView::AddElement( int nType )
{
	CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->AllocTriggerElement();
	pElement->SetType( (CEtTriggerElement::TriggerElementTypeEnum)nType );

	if( OpenElementDialog( pElement, true ) == false ) {
		SAFE_DELETE( pElement );
		return;
	}
}

void CTriggerPropertiesPaneView::CalcSelectItem()
{
	m_SelectItemList.RemoveAll();
	m_TreeCtrl.GetSelectedList( m_SelectItemList );
}

LRESULT CTriggerPropertiesPaneView::OnPickObject( WPARAM wParam, LPARAM lParam )
{
	CWnd *pWnd = GetPaneWnd( TRIGGER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( TRIGGER_PROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( PROPLIST_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );
	pWnd = GetPaneWnd( EVENT_PANE );
	if( pWnd ) pWnd->SendMessage( UM_ENABLE_PANE_VIEW, TRUE );

	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return S_OK;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return S_OK;

	CUnionValueProperty *pVariable = CGlobalValue::GetInstance().m_pPickStandbyElement->GetParamVariable( CGlobalValue::GetInstance().m_nPickParamIndex );
	switch( CGlobalValue::GetInstance().m_nPickType ) {
		case CEtTrigger::Prop:
			{
				CTEtWorldProp *pProp = (CTEtWorldProp *)wParam;
				pVariable->SetVariable( pProp->GetCreateUniqueID() );
			}
			break;
		case CEtTrigger::EventArea:
			{
				CEtWorldEventArea *pArea = (CEtWorldEventArea *)wParam;
				pVariable->SetVariable( pArea->GetCreateUniqueID() );
			}
			break;
		case CEtTrigger::Position:
			{
				EtVector3 *pPos = (EtVector3 *)wParam;
				pVariable->SetVariable( *pPos );
			}
			break;
	}
	if( wParam != NULL || lParam != NULL )
		CGlobalValue::GetInstance().m_pPickStandbyElement->SetModifyParam( CGlobalValue::GetInstance().m_nPickParamIndex );

	CTEtTriggerElement *pElement = CGlobalValue::GetInstance().m_pPickStandbyElement;
	CGlobalValue::GetInstance().m_pPickStandbyElement = NULL;
	CGlobalValue::GetInstance().m_nPickType = -1;
	CGlobalValue::GetInstance().m_nPickParamIndex = 0;
	if( OpenElementDialog( pElement, m_bLastAddElement ) == false ) {
		if( m_bLastAddElement ) {
			SAFE_DELETE( pElement );
		}
		else {
			*pElement = *m_pOriginalElement;
		}
	}
	else {
		if( CGlobalValue::GetInstance().m_nPickType == -1 )
			SAFE_DELETE( m_pOriginalElement );
	}

	return S_OK;
}
void CTriggerPropertiesPaneView::OnTriggerpropertiesProperties()
{
	// TODO: Add your command handler code here
	HTREEITEM hti = m_SelectItemList.GetHead();
	CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
	if( pElement ) {
		SAFE_DELETE( m_pOriginalElement );
		m_pOriginalElement = (CTEtTriggerElement *)m_pTriggerObj->AllocTriggerElement();
		*m_pOriginalElement = *pElement;
		if( OpenElementDialog( pElement, false ) == false ) {
			*pElement = *m_pOriginalElement;
			SAFE_DELETE( m_pOriginalElement );
		}
		else {
			if( CGlobalValue::GetInstance().m_nPickType == -1 )
				SAFE_DELETE( m_pOriginalElement );
		}
	}
	else {
		if( IsOperatorTreeItem( hti, 0 ) || IsOperatorTreeItem( hti, 1 ) ) {
			CString szStr = m_TreeCtrl.GetItemText( hti );
			if( strstr( szStr, "Operator - OR" ) ) {
				// OR 은 할게없다.
			}
			if( strstr( szStr, "Operator - Random" ) ) {
				std::vector<CTEtTriggerElement *> VecList;
				GetOperatorElementList( GetOperatorType( hti, 1 ), GetOperatorIndex( hti, 1 ), 1, VecList );

				CTriggerOperatorDlg Dlg;
				Dlg.SetValue( 0, GetOperatorValue( hti, 1, 0 ) );
				if( Dlg.DoModal() == IDOK ) {
					SetOperatorValue( hti, 1, 0, Dlg.GetValue(0) );
					for( DWORD i=0; i<VecList.size(); i++ ) {
						VecList[i]->SetOperatorValue( 0, Dlg.GetValue(0) );
					}
				}
				CString szTemp;
				szTemp.Format( "Operator - Random %d %%", GetOperatorValue( hti, 1, 0 ) );
				m_TreeCtrl.SetItemText( hti, szTemp );
			}
		}
	}
}

LRESULT CTriggerPropertiesPaneView::OnEnablePane( WPARAM wParam, LPARAM lParam )
{
	m_DescriptionCtrl.EnableWindow( (BOOL)wParam );
	m_TreeCtrl.EnableWindow( (BOOL)wParam );
	return S_OK;
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesDelete()
{
	// TODO: Add your command handler code here
	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr, szCategory;
	std::vector<HTREEITEM> hVecCategory;
	std::vector<HTREEITEM> hVecList;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree || hti == m_hConditionTree || hti == m_hActionTree || hti == m_hEventTree ) continue;
		hVecList.push_back( hti );
	}
	for( DWORD i=0; i<hVecList.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->GetElementFromTreeIndex( hVecList[i] );
		if( pElement ) {
			std::vector<CEtTriggerElement*>::iterator it = std::find( m_pVecClipElementList.begin(), m_pVecClipElementList.end(), pElement );
			if( it != m_pVecClipElementList.end() ) m_pVecClipElementList.erase( it );
		}
		if( !CheckDeleteOperatorElement( hVecList[i] ) )
			m_pTriggerObj->RemoveElementFromTreeIndex( hVecList[i] );

		m_TreeCtrl.DeleteItem( hVecList[i] );
	}
	CGlobalValue::GetInstance().SetModify();

}

void CTriggerPropertiesPaneView::OnTriggerpropertiesCopy()
{
	for( DWORD i=0; i<m_pVecClipElementList.size(); i++ ) {
		HTREEITEM hti = m_TreeCtrl.FindItem( ((CTEtTriggerElement*)m_pVecClipElementList[i])->MakeString().c_str(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
		}
	}
	m_pVecClipElementList.clear();
	m_pClipObject = m_pTriggerObj;

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr, szCategory;
	std::vector<HTREEITEM> hVecCategory;
	m_nClipElementType = -1;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree || hti == m_hConditionTree || hti == m_hActionTree || hti == m_hEventTree ) continue;
		CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
		if( !pElement ) continue;

		m_pVecClipElementList.push_back( pElement );
		if( m_nClipElementType == -1 ) m_nClipElementType = (int)pElement->GetType();
		else {
			if( m_nClipElementType != (int)pElement->GetType() ) {
				m_pVecClipElementList.clear();
				m_pClipObject = NULL;
				break;
			}
		}
	}

	m_bClipObjectCut = false;
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesCut()
{
	for( DWORD i=0; i<m_pVecClipElementList.size(); i++ ) {
		HTREEITEM hti = m_TreeCtrl.FindItem( ((CTEtTriggerElement*)m_pVecClipElementList[i])->MakeString().c_str(), TRUE, TRUE, TRUE );
		if( hti ) {
			m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
		}
	}
	m_pVecClipElementList.clear();
	m_pClipObject = m_pTriggerObj;

	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti;
	CString szStr, szCategory;
	std::vector<HTREEITEM> hVecCategory;
	m_nClipElementType = -1;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hRootTree || hti == m_hConditionTree || hti == m_hActionTree || hti == m_hEventTree ) continue;
		CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
		if( !pElement ) continue;

		m_TreeCtrl.SetItemState( hti, TVIS_CUT, TVIS_CUT );

		m_pVecClipElementList.push_back( pElement );
		if( m_nClipElementType == -1 ) m_nClipElementType = (int)pElement->GetType();
		else {
			if( m_nClipElementType != (int)pElement->GetType() ) {
				for( DWORD i=0; i<m_pVecClipElementList.size(); i++ ) {
					HTREEITEM hti = m_TreeCtrl.FindItem( ((CTEtTriggerElement*)m_pVecClipElementList[i])->MakeString().c_str(), TRUE, TRUE, TRUE );
					if( hti ) {
						m_TreeCtrl.SetItemState( hti, 0, TVIS_CUT );
					}
				}
				m_pVecClipElementList.clear();
				m_pClipObject = NULL;
				break;
			}
		}
	}

	m_bClipObjectCut = true;
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesPaste()
{
	POSITION p = m_SelectItemList.GetHeadPosition();
	HTREEITEM hti = NULL;
	CString szStr, szCategory;
//	m_nClipElementType = -1;
	CTEtTriggerElement *pFirst = NULL;
	while(p) {
		hti = m_SelectItemList.GetNext(p);
		if( hti == m_hConditionTree || hti == m_hActionTree || hti == m_hEventTree ) break;

		CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
		if( !pElement ) return;
		pFirst = pElement;
		switch( m_nClipElementType ) {
			case 0: hti = m_hConditionTree; break;
			case 1: hti = m_hActionTree; break;
			case 2: hti = m_hEventTree; break;
		}
		break;
	}
	if( !hti ) return;

	HTREEITEM hChild = TVI_FIRST;
	if( pFirst ) hChild = pFirst->GetTreeItem();
	//for( DWORD i=0; i<m_pVecClipElementList.size(); i++ ) {
	for( int i=m_pVecClipElementList.size()-1; i>=0; i-- ) {
		CTEtTriggerElement *pCopy = (CTEtTriggerElement *)m_pTriggerObj->AllocTriggerElement();
		*pCopy = *(CTEtTriggerElement *)m_pVecClipElementList[i];
		HTREEITEM hti2 = m_TreeCtrl.InsertItem( ((CTEtTriggerElement*)m_pVecClipElementList[i])->MakeString().c_str(), 5, 5, hti, hChild );
		((CTEtTriggerElement*)pCopy)->SetTreeItem( hti2 );
		switch( m_nClipElementType ) {
			case 0: m_pTriggerObj->AddConditionElement( pCopy ); break;
			case 1: m_pTriggerObj->AddActionElement( pCopy ); break;
			case 2: m_pTriggerObj->AddEventElement( pCopy ); break;
		}
		m_pTriggerObj->ChangeConditionOrder( pCopy, pFirst );
	}
	if( m_bClipObjectCut && m_pClipObject ) {
		for( DWORD i=0; i<m_pVecClipElementList.size(); i++ ) {
			HTREEITEM hItem = ((CTEtTriggerElement*)m_pVecClipElementList[i])->GetTreeItem();
			if( m_pClipObject->RemoveElement( m_pVecClipElementList[i] ) ) {
				if( m_pClipObject == m_pTriggerObj ) {
					if( !CheckDeleteOperatorElement( hItem ) )
						m_TreeCtrl.DeleteItem( hItem );
				}
			}
		}
		m_pVecClipElementList.clear();
		m_pClipObject = NULL;
	}
	m_bClipObjectCut = false;

	m_TreeCtrl.Expand( m_hRootTree, TVE_EXPAND );
	m_TreeCtrl.Expand( m_hEventTree, TVE_EXPAND );
	m_TreeCtrl.Expand( m_hConditionTree, TVE_EXPAND );
	m_TreeCtrl.Expand( m_hActionTree, TVE_EXPAND );
}


bool CTriggerPropertiesPaneView::IsOperatorTreeItem( HTREEITEM hti, int nConditionAction )
{
	for( DWORD i=0; i<m_VecOperatorList[nConditionAction].size(); i++ ) {
		if( m_VecOperatorList[nConditionAction][i].hti == hti ) return true;
	}
	return false;
}

CEtTriggerElement::OperatorTypeEnum CTriggerPropertiesPaneView::GetOperatorType( HTREEITEM hti, int nConditionAction )
{
	for( DWORD i=0; i<m_VecOperatorList[nConditionAction].size(); i++ ) {
		if( m_VecOperatorList[nConditionAction][i].hti == hti ) return m_VecOperatorList[nConditionAction][i].Type;
	}
	return CEtTriggerElement::And;
}

int CTriggerPropertiesPaneView::GetOperatorIndex( HTREEITEM hti, int nConditionAction )
{
	for( DWORD i=0; i<m_VecOperatorList[nConditionAction].size(); i++ ) {
		if( m_VecOperatorList[nConditionAction][i].hti == hti ) return i;
	}
	return 0;
}

int CTriggerPropertiesPaneView::GetOperatorValue( HTREEITEM hti, int nConditionAction, int nIndex )
{
	for( DWORD i=0; i<m_VecOperatorList[nConditionAction].size(); i++ ) {
		if( m_VecOperatorList[nConditionAction][i].hti == hti ) {
			return m_VecOperatorList[nConditionAction][i].nValue[nIndex];
		}
	}
	return 0;
}

void CTriggerPropertiesPaneView::SetOperatorValue( HTREEITEM hti, int nConditionAction, int nIndex, int nValue )
{
	for( DWORD i=0; i<m_VecOperatorList[nConditionAction].size(); i++ ) {
		if( m_VecOperatorList[nConditionAction][i].hti == hti ) {
			m_VecOperatorList[nConditionAction][i].nValue[nIndex] = nValue;
			return;
		}
	}
}

void CTriggerPropertiesPaneView::GetOperatorElementList( CEtTriggerElement::OperatorTypeEnum Type, int nOperatorIndex, int nConditionAction, std::vector<CTEtTriggerElement*> &VecResult )
{
	switch( nConditionAction ) {
		case 0:
			{
				for( DWORD i=0; i<m_pTriggerObj->GetConditionCount(); i++ ) {
					CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->GetConditionFromIndex(i);
					if( pElement->GetOperatorType() == Type && pElement->GetOperatorIndex() == nOperatorIndex ) {
						VecResult.push_back( pElement );
					}
				}
			}
			break;
		case 1:
			{
				for( DWORD i=0; i<m_pTriggerObj->GetActionCount(); i++ ) {
					CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->GetActionFromIndex(i);
					if( pElement->GetOperatorType() == Type && pElement->GetOperatorIndex() == nOperatorIndex ) {
						VecResult.push_back( pElement );
					}
				}
			}
			break;
	}
}

CTEtTriggerElement *CTriggerPropertiesPaneView::GetOperatorPrevElement( HTREEITEM hti )
{
	HTREEITEM hItem = m_TreeCtrl.GetPrevItem( hti );
	if( !hItem ) return NULL;

	if( hItem == m_hDrag ) hItem = m_TreeCtrl.GetPrevItem( hItem );
	/*
	while(1) {
		if( hItem == m_hDrag ) hItem = m_TreeCtrl.GetPrevItem( hItem );
		if( !hItem ) break;
		if( !m_pTriggerObj->GetElementFromTreeIndex( hItem ) ) break;
	}
	return m_pTriggerObj->GetElementFromTreeIndex( hItem );
	*/
	while(1) {
		if( hItem == m_hConditionTree || hItem == m_hActionTree ) return NULL;
		if( m_pTriggerObj->GetElementFromTreeIndex( hItem ) != NULL ) break;
		hItem = m_TreeCtrl.GetPrevItem( hItem );
		if( !hItem ) break;
//		if( !m_pTriggerObj->GetElementFromTreeIndex( hItem ) ) break;

	}
	return m_pTriggerObj->GetElementFromTreeIndex( hItem );
}

HTREEITEM CTriggerPropertiesPaneView::GetOperatorTreeHandle( CTEtTriggerElement *pElement, int nConditionAction )
{
	switch( pElement->GetOperatorType() ) {
		case CEtTriggerElement::And:
			if( pElement->GetOperatorIndex() == 0 ) {
				switch( nConditionAction ) {
					case 0: return m_hConditionTree;
					case 1: return m_hActionTree;
				}
			}
			else {
				if( pElement->GetOperatorIndex() - 1 < (int)m_VecOperatorList[nConditionAction].size() ) 
					return m_VecOperatorList[nConditionAction][pElement->GetOperatorIndex() - 1].hti;
			}
			break;
		case CEtTriggerElement::Or:
		case CEtTriggerElement::Random:
			{
				if( pElement->GetOperatorIndex() < (int)m_VecOperatorList[nConditionAction].size() ) 
					return m_VecOperatorList[nConditionAction][pElement->GetOperatorIndex()].hti;
			}
			break;
	}
	return NULL;
}

CTriggerPropertiesPaneView::OperatorTreeStruct *CTriggerPropertiesPaneView::GetOperatorStruct( CTEtTriggerElement *pElement, int nConditionAction )
{
	switch( pElement->GetOperatorType() ) {
		case CEtTriggerElement::And:
			if( pElement->GetOperatorIndex() == 0 ) {
				switch( nConditionAction ) {
					case 0: return NULL;
					case 1: return NULL;
				}
			}
			else {
				if( pElement->GetOperatorIndex() - 1 < (int)m_VecOperatorList[nConditionAction].size() ) 
					return &m_VecOperatorList[nConditionAction][pElement->GetOperatorIndex() - 1];
			}
			break;
		case CEtTriggerElement::Or:
		case CEtTriggerElement::Random:
			{
				if( pElement->GetOperatorIndex() < (int)m_VecOperatorList[nConditionAction].size() ) 
					return &m_VecOperatorList[nConditionAction][pElement->GetOperatorIndex()];
			}
			break;
	}
	return NULL;
}

bool CTriggerPropertiesPaneView::CheckDeleteOperatorElement( HTREEITEM hti )
{
	bool bResult = false;
	for( int i=0; i<2; i++ ) {
		for( DWORD j=0; j<m_VecOperatorList[i].size(); j++ ) {
			if( m_VecOperatorList[i][j].hti == hti ) {
				bResult = true;
				// 차일드들 지워줘야한다.
				switch( i ) {
					case 0:
						{
							for( DWORD k=0; k<m_pTriggerObj->GetConditionCount(); k++ ) {
								CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->GetConditionFromIndex(k);
								if( m_VecOperatorList[i][j].Type == pElement->GetOperatorType() && pElement->GetOperatorIndex() == j ) {
									m_pTriggerObj->RemoveElementFromTreeIndex( pElement->GetTreeItem() );
									m_TreeCtrl.DeleteItem( pElement->GetTreeItem() );
									k--;
								}
							}
						}
						break;
					case 1:
						{
							for( DWORD k=0; k<m_pTriggerObj->GetActionCount(); k++ ) {
								CTEtTriggerElement *pElement = (CTEtTriggerElement *)m_pTriggerObj->GetActionFromIndex(k);
								if( m_VecOperatorList[i][j].Type == pElement->GetOperatorType() && pElement->GetOperatorIndex() == j ) {
									m_pTriggerObj->RemoveElementFromTreeIndex( pElement->GetTreeItem() );
									m_TreeCtrl.DeleteItem( pElement->GetTreeItem() );
									k--;
								}
							}
						}
						break;
				}
				m_VecOperatorList[i].erase( m_VecOperatorList[i].begin() + j );
			}
		}
	}
	return bResult;
}

void CTriggerPropertiesPaneView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if( nChar == VK_DELETE ) {
		OnTriggerpropertiesDelete();
	}

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTriggerPropertiesPaneView::OnEnChangeRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CTriggerPropertiesPaneView::OnEnUpdateRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	if( m_pTriggerObj ) {
		CString szStr;
		m_DescriptionCtrl.GetWindowText( szStr );
		m_pTriggerObj->SetDescription( szStr );
		CGlobalValue::GetInstance().SetModify();
	}
}

void CTriggerPropertiesPaneView::OnTvnBegindragTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	// So user cant drag root node
	if( m_TreeCtrl.GetParentItem(pNMTreeView->itemNew.hItem) == NULL ) return; 

	m_hDrag = pNMTreeView->itemNew.hItem;
	CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( m_hDrag );
	if( !pElement ) return;

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

void CTriggerPropertiesPaneView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	HTREEITEM	hitem;
	UINT		flags;

	if( m_bDragging ) {
		POINT pt = point;
		ClientToScreen( &pt );
		CImageList::DragMove(pt);

		CRect rcRect;
		GetClientRect( &rcRect );
		POINT pt2 = point;
		int nHeight = ( rcRect.Height() / 3 ) + 10;
		pt2.y -= nHeight;

		if( (hitem = m_TreeCtrl.HitTest(pt2, &flags) ) != NULL ) {
			CImageList::DragShowNolock(FALSE);

			CTEtTriggerElement *pDragElement = m_pTriggerObj->GetElementFromTreeIndex( m_hDrag );
			CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hitem );

			bool bValid = false;
			if( pElement == NULL ) {
				switch( pDragElement->GetType() ) {
					case CEtTriggerElement::Event: if( hitem == m_hEventTree ) bValid = true; break;
					case CEtTriggerElement::Condition: 
						if( hitem == m_hConditionTree ) bValid = true; 
						if( IsOperatorTreeItem( hitem, 0 ) ) bValid = true;
						break;
					case CEtTriggerElement::Action: 
						if( hitem == m_hActionTree ) bValid = true; 
						if( IsOperatorTreeItem( hitem, 1 ) ) bValid = true;
						break;
				}
			}
			else if( pDragElement->GetType() == pElement->GetType() ) bValid = true;
			if( bValid ) {
				m_TreeCtrl.SelectDropTarget(hitem);
				m_hDrop = hitem;
			}
			else {
				m_TreeCtrl.SelectDropTarget(NULL);
				m_hDrop = NULL;
			}

			CImageList::DragShowNolock(TRUE);
		}
	}

	CFormView::OnMouseMove(nFlags, point);
}

void CTriggerPropertiesPaneView::OnLButtonUp(UINT nFlags, CPoint point)
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
			CRect rcRect;
			GetClientRect( &rcRect );
			POINT pt2 = point;
			int nHeight = ( rcRect.Height() / 3 ) + 10;
			pt2.y -= nHeight;

			if( ((hitem = m_TreeCtrl.HitTest(pt2, &nFlags)) == NULL)  ) return ;


			CTEtTriggerElement *pDragElement = m_pTriggerObj->GetElementFromTreeIndex( m_hDrag );
			CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hitem );

			bool bValid = false;
			HTREEITEM hRoot;
			if( pElement == NULL ) {
				switch( pDragElement->GetType() ) {
					case CEtTriggerElement::Event: if( hitem == m_hEventTree ) bValid = true; break;
					case CEtTriggerElement::Condition: 
						if( hitem == m_hConditionTree ) bValid = true; 
						if( IsOperatorTreeItem( hitem, 0 ) ) {
							bValid = true;
//							pElement = GetOperatorPrevElement( hitem );
						}
						break;
					case CEtTriggerElement::Action: 
						if( hitem == m_hActionTree ) bValid = true; 
						if( IsOperatorTreeItem( hitem, 1 ) ) {
							bValid = true;
//							pElement = GetOperatorPrevElement( hitem );
						}
						break;
				}
			}
			else if( pDragElement->GetType() == pElement->GetType() ) {
				bValid = true;
				pDragElement->SetOperatorType( pElement->GetOperatorType() );
				pDragElement->SetOperatorIndex( pElement->GetOperatorIndex() );
				for( DWORD m=0; m<4; m++ ) {
					pDragElement->SetOperatorValue( m, pElement->GetOperatorValue(m) );
				}
			}


			if( bValid ) {
				CEtTriggerElement::OperatorTypeEnum OrigType = pDragElement->GetOperatorType();
				int nOrigIndex = pDragElement->GetOperatorIndex();
				int nOrigValue[4] = { pDragElement->GetOperatorValue(0), pDragElement->GetOperatorValue(1), pDragElement->GetOperatorValue(2), pDragElement->GetOperatorValue(3) };

				if( m_bCopyDrag ) {
					if( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) m_bCopyDrag = false;
				}

				switch( pDragElement->GetType() ) {
					case CEtTriggerElement::Event: hRoot = m_hEventTree; break;
					case CEtTriggerElement::Condition: 
						hRoot = m_hConditionTree; 
						if( IsOperatorTreeItem( hitem, 0 ) ) {
							hRoot = hitem;
							pDragElement->SetOperatorType( GetOperatorType( hitem, 0 ) );
							pDragElement->SetOperatorIndex( GetOperatorIndex( hitem, 0 ) );
							for( DWORD m=0; m<4; m++ ) {
								pDragElement->SetOperatorValue( m, GetOperatorValue( hitem, 0, m ) );
							}
							if( !pElement ) {
								pElement = GetOperatorPrevElement( hitem );
								hitem = TVI_FIRST;
//								if( pElement ) hitem = pElement->GetTreeItem();
							}
						}
						else {
							if( pElement ) {
								HTREEITEM hTemp = GetOperatorTreeHandle( pElement, 0 );
								if( hTemp ) hRoot = hTemp;
							}
							else {
								pDragElement->SetOperatorType( CEtTriggerElement::And );
								pDragElement->SetOperatorIndex( 0 );
								for( DWORD m=0; m<4; m++ ) {
									pDragElement->SetOperatorValue( m, 0 );
								}
							}
						}
						break;
					case CEtTriggerElement::Action: 
						hRoot = m_hActionTree; 
						if( IsOperatorTreeItem( hitem, 1 ) ) {
							hRoot = hitem;
							pDragElement->SetOperatorType( GetOperatorType( hitem, 1 ) );
							pDragElement->SetOperatorIndex( GetOperatorIndex( hitem, 1 ) );
							for( DWORD m=0; m<4; m++ ) {
								pDragElement->SetOperatorValue( m, GetOperatorValue( hitem, 1, m ) );
							}
							if( !pElement ) {
								pElement = GetOperatorPrevElement( hitem );
								hitem = TVI_FIRST;
//								if( pElement ) hitem = pElement->GetTreeItem();
							}
						}
						else {
							if( pElement ) {
								HTREEITEM hTemp = GetOperatorTreeHandle( pElement, 1 );
								if( hTemp ) hRoot = hTemp;
							}
							else {
								pDragElement->SetOperatorType( CEtTriggerElement::And );
								pDragElement->SetOperatorIndex( 0 );
								for( DWORD m=0; m<4; m++ ) {
									pDragElement->SetOperatorValue( m, 0 );
								}
							}
						}
						break;
				}
				if( m_bCopyDrag ) {
					CTEtTriggerElement *pCopy = (CTEtTriggerElement *)m_pTriggerObj->AllocTriggerElement();
					*pCopy = *pDragElement;
					// 아 진짜 구리다..
					pDragElement->SetOperatorType( OrigType );
					pDragElement->SetOperatorIndex( nOrigIndex );
					pDragElement->SetOperatorValue( 0, nOrigValue[0] );
					pDragElement->SetOperatorValue( 1, nOrigValue[1] );
					pDragElement->SetOperatorValue( 2, nOrigValue[2] );
					pDragElement->SetOperatorValue( 3, nOrigValue[3] );
					pDragElement = pCopy;
					switch( pDragElement->GetType() ) {
						case CEtTriggerElement::Event: m_pTriggerObj->AddEventElement( pCopy ); break;
						case CEtTriggerElement::Condition: m_pTriggerObj->AddConditionElement( pCopy ); break;
						case CEtTriggerElement::Action: m_pTriggerObj->AddActionElement( pCopy ); break;
					}
				}


				m_pTriggerObj->ChangeConditionOrder( pDragElement, pElement );

				CString szStr = m_TreeCtrl.GetItemText( m_hDrag );
				if( !m_bCopyDrag ) m_TreeCtrl.DeleteItem( m_hDrag );
				if( pElement == NULL ) hitem = TVI_FIRST;
				HTREEITEM hInsertItem = m_TreeCtrl.InsertItem( szStr, 5, 5, hRoot, hitem );
				m_TreeCtrl.Expand( hRoot, TVE_EXPAND );
				pDragElement->SetTreeItem( hInsertItem );
			}
		}
	}

	CFormView::OnLButtonUp(nFlags, point);
}

void CTriggerPropertiesPaneView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	OnTriggerpropertiesProperties();

	CFormView::OnLButtonDblClk(nFlags, point);
}

BOOL CTriggerPropertiesPaneView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->message == WM_LBUTTONDBLCLK ) {
		CalcSelectItem();
		if( m_SelectItemList.GetCount() == 1 ) {
			HTREEITEM hti = m_SelectItemList.GetHead();
			CTEtTriggerElement *pElement = m_pTriggerObj->GetElementFromTreeIndex( hti );
			if( pElement && hti != m_hEventTree && hti != m_hConditionTree && hti != m_hActionTree && hti != m_hRootTree ) {
				BOOL bResult = CFormView::PreTranslateMessage(pMsg);
				OnTriggerpropertiesProperties();
				return bResult;
			}
		}
	}

	return CFormView::PreTranslateMessage(pMsg);
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesAddOperator_OR()
{
	CalcSelectItem();
	if( m_SelectItemList.GetCount() != 1 ) return;

	/*
	HTREEITEM hti = m_TreeCtrl.InsertItem( "Operator - OR", 6, 6, m_SelectItemList.GetHead() );
	OperatorTreeStruct Struct;
	Struct.hti = hti;
	Struct.Type = CEtTriggerElement::Or;
	memset( Struct.nValue, 0, sizeof(Struct.nValue) );
	if( m_SelectItemList.GetHead() == m_hConditionTree )
		m_VecOperatorList[0].push_back( Struct ); 
	else if( m_SelectItemList.GetHead() == m_hActionTree )
		m_VecOperatorList[1].push_back( Struct ); 
	*/
	InsertOperator( m_SelectItemList.GetHead(), TVI_LAST, 0, CEtTriggerElement::Or, 0 );
}

void CTriggerPropertiesPaneView::OnTriggerpropertiesAddOperator_Random()
{
	CalcSelectItem();
	if( m_SelectItemList.GetCount() != 1 ) return;

	/*
	CString szTemp;
	szTemp.Format( "Operator - Random %d %%", 100 );
	HTREEITEM hti = m_TreeCtrl.InsertItem( szTemp, 7, 7, m_SelectItemList.GetHead() );
	OperatorTreeStruct Struct;
	Struct.hti = hti;
	Struct.Type = CEtTriggerElement::Or;
	memset( Struct.nValue, 0, sizeof(Struct.nValue) );
	Struct.nValue[0] = 100;
	if( m_SelectItemList.GetHead() == m_hConditionTree )
		m_VecOperatorList[0].push_back( Struct ); 
	else if( m_SelectItemList.GetHead() == m_hActionTree )
		m_VecOperatorList[1].push_back( Struct ); 
	*/
	InsertOperator( m_SelectItemList.GetHead(), TVI_LAST, 1, CEtTriggerElement::Random, 100 );
}

CTriggerPropertiesPaneView::OperatorTreeStruct *CTriggerPropertiesPaneView::InsertOperator( HTREEITEM hParent, HTREEITEM hInsertAfter, int nConditionAction, CEtTriggerElement::OperatorTypeEnum Type, int nValue, CTEtTriggerElement *pElement )
{
	CString szTemp;
	OperatorTreeStruct Struct;
	HTREEITEM hti;
	if( pElement ) {
		OperatorTreeStruct *pStruct = GetOperatorStruct( pElement, nConditionAction );
		if( pStruct ) return pStruct;
	}
	switch( Type ) {
		case CEtTriggerElement::And:
			return NULL;
		case CEtTriggerElement::Or:
			szTemp.Format( "Operator - Or", nValue );
			hti = m_TreeCtrl.InsertItem( szTemp, 6, 6, hParent, hInsertAfter );
			Struct.Type = CEtTriggerElement::Or;
			break;
		case CEtTriggerElement::Random:
			szTemp.Format( "Operator - Random %d %%", nValue );
			hti = m_TreeCtrl.InsertItem( szTemp, 7, 7, hParent, hInsertAfter );
			Struct.Type = CEtTriggerElement::Random;
			break;
	}

	Struct.hti = hti;
	memset( Struct.nValue, 0, sizeof(Struct.nValue) );
	Struct.nValue[0] = nValue;

	m_VecOperatorList[nConditionAction].push_back( Struct ); 

	
	return &m_VecOperatorList[nConditionAction][ m_VecOperatorList[nConditionAction].size() - 1 ];
}