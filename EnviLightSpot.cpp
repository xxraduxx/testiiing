#include "StdAfx.h"
#include "EnviLightSpot.h"
#include "RenderBase.h"
#include "InputReceiver.h"
#include "GlobalValue.h"
#include "SundriesFuncEx.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "UserMessage.h"

PropertyGridBaseDefine g_PropertyDefineSpot[] = {
	{ "Setting", "Diffuse", CUnionValueProperty::Vector4Color, "Diffuse Color", TRUE },
	{ "Setting", "Specular", CUnionValueProperty::Vector4Color, "Specular Color", TRUE },
	{ "Setting", "Position", CUnionValueProperty::Vector3, "Position", TRUE },
	{ "Setting", "Direction", CUnionValueProperty::Vector3, "Direction!", TRUE },
	{ "Setting", "Range", CUnionValueProperty::Float, "Range", TRUE },
	{ "Setting", "Theta", CUnionValueProperty::Float, "Theta", TRUE },
	{ "Setting", "Phi", CUnionValueProperty::Float, "Phi", TRUE },
	{ "Setting", "FallOff", CUnionValueProperty::Float, "FallOff", TRUE },
	{ "Setting", "CastShadow", CUnionValueProperty::Boolean, "Cast Shadow", TRUE },
	NULL,
};

CEnviLightSpot::CEnviLightSpot( CEnviElement *pElement )
: CEnviLightBase( pElement )
{
	m_nMouseFlag = 0;
}

CEnviLightSpot::~CEnviLightSpot()
{
}

void CEnviLightSpot::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (D3DXVECTOR4)m_pElementPtr->pInfo->Diffuse );
			break;
		case 1:
			pVariable->SetVariable( (D3DXVECTOR4)m_pElementPtr->pInfo->Specular );
			break;
		case 2:
			pVariable->SetVariable( m_pElementPtr->pInfo->Position );
			break;
		case 3:
			pVariable->SetVariable( m_pElementPtr->pInfo->Direction );
			break;
		case 4:
			pVariable->SetVariable( m_pElementPtr->pInfo->fRange );
			break;
		case 5:
			pVariable->SetVariable( EtToDegree( acos( m_pElementPtr->pInfo->fTheta ) ) );
			break;
		case 6:
			pVariable->SetVariable( EtToDegree( acos( m_pElementPtr->pInfo->fPhi ) ) );
			break;
		case 7:
			pVariable->SetVariable( m_pElementPtr->pInfo->fFalloff );
			break;
	}
}

void CEnviLightSpot::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			m_pElementPtr->pInfo->Diffuse = (EtColor)pVariable->GetVariableVector4();
			break;
		case 1:
			m_pElementPtr->pInfo->Specular = (EtColor)pVariable->GetVariableVector4();
			break;
		case 2:
			m_pElementPtr->pInfo->Position = (EtVector3)pVariable->GetVariableVector3();
			break;
		case 3:
			m_pElementPtr->pInfo->Direction = (EtVector3)pVariable->GetVariableVector3();
			break;
		case 4:
			m_pElementPtr->pInfo->fRange = pVariable->GetVariableFloat();
			break;
		case 5:
			m_pElementPtr->pInfo->fTheta = cos( EtToRadian( pVariable->GetVariableFloat() ) );
			break;
		case 6:
			m_pElementPtr->pInfo->fPhi = cos( EtToRadian( pVariable->GetVariableFloat() ) );
			break;
		case 7:
			m_pElementPtr->pInfo->fFalloff = pVariable->GetVariableFloat();
			break;
	}
}

bool CEnviLightSpot::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver )
{
	if( GetAKState( VK_LCONTROL ) >= 0 ) return true;

	m_matExWorld.m_vPosition = m_pElementPtr->pInfo->Position;
	m_matExWorld.m_vZAxis = m_pElementPtr->pInfo->Direction;
	m_matExWorld.MakeUpCartesianByZAxis();

	if( nReceiverState & CInputReceiver::IR_MOUSE_MOVE ) {
		char cUpdate = -1;
		if( ( m_nMouseFlag & LB_DOWN ) && ( m_nMouseFlag & RB_DOWN ) ) {
			m_pElementPtr->pInfo->fRange -= pReceiver->GetMouseVariation().y;
			if( m_pElementPtr->pInfo->fRange <= 0.f ) m_pElementPtr->pInfo->fRange = 0.f;
			cUpdate = 1;
		}
		else if( ( m_nMouseFlag & WB_DOWN ) && ( m_nMouseFlag & LB_DOWN ) ) {
			m_pElementPtr->pInfo->fPhi = EtToDegree( acos( m_pElementPtr->pInfo->fPhi ) );
			m_pElementPtr->pInfo->fTheta = EtToDegree( acos( m_pElementPtr->pInfo->fTheta ) );
			m_pElementPtr->pInfo->fPhi -= pReceiver->GetMouseVariation().y / 4.f;
			if( m_pElementPtr->pInfo->fPhi <= m_pElementPtr->pInfo->fTheta ) m_pElementPtr->pInfo->fPhi = m_pElementPtr->pInfo->fTheta;
			else if( m_pElementPtr->pInfo->fPhi >= 180.f ) m_pElementPtr->pInfo->fPhi = 180.f;
			cUpdate = 3;
		}
		else if( ( m_nMouseFlag & WB_DOWN ) && ( m_nMouseFlag & RB_DOWN ) ) {
			m_pElementPtr->pInfo->fPhi = EtToDegree( acos( m_pElementPtr->pInfo->fPhi ) );
			m_pElementPtr->pInfo->fTheta = EtToDegree( acos( m_pElementPtr->pInfo->fTheta ) );
			m_pElementPtr->pInfo->fTheta -= pReceiver->GetMouseVariation().y / 4.f;
			if( m_pElementPtr->pInfo->fTheta >= m_pElementPtr->pInfo->fPhi ) m_pElementPtr->pInfo->fTheta = m_pElementPtr->pInfo->fPhi;
			else if( m_pElementPtr->pInfo->fTheta <= 0.f ) m_pElementPtr->pInfo->fTheta = 0.f;
			cUpdate = 4;
		}
		else if( m_nMouseFlag & LB_DOWN ) {
			MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();
			m_matExWorld.m_vXAxis = pCamera->m_vXAxis;
			m_matExWorld.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
			EtVec3Cross( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vXAxis, &m_matExWorld.m_vYAxis );
			m_matExWorld.MoveLocalXAxis( pReceiver->GetMouseVariation().x );
			m_matExWorld.MoveLocalZAxis( -pReceiver->GetMouseVariation().y );
			m_pElementPtr->pInfo->Position = m_matExWorld.m_vPosition;
			cUpdate = 0;
		}
		else if( m_nMouseFlag & RB_DOWN ) {
			m_matExWorld.m_vZAxis = EtVector3( 0.f, 0.f, 1.f );
			m_matExWorld.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
			m_matExWorld.m_vXAxis = EtVector3( 1.f, 0.f, 0.f );

			m_matExWorld.MoveLocalYAxis( -pReceiver->GetMouseVariation().y );
			m_pElementPtr->pInfo->Position = m_matExWorld.m_vPosition;
			cUpdate = 0;
		}
		else if( m_nMouseFlag & WB_DOWN ) {

			m_matExWorld.RotateYAxis( pReceiver->GetMouseVariation().x / 2.f );
			m_matExWorld.RotatePitch( pReceiver->GetMouseVariation().y / 2.f );
			m_pElementPtr->pInfo->Direction = m_matExWorld.m_vZAxis;
			cUpdate = 2;
		}

		if( cUpdate != -1 ) {
			if( cUpdate == 0 ) {
				CUnionValueProperty Variable( CUnionValueProperty::Vector3 );
				Variable.SetVariable(m_pElementPtr->pInfo->Position);

				CWnd *pWnd = GetPaneWnd( ENVI_PANE );
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 2, (LPARAM)&Variable );
			}
			else if( cUpdate == 1 ) {
				CUnionValueProperty Variable( CUnionValueProperty::Float );
				Variable.SetVariable(m_pElementPtr->pInfo->fRange);

				CWnd *pWnd = GetPaneWnd( ENVI_PANE );
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 4, (LPARAM)&Variable );
			}
			else if( cUpdate == 2 ) {
				CUnionValueProperty Variable( CUnionValueProperty::Vector3 );
				Variable.SetVariable(m_pElementPtr->pInfo->Direction);

				CWnd *pWnd = GetPaneWnd( ENVI_PANE );
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 3, (LPARAM)&Variable );
			}
			else if( cUpdate == 3 ) {
				CUnionValueProperty Variable( CUnionValueProperty::Float );
				Variable.SetVariable(m_pElementPtr->pInfo->fPhi);

				CWnd *pWnd = GetPaneWnd( ENVI_PANE );
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 6, (LPARAM)&Variable );

				m_pElementPtr->pInfo->fPhi = cos( EtToRadian( m_pElementPtr->pInfo->fPhi ) );
				m_pElementPtr->pInfo->fTheta = cos( EtToRadian( m_pElementPtr->pInfo->fTheta ) );
			}
			else if( cUpdate == 4 ) {
				CUnionValueProperty Variable( CUnionValueProperty::Float );
				Variable.SetVariable(m_pElementPtr->pInfo->fTheta);

				CWnd *pWnd = GetPaneWnd( ENVI_PANE );
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 5, (LPARAM)&Variable );

				m_pElementPtr->pInfo->fPhi = cos( EtToRadian( m_pElementPtr->pInfo->fPhi ) );
				m_pElementPtr->pInfo->fTheta = cos( EtToRadian( m_pElementPtr->pInfo->fTheta ) );
			}
			CRenderBase::GetInstance().InitializeEnviLight( m_pElement, false );
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_WHEEL ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
			m_nMouseFlag |= LB_DOWN;
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_RB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
			m_nMouseFlag |= RB_DOWN;
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_RB_UP ) {
		m_nMouseFlag &= ~RB_DOWN;
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_WB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) )
			m_nMouseFlag |= WB_DOWN;
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_WB_UP ) {
		m_nMouseFlag &= ~WB_DOWN;
	}

	CGlobalValue::GetInstance().RefreshRender();
	return false;
}

PropertyGridBaseDefine *CEnviLightSpot::GetPropertyDefine()
{
	return g_PropertyDefineSpot;
}

void CEnviLightSpot::OnCustomDraw()
{
	EtVector3 vVec[2], vVec2[2];
	MatrixEx Cross;
	float fTheta = EtToDegree( acos( m_pElementPtr->pInfo->fTheta ) );
	float fPhi = EtToDegree( acos( m_pElementPtr->pInfo->fPhi ) );

	// 외각
	DrawSpotFunc1( fPhi, m_pElementPtr->pInfo, false, 0xFF999900 );
	DrawSpotFunc1( -fPhi, m_pElementPtr->pInfo, false, 0xFF999900 );
	DrawSpotFunc1( fPhi, m_pElementPtr->pInfo, true, 0xFF999900 );
	DrawSpotFunc1( -fPhi, m_pElementPtr->pInfo, true, 0xFF999900 );

	// 내각
	DrawSpotFunc1( fTheta, m_pElementPtr->pInfo, false, 0xFFFFFF00 );
	DrawSpotFunc1( -fTheta, m_pElementPtr->pInfo, false, 0xFFFFFF00 );
	DrawSpotFunc1( fTheta, m_pElementPtr->pInfo, true, 0xFFFFFF00 );
	DrawSpotFunc1( -fTheta, m_pElementPtr->pInfo, true, 0xFFFFFF00 );

	// 외각 내각 원
	int nDist = 30;
	float fAngle = 360 / (float)(nDist-1.f);


	Cross.m_vPosition = m_pElementPtr->pInfo->Position + ( m_pElementPtr->pInfo->Direction * m_pElementPtr->pInfo->fRange );
	Cross.m_vZAxis = -m_pElementPtr->pInfo->Direction;
	Cross.MakeUpCartesianByZAxis();

	float fRangePhi = m_pElementPtr->pInfo->fRange * tan( EtToRadian( fPhi ) );
	float fRangeTheta = m_pElementPtr->pInfo->fRange * tan( EtToRadian( fTheta ) );

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

void CEnviLightSpot::DrawSpotFunc1( float fAngle, SLightInfo *pInfo, bool bYawPitch, DWORD dwColor )
{
	EtVector3 vVec[2];
	MatrixEx Cross;
	Cross.m_vZAxis = pInfo->Direction;
	Cross.MakeUpCartesianByZAxis();
	if( bYawPitch == false ) Cross.RotateYaw( fAngle );
	else Cross.RotatePitch( fAngle );

	float fRange = pInfo->fRange * tan( EtToRadian( abs(fAngle) ) );

	vVec[0] = pInfo->Position;
	vVec[1] = pInfo->Position + ( Cross.m_vZAxis * sqrtf( (pInfo->fRange*pInfo->fRange) + (fRange*fRange) ) );

	EternityEngine::DrawLine3D( vVec[0], vVec[1], dwColor );
}
