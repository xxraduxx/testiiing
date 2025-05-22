#include "StdAfx.h"
#include "EnviLightDirection.h"
#include "RenderBase.h"
#include "InputReceiver.h"
#include "GlobalValue.h"
#include "SundriesFuncEx.h"
#include "resource.h"
#include "MainFrm.h"
#include "PaneDefine.h"
#include "UserMessage.h"
#include "EnviLightPaneView.h"

PropertyGridBaseDefine g_PropertyDefineDirection[] = {
	{ "Setting", "Diffuse", CUnionValueProperty::Vector4Color, "Diffuse Color", TRUE },
	{ "Setting", "Specular", CUnionValueProperty::Vector4Color, "Specular Color", TRUE },
	{ "Setting", "Direction", CUnionValueProperty::Vector3, "Direction!", TRUE },
	{ "Setting", "Cast Shadow", CUnionValueProperty::Boolean, "Shadow", TRUE },
	{ "Effect", "Enable Lens Flare", CUnionValueProperty::Boolean, "Enable LensFlare Effect", TRUE },
	{ "Effect", "Sun Texture", CUnionValueProperty::String_FileOpen, "Sun Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Effect", "LensFlare Texture", CUnionValueProperty::String_FileOpen, "LensFlare Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Effect", "Sun Radius", CUnionValueProperty::Float, "Sun Radius|0.f|.5f|0.0001f", TRUE },
	{ "Effect", "LensFlare Radius", CUnionValueProperty::Float, "LensFlare Radius|0.f|2.0f|0.0001f", TRUE },
	{ "Effect", "X Axis Revision Angle", CUnionValueProperty::Float, "Revision X Axis Angle|-180.f|180.0f|0.0001f", TRUE },
	{ "Effect", "Y Axis Revision Angle", CUnionValueProperty::Float, "Revision X Axis Angle|-180.f|180.0f|0.0001f", TRUE },
	NULL,
};

EtVector3 CEnviLightDirection::s_vDirection[18];
int CEnviLightDirection::s_nRefCount = 0;


CEnviLightDirection::CEnviLightDirection( CEnviElement *pElement )
: CEnviLightBase( pElement )
{
	m_nMouseFlag = 0;

	if( s_nRefCount == 0 ) {
		// Make Direction
		s_vDirection[0] = D3DXVECTOR3( 0, 0, -10 );
		s_vDirection[1] = D3DXVECTOR3( 0, 0, 10 );
		s_vDirection[2] = D3DXVECTOR3( 2, -2, -10 );
		s_vDirection[3] = D3DXVECTOR3( 2, 2, -10 );
		s_vDirection[4] = D3DXVECTOR3( -2, -2, -10 );
		s_vDirection[5] = D3DXVECTOR3( -2, 2, -10 );
		s_vDirection[6] = D3DXVECTOR3( 2, -2, -10 );
		s_vDirection[7] = D3DXVECTOR3( -2, -2, -10 );
		s_vDirection[8] = D3DXVECTOR3( 2, 2, -10 );
		s_vDirection[9] = D3DXVECTOR3( -2, 2, -10 );
		s_vDirection[10] = D3DXVECTOR3( 2, 2, -10 );
		s_vDirection[11] = D3DXVECTOR3( 0, 0, -15 );
		s_vDirection[12] = D3DXVECTOR3( 2, -2, -10 );
		s_vDirection[13] = D3DXVECTOR3( 0, 0, -15 );
		s_vDirection[14] = D3DXVECTOR3( -2, 2, -10 );
		s_vDirection[15] = D3DXVECTOR3( 0, 0, -15 );
		s_vDirection[16] = D3DXVECTOR3( -2, -2, -10 );
		s_vDirection[17] = D3DXVECTOR3( 0, 0, -15 );
		s_nRefCount++;
	}
}

CEnviLightDirection::~CEnviLightDirection()
{
	s_nRefCount--;
}

void CEnviLightDirection::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CWnd *pWnd = GetPaneWnd( ENVI_PANE );
	CEnviLightPaneView *pLightPane = ((CEnviPaneView*)pWnd)->GetLightView();
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (D3DXVECTOR4)m_pElementPtr->pInfo->Diffuse );
			break;
		case 1:
			pVariable->SetVariable( (D3DXVECTOR4)m_pElementPtr->pInfo->Specular );
			break;
		case 2:
			pVariable->SetVariable( m_pElementPtr->pInfo->Direction );
			break;
		case 3:
			pVariable->SetVariable( m_pElementPtr->pInfo->bCastShadow );
			break;
		case 4:
			pVariable->SetVariable( m_pElementPtr->bEnable );
			break;
		case 5:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( (CHAR*)m_pElementPtr->szSunTexture.c_str() );
			break;
		case 6:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( (CHAR*)m_pElementPtr->szLensFlareTexture.c_str() );
			break;
		case 7:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( m_pElementPtr->fSunScale );
			break;
		case 8:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( m_pElementPtr->fLensScale );
			break;
		case 9:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( m_pElementPtr->fRevisionXAxisAngle );
			break;
		case 10:
			pVariable->SetDefaultEnable( m_pElementPtr->bEnable );
			pVariable->SetVariable( m_pElementPtr->fRevisionYAxisAngle );
			break;
	}
}

void CEnviLightDirection::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CWnd *pWnd = GetPaneWnd( ENVI_PANE );
	CEnviLightPaneView *pLightPane = ((CEnviPaneView*)pWnd)->GetLightView();
	switch( dwIndex ) {
		case 0:
			m_pElementPtr->pInfo->Diffuse = (EtColor)pVariable->GetVariableVector4();
			break;
		case 1:
			m_pElementPtr->pInfo->Specular = (EtColor)pVariable->GetVariableVector4();
			break;
		case 2:
			m_pElementPtr->pInfo->Direction = (EtVector3)pVariable->GetVariableVector3();
			if( m_pElementPtr->bEnable ) {
				CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
			}
			break;
		case 3:
			m_pElementPtr->pInfo->bCastShadow = pVariable->GetVariableBool();
			break;
		case 4:
			m_pElementPtr->bEnable = pVariable->GetVariableBool();
			pLightPane->SetReadOnly( 5, !m_pElementPtr->bEnable );
			pLightPane->SetReadOnly( 6, !m_pElementPtr->bEnable );
			pLightPane->SetReadOnly( 7, !m_pElementPtr->bEnable );
			pLightPane->SetReadOnly( 8, !m_pElementPtr->bEnable );

			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, true );
			break;
		case 5:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				m_pElementPtr->szSunTexture = szTemp;
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetPaneWnd( ENVI_PANE );
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				pLightPane->UpdateItem();

				CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 6:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				m_pElementPtr->szLensFlareTexture = szTemp;
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetPaneWnd( ENVI_PANE );
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				pLightPane->UpdateItem();
				CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 7:
			m_pElementPtr->fSunScale = pVariable->GetVariableFloat();
			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 8:
			m_pElementPtr->fLensScale = pVariable->GetVariableFloat();
			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 9:
			m_pElementPtr->fRevisionXAxisAngle = pVariable->GetVariableFloat();
			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 10:
			m_pElementPtr->fRevisionYAxisAngle = pVariable->GetVariableFloat();
			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
			CGlobalValue::GetInstance().RefreshRender();
			break;
	}
}

bool CEnviLightDirection::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime, CInputReceiver *pReceiver )
{
	if( GetAKState( VK_LCONTROL ) >= 0 ) return true;

	if( nReceiverState & CInputReceiver::IR_MOUSE_MOVE ) {
		if( m_nMouseFlag & LB_DOWN ) {
			m_matExWorld.RotateYAxis( pReceiver->GetMouseVariation().x / 2.f );
			m_matExWorld.RotatePitch( pReceiver->GetMouseVariation().y / 2.f );

			m_pElementPtr->pInfo->Direction = m_matExWorld.m_vZAxis;
			static CUnionValueProperty Variable( CUnionValueProperty::Vector3 );
			Variable.SetVariable(m_pElementPtr->pInfo->Direction);

			CWnd *pWnd = GetPaneWnd( ENVI_PANE );
			if( pWnd ) pWnd->SendMessage( UM_PROPERTY_PANE_SET_MODIFY, 2, (LPARAM)&Variable );

			CRenderBase::GetInstance().InitializeEnviLight( m_pElement, false );
			CRenderBase::GetInstance().InitializeEnviLensFlare( m_pElement, false );
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_DOWN ) {
		if( IsInMouseRect( CGlobalValue::GetInstance().m_pParentView ) ) {
			m_nMouseFlag |= LB_DOWN;

			m_matExWorld.Identity();
			m_matExWorld.m_vZAxis = m_pElementPtr->pInfo->Direction;
			m_matExWorld.MakeUpCartesianByZAxis();
		}
	}
	if( nReceiverState & CInputReceiver::IR_MOUSE_LB_UP ) {
		m_nMouseFlag &= ~LB_DOWN;
	}
	CGlobalValue::GetInstance().RefreshRender();
	return false;
}

PropertyGridBaseDefine *CEnviLightDirection::GetPropertyDefine()
{
	return g_PropertyDefineDirection;
}

void CEnviLightDirection::OnCustomDraw()
{
	float fDistance = EtVec3Length( &CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition );
	float fScale = fDistance / 10.f;

	EtVector3 vDirection[18];
	MatrixEx Cross;
	Cross.m_vZAxis = -m_pElementPtr->pInfo->Direction;
	Cross.MakeUpCartesianByZAxis();

	for( int i=0; i<18; i++ ) {
		vDirection[i] = s_vDirection[i] * fScale;
		EtVec3TransformCoord( &vDirection[i], &vDirection[i], Cross );
		if( i % 2 == 1 ) {
			EternityEngine::DrawLine3D( vDirection[i-1], vDirection[i], 0xFFFFFF00 );
		}
	}
}