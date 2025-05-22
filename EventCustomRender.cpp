#include "StdAfx.h"
#include "EventCustomRender.h"

#include "ECRVolumeFog.h"

std::vector<CEventCustomRender *> CEventCustomRender::s_pVecList;

void CEventCustomRender::RegisterClass()
{
	s_pVecList.push_back( new CECRVolumeFog( "VolumeFog" ) );
}

void CEventCustomRender::UnregisterClass()
{
	SAFE_DELETE_PVEC( s_pVecList );
}

CEventCustomRender *CEventCustomRender::AllocCustomRender( const char *szControlName )
{
	for( DWORD i=0; i<s_pVecList.size(); i++ ) {
		if( strcmp( s_pVecList[i]->GetEventControlName(), szControlName ) == NULL ) return s_pVecList[i]->Clone();
	}
	return NULL;
}