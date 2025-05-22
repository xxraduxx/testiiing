#include "StdAfx.h"
#include "TEtBuffProp.h"

CTEtBuffProp::CTEtBuffProp()
{
}

CTEtBuffProp::~CTEtBuffProp()
{
}

bool CTEtBuffProp::Render( LOCAL_TIME LocalTime )
{
	if( !m_Handle ) return false;

	CTEtWorldActProp::Render( LocalTime );

	if( m_bSelect ) DrawBuffRange();

	return true;
}

void CTEtBuffProp::DrawBuffRange()
{
	EtVector3 vVec[2];
	MatrixEx Cross;
	int nDist = 40;
	float fAngle = 360 / (float)(nDist-1.f);
	float fRange = GetRange();
	if( fRange <= 0.0f ) return;

	Cross.m_vPosition = m_matExWorld.m_vPosition;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateYaw( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = m_matExWorld.m_vPosition;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotatePitch( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = m_matExWorld.m_vPosition;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vXAxis * fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateRoll( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vXAxis * fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}
}

float CTEtBuffProp::GetRange()
{
	return GetProperty(m_nCustomParamOffset+3)->GetVariableFloat();
}