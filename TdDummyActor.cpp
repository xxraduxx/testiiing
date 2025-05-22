#include "StdAfx.h"
#include "TdDummyActor.h"
#include "resource.h"
#include "SundriesFuncEx.h"
#include "RenderBase.h"
#include "GlobalValue.h"
#include "TEtWorld.h"
#include "TimeImp.h"
#include "EtSoundEngine.h"

CTdDummyActor g_DummyActor;

float CalcMovement( float &fCurSpeed, float fElapsedTime, float fMaxSpeed, float fMinSpeed, float fAccel )
{
	float fFinalSpeed, fMovement, fDiffTime;

	fElapsedTime *= 1.f / 60.f * CGlobalValue::GetInstance().m_fFps;

	fFinalSpeed = fCurSpeed + fElapsedTime * fAccel;
	if( ( fFinalSpeed > fMaxSpeed ) && ( fCurSpeed <= fMaxSpeed ) )
	{
		fDiffTime =  ( fMaxSpeed - fCurSpeed ) / ( fFinalSpeed - fCurSpeed ) * fElapsedTime;
		fMovement = fCurSpeed * fDiffTime + fAccel * fDiffTime * fDiffTime * 0.5f;
		fMovement += fMaxSpeed * ( fElapsedTime - fDiffTime );
		fCurSpeed = fMaxSpeed;
	}
	else if( ( fFinalSpeed < fMinSpeed ) && ( fCurSpeed >= fMinSpeed ) )
	{
		fDiffTime =  ( fCurSpeed - fMinSpeed ) / ( fCurSpeed - fFinalSpeed ) * fElapsedTime;
		fMovement = fCurSpeed * fDiffTime + fAccel * fDiffTime * fDiffTime * 0.5f;
		fMovement += fMinSpeed * ( fElapsedTime - fDiffTime );
		fCurSpeed = fMinSpeed;
	}
	else
	{
		fMovement = fCurSpeed * fElapsedTime + fAccel * fElapsedTime * fElapsedTime * 0.5f;
		fCurSpeed = fFinalSpeed;
	}

	return fMovement * 100.f;
}


CTdDummyActor::CTdDummyActor()
: CInputReceiver( true )
{
	m_PrevTime = 0;
}

CTdDummyActor::~CTdDummyActor()
{
	Destroy();
}

bool CTdDummyActor::Initialize()
{
	m_vVelocity = m_vCurVelocity = EtVector3( 0.f, 0.f, 0.f );
	m_Camera.Identity();
	m_matExWorld.Identity();
	m_nAniIndex = 0;
	m_fCameraDistance = 600.f;
	SAFE_RELEASE_SPTR( m_Handle );

	CString szDrive;
	CString szSkin, szMesh, szAni;

	for( int i=1; i<27; i++ ) {
		if( !_chdrive(i) ) {
			szDrive.Format( "%c:", i + 'A' - 1 );

			szSkin.Format( "%s\\DummyActor.skn", szDrive.GetBuffer() );
			szMesh.Format( "%s\\DummyActor.msh", szDrive.GetBuffer() );
			szAni.Format( "%s\\DummyActor.ani", szDrive.GetBuffer() );
			MakeResourceToFile( "DDS", IDR_DUMMY_SKN, szSkin );
			MakeResourceToFile( "DDS", IDR_DUMMY_MSH, szMesh );
			MakeResourceToFile( "DDS", IDR_DUMMY_ANI, szAni );

			m_Handle = EternityEngine::CreateAniObject( szSkin, szAni );
			DeleteFile( szSkin );
			DeleteFile( szMesh );
			DeleteFile( szAni );
			if( !m_Handle ) return false;

			m_Handle->ShowObject( false );
			m_Handle->EnableShadowReceive( true );
			m_Handle->EnableShadowCast( true );
			m_Handle->SetCollisionGroup( COLLISION_GROUP_DYNAMIC(1) );
			m_Handle->EnableCollision( false );

			break;
		}
	}
	return true;
}
//ÁöÈ£ ÇÇ¶Ë²¿
void CTdDummyActor::Destroy()
{
	SAFE_RELEASE_SPTR( m_LightHandle );
	SAFE_RELEASE_SPTR( m_Handle );
}

void CTdDummyActor::Render( LOCAL_TIME LocalTime )
{
	if( m_PrevTime == 0 ) m_PrevTime = LocalTime;
	bool bMove = false;
	EtVector3 vNormal[2];
	EtVector3 vTargetPos;
	EtVector3 vPrevPos = m_matExWorld.m_vPosition;
	MatrixEx TempVec = m_Camera;
	TempVec.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
	TempVec.MakeUpCartesianByYAxis();
	m_matExWorld.m_vYAxis = TempVec.m_vYAxis;
	m_matExWorld.MakeUpCartesianByYAxis();
	m_matExWorld.m_vXAxis = m_Camera.m_vXAxis;
	m_matExWorld.MakeUpCartesianByXAxis();

	vTargetPos = m_matExWorld.m_vPosition;
	if( IsPushKey( DIK_W ) ) {
		vTargetPos += m_matExWorld.m_vZAxis * ( ( 300.f * GetPushKeyDelta(DIK_W,LocalTime) ) * 1.0f );
		ChangeAnimation(1);
		bMove = true;
	}
	if( IsPushKey( DIK_S ) ) {
		vTargetPos -= m_matExWorld.m_vZAxis * ( ( 300.f * GetPushKeyDelta(DIK_S,LocalTime) ) * 1.0f );
		ChangeAnimation(1);
		bMove = true;
	}
	if( IsPushKey( DIK_A ) ) {
		vTargetPos -= m_matExWorld.m_vXAxis * ( ( 300.f * GetPushKeyDelta(DIK_A,LocalTime) ) * 1.0f );
		ChangeAnimation(1);
		bMove = true;
	}
	if( IsPushKey( DIK_D ) ) {
		vTargetPos += m_matExWorld.m_vXAxis * ( ( 300.f * GetPushKeyDelta(DIK_D,LocalTime) ) * 1.0f );
		ChangeAnimation(1);
		bMove = true;
	}
	if( bMove == false ) ChangeAnimation(0);

	if( bMove == true ) {
		EtVector3 vDot;
		vDot.x = vTargetPos.x - m_matExWorld.m_vPosition.x;
		vDot.z = vTargetPos.z - m_matExWorld.m_vPosition.z;
		vDot.y = 0.f;
		EtVec3Normalize( &vDot, &vDot );

		vTargetPos.y = CTEtWorld::GetInstance().GetHeight( vTargetPos );
		EtVector3 vTemp = vTargetPos - m_matExWorld.m_vPosition;

		if( vTargetPos.y > m_matExWorld.m_vPosition.y ) {
			EtVector3 vTempNor;
			EtVec3Normalize( &vTempNor, &vTemp );
			float fDot = EtVec3Dot( &vDot, &vTempNor );
			fDot = powf( fDot, 3.f );
			if( fDot < 0.2f ) fDot = 0.f;
			vTemp *= fDot;
		}

		m_matExWorld.m_vPosition.x += vTemp.x;
		m_matExWorld.m_vPosition.z += vTemp.z;
	}

	float fHeight = CTEtWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );

	if( m_vVelocity.y != 0.f ) {
		float fDelta = ( LocalTime - m_PrevTime ) / 1000.f;
		m_matExWorld.m_vPosition.y += CalcMovement( m_vVelocity.y, fDelta, FLT_MAX, -FLT_MAX, -32.f );
	}
	if( m_matExWorld.m_vPosition.y <= fHeight ) {
		m_matExWorld.m_vPosition.y = fHeight;
		m_vVelocity.y = 0.f;
	}
	else {
		if( m_vVelocity.y == 0.f ) 
			m_matExWorld.m_vPosition.y = fHeight;
	}

	m_Handle->Update( m_matExWorld );
	UpdateCamera();

	// ³¯¾¾È¿°ú¶«½Ã ¼ÂÆÃÇØÁØ´Ù.
	EtVector3 vMovement = m_matExWorld.m_vPosition - vPrevPos;
	CRenderBase::GetInstance().SetDummyMovement( vMovement );

	if( m_nAniIndex != -1 ) {
		float fTemp = ( ( LocalTime - m_AniTime ) / 1000.f ) * 60.f;
		float fTemp2 = fTemp;
		float fAniLength = (float)m_Handle->GetAniLength( m_nAniIndex );

		float fFrame = fTemp2 - (float)( ((int)fTemp / (int)fAniLength) * fAniLength );
		m_Handle->SetAniFrame( m_nAniIndex, fFrame );
	}
	m_PrevTime = LocalTime;

	if( CRenderBase::GetInstance().GetSoundEngine() ) {
		CRenderBase::GetInstance().GetSoundEngine()->SetListener( m_matExWorld.m_vPosition, m_matExWorld.m_vZAxis, m_matExWorld.m_vYAxis );
	}
}

void CTdDummyActor::ChangeAnimation( int nAniIndex )
{
	if( m_nAniIndex == nAniIndex ) return;
	m_nAniIndex = nAniIndex;
	m_AniTime = CRenderBase::GetInstance().GetLocalTime();
}

void CTdDummyActor::UpdateCamera()
{
	// Camera Process
	float fYawDelta = 0.f;
	float fPitchDelta = 0.f;
	if( IsPushMouseButton(1) ) {
		if( GetMouseVariation().x != 0.f ) {
			fYawDelta = GetMouseVariation().x / 4.f;
		}
		if( GetMouseVariation().y != 0.f ) {
			fPitchDelta = GetMouseVariation().y / 4.f;
		}
	}
	if( GetMouseVariation().z != 0.f ) {
		m_fCameraDistance -= ( GetMouseVariation().z * 0.3f );
		if( m_fCameraDistance < 200.f ) m_fCameraDistance = 200.f;
		else if( m_fCameraDistance > 1500.f ) m_fCameraDistance = 1500.f;
	}

	if( fYawDelta != 0.f || fPitchDelta != 0.f ) {

		MatrixEx TempCross;
		TempCross = m_Camera;
		TempCross.RotateYaw(180);
		TempCross.m_vPosition = m_matExWorld.m_vPosition;

		TempCross.RotateYAxis( fYawDelta );
		TempCross.RotatePitch( -fPitchDelta );

		EtVector3 vTemp = m_Camera.m_vPosition - TempCross.m_vPosition;

		m_Camera = TempCross;
		m_Camera.RotateYaw(180);

	}
	m_Camera.m_vPosition = m_matExWorld.m_vPosition + ( m_Camera.m_vZAxis * -m_fCameraDistance );
	m_Camera.m_vPosition.y += 70.f;

	CRenderBase::GetInstance().UpdateCamera( m_Camera );
}

void CTdDummyActor::BeginActor()
{
	if( CGlobalValue::GetInstance().m_bSimulation == true ) return;
	CGlobalValue::GetInstance().m_bSimulation = true;
	m_Handle->ShowObject( true );
	m_Handle->EnableShadowReceive( true );
	m_Handle->EnableShadowCast( false );

	m_PrevTime = 0;
	m_vVelocity = m_vCurVelocity = EtVector3( 0.f, 0.f, 0.f );
	m_Camera.Identity();
	m_matExWorld.Identity();
	m_nAniIndex = 0;
	m_fCameraDistance = 600.f;
	CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
	CEtSoundChannelGroup *pGroup = pEngine->GetChannelGroup( "SoundManager" );

	m_vVelocity.y = 8.f;
//	pGroup->


	MatrixEx *pCamera = CRenderBase::GetInstance().GetLastUpdateCamera();

	m_matExWorld.m_vPosition = pCamera->m_vPosition;
	m_matExWorld.m_vZAxis = pCamera->m_vZAxis;
	m_matExWorld.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
	m_matExWorld.MakeUpCartesianByZAxis();

	m_Camera = *pCamera;

	UpdateCamera();
}

void CTdDummyActor::EndActor()
{
	if( CGlobalValue::GetInstance().m_bSimulation == false ) return;
	CGlobalValue::GetInstance().m_bSimulation = false;

	m_Handle->ShowObject( false );

//	MatrixEx *pCross = CRenderBase::GetInstance().GetLastUpdateCamera();
//	*pCross = m_matExWorld;
	CRenderBase::GetInstance().UpdateCamera( m_Camera );

}

void CTdDummyActor::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_KEY_DOWN ) {

		if( IsPushKey( DIK_SPACE ) ) {
			m_vVelocity.y = 8.f;
		}
	}
}