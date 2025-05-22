#pragma once

#include "EtWorldSound.h"
#include "EtSoundChannel.h"
class CUnionValueProperty;
class CEtWorldSector;
class CTEtWorldSound : public CEtWorldSound {
public:
	CTEtWorldSound( CEtWorldSector *pSector );
	virtual ~CTEtWorldSound();

	virtual CEtWorldSoundEnvi *AllocSoundEnvi();

protected:
	int m_nSoundIndex;
	EtSoundChannelHandle m_hChannel;

public:
	bool Save( const char *szFileName );

	virtual bool Load( const char *szFileName );
	virtual void Play();
	virtual void Stop();
	virtual void SetVolume( float fValue );

	void SetBGM( const char *szName ) { m_szBGM = szName; }

	void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );



};