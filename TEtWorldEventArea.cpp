#include "StdAfx.h"
#include "TEtWorldEventArea.h"
#include "SundriesFuncEx.h"
#include "EventSignalManager.h"
#include "EventSignalItem.h"
#include "EventCustomRender.h"
#include "TEtWorldEventControl.h"

#include "EtWorldGrid.h"
#include "EtWorldSector.h"


PropertyGridBaseDefine g_EventAreaPropertyDefine[] = {
	{ "Common", "Rotation", CUnionValueProperty::Float, "Rotation", TRUE },
	{ NULL },
};


CTEtWorldEventArea::CTEtWorldEventArea( CEtWorldEventControl *pControl, const char *szName )
: CEtWorldEventArea( pControl )
{
	m_szControlName = szName;
	m_pCustomRender = NULL;

	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromName( CString(szName) );
	if( pItem == NULL ) return;

	CUnionValueProperty *pPropItem;
	m_nCustomParamOffset = 0;
	for( int i=0; ; i++ ) {
		if( g_EventAreaPropertyDefine[i].szCategory == NULL ) break;
		pPropItem = new CUnionValueProperty( g_EventAreaPropertyDefine[i].Type );
		pPropItem->SetCategory( g_EventAreaPropertyDefine[i].szCategory );
		pPropItem->SetDescription( g_EventAreaPropertyDefine[i].szStr );
		pPropItem->SetSubDescription( g_EventAreaPropertyDefine[i].szDescription );
		pPropItem->SetDefaultEnable( g_EventAreaPropertyDefine[i].bEnable );
		m_pVecList.push_back( pPropItem );
		m_nCustomParamOffset = i + 1;
	}
	for( DWORD i=0; i<pItem->GetParameterCount(); i++ ) {
		CUnionValueProperty *pVariable = pItem->GetParameter(i);
		pPropItem = new CUnionValueProperty( pVariable->GetType() );
		*pPropItem = *pVariable;
		pPropItem->SetCategory( "Parameters" );
		m_pVecList.push_back( pPropItem );
	}
}

CTEtWorldEventArea::~CTEtWorldEventArea()
{
	SAFE_DELETE( m_pCustomRender );
	SAFE_DELETE_PVEC( m_pVecList );
}

std::vector<CUnionValueProperty *> *CTEtWorldEventArea::OnInitProperty()
{
	return &m_pVecList;
}

void CTEtWorldEventArea::Load( FILE *fp )
{
	union {
		int nBuffer[256];
		float fBuffer[256];
	};
	memset( nBuffer, 0, sizeof(nBuffer));
	fread( nBuffer, sizeof(nBuffer), 1, fp );

	int nVector2Index = 0;
	int nVector3Index = 0;
	int nVector4Index = 0;
	int nStringIndex = 0;
	m_VecVector2List.clear();
	m_VecVector3List.clear();
	m_VecVector4List.clear();
	m_VecStringList.clear();
	m_nVecVector2TableIndexList.clear();
	m_nVecVector3TableIndexList.clear();
	m_nVecVector4TableIndexList.clear();
	m_nVecStringTableIndexList.clear();

	int nCount;
	EtVector2 Vector2;
	EtVector3 Vector3;
	EtVector4 Vector4;
	CString szStr;

	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector2List.resize( nCount );
		fread( &m_VecVector2List[0], sizeof(EtVector2), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector3List.resize( nCount );
		fread( &m_VecVector3List[0], sizeof(EtVector3), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector4List.resize( nCount );
		fread( &m_VecVector4List[0], sizeof(EtVector4), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	fseek( fp, sizeof(int) * nCount, SEEK_CUR );
	for( int i=0; i<nCount; i++ ) {
		ReadCString( &szStr, fp );
		m_VecStringList.push_back( szStr );
	}

	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromUniqueIndex( ((CTEtWorldEventControl*)m_pControl)->GetUniqueID() );

	int nOffset;
	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		CUnionValueProperty *pSource = pItem->GetParameter(i-m_nCustomParamOffset);
		CUnionValueProperty *pVariable = m_pVecList[i];
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);

		switch( pSource->GetType() ) {
			case CUnionValueProperty::Integer:
			case CUnionValueProperty::Integer_Combo:
			case CUnionValueProperty::Color:
				pVariable->SetVariable( (int)nBuffer[nOffset] );
				break;
			case CUnionValueProperty::Boolean:
				pVariable->SetVariable( (bool)(nBuffer[nOffset]==TRUE) );
				break;
			case CUnionValueProperty::Float:
				pVariable->SetVariable( fBuffer[nOffset] );
				break;
			case CUnionValueProperty::Vector2:
				if( nVector2Index < (int)m_VecVector2List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR2)m_VecVector2List[nVector2Index] );
					nVector2Index++;
				}
				break;
			case CUnionValueProperty::Vector3:
				if( nVector3Index < (int)m_VecVector3List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR3)m_VecVector3List[nVector3Index] );
					nVector3Index++;
				}
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				if( nVector4Index < (int)m_VecVector4List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR4)m_VecVector4List[nVector4Index] );
					nVector4Index++;
				}
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				if( nStringIndex < (int)m_VecStringList.size() ) {
					pVariable->SetVariable( (char*)m_VecStringList[nStringIndex].GetBuffer() );
					nStringIndex++;
				}
				break;
		}
	}
}

void CTEtWorldEventArea::Save( FILE *fp )
{
	CalcArray();

	union {
		int nBuffer[256];
		float fBuffer[256];
	};
	memset( nBuffer, 0, sizeof(nBuffer));

	int nVector2Index = 0;
	int nVector3Index = 0;
	int nVector4Index = 0;
	int nStringIndex = 0;

	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromUniqueIndex( ((CTEtWorldEventControl*)m_pControl)->GetUniqueID() );

	int nOffset;
	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);
		switch( m_pVecList[i]->GetType() ) {
			case CUnionValueProperty::Integer:
			case CUnionValueProperty::Integer_Combo:
			case CUnionValueProperty::Color:
				nBuffer[nOffset] = m_pVecList[i]->GetVariableInt();
				break;
			case CUnionValueProperty::Boolean:
				nBuffer[nOffset] = (BOOL)m_pVecList[i]->GetVariableBool();
				break;
			case CUnionValueProperty::Float:
				fBuffer[nOffset] = m_pVecList[i]->GetVariableFloat();
				break;
			case CUnionValueProperty::Vector2:
				nBuffer[nOffset] = nVector2Index;
				nVector2Index++;
				break;
			case CUnionValueProperty::Vector3:
				nBuffer[nOffset] = nVector3Index;
				nVector3Index++;
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				nBuffer[nOffset] = nVector4Index;
				nVector4Index++;
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				nBuffer[nOffset] = nStringIndex;
				nStringIndex++;
				break;
		}
	}
	fwrite( nBuffer, sizeof(nBuffer), 1, fp );

	// Array 저장
	fwrite( &nVector2Index, sizeof(int), 1, fp );
	if( m_VecVector2List.size() ) {
		fwrite( &m_nVecVector2TableIndexList[0], sizeof(int), m_nVecVector2TableIndexList.size(), fp );
		fwrite( &m_VecVector2List[0], sizeof(EtVector2), m_VecVector2List.size(), fp );
	}

	fwrite( &nVector3Index, sizeof(int), 1, fp );
	if( m_VecVector3List.size() ) {
		fwrite( &m_nVecVector3TableIndexList[0], sizeof(int), m_nVecVector3TableIndexList.size(), fp );
		fwrite( &m_VecVector3List[0], sizeof(EtVector3), m_VecVector3List.size(), fp );
	}

	fwrite( &nVector4Index, sizeof(int), 1, fp );
	if( m_VecVector4List.size() ) {
		fwrite( &m_nVecVector4TableIndexList[0], sizeof(int), m_nVecVector4TableIndexList.size(), fp );
		fwrite( &m_VecVector4List[0], sizeof(EtVector4), m_VecVector4List.size(), fp );
	}

	fwrite( &nStringIndex, sizeof(int), 1, fp );
	if( m_VecStringList.size() )
		fwrite( &m_nVecStringTableIndexList[0], sizeof(int), m_nVecStringTableIndexList.size(), fp );
	for( DWORD i=0; i<m_VecStringList.size(); i++ ) {
		WriteCString( &m_VecStringList[i], fp );
	}
}

void CTEtWorldEventArea::CalcArray()
{
	m_VecVector2List.clear();
	m_VecVector3List.clear();
	m_VecVector4List.clear();
	m_VecStringList.clear();
	m_nVecVector2TableIndexList.clear();
	m_nVecVector3TableIndexList.clear();
	m_nVecVector4TableIndexList.clear();
	m_nVecStringTableIndexList.clear();

	//#43591 UnitArea 랜덤 리스폰 확인 요청
	//UnitArea 중간에 char cReserved1[4]가 있어서 TableIndex가 밀리는 경우가 발생.
	//ActionSignal에서와 같이 Offset값으로 저장 하도록 수정.
	int nOffset;
	CEventSignalItem *pItem = CEventSignalManager::GetInstance().GetSignalItemFromUniqueIndex( ((CTEtWorldEventControl*)m_pControl)->GetUniqueID() );

	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		CUnionValueProperty *pVariable = m_pVecList[i];
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);
		switch( pVariable->GetType() ) {
			case CUnionValueProperty::Vector2:
				m_VecVector2List.push_back( pVariable->GetVariableVector2() );
				m_nVecVector2TableIndexList.push_back(nOffset);
				break;
			case CUnionValueProperty::Vector3:
				m_VecVector3List.push_back( pVariable->GetVariableVector3() );
				m_nVecVector3TableIndexList.push_back(nOffset);
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				m_VecVector4List.push_back( pVariable->GetVariableVector4() );
				m_nVecVector4TableIndexList.push_back(nOffset);
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				m_VecStringList.push_back( CString( pVariable->GetVariableString() ) );
				m_nVecStringTableIndexList.push_back(nOffset);
				break;
		}
	}
}


void CTEtWorldEventArea::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}

void CTEtWorldEventArea::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	if( m_pCustomRender ) m_pCustomRender->OnModify();
	switch( pVariable->GetType() ) {
		case CUnionValueProperty::String_FileOpen:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				pVariable->SetVariable( szTemp );
			}
			break;
		case CUnionValueProperty::Vector3:
			{
				if (_stricmp(pVariable->GetDescription(), "Position") == 0)
				{
					UpdateMinMaxInfo();
				}
			}
			break;
		case CUnionValueProperty::Float:
			{
				if (_stricmp(pVariable->GetDescription(), "Rotation") == 0)
				{
					float fValue = pVariable->GetVariableFloat();
					SetRotate(fValue);
				}
			}
			break;
	}
}


void CTEtWorldEventArea::InitializeCustomRender()
{
	m_pCustomRender = CEventCustomRender::AllocCustomRender( m_szControlName.c_str() );
	if( m_pCustomRender ) {
		m_pCustomRender->SetEventArea( this );
		m_pCustomRender->Initialize();
	}

}
void CTEtWorldEventArea::ModifyCustomRender()
{
	if( m_pCustomRender ) {
		m_pCustomRender->OnModify();
	}

	UpdatePropertyPositionInfo();
}


void CTEtWorldEventArea::UpdatePropertyPositionInfo()
{
	EtVector3 Center = m_vMin + ( m_vMax - m_vMin ) / 2.f;
	/*
	CalcOBB();
	SOBB *pSOBB = GetOBB();
	if (pSOBB)
		Center = pSOBB->Center;
	*/
	
	EtVector3 Offset = EtVector3(0.0f, 0.0f, 0.0f);
	CEtWorldSector *pSector = NULL;
	CEtWorldGrid *pParentGrid = NULL;
	CEtWorldEventControl *pControl = GetControl();

	pSector = pControl ? pControl->GetSector() : NULL;
	pParentGrid = pSector ? pSector->GetParentGrid() : NULL;

	
	if (pParentGrid)
	{
		Offset.x = ( ( pParentGrid->GetGridX() * 0.5f ) * (float)pParentGrid->GetGridWidth() * 100.f );
		Offset.y = 0.f;
		Offset.z = ( ( pParentGrid->GetGridY() * 0.5f ) * (float)pParentGrid->GetGridHeight() * 100.f );
	}

	//Prop/Event영역 기준 좌표가 다름.
	//Prop은 맵 중심이 0, 0, Event는 실 좌표?
	//실제 Event영역이 0, 0이고 맵의 크기가 200*200일때 Prop좌표로 -100, -100
	//Offset만큼 event좌표에서 빼줘야 한다.(Prop기준으로 표시 해야함.)
	Center -= Offset;

	DWORD dwCount = GetPropertyCount();
	for (DWORD i = 0; i < dwCount; ++i)
	{
		CUnionValueProperty* propertyValue = GetProperty(i);
		if (propertyValue && 
			_stricmp(propertyValue->GetDescription(), "Position") == 0 && 
			propertyValue->GetType() == CUnionValueProperty::Vector3)
		{
			propertyValue->SetVariable(Center);
			break;
		}
	}
}

void CTEtWorldEventArea::UpdatePropertyRotationInfo()
{
	DWORD dwCount = GetPropertyCount();
	for (DWORD i = 0; i < dwCount; ++i)
	{
		CUnionValueProperty* propertyValue = GetProperty(i);
		if (propertyValue && 
			_stricmp(propertyValue->GetDescription(), "Rotation") == 0 && 
			propertyValue->GetType() == CUnionValueProperty::Float)
		{
			propertyValue->SetVariable(GetRotate());
			break;
		}
	}
}

void CTEtWorldEventArea::UpdateMinMaxInfo()
{
	EtVector3 Center = EtVector3(0.0f, 0.0f, 0.0f);
	bool bFound = false;

	EtVector3 Offset = EtVector3(0.0f, 0.0f, 0.0f);
	CEtWorldSector *pSector = NULL;
	CEtWorldGrid *pParentGrid = NULL;
	CEtWorldEventControl *pControl = GetControl();

	pSector = pControl ? pControl->GetSector() : NULL;
	pParentGrid = pSector ? pSector->GetParentGrid() : NULL;


	if (pParentGrid)
	{
		Offset.x = ( ( pParentGrid->GetGridX() * 0.5f ) * (float)pParentGrid->GetGridWidth() * 100.f );
		Offset.y = 0.f;
		Offset.z = ( ( pParentGrid->GetGridY() * 0.5f ) * (float)pParentGrid->GetGridHeight() * 100.f );
	}

	
	DWORD dwCount = GetPropertyCount();
	for (DWORD i = 0; i < dwCount; ++i)
	{
		CUnionValueProperty* propertyValue = GetProperty(i);
		if (propertyValue && 
			_stricmp(propertyValue->GetDescription(), "Position") == 0 && 
			propertyValue->GetType() == CUnionValueProperty::Vector3)
		{
			Center = propertyValue->GetVariableVector3();

			//Property에 저장된 값은 Prop기준 좌표로 저장되어 있으므로
			//Event의 실제 좌표로 계산을 위해서 Offset값 더해줌.
			//실제 Event좌표가 0, 0이면 Prop기준으로 -100, -100인데, event기준으로 바꾸기 위해서 Offset더함.(0, 0)
			Center += Offset;

			bFound = true;
			break;
		}
	}

	if (bFound)
	{
		EtVector3 vOrigCenter = m_vMin + ( m_vMax - m_vMin ) / 2.f;
		
/*
		CalcOBB();
		SOBB *pSOBB = GetOBB();
		if (pSOBB)
			vOrigCenter = pSOBB->Center;
*/

		EtVector3 vDiff = Center - vOrigCenter;

		EtVector3 vMin = m_vMin + vDiff;
		EtVector3 vMax = m_vMax + vDiff;

		SetMin(vMin);
		SetMax(vMax);
	}

}