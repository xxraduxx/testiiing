#pragma once

#include "EtWorldEventArea.h"
#include "PropertyGridCreator.h"

class CUnionValueProperty;
class CEventCustomRender;
class CTEtWorldEventControl;
class CEtWorldEventControl;

extern PropertyGridBaseDefine g_EventAreaPropertyDefine[];

class CTEtWorldEventArea : public CEtWorldEventArea
{
public:
	CTEtWorldEventArea( CEtWorldEventControl *pControl, const char *szName );
	virtual ~CTEtWorldEventArea();

protected:
	int m_nCustomParamOffset;

	std::string m_szControlName;
	std::vector<CUnionValueProperty *> m_pVecList;
	std::vector<EtVector2> m_VecVector2List;
	std::vector<EtVector3> m_VecVector3List;
	std::vector<EtVector4> m_VecVector4List;
	std::vector<CString> m_VecStringList;
	std::vector<int> m_nVecVector2TableIndexList;
	std::vector<int> m_nVecVector3TableIndexList;
	std::vector<int> m_nVecVector4TableIndexList;
	std::vector<int> m_nVecStringTableIndexList;

	CEventCustomRender *m_pCustomRender;

public:
	std::vector<CUnionValueProperty *> *OnInitProperty();
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

	void Save( FILE *fp );
	void Load( FILE *fp );
	void CalcArray();

	void InitializeCustomRender();
	void ModifyCustomRender();

	DWORD GetPropertyCount() { return (DWORD)m_pVecList.size(); }
	CUnionValueProperty *GetProperty( DWORD dwIndex ) { return m_pVecList[dwIndex]; }

	void UpdatePropertyPositionInfo();	//Min/Max값으로 center값 계산해서 Property로 설정되어 있는 "Position"값 갱신..
	void UpdateMinMaxInfo();			//Property값 변경시 Min/Max값 갱신..

	void UpdatePropertyRotationInfo();
};
