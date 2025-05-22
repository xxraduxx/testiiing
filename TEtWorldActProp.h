#pragma once

#include "TEtWorldProp.h"
#include "EtActionBase.h"
#include "Timer.h"
class CTEtWorldActProp : public CTEtWorldProp, public CEtActionBase
{
public:
	CTEtWorldActProp();
	virtual ~CTEtWorldActProp();

protected:
	LOCAL_TIME m_LocalTime;
	LOCAL_TIME m_ActionTime;
	std::vector<int> m_nVecAniIndexList;
	std::string m_szAction;
	float m_fFrame;
	float m_fPrevFrame;
	int m_nActionIndex;
	int m_nLoopCount;
	bool m_bExistAction;

public:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool Render( LOCAL_TIME LocalTime );
	virtual bool LoadObject();
	virtual bool FreeObject();

	virtual void OnSignal( int nIndex, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalEndTime );

	static void CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
	static void CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );

	void ProcessAction( LOCAL_TIME LocalTime );
	void ProcessSignal( ActionElementStruct *pStruct, float fFrame, float fPrevFrame );
	void SetAction( const char *szActionName, float fFrame, float fBlendFrame );
	void CacheAniIndex();

	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );

	int GetActionIndex() { return m_nActionIndex; }
};