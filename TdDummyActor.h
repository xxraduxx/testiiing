#pragma once

#include "Timer.h"
#include "InputReceiver.h"
#include "EtMatrixEx.h"
#include "Singleton.h"

class CTdDummyActor : public CInputReceiver, public CSingleton< CTdDummyActor >
{
public:
	CTdDummyActor();
	virtual ~CTdDummyActor();

protected:
	EtAniObjectHandle m_Handle;
	MatrixEx m_matExWorld;
	MatrixEx m_Camera;
	float m_fCameraDistance;
	EtVector3 m_vVelocity;
	EtVector3 m_vCurVelocity;
	LOCAL_TIME m_AniTime;
	LOCAL_TIME m_PrevTime;
	int m_nAniIndex;

	EtLightHandle m_LightHandle;
	void UpdateCamera();

	void ChangeAnimation( int nAniIndex );
public:
	virtual bool Initialize();
	virtual void Destroy();

	virtual void Render( LOCAL_TIME LocalTime );
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	void BeginActor();
	void EndActor();

	MatrixEx *GetDummyCross() { return &m_matExWorld; }

};