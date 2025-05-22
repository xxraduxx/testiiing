#pragma once

#include "EnviLightBase.h"
#include "EtMatrixEx.h"
class CEnviLightDirection : public CEnviLightBase
{
public:
	CEnviLightDirection( CEnviElement *pElement );
	virtual ~CEnviLightDirection();

	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual bool OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver );
	virtual PropertyGridBaseDefine *GetPropertyDefine();
	virtual void OnCustomDraw();

protected:
	static EtVector3 s_vDirection[18];
	static int s_nRefCount;
	MatrixEx m_matExWorld;
	int m_nMouseFlag;

};