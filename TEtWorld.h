#pragma once

#include "EtWorld.h"
#include "Singleton.h"
class CTEtWorldSector;
class CEtWorldGrid;
class CTEtWorldGrid;
struct PropertyGridBaseDefine;

class CTEtWorld : public CEtWorld, public CSingleton<CTEtWorld>
{
public:
	CTEtWorld();
	virtual ~CTEtWorld();

protected:
	bool m_bAccesThread;

	struct DefineEventControlStruct {
		std::string szControlName;
		std::vector<PropertyGridBaseDefine> VecDefine;
	};

	std::vector<DefineEventControlStruct> m_VecDefineEventControlList;

protected:
	virtual CEtWorldGrid *AllocGrid();
	void DrawGrid( CEtWorldGrid *pGrid );
	void DrawGrid( float fLeft, float fTop, float fRight, float fBottom, DWORD dwColor, CEtWorldGrid *pGrid, SectorIndex Index );
	void CalcGridHeightPos( CEtWorldGrid *pGrid, float fValue, float &fMaxValue );
	bool DeleteFileInSubDirectory( CString szPath );

public:
	virtual bool Initialize( const char *szWorldFolder, const char *szGridName );
	virtual void Destroy();
	void RenderGrid();
	void RenderHeightmap( SectorIndex Index );

	bool CreateEmptyWorld();
	bool CreateEmptyGrid( const char *szGridName, const char *szParentName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize );
	bool DestroyGrid( const char *szGridName );
	bool IsDestroyGrid( const char *szGridName );
	bool MoveGrid( const char *szGridName, const char *szParentName );
	bool ChangeGridName( const char *szGridName, const char *szNewName );

	bool IsExistGrid( CString szGridName );
	void GetGridRect( CEtWorldGrid *pGrid, DWORD dwX, DWORD dwY, D3DXVECTOR3 *vMin, D3DXVECTOR3 *vMax );

	void RefreshCommonInfo();

	CTEtWorldSector *GetSector( SectorIndex Index, const char *szGridName = NULL );
	bool IsEmptySector( const char *szGridName, SectorIndex Index );
	void SetCurrentGrid( const char *szGridName );
	CTEtWorldGrid *GetCurrentGrid();
	const char *GetCurrentGridName();

	void LoadNavMesh();
};