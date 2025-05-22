#pragma once

#include "EtWorldProp.h"
#include "Timer.h"
#include "EtMatrixEx.h"
#include "PropertyGridCreator.h"
#include "navigationmesh.h"


class CAxisRenderObject;
class CTEtWorldProp : public CEtWorldProp
{
public:
	CTEtWorldProp();
	virtual ~CTEtWorldProp();

	/*
	enum PropTypeEnum {
		Unknown = -1, 
		Static = 0,
		Action,
		Broken,
		Trap,
		Light,
		Operation,
		Chest,

		PropTypeEnum_Amount,
	};
	*/


protected:
	MatrixEx m_matExWorld;
	CAxisRenderObject *m_pAxisRenderObject;
	NavigationMesh m_NavData;
	bool m_bSelect;
	bool m_bShow;
	bool m_bIncludeNavMesh;
	int m_nCustomParamOffset;

	// Custom Parameter
	std::vector<CUnionValueProperty *> m_pVecList;
	std::vector<EtVector2> m_VecVector2List;
	std::vector<EtVector3> m_VecVector3List;
	std::vector<EtVector4> m_VecVector4List;
	std::vector<CString> m_VecStringList;
	std::vector<int> m_nVecVector2TableIndexList;
	std::vector<int> m_nVecVector3TableIndexList;
	std::vector<int> m_nVecVector4TableIndexList;
	std::vector<int> m_nVecStringTableIndexList;

protected:
	void DrawBoundingBox( EtMatrix &mat, D3DCOLOR Color = 0xFFFFFFFF, float fLengthPer = 0.2f );

public:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool Render( LOCAL_TIME LocalTime );

	void SetSelect( bool bValue );
	virtual bool LoadObject();
	virtual bool FreeObject();
	virtual void UpdateMatrixEx();
	virtual void Show( bool bShow );
	void LoadNavMesh();

	void EnableCastShadow( bool bEnable );
	void EnableReceiveShadow( bool bEnable );
	void EnableCastLightmap( bool bEnable );
	void EnableLightmapInfluence( bool bEnable );
	void EnableCashWater( bool bEnable );

	bool CheckAxis( EtVector3 &vOrig, EtVector3 &vDir );
	virtual void MoveAxis( int nX, int nY, float fSpeed );

	bool IsIncludeNavMesh() { return m_bIncludeNavMesh; }

	bool IsEmptyPickMesh();

	MatrixEx *GetMatEx() { return &m_matExWorld; }

//	virtual int GetAdditionalPropInfoSize();
//	virtual void AllocAdditionalPropInfo();
	void WritePropInfo( FILE *fp );
	bool ReadPropInfo( FILE *fp );

	std::vector<CUnionValueProperty *> *GetPropertyDefine() { return &m_pVecList; }
//	virtual PropertyGridBaseDefine *GetPropertyDefine();
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty );


	void CopyCustomParameter( CTEtWorldProp *pProp );
	void CalcArray();
	DWORD GetPropertyCount() { return (DWORD)m_pVecList.size(); }
	CUnionValueProperty *GetProperty( DWORD dwIndex ) { return m_pVecList[dwIndex]; }
	void AllocCustomInfo();
	void SetDefaultCustomValue( int nClass, DWORD dwIndex );

	NavigationMesh *GetNavMesh() { return &m_NavData; }

	void WriteCustomInfo( FILE *fp );
	void ReadCustomInfo( FILE *fp );

#ifdef _CHECK_MISSING_INDEX
	bool ReadPropInfo_ForTheLog( FILE *fp );
	void ReadCustomInfo_ForTheLog( FILE *fp );
#endif
};

#define PROP_DEFAULT_PROP_COUNT 10