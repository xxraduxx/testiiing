#pragma once

class CUnionValueProperty;
class CInputReceiver;

#include "EnviControl.h"
#include "PropertyGridCreator.h"
#include "Timer.h"

class CEnviLightBase {
public:
	CEnviLightBase( CEnviElement *pElement );
	virtual ~CEnviLightBase();

	static CEnviLightBase *s_pFocus;
protected:
	HTREEITEM m_hItem;
	CEnviElement *m_pElement;
	CEnvInfo::LightInfoStruct *m_pElementPtr;
	bool m_bFocus;
	static std::vector<CEnviLightBase *> s_pVecList;

public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex ) {}
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex ) {}
	virtual bool OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver ) { return true; }
	virtual void OnCustomDraw() {}
	virtual PropertyGridBaseDefine *GetPropertyDefine() { return NULL; }

	static bool AddLight( HTREEITEM hItem, CEnviElement *pElement, DWORD dwIndex );
	static bool RemoveLight( HTREEITEM hItem );
	static CEnviLightBase *FindLight( HTREEITEM hItem );
	static void Reset();
	void SetFocus( bool bValue ) { 
		m_bFocus = bValue; 
		if( m_bFocus == true ) s_pFocus = this;
	}
	bool IsFocus() { return m_bFocus; }
//	static void SetFocus( HTREEITEM hItem );

	CEnviElement *GetElement() { return m_pElement; }
	SLightInfo *GetLightInfo() { return m_pElementPtr->pInfo; }

};