#include "StdAfx.h"
#include "PropPoolControl.h"

CPropPoolControl g_PropPoolControl;

CPropPoolStage::CPropPoolStage()
{
}

CPropPoolStage::~CPropPoolStage()
{
	SAFE_DELETE_VEC( m_VecList );
}

bool CPropPoolStage::LoadStage( CString szFullName )
{
	return true;
}

bool CPropPoolStage::SaveStage( CString szFullName )
{
	return true;
}

void CPropPoolStage::AddProp( CString szName, EtVector2 vOffset )
{
	PropPoolStruct Struct;
	Struct.szPropName = szName;
	Struct.vOffset = vOffset;

	m_VecList.push_back( Struct );
}

void CPropPoolStage::RemoveProp( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecList.size() ) return;
	m_VecList.erase( m_VecList.begin() + dwIndex );
}

void CPropPoolStage::Empty()
{
	SAFE_DELETE_VEC( m_VecList );
}

DWORD CPropPoolStage::GetPropCount()
{
	return (DWORD)m_VecList.size();
}

CString CPropPoolStage::GetPropName( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecList.size() ) return CString("");

	return m_VecList[dwIndex].szPropName;
}

EtVector2 CPropPoolStage::GetPropOffset( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecList.size() ) return EtVector2(0.f,0.f);

	return m_VecList[dwIndex].vOffset;
}

void CPropPoolStage::SetPropOffset( DWORD dwIndex, EtVector2 vOffset )
{
	if( dwIndex < 0 || dwIndex >= m_VecList.size() ) return;
	m_VecList[dwIndex].vOffset = vOffset;
}


CPropPoolControl::CPropPoolControl()
{
	m_nCurrentStageIndex = -1;

	m_pNoneStage = new CPropPoolStage;
	m_pNoneStage->SetStageName( "NULL" );
}

CPropPoolControl::~CPropPoolControl()
{
	SAFE_DELETE_PVEC( m_pVecList );
	SAFE_DELETE( m_pNoneStage );
}

void CPropPoolControl::LoadStage()
{
}

void CPropPoolControl::SaveStage()
{
}

void CPropPoolControl::SetCurrentStage( CString szStageName )
{
	if( strcmp( szStageName, "None" ) == NULL ) {
		m_nCurrentStageIndex = -1;
		return;
	}
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( strcmp( m_pVecList[i]->GetStageName(), szStageName ) == NULL ) {
			m_nCurrentStageIndex = (int)i;
			return;
		}
	}
}

CPropPoolStage *CPropPoolControl::GetCurrentStage()
{
	if( m_nCurrentStageIndex == -1 ) return m_pNoneStage;
	else return m_pVecList[m_nCurrentStageIndex];
}

DWORD CPropPoolControl::GetStageCount()
{
	return (DWORD)m_pVecList.size();
}

CString CPropPoolControl::GetStageName( DWORD dwIndex )
{
	return m_pVecList[dwIndex]->GetStageName();
}
