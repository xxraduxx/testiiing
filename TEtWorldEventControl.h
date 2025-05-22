#pragma once

#include "EtWorldEventControl.h"

class CEtWorldSector;
class CTEtWorldSector;
class CTEtWorldEventControl : public CEtWorldEventControl 
{
public:
	CTEtWorldEventControl( CEtWorldSector *pSector );
	virtual ~CTEtWorldEventControl();

protected:
	std::string m_szName;

public:
	virtual bool Initialize();

	virtual CEtWorldEventArea *AllocArea();

	const char *GetName() { return m_szName.c_str(); }
	void SetName( const char *szName ) { m_szName = szName; }

	bool SaveControl( FILE *fp );
	bool LoadControl( FILE *fp );

	CEtWorldEventArea *InsertArea( EtVector3 &vMin, EtVector3 &vMax, const char *szName, int nUniqueID );
};