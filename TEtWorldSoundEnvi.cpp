#include "StdAfx.h"
#include "TEtWorldSoundEnvi.h"
#include "SundriesFuncEx.h"
#include "UnionValueProperty.h"
#include "EtSoundEngine.h"
#include "EtSoundChannel.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "AxisLockRenderObject.h"
#include "Resource.h"
#include "SoundRenderView.h"

CTEtWorldSoundEnvi::CTEtWorldSoundEnvi( CEtWorldSound *pSound )
: CEtWorldSoundEnvi( pSound )
{
	m_nSoundIndex = -1;

	m_pAxis = NULL;
	m_pAxis = new CAxisLockRenderObject( this );

	ShowAxis( false );
}

CTEtWorldSoundEnvi::~CTEtWorldSoundEnvi()
{
	SAFE_DELETE( m_pAxis );
	if( m_nSoundIndex != -1 ) {
		CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
		pEngine->RemoveSound( m_nSoundIndex );
	}
}


bool CTEtWorldSoundEnvi::Save( FILE *fp )
{
	WriteCString( &CString(m_szName.c_str()), fp );
	WriteCString( &CString(m_szFileName.c_str()), fp );
	fwrite( &m_vPosition, sizeof(EtVector3), 1, fp );
	fwrite( &m_fRange, sizeof(float), 1, fp );
	fwrite( &m_fRollOff, sizeof(float), 1, fp );
	fwrite( &m_fVolume, sizeof(float), 1, fp );
	fwrite( &m_bStream, sizeof(bool), 1, fp );


	return true;
}

void CTEtWorldSoundEnvi::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (char*)m_szFileName.c_str() );
			break;
		case 1:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

				EtVector3 vPos = pVariable->GetVariableVector3();
				EtVector3 vOffset = *pSector->GetOffset();
				vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.y = 0.f;

				pVariable->SetVariable( m_vPosition + vOffset );
			}
			break;
		case 2:
			pVariable->SetVariable( m_fRange );
			break;
		case 3:
			pVariable->SetVariable( m_fRollOff );
			break;
		case 4:
			pVariable->SetVariable( m_fVolume );
			break;
		case 5:
			pVariable->SetVariable( m_bStream );
			break;
	}
}

void CTEtWorldSoundEnvi::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();

	CActionElementModifySound *pAction = NULL;

	switch( dwIndex ) {
		case 1: 
			pAction = new CActionElementModifySound( 0, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
			pAction->SetDesc( "Modify Sound - Move" );
			break;
		case 2:
			pAction = new CActionElementModifySound( 1, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
			pAction->SetDesc( "Modify Sound - Range" );
			break;
		case 3:
			pAction = new CActionElementModifySound( 2, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
			pAction->SetDesc( "Modify Sound - RollOff" );
			break;
		case 4:
			pAction = new CActionElementModifySound( 3, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
			pAction->SetDesc( "Modify Sound - Volume" );
			break;
		case 5:
			pAction = new CActionElementModifySound( 4, (CTEtWorldSoundEnvi*)CGlobalValue::GetInstance().m_pSelectSound );
			pAction->SetDesc( "Modify Sound - Stream" );
			break;
	}

	switch( dwIndex ) {
		case 0:
			SetFileName( pVariable->GetVariableString() );
			if( !CGlobalValue::GetInstance().m_bPlaySound ) break;
			Stop();
			Play();
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 1:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

				EtVector3 vPos = pVariable->GetVariableVector3();
				EtVector3 vOffset = *pSector->GetOffset();
				vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.y = 0.f;

				m_vPosition = pVariable->GetVariableVector3() - vOffset;
			}
			if( m_hChannel ) {
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

				EtVector3 vOffset;
				float fWidth = CTEtWorld::GetInstance().GetGridWidth() * 100.f;
				float fHeight = CTEtWorld::GetInstance().GetGridHeight() * 100.f;

				vOffset.x = pSector->GetOffset()->x - ( fWidth / 2.f );
				vOffset.z = pSector->GetOffset()->z - ( fHeight / 2.f );

				m_hChannel->SetPosition( EtVector3( m_vPosition.x + vOffset.x, m_vPosition.y, m_vPosition.z + vOffset.z ) );
			}

			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 2:
			m_fRange = pVariable->GetVariableFloat();
			if( m_hChannel ) {
				m_hChannel->SetRollOff( 3, 0.f, 1.f, m_fRange * GetRollOff(), 1.f, m_fRange, 0.f );
			}
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 3:
			m_fRollOff = pVariable->GetVariableFloat();
			if( m_hChannel ) {
				m_hChannel->SetRollOff( 3, 0.f, 1.f, m_fRange * GetRollOff(), 1.f, m_fRange, 0.f );
			}
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 4:
			m_fVolume = pVariable->GetVariableFloat();
			if( m_hChannel ) {
				m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
			}
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 5:
			m_bStream = pVariable->GetVariableBool();
			break;
	}

	if( pAction ) pAction->AddAction();
}


void CTEtWorldSoundEnvi::Play()
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();

	if( !m_szFileName.empty() ) {
		m_nSoundIndex = pEngine->LoadSound( GetFileName(), true, false );
		if( m_nSoundIndex == -1 ) return;
		m_hChannel = pEngine->PlaySound( "3D", m_nSoundIndex, true, true );
		if( m_hChannel ) {
			m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );

			SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
			CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

			EtVector3 vOffset;
			float fWidth = CTEtWorld::GetInstance().GetGridWidth() * 100.f;
			float fHeight = CTEtWorld::GetInstance().GetGridHeight() * 100.f;

			vOffset.x = pSector->GetOffset()->x - ( fWidth / 2.f );
			vOffset.z = pSector->GetOffset()->z - ( fHeight / 2.f );

			m_hChannel->SetPosition( EtVector3( m_vPosition.x + vOffset.x, m_vPosition.y, m_vPosition.z + vOffset.z ) );
			m_hChannel->SetRollOff( 3, 0.f, 1.f, m_fRange * GetRollOff(), 1.f, m_fRange, 0.f );
			m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );

			m_hChannel->Resume();
		}
	}
}

void CTEtWorldSoundEnvi::SetStream( bool bStream )
{
	m_bStream = bStream;
}

void CTEtWorldSoundEnvi::Stop()
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
	if( m_nSoundIndex != -1 ) {
		pEngine->RemoveSound( m_nSoundIndex );
		m_hChannel.Identity();
		m_nSoundIndex = -1;
	}
}


void CTEtWorldSoundEnvi::ShowAxis( bool bShow )
{
	m_pAxis->ShowObject( bShow );
}

void CTEtWorldSoundEnvi::UpdateAxis( EtMatrix *pmat )
{
	if( m_pAxis ) m_pAxis->Update( pmat );
}

void CTEtWorldSoundEnvi::MoveAxis( int nX, int nY )
{
	if( m_pAxis ) m_pAxis->MoveAxis( nX, nY );
}

bool CTEtWorldSoundEnvi::CheckAxis( EtVector3 &vPos, EtVector3 &vDir )
{
	if( m_pAxis ) {
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

		EtVector3 vOffset;
		float fWidth = CTEtWorld::GetInstance().GetGridWidth() * 100.f;
		float fHeight = CTEtWorld::GetInstance().GetGridHeight() * 100.f;

		vOffset.x = pSector->GetOffset()->x - ( fWidth / 2.f );
		vOffset.z = pSector->GetOffset()->z - ( fHeight / 2.f );
		vOffset.y = 0.f;

		return ( m_pAxis->CheckAxis( vPos, vDir, vOffset ) != -1 ) ? true : false;
	}
	return false;
}

void CTEtWorldSoundEnvi::SetAxisScale( float fValue )
{
	if( m_pAxis ) m_pAxis->SetScale( fValue );
}

void CTEtWorldSoundEnvi::SetVolume( float fValue )
{
	if( !m_hChannel ) return;
	m_hChannel->SetVolume( fValue );

}