#include "StdAfx.h"
#include "EnviLightPoint.h"
#include "RenderBase.h"
#include "InputReceiver.h"
#include "GlobalValue.h"
#include "SundriesFuncEx.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "UserMessage.h"

PropertyGridBaseDefine g_PropertyDefinePoint[] = {
	{ "Setting", "Diffuse", CUnionValueProperty::Vector4Color, "Diffuse Color", TRUE },
	{ "Setting", "Specular", CUnionValueProperty::Vector4Color, "Specular Color", TRUE },
	{ "Setting", "Position", CUnionValueProperty::Vector3, "Position", TRUE },
	{ "Setting", "Range", CUnionValueProperty::Float, "Range", TRUE },
	NULL,
};

EtVector3 CEnviLightPoint::s_vPoint[30];
int CEnviLightPoint::s_nRefCount = 0;


CEnviLightPoint::CEnviLightPoint( CEnviElement *pElement )
: CEnviLightBase( pElement )
{
	m_nMouseFlag = 0;

	if( s_nRefCount == 0 ) {
		// Make Point
		s_vPoint[0] = D3DXVECTOR3( -10, 0, 0 );
		s_vPoint[1] = D3DXVECTOR3( 10, 0, 0 );
		s_vPoint[2] = D3DXVECTOR3( 0, -10, 0 );
		s_vPoint[3] = D3DXVECTOR3( 0, 10, 0 );
		s_vPoint[4] = D3DXVECTOR3( 0, 0, -10 );
		s_vPoint[5] = D3DXVECTOR3( 0, 0, 10 );
		s_vPoint[6] = D3DXVECTOR3( 0, -10, 0 );
		s_vPoint[7] = D3DXVECTOR3( 0, 0, -10 );
		s_vPoint[8] = D3DXVECTOR3( 0, 10, 0 );
		s_vPoint[9] = D3DXVECTOR3( 0, 0, 10 );
		s_vPoint[10] = D3DXVECTOR3( 0, -10, 0 );
		s_vPoint[11] = D3DXVECTOR3( 0, 0, 10 );
		s_vPoint[12] = D3DXVECTOR3( 0, 10, 0 );
		s_vPoint[13] = D3DXVECTOR3( 0, 0, -10 );
		s_vPoint[14] = D3DXVECTOR3( -10, 0, 0 );
		s_vPoint[15] = D3DXVECTOR3( 0, 10, 0 );
		s_vPoint[16] = D3DXVECTOR3( 10, 0, 0 );
		s_vPoint[17] = D3DXVECTOR3( 0, 10, 0 );
		s_vPoint[18] = D3DXVECTOR3( -10, 0, 0 );
		s_vPoint[19] = D3DXVECTOR3( 0, -10, 0 );
		s_vPoint[20] = D3DXVECTOR3( 10, 0, 0 );
		s_vPoint[21] = D3DXVECTOR3( 0, -10, 0 );
		s_vPoint[22] = D3DXVECTOR3( 10, 0, 0 );
		s_vPoint[23] = D3DXVECTOR3( 0, 0, 10 );
		s_vPoint[24] = D3DXVECTOR3( 0, 0, 10 );
		s_vPoint[25] = D3DXVECTOR3( -10, 0, 0 );
		s_vPoint[26] = D3DXVECTOR3( -10, 0, 0 );
		s_vPoint[27] = D3DXVECTOR3( 0, 0, -10 );
		s_vPoint[28] = D3DXVECTOR3( 0, 0, -10 );
		s_vPoint[29] = D3DXVECTOR3( 10, 0, 0 );
		s_nRefCount++;
	}
}

CEnviLightPoint::~CEnviLightPoint()
{
	s_nRefCount--;
}

void CEnviLightPoint::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
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
			pVariable->SetVariable( m_pElementPtr->pInfo->fRange );
			break;

	}
}

void CEnviLightPoint::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
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
			m_pElementPtr->pInfo->fRange = pVariable->GetVariableFloat();
			break;
	}
}

bool CEnviLightPoint::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver )
{
	if( GetAKState( VK_LCONTROL ) >= 0 ) return true;

	if( nReceiverState & CInputReceiver::IR_MOUSE_MOVE ) {
		char cUpdate = -1;
		if( m_nMouseFlag & LB_DOWN ) {
			MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();
			m_matExWorld.m_vXAxis = pCamera->m_vXAxis;
			m_matExWorld.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
			EtVec3Cross( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vXAxis, &m_matExWorld.m_vYAxis );
			m_matExWorld.MoveLocalXAxis( pReceiver->GetMouseVariation().x );
			m_matExWorld.MoveLocalZAxis( -pReceiver->GetMouseVariation().y );
			m_pElementPtr->pInfo->Position = m_matExWorld.m_vPosition;
			cUpdate = 0;
		}
		if( m_nMouseFlag & RB_DOWN ) {
			m_matExWorld.MoveLocalYAxis( -pReceiver->GetMouseVariation().y );
			m_pElementPtr->pInfo->Position = m_matExWorld.m_vPosition;
			cUpdate = 0;
		}
		if( m_nMouseFlag & WB_DOWN ) {
			m_pElementPtr->pInfo->fRange -= pReceiver->GetMouseVariation().y;
			if( m_pElementPtr->pInfo->fRange <= 0.f ) m_pElementPtr->pInfo->fRange = 0.f;
			cUpdate = 1;
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
				if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 3, (LPARAM)&Variable );
			}
			CRenderBase::GetInstance().InitializeEnviLight( m_pElement, false );
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
			m_nMouseFlag |= LB_DOWN;

			m_matExWorld.Identity();
			m_matExWorld.m_vPosition = m_pElementPtr->pInfo->Position;
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_RB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
			m_nMouseFlag |= RB_DOWN;

			m_matExWorld.Identity();
			m_matExWorld.m_vPosition = m_pElementPtr->pInfo->Position;
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

PropertyGridBaseDefine *CEnviLightPoint::GetPropertyDefine()
{
	return g_PropertyDefinePoint;
}

void CEnviLightPoint::OnCustomDraw()
{
	float fScale = 100.f;
	EtVector3 vPoint[30];
	for( int i=0; i<30; i++ ) {
		vPoint[i] = s_vPoint[i] * ( fScale * 0.2f );
		vPoint[i] += m_pElementPtr->pInfo->Position;
		if( i % 2 == 1 ) {
			EternityEngine::DrawLine3D( vPoint[i-1], vPoint[i], 0xFFFFFF00 );
		}
	}

	EtVector3 vVec[2];
	MatrixEx Cross;
	int nDist = 40;
	float fAngle = 360 / (float)(nDist-1.f);

	Cross.m_vPosition = m_pElementPtr->pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * m_pElementPtr->pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateYaw( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * m_pElementPtr->pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = m_pElementPtr->pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vZAxis * m_pElementPtr->pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotatePitch( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vZAxis * m_pElementPtr->pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}

	Cross.Identity();
	Cross.m_vPosition = m_pElementPtr->pInfo->Position;

	vVec[0] = Cross.m_vPosition + ( Cross.m_vXAxis * m_pElementPtr->pInfo->fRange );
	for( int i=0; i<nDist; i++ ) {
		Cross.RotateRoll( fAngle );
		vVec[1] = Cross.m_vPosition + ( Cross.m_vXAxis * m_pElementPtr->pInfo->fRange );
		EternityEngine::DrawLine3D( vVec[0], vVec[1], 0xFFFFFF00 );
		vVec[0] = vVec[1];
	}
}