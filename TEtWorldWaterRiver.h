#pragma once

#include "EtWorldWaterRiver.h"
#include "Timer.h"
#include "EtCustomRender.h"

class CPointAxisRenderObject;
class CUnionValueProperty;
class CPropertyGridImp;
class CTEtWorldWaterRiver : public CEtWorldWaterRiver, public CEtCustomRender
{
public:
	CTEtWorldWaterRiver( CEtWorldWater *pWater );
	virtual ~CTEtWorldWaterRiver();

	struct RiverPoint {
		EtVector3 vPos;
		bool bSelect;
		CPointAxisRenderObject *pAxis;
	};

protected:
	std::vector<RiverPoint *> m_pVecPointList;

	CEtMeshStream *m_pMesh;
	std::vector< SCustomParam > m_vecCustomParam;
	EtMaterialHandle m_hMaterial;
	std::map<std::string, EtTextureHandle> m_MapTextureList;
	EtTextureHandle m_hAlphaTexture;

	std::vector<EtVector3> m_VecPrimitiveList;
	EtVector2 m_vSize;
	EtVector2 m_vOffset;
	bool m_bDestroy;

	std::vector< CUnionValueProperty * > m_pVecPropertyList;


protected:
	bool CreateMesh();

	void RefreshTextureCoord();
	void RefreshVertex();
	void RefreshIndex();
	void RefreshNormal();
	void RefreshShader();
	void RefreshTexture( bool bInit = false );
	void RefreshTextureAlpha();

	void RefreshPropertyList();

	void RefreshSize();
	float GetNewAlpha( float *pBuffer, int x, int y, int nBlurValue, float **ppfTable );
	bool GetRiverHeight( EtVector2 &vPos, float &fHeight );
public:
	RiverPoint *InsertPoint( EtVector3 &vPos, EtVector3 *pInsertAfter = NULL );
	bool RemovePoint( RiverPoint *pPoint );
	int GetPointCount() { return (int)m_pVecPointList.size(); }
	void UpdatePoint();

	void Render( LOCAL_TIME LocalTime );
	virtual void RenderCustom( float fElapsedTime );
	bool CheckSelect( EtVector2 vMin, EtVector2 vMax, char cFlag );

	void RefreshPrimitive();
	void RefreshWaterTexture();

	std::vector<CUnionValueProperty *> *GetPropertyList() { return &m_pVecPropertyList; }
	void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );
	void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );
	void OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp );

	void GenerationAlphaMap();
	bool IsDestroy() { return m_bDestroy; }

	bool Save( FILE *fp );
	virtual bool Load( CStream *pStream );
};