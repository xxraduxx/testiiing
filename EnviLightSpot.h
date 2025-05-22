#pragma once

#include "EnviLightBase.h"
#include "EtMatrixEx.h"

class CEnviLightSpot : public CEnviLightBase
{
public:
	CEnviLightSpot( CEnviElement *pElement );
	virtual ~CEnviLightSpot();

	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual bool OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver );
	virtual PropertyGridBaseDefine *GetPropertyDefine();
	virtual void OnCustomDraw();

	void DrawSpotFunc1( float fAngle, SLightInfo *pInfo, bool bYawPitch, DWORD dwColor );

protected:
	MatrixEx m_matExWorld;
	int m_nMouseFlag;

};