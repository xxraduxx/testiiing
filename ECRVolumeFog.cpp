#include "Stdafx.h"
#include "ECRVolumeFog.h"
#include "UnionValueProperty.h"
#include "TEtWorldEventArea.h"
#include "TEtWorldSector.h"
#include "TEtWorldEventControl.h"

CECRVolumeFog::CECRVolumeFog( const char *szEventControlName )
: CEventCustomRender( szEventControlName )
{
}

CECRVolumeFog::~CECRVolumeFog()
{
	SAFE_RELEASE_SPTR( m_hVolumeFog );
}


void CECRVolumeFog::Initialize()
{
	CTEtWorldSector *pSector = (CTEtWorldSector *)m_pEventArea->GetControl()->GetSector();
	EtVector3 vOffset = *pSector->GetOffset();
	vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;

	int nEventAreaCommonParamCount = 0;
	for( int i=0; ; i++ ) {
		if( g_EventAreaPropertyDefine[i].szCategory == NULL ) break;
		nEventAreaCommonParamCount = i + 1;
	}

	SAFE_RELEASE_SPTR( m_hVolumeFog );
	m_pEventArea->CalcOBB();
	float fRadius = max( m_pEventArea->GetOBB()->Extent[0], m_pEventArea->GetOBB()->Extent[2] );
	float fHeight = m_pEventArea->GetMax()->y - m_pEventArea->GetMin()->y;
	D3DXCOLOR Color = m_pEventArea->GetProperty(2+nEventAreaCommonParamCount)->GetVariableVector4();
	EtVector3 vCenter = m_pEventArea->GetOBB()->Center;
	vCenter.y = pSector->GetHeight( m_pEventArea->GetOBB()->Center.x, m_pEventArea->GetOBB()->Center.z );
	vCenter.y += m_pEventArea->GetMin()->y;
	float fDensity = m_pEventArea->GetProperty(1+nEventAreaCommonParamCount)->GetVariableFloat();
	if( fDensity <= 0.f ) fDensity = 0.001f;
	char *szTexName = m_pEventArea->GetProperty(0+nEventAreaCommonParamCount)->GetVariableString();
	m_hVolumeFog = CEtVolumeFog::CreateVolumeFog( ( szTexName ) ? szTexName : "", vCenter + vOffset, fRadius, fDensity, fHeight, Color );
}

void CECRVolumeFog::OnSelect()
{
}

void CECRVolumeFog::OnUnselect()
{
}

void CECRVolumeFog::OnModify()
{
	CTEtWorldSector *pSector = (CTEtWorldSector *)m_pEventArea->GetControl()->GetSector();
	EtVector3 vOffset = *pSector->GetOffset();
	vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;

	int nEventAreaCommonParamCount = 0;
	for( int i=0; ; i++ ) {
		if( g_EventAreaPropertyDefine[i].szCategory == NULL ) break;
		nEventAreaCommonParamCount = i + 1;
	}

	SAFE_RELEASE_SPTR( m_hVolumeFog );
	m_pEventArea->CalcOBB();
	float fRadius = max( m_pEventArea->GetOBB()->Extent[0], m_pEventArea->GetOBB()->Extent[2] );
	float fHeight = m_pEventArea->GetMax()->y - m_pEventArea->GetMin()->y;
	D3DXCOLOR Color = m_pEventArea->GetProperty(2+nEventAreaCommonParamCount)->GetVariableVector4();
	EtVector3 vCenter = m_pEventArea->GetOBB()->Center;
	vCenter.y = pSector->GetHeight( m_pEventArea->GetOBB()->Center.x, m_pEventArea->GetOBB()->Center.z );
	vCenter.y += m_pEventArea->GetMin()->y;
	float fDensity = m_pEventArea->GetProperty(1+nEventAreaCommonParamCount)->GetVariableFloat();
	if( fDensity <= 0.f ) fDensity = 0.001f;
	char *szTexName = m_pEventArea->GetProperty(0+nEventAreaCommonParamCount)->GetVariableString();
	m_hVolumeFog = CEtVolumeFog::CreateVolumeFog( ( szTexName ) ? szTexName : "", vCenter + vOffset, fRadius, fDensity, fHeight, Color );
}
