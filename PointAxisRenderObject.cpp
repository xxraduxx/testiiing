#include "StdAfx.h"
#include "PointAxisRenderObject.h"
#include "EtMatrixEx.h"
#include "Renderbase.h"
#include "EtWorldSector.h"

bool CPointAxisRenderObject::m_bInitialize;
CAxisRenderObject::VertexFormat *CPointAxisRenderObject::m_pAxis[3] = { 0, 0, 0 };
CAxisRenderObject::VertexFormat *CPointAxisRenderObject::m_pAxisBack[3] = { 0, 0, 0 };
CAxisRenderObject::VertexFormat *CPointAxisRenderObject::m_pSelect[3] = { 0, 0, 0 };
int CPointAxisRenderObject::m_nPrimitiveCount[3] = { 0, 0, 0 };
float CPointAxisRenderObject::s_fAxisLength = 35.f;
float CPointAxisRenderObject::s_fAxisPos = 20.f;
float CPointAxisRenderObject::s_fAxisRadius = 2.f;
int CPointAxisRenderObject::s_nRefCount = 0;
int CPointAxisRenderObject::m_nSelectAxis = -1;

CPointAxisRenderObject::CPointAxisRenderObject( EtVector3 *pPoint )
{
	s_nRefCount++;

	m_fScale = 50.f;
	m_nSelectAxis = -1;
	m_pPoint = pPoint;
}

CPointAxisRenderObject::~CPointAxisRenderObject()
{
	s_nRefCount--;
	if( s_nRefCount == 0 ) {
		Destroy();
	}
}

void CPointAxisRenderObject::Initialize()
{
	if( m_bInitialize == true ) return;
	m_bInitialize = true;

	float fLength = s_fAxisLength - s_fAxisPos;

	m_nPrimitiveCount[0] = CAxisRenderObject::CreateCone( &m_pAxis[0], 0xFFFF0000, s_fAxisRadius, fLength, 10, 0, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pAxisBack[0], 0xFFFF0000, s_fAxisRadius, 0.f, 10, 0, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pSelect[0], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 0, s_fAxisPos );

	m_nPrimitiveCount[1] = CAxisRenderObject::CreateCone( &m_pAxis[1], 0xFF00FF00, s_fAxisRadius, fLength, 10, 1, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pAxisBack[1], 0xFF00FF00, s_fAxisRadius, 0.f, 10, 1, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pSelect[1], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 1, s_fAxisPos );

	m_nPrimitiveCount[2] = CAxisRenderObject::CreateCone( &m_pAxis[2], 0xFF0000FF, s_fAxisRadius, fLength, 10, 2, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pAxisBack[2], 0xFF0000FF, s_fAxisRadius, 0.f, 10, 2, s_fAxisPos );
	CAxisRenderObject::CreateCone( &m_pSelect[2], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 2, s_fAxisPos );
}

void CPointAxisRenderObject::Destroy()
{
	for( int i=0; i<3; i++ ) {
		SAFE_DELETEA( m_pAxis[i] );
		SAFE_DELETEA( m_pAxisBack[i] );
		SAFE_DELETEA( m_pSelect[i] );
	}
	m_bInitialize = false;
}

void CPointAxisRenderObject::RenderCustom( float fElapsedTime )
{
	float fLength = EtVec3Length( &( *m_pPoint - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );
	SetScale( fLength / 250.f );
	DrawAxis();
}

void CPointAxisRenderObject::DrawAxis()
{
	DWORD dwLightFlag, dwCullMode, dwZEnable;
	
	GetEtDevice()->GetRenderState( D3DRS_LIGHTING, &dwLightFlag );
	GetEtDevice()->GetRenderState( D3DRS_CULLMODE, &dwCullMode );
	GetEtDevice()->GetRenderState( D3DRS_ZENABLE, &dwZEnable );

	GetEtDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMATRIX mat, matScale;
	EtMatrixIdentity( &mat );
	*((EtVector3*)&mat._41) = &m_pPoint->x;

	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	D3DXMatrixScaling( &matScale, m_fScale, m_fScale, m_fScale );
	D3DXMatrixMultiply( &mat, &matScale, &mat );
	GetEtDevice()->SetWorldTransform( &mat );
	GetEtDevice()->SetViewTransform( hCamera->GetViewMat() );
	GetEtDevice()->SetProjTransform( hCamera->GetProjMat() );
	GetEtDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	GetEtDevice()->SetVertexShader( NULL );
	GetEtDevice()->SetPixelShader( NULL );

	for( int i=0; i<4; i++ ) GetEtDevice()->SetTexture( 0, NULL );

	for( DWORD i=0; i<3; i++ ) {
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pAxis[i], sizeof(CAxisRenderObject::VertexFormat) );
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pAxisBack[i], sizeof(CAxisRenderObject::VertexFormat) );
		if( m_nSelectAxis == i ) {
			DWORD dwAlphaBlend;
			DWORD dwBlendOP, dwSrcBlend, dwDestBlend;

			GetEtDevice()->GetRenderState( D3DRS_ALPHABLENDENABLE, &dwAlphaBlend );
			GetEtDevice()->GetRenderState( D3DRS_BLENDOP, &dwBlendOP );
			GetEtDevice()->GetRenderState( D3DRS_SRCBLEND, &dwSrcBlend );
			GetEtDevice()->GetRenderState( D3DRS_DESTBLEND, &dwDestBlend );

			GetEtDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			GetEtDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

			GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pSelect[i], sizeof(CAxisRenderObject::VertexFormat) );

			GetEtDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, dwAlphaBlend );
			GetEtDevice()->SetRenderState( D3DRS_BLENDOP, dwBlendOP );
			GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, dwSrcBlend );
			GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, dwDestBlend );
		}
	}

	CAxisRenderObject::VertexFormat vPos[2];

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( s_fAxisPos, 0, 0 );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 0 ) ? 0xFFFFFF20 : 0xFFFF0000;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, &vPos[0], sizeof(CAxisRenderObject::VertexFormat) );

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( 0, s_fAxisPos, 0 );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 1 ) ? 0xFFFFFF20 : 0xFF00FF00;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, vPos, sizeof(CAxisRenderObject::VertexFormat) );

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( 0.f, 0.f, s_fAxisPos );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 2 ) ? 0xFFFFFF20 : 0xFF0000FF;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, vPos, sizeof(CAxisRenderObject::VertexFormat) );

	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, dwLightFlag );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, dwCullMode );
	GetEtDevice()->SetRenderState( D3DRS_ZENABLE, dwZEnable );
}

int CPointAxisRenderObject::CheckAxis( EtVector3 &vOrig, EtVector3 &vDir )
{
	if( m_pPoint == NULL ) return -1;
	SOBB Box;
	float fDistX, fDistY, fDistZ;


	m_nSelectAxis = -1;

	EtVector3 vPos = *m_pPoint;

	float fLength = s_fAxisLength - s_fAxisPos;
	Box.Center = vPos;
	Box.Center += EtVector3( 1.f, 0.f, 0.f ) * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
	Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
	Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
	Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
	Box.Extent[0] = ( ( s_fAxisPos / 2.f ) * m_fScale );
	Box.Extent[1] = ( s_fAxisRadius * m_fScale );
	Box.Extent[2] = ( s_fAxisRadius * m_fScale );

	if( TestLineToOBB( vOrig, vDir, Box ) == true ) {
		m_nSelectAxis = 0;
		fDistX = EtVec3Length( &EtVector3( Box.Center - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );
	}

	Box.Center = vPos;
	Box.Center += EtVector3( 0.f, 1.f, 0.f ) * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
	Box.Extent[0] = ( s_fAxisRadius * m_fScale );
	Box.Extent[1] = ( ( s_fAxisPos / 2.f ) * m_fScale ) / 2.f;
	Box.Extent[2] = ( s_fAxisRadius * m_fScale );

	if( TestLineToOBB( vOrig, vDir, Box ) == true ) {
		fDistY = EtVec3Length( &EtVector3( Box.Center - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );

		if( m_nSelectAxis == -1 ) m_nSelectAxis = 1;
		else if( m_nSelectAxis == 0 && fDistY < fDistX )
			m_nSelectAxis = 1;
	}

	Box.Center = vPos;
	Box.Center += EtVector3( 0.f, 0.f, 1.f ) * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
	Box.Extent[0] = ( s_fAxisRadius * m_fScale );
	Box.Extent[1] = ( s_fAxisRadius * m_fScale );
	Box.Extent[2] = ( ( s_fAxisPos / 2.f ) * m_fScale ) / 2.f;

	if( TestLineToOBB( vOrig, vDir, Box ) == true ) {
		fDistZ = EtVec3Length( &EtVector3( Box.Center - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );

		if( m_nSelectAxis == -1 ) m_nSelectAxis = 2;
		else if( m_nSelectAxis == 0 && fDistZ < fDistX )
			m_nSelectAxis = 2;
		else if( m_nSelectAxis == 1 && fDistZ < fDistY )
			m_nSelectAxis = 2;
	}

	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = 10.f;
	if( m_nSelectAxis == -1 ) {
		if( TestLineToSphere( vOrig, vDir, Sphere ) == true ) {
			m_nSelectAxis = 3;
		}
	}

	if( m_nSelectAxis == -1 ) {
		CGlobalValue::GetInstance().m_bControlAxis = false;
		CGlobalValue::GetInstance().m_nSelectAxis = -1;
	}
	else {
		CGlobalValue::GetInstance().m_bControlAxis = true;
		CGlobalValue::GetInstance().m_nSelectAxis = m_nSelectAxis;
	}
	return m_nSelectAxis;

}


void CPointAxisRenderObject::MoveAxis( int nX, int nY, float fSpeed )
{
	if( !m_pPoint ) return;
	EtMatrix matView = *(*CRenderBase::GetInstance().GetLastUpdateCamera());

	EtVector3 vVec, vAxis;
	EtVector3 *pPos = m_pPoint;

	switch( m_nSelectAxis ) {
		case 0:
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			*pPos += ( vAxis * (float)nX * fSpeed );

			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vZAxis;
			vAxis.y = 0.f;
			EtVec3Normalize( &vAxis, &vAxis );
			*pPos += ( vAxis * (float)-nY * fSpeed );
			break;
		case 1:
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
			*pPos += ( vAxis * (float)-nY * fSpeed );
			break;
		case 2:
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			*pPos += ( vAxis * (float)nX * fSpeed );

			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
			EtVec3Normalize( &vAxis, &vAxis );
			*pPos += ( vAxis * (float)-nY * fSpeed );
			break;
		case 3:
			{
				vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
				*pPos += ( vAxis * (float)nX * fSpeed );

				vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
				*pPos += ( vAxis * (float)-nY * fSpeed );
			}
			break;
	}
}