#include "StdAfx.h"
#include "TEtWorldSound.h"
#include "TEtWorldSoundEnvi.h"
#include "SundriesFuncEx.h"
#include "UnionValueProperty.h"
#include "EtSoundEngine.h"
#include "EtSoundChannel.h"
#include "RenderBase.h"
#include "FileServer.h"

CTEtWorldSound::CTEtWorldSound( CEtWorldSector *pSector )
: CEtWorldSound( pSector )
{
	m_nSoundIndex = -1;
}

CTEtWorldSound::~CTEtWorldSound()
{
	Stop();
}

CEtWorldSoundEnvi *CTEtWorldSound::AllocSoundEnvi()
{
	return new CTEtWorldSoundEnvi( this );
}

bool CTEtWorldSound::Load( const char *szFileName )
{
	if( CEtWorldSound::Load( szFileName ) == false ) return false;

	if( CGlobalValue::GetInstance().m_bPlaySound )
		Play();
	return true;
}

bool CTEtWorldSound::Save( const char *szFileName )
{
	FILE *fp;
	fopen_s( &fp, szFileName, "wb" );
	if( fp == NULL ) return false;

	CString szBGM = m_szBGM.c_str();
	WriteCString( &szBGM, fp );
	fwrite( &m_fVolume, sizeof(float), 1, fp );

	int nCount = (int)m_pVecEnvi.size();
	fwrite( &nCount, sizeof(int), 1, fp );
	for( int i=0; i<nCount; i++ ) {
		((CTEtWorldSoundEnvi*)m_pVecEnvi[i])->Save( fp );
	}

	fclose( fp );
	return true;
}

void CTEtWorldSound::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (char*)GetBGM() );
			break;
		case 1:
			pVariable->SetVariable( GetVolume() );
			break;
	}
}

void CTEtWorldSound::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
	switch( dwIndex ) {
		case 0:
			{
				SetBGM( pVariable->GetVariableString() );
				if( !CGlobalValue::GetInstance().m_bPlaySound ) break;
				if( m_nSoundIndex != -1 ) {
					pEngine->RemoveSound( m_nSoundIndex );
					m_hChannel.Identity();
					m_nSoundIndex = -1;
				}
				m_nSoundIndex = pEngine->LoadSound( GetBGM(), false, true );
				if( m_nSoundIndex != -1 ) {
					m_hChannel = pEngine->PlaySound( "BGM", m_nSoundIndex, true );
					m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
				}
			}
			break;
		case 1:
			m_fVolume = pVariable->GetVariableFloat();
			if( m_hChannel ) m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
			break;
	}
}
 
void CTEtWorldSound::Play()
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();

	if( !m_szBGM.empty() ) {
		m_nSoundIndex = pEngine->LoadSound( GetBGM(), false, true );
		if( m_nSoundIndex != -1 ) {
			m_hChannel = pEngine->PlaySound( "BGM", m_nSoundIndex, true );
			if( m_hChannel ) m_hChannel->SetVolume( GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
		}
	}
	for( DWORD i=0; i<m_pVecEnvi.size(); i++ ) {
		m_pVecEnvi[i]->Play();
	}
}

void CTEtWorldSound::Stop()
{
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
	if( m_nSoundIndex != -1 ) {
		pEngine->RemoveSound( m_nSoundIndex );
		m_hChannel.Identity();
		m_nSoundIndex = -1;
	}

	for( DWORD i=0; i<m_pVecEnvi.size(); i++ ) {
		m_pVecEnvi[i]->Stop();
	}
}

void CTEtWorldSound::SetVolume( float fValue )
{
	if( !m_hChannel ) return;
	m_hChannel->SetVolume( fValue );
}
