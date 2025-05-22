#include "StdAfx.h"
#include "AxisRenderObject.h"
#include "EtMatrixEx.h"
#include "TEtWorldProp.h"
#include "Renderbase.h"
#include "EtWorldSector.h"

bool CAxisRenderObject::m_bInitialize;
CAxisRenderObject::VertexFormat *CAxisRenderObject::m_pAxis[3] = { 0, 0, 0 };
CAxisRenderObject::VertexFormat *CAxisRenderObject::m_pAxisBack[3] = { 0, 0, 0 };
CAxisRenderObject::VertexFormat *CAxisRenderObject::m_pSelect[3] = { 0, 0, 0 };
int CAxisRenderObject::m_nPrimitiveCount[3] = { 0, 0, 0 };
float CAxisRenderObject::s_fAxisLength = 35.f;
float CAxisRenderObject::s_fAxisPos = 20.f;
float CAxisRenderObject::s_fAxisRadius = 2.f;
int CAxisRenderObject::s_nRefCount = 0;
int CAxisRenderObject::m_nSelectAxis = -1;

CAxisRenderObject::CAxisRenderObject( CEtWorldProp *pProp )
{
	s_nRefCount++;

	m_fScale = 50.f;
	m_nSelectAxis = -1;
	m_pProp = pProp;
}

CAxisRenderObject::~CAxisRenderObject()
{
	s_nRefCount--;
	if( s_nRefCount == 0 ) {
		Destroy();
	}
}

void CAxisRenderObject::Initialize()
{
	if( m_bInitialize == true ) return;
	m_bInitialize = true;

	float fLength = s_fAxisLength - s_fAxisPos;

	m_nPrimitiveCount[0] = CreateCone( &m_pAxis[0], 0xFFFF0000, s_fAxisRadius, fLength, 10, 0, s_fAxisPos );
	CreateCone( &m_pAxisBack[0], 0xFFFF0000, s_fAxisRadius, 0.f, 10, 0, s_fAxisPos );
	CreateCone( &m_pSelect[0], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 0, s_fAxisPos );

	m_nPrimitiveCount[1] = CreateCone( &m_pAxis[1], 0xFF00FF00, s_fAxisRadius, fLength, 10, 1, s_fAxisPos );
	CreateCone( &m_pAxisBack[1], 0xFF00FF00, s_fAxisRadius, 0.f, 10, 1, s_fAxisPos );
	CreateCone( &m_pSelect[1], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 1, s_fAxisPos );

	m_nPrimitiveCount[2] = CreateCone( &m_pAxis[2], 0xFF0000FF, s_fAxisRadius, fLength, 10, 2, s_fAxisPos );
	CreateCone( &m_pAxisBack[2], 0xFF0000FF, s_fAxisRadius, 0.f, 10, 2, s_fAxisPos );
	CreateCone( &m_pSelect[2], 0x80EEEE20, s_fAxisRadius + 2.f, fLength + 2.f, 10, 2, s_fAxisPos );
}

void CAxisRenderObject::Destroy()
{
	for( int i=0; i<3; i++ ) {
		SAFE_DELETEA( m_pAxis[i] );
		SAFE_DELETEA( m_pAxisBack[i] );
		SAFE_DELETEA( m_pSelect[i] );
	}
	m_bInitialize = false;
}

void CAxisRenderObject::RenderCustom( float fElapsedTime )
{
	DrawAxis();
}

void CAxisRenderObject::Update( EtMatrix *pMatrix )
{
	m_WorldMat = *pMatrix;
}

void CAxisRenderObject::DrawAxis()
{
	DWORD dwLightFlag, dwCullMode, dwZEnable;

	GetEtDevice()->GetRenderState( D3DRS_LIGHTING, &dwLightFlag );
	GetEtDevice()->GetRenderState( D3DRS_CULLMODE, &dwCullMode );
	GetEtDevice()->GetRenderState( D3DRS_ZENABLE, &dwZEnable );

	GetEtDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMATRIX mat, matScale;
	mat = m_WorldMat;

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
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pAxis[i], sizeof(VertexFormat) );
		GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pAxisBack[i], sizeof(VertexFormat) );
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

			GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, m_nPrimitiveCount[i], m_pSelect[i], sizeof(VertexFormat) );

			GetEtDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, dwAlphaBlend );
			GetEtDevice()->SetRenderState( D3DRS_BLENDOP, dwBlendOP );
			GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, dwSrcBlend );
			GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, dwDestBlend );
		}
	}

	VertexFormat vPos[2];

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( s_fAxisPos, 0, 0 );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 0 ) ? 0xFFFFFF20 : 0xFFFF0000;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, &vPos[0], sizeof(VertexFormat) );

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( 0, s_fAxisPos, 0 );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 1 ) ? 0xFFFFFF20 : 0xFF00FF00;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, vPos, sizeof(VertexFormat) );

	vPos[0].vPos = EtVector3( 0.f, 0.f, 0.f );
	vPos[1].vPos = EtVector3( 0.f, 0.f, s_fAxisPos );
	vPos[0].dwColor = vPos[1].dwColor = ( m_nSelectAxis == 2 ) ? 0xFFFFFF20 : 0xFF0000FF;
	GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 2, vPos, sizeof(VertexFormat) );

	GetEtDevice()->SetRenderState( D3DRS_LIGHTING, dwLightFlag );
	GetEtDevice()->SetRenderState( D3DRS_CULLMODE, dwCullMode );
	GetEtDevice()->SetRenderState( D3DRS_ZENABLE, dwZEnable );
}

int CAxisRenderObject::CreateCone( VertexFormat **pVertex, DWORD dwColor, float fRadius, float fLength, int nSegment, int nAxis, float fAxisPos )
{
	*pVertex = new VertexFormat[(nSegment+1) * 2];

	EtVector3 vPos;
	switch( nAxis ) 
	{
		case 0: vPos = EtVector3( fAxisPos + fLength, 0, 0 ); break;
		case 1: vPos = EtVector3( 0, fAxisPos + fLength, 0 ); break;
		case 2: vPos = EtVector3( 0, 0, fAxisPos + fLength ); break;
	}

	if( nAxis > 2 ) (*pVertex)[0].vPos = -vPos;
	else (*pVertex)[0].vPos = vPos;

	(*pVertex)[0].dwColor = dwColor;

	int nOffset = 360.0 / ( nSegment - 1 );
	for( int i=0; i<nSegment; i++ ) 
	{
		switch( nAxis ) {
			case 0:
				vPos.x = fAxisPos;
				vPos.y = cos( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				vPos.z = sin( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				break;
			case 1:
				vPos.x = cos( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				vPos.y = fAxisPos;
				vPos.z = sin( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				break;
			case 2:
				vPos.x = cos( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				vPos.y = sin( ( nOffset * i ) / 180.f * D3DX_PI ) * fRadius;
				vPos.z = fAxisPos;
				break;
		}
		if( nAxis > 2 ) (*pVertex)[1+i].vPos = -vPos;
		else  (*pVertex)[1+i].vPos = vPos;

		(*pVertex)[1+i].dwColor = dwColor;
	}

	return ( nSegment - 1 );
}

int CAxisRenderObject::CheckAxis( EtVector3 &vOrig, EtVector3 &vDir )
{
	if( m_pProp == NULL ) return -1;
	SOBB Box;
	float fDistX, fDistY, fDistZ;


	m_nSelectAxis = -1;

	EtVector3 vPos;
	vPos = ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vPosition;

	float fLength = s_fAxisLength - s_fAxisPos;
	Box.Center = vPos;
	Box.Center += ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vXAxis * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
	Box.Axis[0] = ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vXAxis;
	Box.Axis[1] = ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vYAxis;
	Box.Axis[2] = ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vZAxis;
	Box.Extent[0] = ( ( s_fAxisPos / 2.f ) * m_fScale );
	Box.Extent[1] = ( s_fAxisRadius * m_fScale );
	Box.Extent[2] = ( s_fAxisRadius * m_fScale );

	if( TestLineToOBB( vOrig, vDir, Box ) == true ) {
		m_nSelectAxis = 0;
		fDistX = EtVec3Length( &EtVector3( Box.Center - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );
	}

	Box.Center = vPos;
	Box.Center += ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vYAxis * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
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
	Box.Center += ((CTEtWorldProp*)m_pProp)->GetMatEx()->m_vZAxis * ( ( fLength + ( s_fAxisPos / 2.f ) ) * m_fScale );
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
	// 전체 Check
	SAABox ABox;
	float fDist;
	((CTEtWorldProp*)m_pProp)->GetBoundingBox( ABox );
	if( m_nSelectAxis == -1 ) {
		if( TestLineToBox( vOrig, vDir, ABox, fDist ) == true ) {
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

void CAxisRenderObject::MoveAxis( int nX, int nY, float fSpeed )
{
	if( !m_pProp ) return;
	EtMatrix matView = *(*CRenderBase::GetInstance().GetLastUpdateCamera());

	EtVector3 vVec, vAxis;
	EtVector3 *pPos = m_pProp->GetPosition();
	*m_pProp->GetPrePosition() = *pPos;			// 수정
												// UnDo를 위해서 저장하는 위치값이 GetPosiotion() 이다. 그러나 Undo를 위해서 저장하는 위치벡터값이 
												// 이미 마우스 움직임을 계산하고 난 뒤의 위치값이 적용이 된다. 그래서 기존의 고유 위치값을 저장하기 위해	
												// PrePosition을 하나 만들어서 오리지널 위치값을 저장해두고 Undo를 위한 벡터값을 넣어줄때 PrePosition을 이용한다. 
	
	EtVector3 vAxisZ, vAxisY, vResult; 
	float fDot;
	switch( m_nSelectAxis ) {
		case 0:
			// X축
			vAxis =  CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			EtVec3Normalize( &vAxis, &vAxis );
			vAxis.y = 0.f;
			*pPos  += ( vAxis * (float)nX * fSpeed );
			
			vAxisZ =  CRenderBase::GetInstance().GetLastUpdateCamera()->m_vZAxis;
			vAxisY =  CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
			fDot = EtVec3Dot( &CRenderBase::GetInstance().GetLastUpdateCamera()->m_vZAxis, &EtVector3( 0.0f, -1.0f, 0.0f ) );
			vResult = vAxisZ * (1.0f - fDot) + vAxisY * fDot;
			EtVec3Normalize(&vResult, &vResult);
			vResult.y = 0.f; 
			*pPos  += ( vResult * (float)-nY * fSpeed );

			break;	 
		case 1:
			// Y축
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
			EtVec3Normalize( &vAxis, &vAxis );
			vAxis.x = vAxis.z = 0.f;			
			*pPos += ( vAxis * (float)-nY * fSpeed );

			break;
		case 2:
			// Z축
			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vXAxis;
			EtVec3Normalize( &vAxis, &vAxis );
			*pPos += ( vAxis * (float)nX * fSpeed );

			vAxis = CRenderBase::GetInstance().GetLastUpdateCamera()->m_vYAxis;
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
	((CTEtWorldProp*)m_pProp)->UpdateMatrixEx();

}