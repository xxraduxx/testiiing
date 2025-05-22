#include "StdAfx.h"
#include "TEtWorldDecal.h"
#include "TEtWorld.h"
#include "SundriesFuncEx.h"

CTEtWorldDecal::CTEtWorldDecal( CEtWorldSector *pSector )
: CEtWorldDecal( pSector )
{
	m_pCurrentNode = NULL;
	m_vPos = EtVector2( 0.f, 0.f );
	m_fRadius = 0.f;
	m_fRotate = 0.f;
	m_vColor = D3DCOLOR_ARGB(255,255,255,255);
	m_fAlpha = 1.f;
	m_bSelect = false;
}

CTEtWorldDecal::~CTEtWorldDecal()
{
	Finalize();
}

bool CTEtWorldDecal::Initialize( EtVector2 vPos, float fRadius, float fRotate, EtColor Color, float fAlpha, const char *szTextureName )
{
	if( CEtWorldDecal::Initialize( vPos, fRadius, fRotate, Color, fAlpha, szTextureName ) == false ) return false;
	if( m_szTextureName.empty() ) return false;
	m_hTexture = EternityEngine::LoadTexture( szTextureName );

	Reinitialize();

	return true;
}

void CTEtWorldDecal::Reinitialize( bool bReload )
{
	if( bReload ) {
		if( CEtWorldDecal::Initialize( m_vPos, m_fRadius, m_fRotate, m_vColor, m_fAlpha, m_szTextureName.c_str() ) == false ) return;
		m_hTexture = EternityEngine::LoadTexture( m_szTextureName.c_str() );
	}

	SAFE_RELEASE_SPTR( m_hDecal );
	m_hDecal = (new CEtDecal)->GetMySmartPtr();
	m_hDecal->UseFixedUV( true );
	m_hDecal->Initialize( m_hTexture, m_vPos.x, m_vPos.y, m_fRadius, FLT_MAX, 0.f, m_fRotate, MakeColor() );
}

void CTEtWorldDecal::Finalize()
{
	SAFE_RELEASE_SPTR( m_hTexture );
	SAFE_RELEASE_SPTR( m_hDecal );
	SAFE_RELEASE_SPTR( m_hSelectTexture );
	SAFE_RELEASE_SPTR( m_hSelectDecal );
}

EtColor CTEtWorldDecal::MakeColor()
{
	EtColor ColorTemp = m_vColor;
	ColorTemp *= m_vColor.a;
	ColorTemp.a = m_fAlpha;

	return ColorTemp;
}

void CTEtWorldDecal::Render( LOCAL_TIME LocalTime )
{
	if( !m_bSelect ) return;
	float fCurAngle = 0.f;
	EtVector3 vPos[2];

	vPos[0].x = m_vPos.x + ( cos( EtToRadian( fCurAngle ) ) * m_fRadius );
	vPos[0].z = m_vPos.y + ( sin( EtToRadian( fCurAngle ) ) * m_fRadius );
	vPos[0].y = CTEtWorld::GetInstance().GetHeight( vPos[0].x, vPos[0].z ) + 30.f;

	for( DWORD i=0;; i++ ) {
		vPos[1].x = m_vPos.x + ( cos( EtToRadian( fCurAngle ) ) * m_fRadius );
		vPos[1].z = m_vPos.y + ( sin( EtToRadian( fCurAngle ) ) * m_fRadius );

		vPos[1].y = CTEtWorld::GetInstance().GetHeight( vPos[1].x, vPos[1].z ) + 30.f;

		EternityEngine::DrawLine3D( vPos[0], vPos[1] , 0xFFFFFF00 );

		vPos[0] = vPos[1];

		if( fCurAngle >= 360.f ) break;
		fCurAngle += 5.f;
	}
}

void CTEtWorldDecal::SetTextureName( const char *szName )
{
	SAFE_RELEASE_SPTR( m_hDecal );
	SAFE_RELEASE_SPTR( m_hTexture );

	m_szTextureName = szName;
	m_hTexture = EternityEngine::LoadTexture( szName );

	m_hDecal = (new CEtDecal)->GetMySmartPtr();
	m_hDecal->Initialize( m_hTexture, m_vPos.x, m_vPos.y, m_fRadius, FLT_MAX, 0.f, m_fRotate, MakeColor() );
}

void CTEtWorldDecal::SetRadius( float fValue )
{
	m_fRadius = fValue;
	Reinitialize();

	MakeSelectDecal();
}

void CTEtWorldDecal::SetRotate( float fValue )
{
	m_fRotate = fValue;
	Reinitialize();
}

void CTEtWorldDecal::SetColor( EtColor Color )
{
	m_vColor = Color;
	Reinitialize();
}

void CTEtWorldDecal::SetAlpha( float fAlpha )
{
	m_fAlpha = fAlpha;
	Reinitialize();
}

void CTEtWorldDecal::SetPosition( EtVector2 vPos )
{
	m_vPos = vPos;
	Reinitialize();
	MakeSelectDecal();
}


void CTEtWorldDecal::SetSelect( bool bValue ) 
{ 
	if( m_bSelect == bValue ) return;

	m_bSelect = bValue; 

	if( m_bSelect ) {
		MakeSelectDecal();
	}
	else {
		SAFE_RELEASE_SPTR( m_hSelectTexture );
		SAFE_RELEASE_SPTR( m_hSelectDecal );
	}
}

void CTEtWorldDecal::MakeSelectDecal()
{
	if( !m_bSelect ) return;
	SAFE_RELEASE_SPTR( m_hSelectTexture );
	SAFE_RELEASE_SPTR( m_hSelectDecal );

	m_hSelectTexture = CEtTexture::CreateNormalTexture( 2, 2, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );

	int nStride = 0;
	DWORD *pBuf = (DWORD *)m_hSelectTexture->Lock( nStride, true );

	for( int i=0; i<4; i++ ) {
		*pBuf = D3DCOLOR_ARGB( 128, 220, 50, 50 );
		pBuf++;
	}

	m_hSelectTexture->Unlock();

	m_hSelectDecal = (new CEtDecal)->GetMySmartPtr();
	m_hSelectDecal->UseFixedUV( true );
	m_hSelectDecal->Initialize( m_hSelectTexture, m_vPos.x, m_vPos.y, m_fRadius, FLT_MAX, 0.f, m_fRotate, MakeColor() );
}

void CTEtWorldDecal::WriteDecalInfo( FILE *fp )
{
	/*
	fwrite( &m_vPos, sizeof(EtVector2), 1, fp );
	fwrite( &m_fRadius, sizeof(float), 1, fp );
	fwrite( &m_fRotate, sizeof(float), 1, fp );
	fwrite( &m_vColor, sizeof(EtColor), 1, fp );
	fwrite( &m_fAlpha, sizeof(float), 1, fp );

	char szBuf[64] = { 0, };
	fwrite( szBuf, 64, 1, fp );

	WriteCString( &CString(m_szTextureName.c_str()), fp );
	*/
	DecalStruct Struct;
	memset( &Struct, 0, sizeof(DecalStruct) );
	Struct.vPos = m_vPos;
	Struct.fRadius = m_fRadius;
	Struct.fRotate = m_fRotate;
	Struct.vColor = m_vColor;
	Struct.fAlpha = m_fAlpha;
	sprintf_s( Struct.szTextureName, m_szTextureName.c_str() );

	fwrite( &Struct, sizeof(DecalStruct), 1, fp );
}

bool CTEtWorldDecal::ReadDecalInfo( FILE *fp )
{
	/*
	fread( &m_vPos, sizeof(EtVector2), 1, fp );
	fread( &m_fRadius, sizeof(float), 1, fp );
	fread( &m_fRotate, sizeof(float), 1, fp );
	fread( &m_vColor, sizeof(EtColor), 1, fp );
	fread( &m_fAlpha, sizeof(float), 1, fp );

	char szBuf[64];
	fread( szBuf, 64, 1, fp );

	CString szStr;
	ReadCString( &szStr, fp );
	m_szTextureName = szStr;
	*/
	DecalStruct Struct;
	fread( &Struct, sizeof(DecalStruct), 1, fp );
	m_vPos = Struct.vPos;
	m_fRadius = Struct.fRadius;
	m_fRotate = Struct.fRotate;
	m_vColor = Struct.vColor;
	m_fAlpha = Struct.fAlpha;
	m_szTextureName = Struct.szTextureName;

	return true;
}
