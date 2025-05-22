#pragma once

#include "TEtWorldProp.h"

class CTEtWorldLightProp : public CTEtWorldProp {
public:
	CTEtWorldLightProp();
	virtual ~CTEtWorldLightProp();

protected:
	/*
	struct LightInfoStruct {
		SLightInfo Info;
		char cReserved[432];	// Client와 함께 고쳐주는 센스
	};
	*/
	EtLightHandle m_LightHandle;

	EtVector3 m_vDirection[18];
	EtVector3 m_vPoint[30];

protected:
	void DrawDirectionLight();
	void DrawPointLight();
	void DrawSpotLight();

	void CreateLight();
	void RefreshLight();
	void DrawSpotFunc1( float fAngle, SLightInfo *pInfo, bool bYawPitch, DWORD dwColor );
	SLightInfo GetLightInfo();
	void SetLightInfo( SLightInfo Info );


public:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool Render( LOCAL_TIME LocalTime );

	virtual bool LoadObject();
	virtual bool FreeObject();
	virtual void UpdateMatrixEx();
	virtual void Show( bool bShow );
	virtual void MoveAxis( int nX, int nY, float fSpeed );

//	virtual int GetAdditionalPropInfoSize();
//	virtual void AllocAdditionalPropInfo();

//	virtual PropertyGridBaseDefine *GetPropertyDefine();
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );

	//Prop용 m_Handle만 show/hide
	void ShowProp( bool bShow );
};