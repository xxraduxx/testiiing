#pragma once

#include "EtTriggerObject.h"
class CEtTriggerElement;
class CTEtTriggerObject : public CEtTriggerObject
{
public:
	CTEtTriggerObject( CEtTrigger *pTrigger );
	virtual ~CTEtTriggerObject();

protected:

public:
	virtual CEtTriggerElement *AllocTriggerElement();
	bool Save( FILE *fp );

	void SetCategory( const char *szName );
	void SetTriggerName( const char *szName );

	void AddConditionElement( CEtTriggerElement *pElement );
	void AddActionElement( CEtTriggerElement *pElement );
	void AddEventElement( CEtTriggerElement *pElement );
	CTEtTriggerElement *GetElementFromTreeIndex( HTREEITEM hti );
	void RemoveElementFromTreeIndex( HTREEITEM hti );
	bool RemoveElement( CEtTriggerElement *pElement );

	void ChangeConditionOrder( CEtTriggerElement *pElement, CEtTriggerElement *pPrevElement );

	void SetDescription( const char *szStr ) { m_szDescription = szStr; }
	const char *GetDescription() { return m_szDescription.c_str(); }

	CTEtTriggerObject & operator = ( CTEtTriggerObject &e );
};