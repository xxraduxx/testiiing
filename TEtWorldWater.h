#pragma once

#include "EtWorldWater.h"

class CUnionValueProperty;
class CPropertyGridImp;
class CTEtWorldWaterRiver;
class CTEtWorldWater : public CEtWorldWater
{
public:
	CTEtWorldWater( CEtWorldSector *pSector );
	virtual ~CTEtWorldWater();

protected:
	CEtMeshStream *m_pMesh;
//	float *m_pAlphaTable;
	std::vector< SCustomParam > m_vecCustomParam;
	EtMaterialHandle m_hMaterial;
	std::map<std::string, EtTextureHandle> m_MapTextureList;

	std::vector< CUnionValueProperty * > m_pVecPropertyList;
	EtTextureHandle m_hAlphaTexture;
	int m_nEngineWaterCount;

protected:
	bool CreateMesh();

	void RefreshTextureCoord();
	void RefreshVertex();
	void RefreshIndex();
	void RefreshNormal();
	void RefreshShader();
	void RefreshTexture( bool bInit = false );
	void RefreshTextureAlpha();

	float GetNewAlpha( float *pBuffer, int x, int y, int nBlurValue, float **ppfTable );

	void RefreshWaterTexture();

public:
	bool Save( const char *szFileName );

	virtual bool Load( const char *szFileName );
	virtual CEtWorldWaterRiver *AllocRiver();

	virtual void Render( LOCAL_TIME LocalTime );

	void GenerationAlphaMap();

	std::vector<CUnionValueProperty *> *GetPropertyList() { return &m_pVecPropertyList; }
	void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );
	void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );
	void OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );

	CTEtWorldWaterRiver *AddRiver( EtVector3 &vPos );
	void RemoveRiver( CTEtWorldWaterRiver *pRiver );

	bool RemoveRiverPoint( CTEtWorldWaterRiver::RiverPoint *pPoint );

	CString GetRiverUniqueName();

	void AddEngineWaterCount();
	void DelEngineWaterCount();
	int GetEngineWaterIndex( void *pWaterObject );
};