#include "StdAfx.h"
#include "AxisLockRenderObject.h"
#include "AxisRenderObject.h"
#include "EtMatrixEx.h"
#include "TEtWorldProp.h"
#include "Renderbase.h"
#include "EtWorldSector.h"
#include "TEtWorldSoundEnvi.h"

CAxisLockRenderObject::CAxisLockRenderObject( CTEtWorldSoundEnvi *pEnvi )
{
	CAxisRenderObject::s_nRefCount++;

	m_fScale = 50.f;
	CAxisRenderObject::m_nSelectAxis = -1;
	m_pEnvi = pEnvi;
}

CAxisLockRenderObject::~CAxisLockRenderObject()
{
	CAxisRenderObject::s_nRefCount--;
	if( CAxisRenderObject::s_nRefCount == 0 ) {
		Destroy();
	}
}

void CAxisLockRenderObject::Initialize()
{
	if( CAxisRenderObject::m_bInitialize == true ) return;
	CAxisRenderObject::m_bInitialize = true;

	float fLength = CAxisRenderObject::s_fAxisLength - CAxisRenderObject::s_fAxisPos;

	CAxisRenderObject::m_nPrimitiveCount[0] = CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxis[0], 0xFFFF0000, CAxisRenderObject::s_fAxisRadius, fLength, 10, 0, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxisBack[0], 0xFFFF0000, CAxisRenderObject::s_fAxisRadius, 0.f, 10, 0, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pSelect[0], 0x80EEEE20, CAxisRenderObject::s_fAxisRadius + 2.f, fLength + 2.f, 10, 0, CAxisRenderObject::s_fAxisPos );

	CAxisRenderObject::m_nPrimitiveCount[1] = CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxis[1], 0xFF00FF00, CAxisRenderObject::s_fAxisRadius, fLength, 10, 1, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxisBack[1], 0xFF00FF00, CAxisRenderObject::s_fAxisRadius, 0.f, 10, 1, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pSelect[1], 0x80EEEE20, CAxisRenderObject::s_fAxisRadius + 2.f, fLength + 2.f, 10, 1, CAxisRenderObject::s_fAxisPos );

	CAxisRenderObject::m_nPrimitiveCount[2] = CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxis[2], 0xFF0000FF, CAxisRenderObject::s_fAxisRadius, fLength, 10, 2, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pAxisBack[2], 0xFF0000FF, CAxisRenderObject::s_fAxisRadius, 0.f, 10, 2, CAxisRenderObject::s_fAxisPos );
	CAxisRenderObject::CreateCone( &CAxisRenderObject::m_pSelect[2], 0x80EEEE20, CAxisRenderObject::s_fAxisRadius + 2.f, fLength + 2.f, 10, 2, CAxisRenderObject::s_fAxisPos );
}

void CAxisLockRenderObject::Destroy()
{
	for( int i=0; i<3; i++ ) {
		SAFE_DELETEA( CAxisRenderObject::m_pAxis[i] );
		SAFE_DELETEA( CAxisRenderObject::m_pAxisBack[i] );
		SAFE_DELETEA( CAxisRenderObject::m_pSelect[i] );
	}
	CAxisRenderObject::m_bInitialize = false;
}

void CAxisLockRenderObject::Render()
{
	DrawAxis();
}

void CAxisLockRenderObject::GetExtent( EtVector3 &Origin, EtVector3 &Extent )
{
	Origin = EtVector3( m_WorldMat._41, m_WorldMat._42, m_WorldMat._43 );
	Extent = EtVector3( m_WorldMat._41, m_WorldMat._42, m_WorldMat._43 );
}

void CAxisLockRenderObject::DrawAxis()
{
	DWORD dwLightFlag, dwCullMode;
	
	GetEtDevice()->GetRenderState( D3DRS_LIGHTING, &dwLightFlag );
	GetEtDevice()->GetRenderState( D3DRS_CULLMODE, &dwCullMode );

	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMATRIX mat, matScale;
	mat = m_WorldMat;

	D3DXMatrixScaling( &matScale, m_fScale, m_fScale, m_fScale );
	D3DXMatrixMultiply( &mat, &matScale, &mat );
	GetEtDevice()->SetWorldTransform( &mat );
	GetEtDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	GetEtDevice()->SetVertexShader( NULL );
	GetEtDevice()->SetPixelShader( NULL );

	for( DWORD i=0; i<3; i++ ) {
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, CAxisRenderObject::m_nPrimitiveCount[i], CAxisRenderObject::m_pAxis[i], sizeof(CAxisRenderObject::VertexFormat) );
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, CAxisRenderObject::m_nPrimitiveCount[i], CAxisRenderObject::m_pAxisBack[i], sizeof(CAxisRenderObject::VertexFormat) );
		if( CAxisRenderObject::m_nSelectAxis == i ) {
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

			GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, CAxisRenderObject::m_nPrimitiveCount[i], CAxisRenderObject::m_pSelect[i], sizeof(CAxisRenderObject::VertexFormat) );

			GetEtDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, dwAlphaBlend );
			GetEtDevice()->SetRenderState( D3DRS_BLENDOP, dwBlendOP );
			GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, dwSrcBlend );
			GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, dwDestBlend );
		}
	}

	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, dwLightFlag );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, dwCullMode );

	DWORD dwDiffuse;

	EtVector3 vPos[2];

	vPos[0] = EtVector3( 0.f, 0.f, 0.f );
	vPos[1] = EtVector3( CAxisRenderObject::s_fAxisPos, 0, 0 );
	dwDiffuse = ( CAxisRenderObject::m_nSelectAxis == 0 ) ? 0xFFFFFF20 : 0xFFFF0000;
	EtVec3TransformCoord( &vPos[0], &vPos[0], &mat );
	EtVec3TransformCoord( &vPos[1], &vPos[1], &mat );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], dwDiffuse );

	vPos[0] = EtVector3( 0.f, 0.f, 0.f );
	vPos[1] = EtVector3( 0, CAxisRenderObject::s_fAxisPos, 0 );
	dwDiffuse = ( CAxisRenderObject::m_nSelectAxis == 1 ) ? 0xFFFFFF20 : 0xFF00FF00;
	EtVec3TransformCoord( &vPos[0], &vPos[0], &mat );
	EtVec3TransformCoord( &vPos[1], &vPos[1], &mat );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], dwDiffuse );

	dwDiffuse = ( CAxisRenderObject::m_nSelectAxis == 2 ) ? 0xFFFFFF20 : 0xFF0000FF;
	vPos[0] = EtVector3( 0.f, 0.f, 0.f );
	vPos[1] = EtVector3( 0.f, 0.f, CAxisRenderObject::s_fAxisPos );
	EtVec3TransformCoord( &vPos[0], &vPos[0], &mat );
	EtVec3TransformCoord( &vPos[1], &vPos[1], &mat );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], dwDiffuse );
}

int CAxisLockRenderObject::CheckAxis( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vOffset )
{
	if( m_pEnvi == NULL ) return -1;
	SAABox Box;
	float fDistX, fDistY, fDistZ;


	CAxisRenderObject::m_nSelectAxis = -1;

	EtVector3 vPos;
	vPos = *m_pEnvi->GetPosition() + vOffset;
	Box.Min.x = vPos.x;
	Box.Min.y = vPos.y - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Min.z = vPos.z - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Max.x = Box.Min.x + ( CAxisRenderObject::s_fAxisLength * m_fScale );
	Box.Max.y = Box.Min.y + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );
	Box.Max.z = Box.Min.z + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );

	if( TestLineToBox( vOrig, vDir, Box, fDistX ) == true ) CAxisRenderObject::m_nSelectAxis = 0;

	Box.Min.x = vPos.x - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Min.y = vPos.y;
	Box.Min.z = vPos.z - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Max.x = Box.Min.x + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );
	Box.Max.y = Box.Min.y + ( CAxisRenderObject::s_fAxisLength * m_fScale );
	Box.Max.z = Box.Min.z + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );

	if( TestLineToBox( vOrig, vDir, Box, fDistY ) == true ) {
		if( CAxisRenderObject::m_nSelectAxis == -1 ) CAxisRenderObject::m_nSelectAxis = 1;
		else if( CAxisRenderObject::m_nSelectAxis == 0 && fDistY < fDistX )
			CAxisRenderObject::m_nSelectAxis = 1;
	}

	Box.Min.x = vPos.x - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Min.y = vPos.y - ( CAxisRenderObject::s_fAxisRadius * m_fScale );
	Box.Min.z = vPos.z;
	Box.Max.x = Box.Min.x + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );
	Box.Max.y = Box.Min.y + ( CAxisRenderObject::s_fAxisRadius * 2.f * m_fScale );
	Box.Max.z = Box.Min.z + ( CAxisRenderObject::s_fAxisLength * m_fScale );

	if( TestLineToBox( vOrig, vDir, Box, fDistZ ) == true ) {
		if( CAxisRenderObject::m_nSelectAxis == -1 ) CAxisRenderObject::m_nSelectAxis = 2;
		else if( CAxisRenderObject::m_nSelectAxis == 0 && fDistZ < fDistX ) CAxisRenderObject::m_nSelectAxis = 2;
		else if( CAxisRenderObject::m_nSelectAxis == 1 && fDistZ < fDistY ) CAxisRenderObject::m_nSelectAxis = 2;
	}

	SSphere Sphere;
	Sphere.Center = *m_pEnvi->GetPosition() + vOffset;
	Sphere.fRadius = m_pEnvi->GetRange();
	if( TestLineToSphere( vOrig, vDir, Sphere ) == true ) {
		if( CAxisRenderObject::m_nSelectAxis == -1 ) CAxisRenderObject::m_nSelectAxis = 3;
	}

	if( CAxisRenderObject::m_nSelectAxis == -1 ) {
		CGlobalValue::GetInstance().m_bControlAxis = false;
		CGlobalValue::GetInstance().m_nSelectAxis = -1;
	}
	else {
		CGlobalValue::GetInstance().m_bControlAxis = true;
		CGlobalValue::GetInstance().m_nSelectAxis = CAxisRenderObject::m_nSelectAxis;
	}
	return CAxisRenderObject::m_nSelectAxis;

}


void CAxisLockRenderObject::MoveAxis( int nX, int nY )
{
	if( !m_pEnvi ) return;
	EtMatrix matView = *(*CRenderBase::GetInstance().GetLastUpdateCamera());

	EtVector3 vVec, vAxis;
	EtVector3 *pPos = m_pEnvi->GetPosition();

	static float fSpeed = 5.f;
	switch( CAxisRenderObject::m_nSelectAxis ) {
		case 0:
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			*pPos += ( vAxis * (float)nX * fSpeed );

			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vZAxis;
			vAxis.y = 0.f;
			EtVec3Normalize( &vAxis, &vAxis );
			*pPos += ( vAxis * (float)-nY * fSpeed );
			break;
		case 1:
			vAxis = EtVector3( 0.f, 1.f, 0.f );
			*pPos += ( vAxis * (float)-nY * fSpeed );
			break;
		case 2:
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			*pPos += ( vAxis * (float)nX * fSpeed );

			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vZAxis;
			vAxis.y = 0.f;
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