#include "StdAfx.h"
#include "TEtWorldLightProp.h"
#include "resource.h"
#include "SundriesFuncEx.h"
#include "AxisRenderObject.h"
#include "RenderBase.h"
#include "TEtWorldSector.h"
#include "TEtWorld.h"

/*
PropertyGridBaseDefine g_LightPropPropertyDefine[] = {
	{ "Common", "Position", CUnionValueProperty::Vector3, "World Position", FALSE },
	{ "Common", "Rotation", CUnionValueProperty::Vector3, "World Rotation", FALSE },
	{ "Common", "Scale", CUnionValueProperty::Vector3, "World Scale", FALSE },
	{ "Common", "Cast Shadow", CUnionValueProperty::Boolean, "Cast Shadow", FALSE },
	{ "Common", "Receive Shadow", CUnionValueProperty::Boolean, "Receive Shadow", FALSE },

	{ "Light", "LightType", CUnionValueProperty::Integer_Combo, "Light Type|Direction|Point|Spot", TRUE },
	{ "Light", "Diffuse", CUnionValueProperty::Vector4Color, "Color", TRUE },
	{ "Light", "Specular", CUnionValueProperty::Vector4Color, "Color", TRUE },
	{ "Light", "Position", CUnionValueProperty::Vector3, "Position", FALSE },
	{ "Light", "Direction", CUnionValueProperty::Vector3, "Direction", FALSE },
	{ "Light", "Range", CUnionValueProperty::Float, "Range", FALSE },
	{ "Light", "Falloff", CUnionValueProperty::Float, "Falloff", FALSE },
	{ "Light", "Theta", CUnionValueProperty::Float, "Theta", FALSE },
	{ "Light", "Phi", CUnionValueProperty::Float, "Phi", FALSE },
	{ "Light", "Cast Shadow", CUnionValueProperty::Boolean, "Cast Shadow", TRUE },
	NULL,
};
*/

CTEtWorldLightProp::CTEtWorldLightProp()
{
	// Make Direction
	m_vDirection[0] = D3DXVECTOR3( 0, 0, -10 );
	m_vDirection[1] = D3DXVECTOR3( 0, 0, 10 );
	m_vDirection[2] = D3DXVECTOR3( 2, -2, -10 );
	m_vDirection[3] = D3DXVECTOR3( 2, 2, -10 );
	m_vDirection[4] = D3DXVECTOR3( -2, -2, -10 );
	m_vDirection[5] = D3DXVECTOR3( -2, 2, -10 );
	m_vDirection[6] = D3DXVECTOR3( 2, -2, -10 );
	m_vDirection[7] = D3DXVECTOR3( -2, -2, -10 );
	m_vDirection[8] = D3DXVECTOR3( 2, 2, -10 );
	m_vDirection[9] = D3DXVECTOR3( -2, 2, -10 );
	m_vDirection[10] = D3DXVECTOR3( 2, 2, -10 );
	m_vDirection[11] = D3DXVECTOR3( 0, 0, -15 );
	m_vDirection[12] = D3DXVECTOR3( 2, -2, -10 );
	m_vDirection[13] = D3DXVECTOR3( 0, 0, -15 );
	m_vDirection[14] = D3DXVECTOR3( -2, 2, -10 );
	m_vDirection[15] = D3DXVECTOR3( 0, 0, -15 );
	m_vDirection[16] = D3DXVECTOR3( -2, -2, -10 );
	m_vDirection[17] = D3DXVECTOR3( 0, 0, -15 );
}

CTEtWorldLightProp::~CTEtWorldLightProp()
{
	FreeObject();
}

bool CTEtWorldLightProp::LoadObject()
{
	bool bResult = CTEtWorldProp::LoadObject();
	if( bResult == false ) return false;

	CreateLight();
	return true;
}

bool CTEtWorldLightProp::FreeObject()
{
	bool bResult = CTEtWorldProp::FreeObject();
	if( bResult == false ) return false;

	SAFE_RELEASE_SPTR( m_LightHandle );
	return true;
}

void CTEtWorldLightProp::Show( bool bShow )
{
	m_bShow = bShow;
	if( m_Handle ) m_Handle->ShowObject( bShow );
	if( bShow ) CreateLight();
	else SAFE_RELEASE_SPTR( m_LightHandle );
}

bool CTEtWorldLightProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( GetPropertyCount() - m_nCustomParamOffset < 10 ) return false;
	if( GetProperty(m_nCustomParamOffset)->GetVariableInt() == 0 &&
		GetProperty(m_nCustomParamOffset+1)->GetVariableVector4() == EtVector4( 0.f, 0.f, 0.f, 0.f ) &&
		GetProperty(m_nCustomParamOffset+2)->GetVariableVector4() == EtVector4( 0.f, 0.f, 0.f, 0.f ) &&
		GetProperty(m_nCustomParamOffset+3)->GetVariableVector3() == EtVector3( 0.f, 0.f, 0.f ) &&
		GetProperty(m_nCustomParamOffset+4)->GetVariableVector3() == EtVector3( 0.f, 0.f, 0.f ) &&
		GetProperty(m_nCustomParamOffset+5)->GetVariableFloat() == 0.f &&
		GetProperty(m_nCustomParamOffset+6)->GetVariableFloat() == 0.f &&
		GetProperty(m_nCustomParamOffset+7)->GetVariableFloat() == 0.f &&
		GetProperty(m_nCustomParamOffset+8)->GetVariableFloat() == 0.f &&
		GetProperty(m_nCustomParamOffset+9)->GetVariableBool() == false ) {
		SLightInfo Info;
		GetProperty(m_nCustomParamOffset)->SetVariable(LT_POINT);
		GetProperty(m_nCustomParamOffset+1)->SetVariable((EtVector4)Info.Diffuse);
		GetProperty(m_nCustomParamOffset+2)->SetVariable((EtVector4)Info.Specular);
		GetProperty(m_nCustomParamOffset+3)->SetVariable((EtVector3)Info.Position);
		GetProperty(m_nCustomParamOffset+4)->SetVariable((EtVector3)Info.Direction);
		GetProperty(m_nCustomParamOffset+5)->SetVariable(Info.fRange);
		GetProperty(m_nCustomParamOffset+6)->SetVariable(Info.fFalloff);
		GetProperty(m_nCustomParamOffset+7)->SetVariable( EtToDegree( acos( Info.fTheta) ) );
		GetProperty(m_nCustomParamOffset+8)->SetVariable( EtToDegree( acos( Info.fPhi) ) );
	}

	bool bResult = CTEtWorldProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	CreateLight();
	return bResult;
}

bool CTEtWorldLightProp::Render( LOCAL_TIME LocalTime )
{
	if( !m_Handle ) return false;

	EtMatrix mat, matScale, matTemp;
	mat = *m_matExWorld;

	switch( GetProperty(m_nCustomParamOffset)->GetVariableInt() ) {
		case LT_DIRECTIONAL: 
			{
				if( m_bSelect ) DrawDirectionLight(); 
			}
			break;
		case LT_POINT: 
			{
				MatrixEx Cross;
				Cross.m_vPosition = m_matExWorld.m_vPosition;
				mat = *Cross;

				if( m_bSelect ) DrawPointLight(); 
			}
			break;
		case LT_SPOT: 
			{
				if( m_bSelect ) DrawSpotLight(); 
			}
			break;
	}

	m_Handle->Update( &mat );

	if( m_bSelect ) DrawBoundingBox( mat );

	return true;
}

/*
PropertyGridBaseDefine *CTEtWorldLightProp::GetPropertyDefine()
{
	return g_LightPropPropertyDefine;
}
*/

void CTEtWorldLightProp::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
	SLightInfo Info = GetLightInfo();

	bool bEnable = true;
	switch( dwIndex ) {
		case 1:
			if( Info.Type == LT_POINT ) bEnable = false;
			break;
		case 2:
		case 5:
		case 6:
			bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT:
			pVariable->SetVariable( (int)Info.Type );
			break;
		case PROP_DEFAULT_PROP_COUNT+1:
			pVariable->SetVariable( (D3DXVECTOR4)Info.Diffuse );
			break;
		case PROP_DEFAULT_PROP_COUNT+2:
			pVariable->SetVariable( (D3DXVECTOR4)Info.Specular );
			break;
		case PROP_DEFAULT_PROP_COUNT+3:
			if( Info.Type == LT_DIRECTIONAL ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+4:
			if( Info.Type == LT_POINT ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+5:
			if( Info.Type == LT_DIRECTIONAL ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+6:
			if( Info.Type != LT_SPOT ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+7:
			if( Info.Type != LT_SPOT ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+8:
			if( Info.Type != LT_SPOT ) bEnable = false;
			break;
		case PROP_DEFAULT_PROP_COUNT+9:
			break;
	}
	pVariable->SetDefaultEnable( bEnable );
//	pProperty->SetReadOnly( dwIndex, !bEnable );

	CTEtWorldProp::OnSetValue( pVariable, dwIndex, pProperty );
}

void CTEtWorldLightProp::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
//	LightInfoStruct *pStruct = (LightInfoStruct *)m_pAdditionalPropInfo;
	SLightInfo Info = GetLightInfo();

	bool bEnable = true;
	switch( dwIndex ) {
		case PROP_DEFAULT_PROP_COUNT:
			Info.Type = (LightType)pVariable->GetVariableInt();
			switch( Info.Type ) {
				case LT_DIRECTIONAL:
					pProperty->SetReadOnly( 1, false );
					pProperty->SetReadOnly( 2, true );

					pProperty->SetReadOnly( 8, true );
					pProperty->SetReadOnly( 9, false );
					pProperty->SetReadOnly( 10, true );
					pProperty->SetReadOnly( 11, true );
					pProperty->SetReadOnly( 12, true );
					pProperty->SetReadOnly( 13, true );
					break;
				case LT_POINT:
					pProperty->SetReadOnly( 1, true );
					pProperty->SetReadOnly( 2, true );

					pProperty->SetReadOnly( 8, false );
					pProperty->SetReadOnly( 9, true );
					pProperty->SetReadOnly( 10, false );
					pProperty->SetReadOnly( 11, true );
					pProperty->SetReadOnly( 12, true );
					pProperty->SetReadOnly( 13, true );
					break;
				case LT_SPOT:
					pProperty->SetReadOnly( 1, false );
					pProperty->SetReadOnly( 2, true );

					pProperty->SetReadOnly( 8, false );
					pProperty->SetReadOnly( 9, false );
					pProperty->SetReadOnly( 10, false );
					pProperty->SetReadOnly( 11, false );
					pProperty->SetReadOnly( 12, false );
					pProperty->SetReadOnly( 13, false );
					break;
			}
			CreateLight();
			break;
		case PROP_DEFAULT_PROP_COUNT+1:
			Info.Diffuse = (EtColor)pVariable->GetVariableVector4();
			break;
		case PROP_DEFAULT_PROP_COUNT+2:
			Info.Specular = (EtColor)pVariable->GetVariableVector4();
			break;
		case PROP_DEFAULT_PROP_COUNT+3:
			{
				if( Info.Type == LT_DIRECTIONAL ) bEnable = false;

				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

				EtVector3 vPos = pVariable->GetVariableVector3();
				EtVector3 vOffset = *pSector->GetOffset();
				vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.y = 0.f;
				vPos -= vOffset;

				SetPosition( vPos );
				UpdateMatrixEx();

			}
			break;
		case PROP_DEFAULT_PROP_COUNT+4:
			if( Info.Type == LT_POINT ) bEnable = false;
			Info.Direction = pVariable->GetVariableVector3();
			m_matExWorld.m_vYAxis = Info.Direction;
			m_matExWorld.MakeUpCartesianByZAxis();
			break;
		case PROP_DEFAULT_PROP_COUNT+5:
			if( Info.Type == LT_DIRECTIONAL ) bEnable = false;
			Info.fRange = pVariable->GetVariableFloat();
			break;
		case PROP_DEFAULT_PROP_COUNT+6:
			if( Info.Type != LT_SPOT ) bEnable = false;
			Info.fFalloff = pVariable->GetVariableFloat();
			break;
		case PROP_DEFAULT_PROP_COUNT+7:
			if( Info.Type != LT_SPOT ) bEnable = false;
//			Info.fTheta = cos( EtToRadian( pVariable->GetVariableFloat() ) );
//			pVariable->SetVariable( Info.fTheta );
			break;
		case PROP_DEFAULT_PROP_COUNT+8:
			if( Info.Type != LT_SPOT ) bEnable = false;
//			Info.fPhi = cos( EtToRadian( pVariable->GetVariableFloat() ) );
//			pVariable->SetVariable( Info.fPhi );
			break;
		case PROP_DEFAULT_PROP_COUNT+9:
			Info.bCastShadow = pVariable->GetVariableBool();
			break;
	}
	SetLightInfo( Info );

	RefreshLight();

	CTEtWorldProp::OnChangeValue( pVariable, dwIndex, pProperty );

	CGlobalValue::GetInstance().RefreshRender();
}

void CTEtWorldLightProp::RefreshLight()
{
	SLightInfo Info = GetLightInfo();

	Info.Diffuse.r *= Info.Diffuse.a;
	Info.Diffuse.g *= Info.Diffuse.a;
	Info.Diffuse.b *= Info.Diffuse.a;
	Info.Specular.r *= Info.Specular.a;
	Info.Specular.g *= Info.Specular.a;
	Info.Specular.b *= Info.Specular.a;
	Info.fTheta = cos( EtToRadian( Info.fTheta ) );
	Info.fPhi = cos( EtToRadian( Info.fPhi ) );

	m_LightHandle->SetLightInfo( &Info );
}

/*
int CTEtWorldLightProp::GetAdditionalPropInfoSize()
{
	return sizeof(LightInfoStruct);
}

void CTEtWorldLightProp::AllocAdditionalPropInfo()
{
	m_pAdditionalPropInfo = new LightInfoStruct;
	((LightInfoStruct*)m_pAdditionalPropInfo)->Info.Type = LT_POINT;
}
*/

void CTEtWorldLightProp::DrawDirectionLight()
{
	EtVector3 vDirection[18];
	MatrixEx Cross;
	Cross.m_vYAxis = -m_LightHandle->GetLightInfo()->Direction;
	Cross.MakeUpCartesianByZAxis();

	for( int i=0; i<18; i++ ) {
		vDirection[i] = m_vDirection[i] * 100.f;
		EtVec3TransformCoord( &vDirection[i], &vDirection[i], Cross );
		if( i % 2 == 1 ) {
			EternityEngine::DrawLine3D( vDirection[i-1], vDirection[i], 0xFFFFFF00 );
		}
	}
}

void CTEtWorldLightProp::DrawPointLight()
{
	EtVector3 vVec[2];
	MatrixEx Cross;
	int nDist = 40;
	float fAngle = 360 / (float)(nDist-1.f);
	SLightInfo *pInfo = m_LightHandle->GetLightInfo();

	Cross.m_vPosition = pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateYaw( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotatePitch( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vXAxis * pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateRoll( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vXAxis * pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}
}

void CTEtWorldLightProp::DrawSpotLight()
{
	EtVector3 vVec[2], vVec2[2];
	MatrixEx Cross;
	SLightInfo Info = GetLightInfo();
	float fTheta = Info.fTheta;
	float fPhi = Info.fPhi;

	// 외각
	DrawSpotFunc1( fPhi, &Info, false, 0xFF999900 );
	DrawSpotFunc1( -fPhi, &Info, false, 0xFF999900 );
	DrawSpotFunc1( fPhi, &Info, true, 0xFF999900 );
	DrawSpotFunc1( -fPhi, &Info, true, 0xFF999900 );

	// 내각
	DrawSpotFunc1( fTheta, &Info, false, 0xFFFFFF00 );
	DrawSpotFunc1( -fTheta, &Info, false, 0xFFFFFF00 );
	DrawSpotFunc1( fTheta, &Info, true, 0xFFFFFF00 );
	DrawSpotFunc1( -fTheta, &Info, true, 0xFFFFFF00 );

	// 외각 내각 원
	int nDist = 30;
	float fAngle = 360 / (float)(nDist-1.f);


	Cross.m_vPosition = Info.Position + ( Info.Direction * Info.fRange );
	Cross.m_vZAxis = Info.Direction;
	Cross.MakeUpCartesianByZAxis();
	if( EtVec3LengthSq( &Cross.m_vXAxis ) == 0.f ) {
		Cross.m_vXAxis = EtVector3( 1.f, 0.f, 0.f );
		Cross.MakeUpCartesianByXAxis();
	}

	float fRangePhi = Info.fRange * tan( EtToRadian( fPhi ) );
	float fRangeTheta = Info.fRange * tan( EtToRadian( fTheta ) );

	vVec[0] = Cross.m_vPosition + ( Cross.m_vXAxis * fRangePhi );
	vVec2[0] = Cross.m_vPosition + ( Cross.m_vXAxis * fRangeTheta );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateRoll( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vXAxis * fRangePhi );
		vVec2[1] = Cross.m_vPosition + ( Cross.m_vXAxis * fRangeTheta );

		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFF999900 );
		EternityEngine::DrawLine3D( vVec2[0], vVec2[1], 0xFFFFFF00 );

		vVec[0] = vVec[1];
		vVec2[0] = vVec2[1];
	}
}


void CTEtWorldLightProp::CreateLight()
{
	SAFE_RELEASE_SPTR( m_LightHandle );
	SLightInfo Info = GetLightInfo();
	Info.Position = m_matExWorld.m_vPosition;
	switch( Info.Type ) {
		case LT_POINT:
			Info.Direction = m_matExWorld.m_vZAxis;
			break;
		case LT_DIRECTIONAL:
		case LT_SPOT:
			Info.Direction = m_matExWorld.m_vYAxis;
			break;
	}
	SetLightInfo( Info );

	Info = GetLightInfo();
	Info.Diffuse.r *= Info.Diffuse.a;
	Info.Diffuse.g *= Info.Diffuse.a;
	Info.Diffuse.b *= Info.Diffuse.a;
	Info.Specular.r *= Info.Specular.a;
	Info.Specular.g *= Info.Specular.a;
	Info.Specular.b *= Info.Specular.a;

	m_LightHandle = EternityEngine::CreateLight( &Info );
}

void CTEtWorldLightProp::MoveAxis( int nX, int nY, float fSpeed )
{
	SLightInfo Info = GetLightInfo();

	bool bUpdateAxis = true;
	switch( Info.Type ) {
		case LT_DIRECTIONAL:
			if( GetAKState( VK_LMENU ) < 0 ) {
				bUpdateAxis = false;
			}
			if( GetAKState( VK_LCONTROL ) < 0 && CAxisRenderObject::m_nSelectAxis == 1 ) {
				bUpdateAxis = false;
			}
			break;
		case LT_POINT:
			if( GetAKState( VK_LMENU ) < 0 ) {
				float fValue = (float)( nX + nY );
				Info.fRange += fValue;
				bUpdateAxis = false;
			}
			if( GetAKState( VK_LCONTROL ) < 0 ) {
				bUpdateAxis = false;
			}
			break;
		case LT_SPOT:
			if( GetAKState( VK_LMENU ) < 0 ) {
				switch( CAxisRenderObject::m_nSelectAxis ) {
					case 0:
						{
							float fValue = ( nX * 0.1f ) + ( nY * 0.1f );
							float fAngle = Info.fPhi;//EtToDegree( acos( Info.fPhi ) );

							fAngle += fValue;
							if( fAngle > 180.f ) fAngle = 180.f;
							else if( fAngle < Info.fTheta ) fAngle = Info.fTheta;
							Info.fPhi = fAngle;//cos( EtToRadian( fAngle ) );
						}
						break;
					case 1:
						{
							float fValue = (float)( nX + nY );
							Info.fRange += fValue;
						}
						break;
					case 2:
						{
							float fValue = ( nX * 0.1f ) + ( nY * 0.1f );
							float fAngle = Info.fTheta;

							fAngle += fValue;
							if( fAngle > Info.fPhi ) fAngle = Info.fPhi;
							else if( fAngle < 0.f ) fAngle = 0.f;
							Info.fTheta = fAngle;
						}
						break;
				}
				bUpdateAxis = false;
			}
			if( GetAKState( VK_LCONTROL ) < 0 && CAxisRenderObject::m_nSelectAxis == 1 ) {
				bUpdateAxis = false;
			}
			break;
	}
	if( bUpdateAxis ) {
		CTEtWorldProp::MoveAxis( nX, nY, fSpeed );
	}


	Info.Position = m_matExWorld.m_vPosition;

	switch( Info.Type ) {
		case LT_POINT:
			Info.Direction = m_matExWorld.m_vZAxis;
			break;
		case LT_DIRECTIONAL:
		case LT_SPOT:
			Info.Direction = m_matExWorld.m_vYAxis;
			break;
	}
	m_matExWorld.MakeUpCartesianByZAxis();

	SetLightInfo( Info );
	RefreshLight();
//	m_LightHandle->SetLightInfo( &Info );
}

void CTEtWorldLightProp::DrawSpotFunc1( float fAngle, SLightInfo *pInfo, bool bYawPitch, DWORD dwColor )
{
	EtVector3 vVec[2];
	MatrixEx Cross;
	Cross.m_vZAxis = pInfo->Direction;
	Cross.MakeUpCartesianByZAxis();
	if( EtVec3LengthSq( &Cross.m_vXAxis ) == 0.f ) {
		Cross.m_vXAxis = EtVector3( 1.f, 0.f, 0.f );
		Cross.MakeUpCartesianByXAxis();
	}

	if( bYawPitch == false ) Cross.RotateYaw( fAngle );
	else Cross.RotatePitch( fAngle );

	float fRange = pInfo->fRange * tan( EtToRadian( abs(fAngle) ) );

	vVec[0] = pInfo->Position;
	vVec[1] = pInfo->Position + ( Cross.m_vZAxis * sqrtf( (pInfo->fRange*pInfo->fRange) + (fRange*fRange) ) );

	EternityEngine::DrawLine3D( vVec[0], vVec[1], dwColor );
}


void CTEtWorldLightProp::UpdateMatrixEx()
{
	switch( GetProperty(m_nCustomParamOffset)->GetVariableInt() ) {
	case LT_POINT:
		m_vRotation = EtVector3( 0.f, 0.f, 0.f );
		break;
	case LT_DIRECTIONAL:
		break;
	case LT_SPOT:
		break;
	}
	m_vScale = EtVector3( 0.f, 0.f, 0.f );

	CTEtWorldProp::UpdateMatrixEx();
	GetProperty(m_nCustomParamOffset+3)->SetVariable( m_matExWorld.m_vPosition );
}

SLightInfo CTEtWorldLightProp::GetLightInfo()
{
	SLightInfo Info;
	Info.Type = (LightType)GetProperty(m_nCustomParamOffset)->GetVariableInt();
	Info.Diffuse = (EtColor)GetProperty(m_nCustomParamOffset+1)->GetVariableVector4();
	Info.Specular = (EtColor)GetProperty(m_nCustomParamOffset+2)->GetVariableVector4();
	Info.Position = GetProperty(m_nCustomParamOffset+3)->GetVariableVector3();
	Info.Direction = GetProperty(m_nCustomParamOffset+4)->GetVariableVector3();
	Info.fRange = GetProperty(m_nCustomParamOffset+5)->GetVariableFloat();
	Info.fFalloff = GetProperty(m_nCustomParamOffset+6)->GetVariableFloat();
	Info.fTheta = GetProperty(m_nCustomParamOffset+7)->GetVariableFloat();
	Info.fPhi = GetProperty(m_nCustomParamOffset+8)->GetVariableFloat();
	Info.bCastShadow = GetProperty(m_nCustomParamOffset+9)->GetVariableBool();

	return Info;
}

void CTEtWorldLightProp::SetLightInfo( SLightInfo Info )
{
	GetProperty(m_nCustomParamOffset)->SetVariable( (int)Info.Type );
	GetProperty(m_nCustomParamOffset+1)->SetVariable( (EtVector4)Info.Diffuse );
	GetProperty(m_nCustomParamOffset+2)->SetVariable( (EtVector4)Info.Specular );
	GetProperty(m_nCustomParamOffset+3)->SetVariable( Info.Position );
	GetProperty(m_nCustomParamOffset+4)->SetVariable( Info.Direction );
	GetProperty(m_nCustomParamOffset+5)->SetVariable( Info.fRange );
	GetProperty(m_nCustomParamOffset+6)->SetVariable( Info.fFalloff );
	GetProperty(m_nCustomParamOffset+7)->SetVariable( Info.fTheta );
	GetProperty(m_nCustomParamOffset+8)->SetVariable( Info.fPhi );
	GetProperty(m_nCustomParamOffset+9)->SetVariable( Info.bCastShadow );
}

void CTEtWorldLightProp::ShowProp( bool bShow )
{
	m_bShow = bShow;
	if( m_Handle ) m_Handle->ShowObject( bShow );
}