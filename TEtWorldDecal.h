#pragma once

#include "EtWorldDecal.h"
#include "Timer.h"

class CTEtWorldDecal : public CEtWorldDecal
{
public:
	CTEtWorldDecal( CEtWorldSector *pSector );
	virtual ~CTEtWorldDecal();

protected:
	bool m_bSelect;

	EtTextureHandle m_hTexture;
	EtDecalHandle m_hDecal;

	EtTextureHandle m_hSelectTexture;
	EtDecalHandle m_hSelectDecal;

protected:
	EtColor MakeColor();
	void MakeSelectDecal();


public:
	virtual bool Initialize( EtVector2 vPos, float fRadius, float fRotate, EtColor Color, float fAlpha, const char *szTextureName );
	void Reinitialize( bool bReload = false );
	void Finalize();

	void SetSelect( bool bValue );
	void Render( LOCAL_TIME LocalTime );

	void SetTextureName( const char *szName );
	void SetRadius( float fValue );
	void SetRotate( float fValue );
	void SetColor( EtColor Color );
	void SetPosition( EtVector2 vPos );
	void SetAlpha( float fAlpha );

	void WriteDecalInfo( FILE *fp );
	bool ReadDecalInfo( FILE *fp );

};