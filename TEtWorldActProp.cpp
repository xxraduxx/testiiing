#include "StdAfx.h"
#include "TEtWorldActProp.h"
#include "EtActionSignal.h"
#include "SignalHeader.h"
#include "EtSoundEngine.h"
#include "EtSoundChannel.h"
#include "TEtWorldSector.h"
#include "DNTableFile.h"
#include "EtResourceMng.h"
#include "EtCollisionEntity.h"
#include "EtEngine.h"


CTEtWorldActProp::CTEtWorldActProp()
{
	m_LocalTime = 0;
	m_nLoopCount = 0;
	m_ActionTime = 0;
	m_fFrame = m_fPrevFrame = 0.f;
	m_nActionIndex = -1;
	m_bExistAction = false;
	m_pCheckPreSignalFunc = CheckPreSignal;
	m_pCheckPostSignalFunc = CheckPostSignal;
}

CTEtWorldActProp::~CTEtWorldActProp()
{
	FreeObject();
	SAFE_DELETE_VEC( m_nVecAniIndexList );
}

bool CTEtWorldActProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CTEtWorldProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
//	m_Handle->SetCollisionGroup( COLLISION_GROUP_STATIC(1) );

	return bResult;
}

bool CTEtWorldActProp::LoadObject()
{
	char szFullName[_MAX_PATH] = { 0, };
	char szFileName[_MAX_PATH] = { 0, };
	char szAniName[_MAX_PATH] = { 0, };
	char szActName[_MAX_PATH] = { 0, };

	_GetFileName( szFileName, _countof(szFileName), m_szPropName.c_str() );
	sprintf_s( szAniName, "%s.ani", szFileName );
	sprintf_s( szActName, "%s.act", szFileName );

	bool bExistAni = false;
	bool bExistAct = false;
	/*
	for( int i = 0; i < ( int )g_vecResourceDir.size(); i++ ) {
		if( !bExistAni && FindFileInDirectory( g_vecResourceDir[ i ].c_str(), szAniName, NULL ) ) {
			bExistAni = true;
		}
		if( !bExistAct && FindFileInDirectory( g_vecResourceDir[ i ].c_str(), szActName, szFullName ) ) {
			bExistAct = true;
		}
	}
	*/
	CEtResourceMng::GetInstance().GetFullName( szAniName, &bExistAni );
	if( !bExistAni ) { // 이름이 같고 확장자만 비교했을때 없을경우 테이블 참조해서 찾아줍니다.
		if( CTEtWorldSector::s_pPropSOX ) {
			int nIndex = CTEtWorldSector::s_pPropSOX->GetItemIDFromField( "_Name", m_szPropName.c_str() );
			if( nIndex != -1 ) {
				sprintf_s( szAniName, CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_AniName" )->GetString() );
				CEtResourceMng::GetInstance().GetFullName( szAniName, &bExistAni );
			}
		}
	}

	sprintf_s( szFullName, CEtResourceMng::GetInstance().GetFullName( szActName, &bExistAct ).c_str() );
	if( !bExistAct ) { // 이름이 같고 확장자만 비교했을때 없을경우 테이블 참조해서 찾아줍니다.
		if( CTEtWorldSector::s_pPropSOX ) {
			int nIndex = CTEtWorldSector::s_pPropSOX->GetItemIDFromField( "_Name", m_szPropName.c_str() );
			if( nIndex != -1 ) {
				sprintf_s( szActName, CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_ActName" )->GetString() );
				sprintf_s( szFullName, CEtResourceMng::GetInstance().GetFullName( szActName, &bExistAct ).c_str() );
			}
		}
	}

	if( bExistAni ) {
		if( g_pEtRenderLock ) g_pEtRenderLock->Lock();
		m_Handle = CreateAniObject( m_szPropName.c_str(), szAniName );
		if( m_Handle ) m_Handle->SetCalcPositionFlag( CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z );
		if( g_pEtRenderLock ) g_pEtRenderLock->UnLock();
	}

	bool bResult = CTEtWorldProp::LoadObject();

	if( bExistAct ) {
		if( LoadAction( szFullName ) == true ) {
			if( bExistAni ) CacheAniIndex();
			if( CTEtWorldSector::s_pPropSOX ) {
				char szDefaultAction[128] = { 0, };
				sprintf_s( szDefaultAction, "Idle" );

				int nIndex = CTEtWorldSector::s_pPropSOX->GetItemIDFromField( "_Name", m_szPropName.c_str() );
				if( nIndex != -1 ) {
					char *szStr = CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_DefaultAction" )->GetString();
					int nActionCount = 1 + _GetNumSubStr( szStr, ';' );
					if( nActionCount > 0 ) {
						sprintf_s( szDefaultAction, _GetSubStrByCountSafe( rand()%nActionCount, szStr, ';' ).c_str() );
					}
				}
				SetAction( szDefaultAction, 0.f, 0.f );
			}
			m_bExistAction = true;
		}
	}
	return bResult;
}

bool CTEtWorldActProp::FreeObject()
{
	FreeAction();
	return CTEtWorldProp::FreeObject();
}


bool CTEtWorldActProp::Render( LOCAL_TIME LocalTime )
{
	bool bResult = CTEtWorldProp::Render( LocalTime );

	ProcessAction( LocalTime );

	return bResult;
}

void CTEtWorldActProp::ProcessAction( LOCAL_TIME LocalTime )
{
	m_LocalTime = LocalTime;
	if( !m_bShow ) return;

	if( m_nActionIndex == -1 ) return;

	ActionElementStruct *pStruct = (*m_pVecActionElementList)[m_nActionIndex];

	m_fFrame = ( ( m_LocalTime - m_ActionTime ) / 1000.f ) * CGlobalValue::GetInstance().m_fFps;

	if( m_fFrame > (float)pStruct->dwLength ) {
		if( m_nLoopCount > 0 ) {
			m_nLoopCount--;
			SetAction( m_szAction.c_str(), (float)0.f, 0.f );
		}
		else {
			if( pStruct->szNextActionName.empty() ) {
				m_nActionIndex = -1;
				m_fFrame = -1.f;
				m_szAction.clear();
			}
			else {
				SetAction( pStruct->szNextActionName.c_str(), (float)pStruct->dwNextActionFrame, (float)pStruct->dwBlendFrame );
				m_fFrame = ( ( m_LocalTime - m_ActionTime ) / 1000.f ) * CGlobalValue::GetInstance().m_fFps;
			}
//			return;
		}
	}

	if( m_nActionIndex != -1 && m_nActionIndex < (int)m_nVecAniIndexList.size() && m_nVecAniIndexList[m_nActionIndex] != -1 ) {
		m_Handle->SetAniFrame( m_nVecAniIndexList[m_nActionIndex], m_fFrame );
	}
	ProcessSignal( pStruct, m_fFrame, m_fPrevFrame );

	m_fPrevFrame = m_fFrame;
}

void CTEtWorldActProp::ProcessSignal( ActionElementStruct *pStruct, float fFrame, float fPrevFrame )
{
	CEtActionSignal *pSignal;
	for( DWORD i=0; i<pStruct->pVecSignalList.size(); i++ ) {
		pSignal = pStruct->pVecSignalList[i];
		if( pSignal->CheckSignal( fPrevFrame, fFrame ) == true ) {
			LOCAL_TIME EndTime = m_LocalTime + (LOCAL_TIME)( 1000.f / CGlobalValue::GetInstance().m_fFps * ( pSignal->GetEndFrame() - fFrame ) );
			OnSignal( pSignal->GetSignalIndex(), pSignal->GetData(), m_LocalTime, EndTime );
		}
	}
}

void CTEtWorldActProp::SetAction( const char *szActionName, float fFrame, float fBlendFrame )
{
	int nIndex = GetElementIndex( szActionName );
	if( nIndex == -1 ) return;

	ActionElementStruct *pStruct = GetElement( (DWORD)nIndex );

	m_nActionIndex = nIndex;
	m_szAction = szActionName;

	m_fFrame = m_fPrevFrame = (float)fFrame;
	m_ActionTime = m_LocalTime - (LOCAL_TIME)( fFrame / CGlobalValue::GetInstance().m_fFps * 1000.f );
}

void CTEtWorldActProp::CacheAniIndex()
{
	m_nVecAniIndexList.clear();
	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		int nIndex = -1;
		if( !(*m_pVecActionElementList)[i]->szLinkAniName.empty() ) {
			if( m_Handle ) {
				for( int j=0; j<m_Handle->GetAniCount(); j++ ) {
					if( strcmp( m_Handle->GetAniName(j), (*m_pVecActionElementList)[i]->szLinkAniName.c_str() ) == NULL ) nIndex = j;
				}
			}
		}
		m_nVecAniIndexList.push_back( nIndex );
	}
}

void CTEtWorldActProp::CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
				pStruct->nSoundIndex = -1;
				if( strlen( pStruct->szFileName ) == 0 ) break;
				pStruct->nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( pStruct->szFileName, true, false );
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pSignal->GetData();
				pStruct->nParticleDataIndex = -1;
				if( strlen( pStruct->szFileName ) == 0 ) break;
				pStruct->nParticleDataIndex = EternityEngine::LoadParticleData( pStruct->szFileName );
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct *) pSignal->GetData();
				pStruct->nFXDataIndex = -1;
				if( strlen(pStruct->szFileName) == 0 ) break;
				pStruct->nFXDataIndex = EternityEngine::LoadEffectData( pStruct->szFileName );
			}
			break;
	}

}

void CTEtWorldActProp::CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
				if( pStruct->nSoundIndex == -1 ) break;
				CEtSoundEngine::GetInstance().RemoveSound( pStruct->nSoundIndex );
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pSignal->GetData();
				if( pStruct->nParticleDataIndex == -1 ) break;
				EternityEngine::DeleteParticleData( pStruct->nParticleDataIndex );
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct *)pSignal->GetData();
				if( pStruct->nFXDataIndex == -1 ) break;
				EternityEngine::DeleteEffectData( pStruct->nFXDataIndex );
			}
			break;
	}
}

void CTEtWorldActProp::OnSignal( int nIndex, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalEndTime )
{
	switch( nIndex ) {
		case STE_Sound:
			{
				SoundStruct *pStruct = (SoundStruct *)pPtr;
				if( pStruct->nSoundIndex == -1 ) break;
				CEtSoundChannel *pChannel = CEtSoundEngine::GetInstance().PlaySound( "3D", pStruct->nSoundIndex, false, true );
				if( pChannel == NULL ) break;
				pChannel->SetVolume( pStruct->fVolume );
				pChannel->SetPosition( m_matExWorld.m_vPosition ); 
				pChannel->SetRollOff( 3, 0.f, 1.f, pStruct->fRange * pStruct->fRollOff, 1.f, pStruct->fRange, 0.f );
				pChannel->Resume();
			}
			break;
		case STE_Particle:
			{
				ParticleStruct *pStruct = (ParticleStruct *)pPtr;
				if( pStruct->nParticleDataIndex == -1 ) break;

				MatrixEx Cross = m_matExWorld;

				Cross.MoveLocalZAxis( pStruct->vPosition->z );
				Cross.MoveLocalXAxis( pStruct->vPosition->x );
				Cross.MoveLocalYAxis( pStruct->vPosition->y * GetScale()->y );

				if( pStruct->vRotation ) {
					Cross.RotateYaw( pStruct->vRotation->y );
					Cross.RotateRoll( pStruct->vRotation->z );
					Cross.RotatePitch( pStruct->vRotation->x );
				}

				EtBillboardEffectHandle hParticle = EternityEngine::CreateBillboardEffect( pStruct->nParticleDataIndex, Cross );				
				hParticle->SetScale( GetScale()->z );
			}
			break;
		case STE_FX:
			{
				FXStruct *pStruct = (FXStruct*)pPtr;
				if( pStruct->nFXDataIndex == -1 ) break;
				MatrixEx Cross = m_matExWorld;
				Cross.MoveLocalZAxis( pStruct->vOffset->z );
				Cross.MoveLocalXAxis( pStruct->vOffset->x ); 
				Cross.MoveLocalYAxis( pStruct->vOffset->y * GetScale()->y );
				if( pStruct->vRotation ) {
					Cross.RotateYaw( pStruct->vRotation->y );
					Cross.RotateRoll( pStruct->vRotation->z );
					Cross.RotatePitch( pStruct->vRotation->x );
				}
				EtEffectObjectHandle hEffect = EternityEngine::CreateEffectObject( pStruct->nFXDataIndex, Cross);
				if( hEffect ) hEffect->SetScale( ( pStruct->fScale + 1.f ) * GetScale()->z );
			}
			break;
	}
}


void CTEtWorldActProp::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
	CTEtWorldProp::OnSetValue( pVariable, dwIndex, pProperty );

	switch( dwIndex ) {
		case 7:
			{
				pProperty->SetReadOnly( pVariable, !m_bExistAction );
				pVariable->SetSubDescription( "Play Action" );

				std::string szActionStr = "Play Action|";
				if( m_bExistAction ) {
					DWORD dwCount = GetElementCount();
					int nCurValue = 0;
					for( DWORD i=0; i<dwCount; i++ ) {
						ActionElementStruct *pStruct = GetElement(i);
						if( m_nActionIndex == i ) nCurValue = i;

						szActionStr += pStruct->szName;

						if( i != dwCount - 1 ) szActionStr += "|";
					}
					pVariable->SetSubDescription( (char*)szActionStr.c_str(), true );
					pVariable->SetVariable( nCurValue );
				}
			}
			break;
	}
}

void CTEtWorldActProp::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
	CTEtWorldProp::OnChangeValue( pVariable, dwIndex, pProperty );

	switch( dwIndex ) {
		case 7:
			{
				if( !m_bExistAction ) break;
				int nCurAction = pVariable->GetVariableInt();

				if( nCurAction < 0 || nCurAction >= (int)GetElementCount() ) break;

				SetAction( GetElement( nCurAction )->szName.c_str(), 0.f, 3.f );
			}
			break;
	}
}
