#pragma once
#include "EtWorldGrid.h"
#include "../EtWorldBase/Common.h"
class CEtWorld;

class CTEtWorldGrid : public CEtWorldGrid
{
public:
	CTEtWorldGrid( CEtWorld *pWorld );
	virtual ~CTEtWorldGrid();

protected:
	CString m_szParentName;
	float m_fYPos;
	CString m_szDescription;

protected:
	void LoadGridDescription( const char *szName );

public:
	virtual CEtWorldSector *AllocSector();
	bool CreateEmptyGrid( CString szGridName, CString szParentName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize );
	virtual bool LoadGrid( const char *szName );

	bool IsEmptySector( SectorIndex Index );
	void CreateSector( SectorIndex Index );
	void EmptySector( SectorIndex Index );

	void LoadCommonInfo( bool bLoadThumbnail = true );

	CString GetParentName() { return m_szParentName; }
	void SetParentName( CString szStr ) { m_szParentName = szStr; }
	bool WriteGridInfo();

	void SetYPos( float fValue ) { m_fYPos = fValue; }
	float GetYPos() { return m_fYPos; }

	CString GetDescription() { return m_szDescription; }
	void SetDescription( CString &szStr ) { m_szDescription = szStr; }

	void WriteGridDescription();
};