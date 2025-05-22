#pragma once

#include "EtWorldSoundEnvi.h"
#include "fmod.hpp"
#include "EtSoundChannel.h"

class CUnionValueProperty;
class CAxisLockRenderObject;
class CEtWorldSound;
class CTEtWorldSoundEnvi : public CEtWorldSoundEnvi
{
public:
	CTEtWorldSoundEnvi( CEtWorldSound *pSound );
	virtual ~CTEtWorldSoundEnvi();

	CAxisLockRenderObject *m_pAxis;
protected:
	int m_nSoundIndex;
	EtSoundChannelHandle m_hChannel;
//	CEtSoundChannel *m_pChannel;
	FMOD_VECTOR m_vRolloffArray[3];

public:
	bool Save( FILE *fp );

	void SetName( const char *szName ) { m_szName = szName; }
	void SetFileName( const char *szName ) { m_szFileName = szName; }
	void SetPosition( EtVector3 &vVec ) { m_vPosition = vVec; }
	void SetRange( float fValue ) { m_fRange = fValue; }
	void SetRollOff( float fValue ) { m_fRollOff = fValue; }
	void SetVolume( float fValue );
	void SetStream( bool bStream );

	void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	void ShowAxis( bool bShow );
	void UpdateAxis( EtMatrix *pmat );
	void MoveAxis( int nX, int nY );
	bool CheckAxis( EtVector3 &vPos, EtVector3 &vDir );
	void SetAxisScale( float fValue );

	void Play();
	void Stop();
};