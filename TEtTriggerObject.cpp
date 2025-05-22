#include "StdAfx.h"
#include "TEtTriggerObject.h"
#include "TEtTriggerElement.h"
#include "SundriesFuncEx.h"

CTEtTriggerObject::CTEtTriggerObject( CEtTrigger *pTrigger )
: CEtTriggerObject( pTrigger )
{

}

CTEtTriggerObject::~CTEtTriggerObject()
{
}

void CTEtTriggerObject::SetCategory( const char *szName )
{
	m_szCategory = szName;
}

void CTEtTriggerObject::SetTriggerName( const char *szName )
{
	m_szTriggerName = szName;
}

void CTEtTriggerObject::AddConditionElement( CEtTriggerElement *pElement )
{
	m_pVecCondition.push_back( pElement );
}

void CTEtTriggerObject::AddActionElement( CEtTriggerElement *pElement )
{
	m_pVecAction.push_back( pElement );
}

void CTEtTriggerObject::AddEventElement( CEtTriggerElement *pElement )
{
	m_pVecEvent.push_back( pElement );
}

CEtTriggerElement *CTEtTriggerObject::AllocTriggerElement()
{
	return new CTEtTriggerElement( this );
}

CTEtTriggerElement *CTEtTriggerObject::GetElementFromTreeIndex( HTREEITEM hti )
{
	for( DWORD i=0; i<m_pVecCondition.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecCondition[i];
		if( pElement->GetTreeItem() == hti ) return pElement;
	}
	for( DWORD i=0; i<m_pVecAction.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecAction[i];
		if( pElement->GetTreeItem() == hti ) return pElement;
	}
	for( DWORD i=0; i<m_pVecEvent.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecEvent[i];
		if( pElement->GetTreeItem() == hti ) return pElement;
	}
	return NULL;
}

void CTEtTriggerObject::RemoveElementFromTreeIndex( HTREEITEM hti )
{
	for( DWORD i=0; i<m_pVecCondition.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecCondition[i];
		if( pElement->GetTreeItem() == hti ) {
			SAFE_DELETE( m_pVecCondition[i] );
			m_pVecCondition.erase( m_pVecCondition.begin() + i );
			return;
		}
	}
	for( DWORD i=0; i<m_pVecAction.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecAction[i];
		if( pElement->GetTreeItem() == hti ) {
			SAFE_DELETE( m_pVecAction[i] );
			m_pVecAction.erase( m_pVecAction.begin() + i );
			return;
		}
	}

	for( DWORD i=0; i<m_pVecEvent.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecEvent[i];
		if( pElement->GetTreeItem() == hti ) {
			SAFE_DELETE( m_pVecEvent[i] );
			m_pVecEvent.erase( m_pVecEvent.begin() + i );
			return;
		}
	}
}

bool CTEtTriggerObject::RemoveElement( CEtTriggerElement *pTriggerElement )
{
	for( DWORD i=0; i<m_pVecCondition.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecCondition[i];
		if( pElement == pTriggerElement ) {
			SAFE_DELETE( m_pVecCondition[i] );
			m_pVecCondition.erase( m_pVecCondition.begin() + i );
			return true;
		}
	}
	for( DWORD i=0; i<m_pVecAction.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecAction[i];
		if( pElement == pTriggerElement ) {
			SAFE_DELETE( m_pVecAction[i] );
			m_pVecAction.erase( m_pVecAction.begin() + i );
			return true;
		}
	}

	for( DWORD i=0; i<m_pVecEvent.size(); i++ ) {
		CTEtTriggerElement *pElement = (CTEtTriggerElement*)m_pVecEvent[i];
		if( pElement == pTriggerElement ) {
			SAFE_DELETE( m_pVecEvent[i] );
			m_pVecEvent.erase( m_pVecEvent.begin() + i );
			return true;
		}
	}
	return false;
}

void CTEtTriggerObject::ChangeConditionOrder( CEtTriggerElement *pElement, CEtTriggerElement *pPrevElement )
{
	std::vector<CEtTriggerElement *> *pVecList;

	switch( pElement->GetType() ) {
		case CEtTriggerElement::Event: pVecList = &m_pVecEvent; break;
		case CEtTriggerElement::Condition: pVecList = &m_pVecCondition; break;
		case CEtTriggerElement::Action: pVecList = &m_pVecAction; break;
	}

	for( DWORD i=0; i<pVecList->size(); i++ ) {
		if( (*pVecList)[i] == pElement ) {
			pVecList->erase( pVecList->begin() + i );
			break;
		}
	}

	if( pPrevElement == NULL ) {
		pVecList->insert( pVecList->begin(), pElement );
	}
	else {
		for( DWORD i=0; i<pVecList->size(); i++ ) {
			if( (*pVecList)[i] == pPrevElement ) {
				pVecList->insert( pVecList->begin() + i + 1, pElement );
				break;
			}
		}
	}
}

bool CTEtTriggerObject::Save( FILE *fp )
{
	CTEtTriggerElement *pElement;

	char *pDummyBuf = new char[ s_nDummySize ];
	memset( pDummyBuf, 0, s_nDummySize );

	WriteCString( &CString(m_szCategory.c_str()), fp );
	WriteCString( &CString(m_szTriggerName.c_str()), fp );
	fwrite( &m_bRepeat, sizeof(bool), 1, fp );
	fwrite( pDummyBuf, s_nDummySize, 1, fp );

	DWORD dwCount = GetConditionCount();
	fwrite( &dwCount, sizeof(DWORD), 1, fp );
	for( DWORD i=0; i<dwCount; i++ ) {
		pElement = (CTEtTriggerElement *)GetConditionFromIndex(i);
		pElement->Save(fp);
	}


	dwCount = GetActionCount();
	fwrite( &dwCount, sizeof(DWORD), 1, fp );
	for( DWORD i=0; i<dwCount; i++ ) {
		pElement = (CTEtTriggerElement *)GetActionFromIndex(i);
		pElement->Save(fp);
	}

	dwCount = GetEventCount();
	fwrite( &dwCount, sizeof(DWORD), 1, fp );
	for( DWORD i=0; i<dwCount; i++ ) {
		pElement = (CTEtTriggerElement *)GetEventFromIndex(i);
		pElement->Save(fp);
	}
	CString szStr = m_szDescription.c_str();
	WriteCString( &szStr, fp );

	SAFE_DELETEA( pDummyBuf );
	return true;
}



CTEtTriggerObject &CTEtTriggerObject::operator = ( CTEtTriggerObject &e )
{
	m_szCategory = e.m_szCategory;
	m_szTriggerName = e.m_szTriggerName;
	m_szDescription = e.m_szDescription;

	for( DWORD i=0; i<e.m_pVecEvent.size(); i++ ) {
		CEtTriggerElement *pElement = AllocTriggerElement();
		*(CTEtTriggerElement*)pElement = *(CTEtTriggerElement*)e.m_pVecEvent[i];
		m_pVecEvent.push_back( pElement );
	}
	for( DWORD i=0; i<e.m_pVecCondition.size(); i++ ) {
		CEtTriggerElement *pElement = AllocTriggerElement();
		*(CTEtTriggerElement*)pElement = *(CTEtTriggerElement*)e.m_pVecCondition[i];
		m_pVecCondition.push_back( pElement );
	}
	for( DWORD i=0; i<e.m_pVecAction.size(); i++ ) {
		CEtTriggerElement *pElement = AllocTriggerElement();
		*(CTEtTriggerElement*)pElement = *(CTEtTriggerElement*)e.m_pVecAction[i];
		m_pVecAction.push_back( pElement );
	}

	m_nTick = e.m_nTick;
	m_bRepeat = e.m_bRepeat;
	m_bEnable = e.m_bEnable;

	return *this;
}