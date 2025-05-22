#include "StdAfx.h"
#include "EnviLightBase.h"
#include "EnviControl.h"
#include "RenderBase.h"
#include "EnviLightDirection.h"
#include "EnviLightPoint.h"
#include "EnviLightSpot.h"

std::vector<CEnviLightBase *> CEnviLightBase::s_pVecList;
CEnviLightBase *CEnviLightBase::s_pFocus = NULL;

CEnviLightBase::CEnviLightBase( CEnviElement *pElement )
{
	m_hItem = NULL;
	m_pElement = NULL;
	m_pElementPtr = NULL;
	m_bFocus = false;
}

CEnviLightBase::~CEnviLightBase()
{
}

bool CEnviLightBase::AddLight( HTREEITEM hItem, CEnviElement *pElement, DWORD dwIndex )
{
	CEnviLightBase *pBase = NULL;
	switch( pElement->GetInfo().GetLight( dwIndex )->pInfo->Type ) {
		case LT_DIRECTIONAL:	pBase = new CEnviLightDirection( pElement );	break;
		case LT_POINT:			pBase = new CEnviLightPoint( pElement );		break;
		case LT_SPOT:			pBase = new CEnviLightSpot( pElement );			break;
	}

	pBase->m_hItem = hItem;
	pBase->m_pElement = pElement;
	pBase->m_pElementPtr = pElement->GetInfo().GetLight( dwIndex );

	s_pVecList.push_back( pBase );
	return true;
}

bool CEnviLightBase::RemoveLight( HTREEITEM hItem )
{
	for( DWORD i=0; i<s_pVecList.size(); i++ ) {
		if( s_pVecList[i]->m_hItem == hItem ) {
			s_pVecList.erase( s_pVecList.begin() + i );
			return true;
		}
	}
	return false;
}

CEnviLightBase *CEnviLightBase::FindLight( HTREEITEM hItem )
{
	for( DWORD i=0; i<s_pVecList.size(); i++ ) {
		if( s_pVecList[i]->m_hItem == hItem ) return s_pVecList[i];
	}
	return NULL;
}


void CEnviLightBase::Reset()
{
	SAFE_DELETE_PVEC( s_pVecList );
	s_pFocus = NULL;
}

/*
void CEnviLightBase::SetFocus( HTREEITEM hItem )
{
	s_pFocus = NULL;
	for( DWORD i=0; i<s_pVecList.size(); i++ ) {
		if( s_pVecList[i]->m_hItem == hItem ) {
			s_pFocus = s_pVecList[i];
			return;
		}
	}
}
*/