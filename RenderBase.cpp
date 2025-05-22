#include "StdAfx.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "TEtWorld.h"
#include "InputDevice.h"
#include "GlobalValue.h"
#include "MainFrm.h"
#include "EtWorldPainter.h"
#include "EtWorldPainterDoc.h"
#include "EtWorldPainterView.h"
#include "DummyView.h"
#include "UserMessage.h"
#include "TdDummyActor.h"
#include "EnviControl.h"
#include "EtSoundEngine.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "TEtWorldSound.h"
#include "EtResourceMng.h"
#include "EtWater.h"
#include "EventCustomRender.h"
#include "GlobalValue.h"
#include "EtActionCoreMng.h"
#include <Process.h>

CRenderBase g_RenderBase;
bool CRenderBase::s_bRenderMainScene = false;
bool CRenderBase::s_bThreadRenderTarget = false;

CRenderBase::CRenderBase()
{
	m_bActivate = false;
	m_pEtSound = NULL;
	m_pColorAdjFilter = NULL;
	m_pBloomFilter = NULL;
	m_pHeatHazeFilter = NULL;
	m_pDOFFilter = NULL;
	m_PrevLocalTime = 0;
	m_fDelta = 0.f;

	char szStr[2048] = { 0, };
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ShaderFolder", szStr, 2048 );
	m_szShaderFolder = szStr;

	/*
	memset( szStr, 0, sizeof(szStr) );
	GetRegistryString( HKEY_CURRENT_USER, REG_SUBKEY, "ResourceFolder", szStr, 2048 );
	m_szResourceFolder = szStr;
	*/

	m_bEnableColorFilter = true;
	m_bEnableBloomFilter = true;
	m_bEnableWeather = true;
	m_bEnableSplashDrop = true;
	m_bEnableHeatHazeFilter = true;
	m_bEnableDOFFilter = true;
	m_bEnableSnow = true;

	m_nEnviBGMIndex = -1;

	m_bToggleCameraTopView = false;
}

CRenderBase::~CRenderBase()
{
	Finalize();
}

void CRenderBase::Finalize()
{
	if( m_bActivate == false ) return;

	CEventCustomRender::UnregisterClass();
	SAFE_RELEASE_SPTRVEC( m_hVecLight );
	SAFE_RELEASE_SPTRVEC( m_hVecLensFlare );

	SAFE_RELEASE_SPTR( m_CameraHandle );
	m_FrameSync.End();
	CTdDummyActor::GetInstance().Destroy();
	SAFE_DELETE( m_pColorAdjFilter );
	SAFE_DELETE( m_pBloomFilter );
	SAFE_DELETE( m_pHeatHazeFilter );
	SAFE_DELETE( m_pDOFFilter );

	if( m_nEnviBGMIndex != -1 ) {
		CEtSoundEngine::GetInstance().RemoveSound( m_nEnviBGMIndex );
		m_nEnviBGMIndex = -1;
	}
	SAFE_RELEASE_SPTR( m_hEnviBGMChannel );
	SAFE_RELEASE_SPTR( m_hEnvironmentTexture );
	SAFE_RELEASE_SPTR( m_hWeather );
	SAFE_RELEASE_SPTR( m_hSplashDrop );
	SAFE_RELEASE_SPTR( m_hSnow );

	CEtActionCoreMng::GetInstance().FlushWaitDelete();
	SAFE_DELETE( m_pEtSound );
	EternityEngine::FinalizeEngine();
	m_bActivate = false;
}

void CRenderBase::Initialize( HWND hWnd )
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Engine.." );

	CRect rcRect;
	m_hWnd = hWnd;

	CalcRenderRect( rcRect );

	SGraphicOption Option;
	Option.DynamicShadowType = ST_DEPTHSHADOWMAP;
	Option.bEnableDOF = true;

	EternityEngine::InitializeEngine( m_hWnd, rcRect.Width(), rcRect.Height(), true, true, &Option );
	GetEtStateManager()->SetEnable( false );
	CEtRenderStack::EnableDepthRender( true );
//	CEtRenderStack::EnableSkyFog( false );
	CEtRenderStack::EnableOcclusion( false );
	

	char szStr[1024] = { 0, };
	GetCurrentDirectory( 1024, szStr );
	strcat_s( szStr, "\\" );

	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Shader.." );
	CEtResourceMng::GetInstance().AddResourcePath( m_szShaderFolder, true );
	std::vector<std::string> szVecFileList;
	FindFileListInDirectory( m_szShaderFolder.GetBuffer(), "*.fx", szVecFileList, false, true, false );
	for( DWORD i=0; i<szVecFileList.size(); i++ ) {
		sprintf_s( szStr, "%s\\%s", m_szShaderFolder.GetBuffer(), szVecFileList[i].c_str() );
		CEtResourceMng::GetInstance().AddCacheMemoryStream( szStr );
	}

	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Environment.." );
	InitializeEnvi( CEnviControl::GetInstance().GetActiveElement() );
	InitializeEnviLight( CEnviControl::GetInstance().GetActiveElement() );
	InitializeEnviLensFlare( CEnviControl::GetInstance().GetActiveElement() );

	m_FrameSync.Begin( -1 );
	m_bActivate = true;

	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Sound.." );
	m_pEtSound = new CEtSoundEngine;
	if( m_pEtSound->Initialize( CEtSoundEngine::AutoDetected ) == false ) {
//		MessageBox( m_hWnd, "m_pEtSound->Initialize() Failed", "Error", MB_OK );
	}
	m_pEtSound->CreateChannelGroup( "SoundManager" );
	m_pEtSound->CreateChannelGroup( "3D" );
	m_pEtSound->CreateChannelGroup( "BGM" );


	CGlobalValue::GetInstance().UpdateInitDesc( "Initialize Dummy.." );
	CEventCustomRender::RegisterClass();
	CTdDummyActor::GetInstance().Initialize();
	CGlobalValue::GetInstance().UpdateInitDesc( "Finish.." );
}

void CRenderBase::InitializeEnvi( CEnviElement *pElement )
{
	SAFE_RELEASE_SPTR( m_CameraHandle );

	CEnvInfo& envInfo = pElement->GetInfo();

	SCameraInfo CamInfo;
	EtColor Color;
	CamInfo.fFogFar = envInfo.GetFogFar();
	CamInfo.fFogNear = envInfo.GetFogNear();
	Color = envInfo.GetFogColor();
	CamInfo.FogColor = EtColor( Color.b, Color.g, Color.r, 1.f );
	CamInfo.fNear = CGlobalValue::GetInstance().m_fCamNear;
	CamInfo.fFar = CGlobalValue::GetInstance().m_fCamFar;
	CamInfo.fFOV = CGlobalValue::GetInstance().m_fCamFOV;

	m_CameraHandle = EternityEngine::CreateCamera( &CamInfo );

	Color = envInfo.GetAmbient();
	
	SetGlobalAmbient( &EtColor( Color.b, Color.g, Color.r, 1.f ) );
	CString szBoxName = envInfo.GetSkyBox();

	if( !szBoxName.IsEmpty() ) {
		EtObjectHandle hTemp = m_hSkyBox;
		m_hSkyBox = CreateStaticObject( szBoxName );
		if( m_hSkyBox ) {
			m_hSkyBox->EnableCull( false );
			m_hSkyBox->EnableWaterCast( true );
		}
		SAFE_RELEASE_SPTR( hTemp );
	}
	else SAFE_RELEASE_SPTR( m_hSkyBox );

	UpdateCamera( m_matExWorld );


	m_bEnableColorFilter = envInfo.IsEnableColorFilter();
	m_bEnableBloomFilter = envInfo.IsEnableBloomFilter();
	m_bEnableHeatHazeFilter = envInfo.IsEnableHeatHaze();
	m_bEnableWeather = envInfo.IsEnableWeather();
	m_bEnableSplashDrop = envInfo.IsEnableSplashDrop();
	m_bEnableDOFFilter = envInfo.IsEnableDOF();
	m_bEnableSnow = envInfo.IsEnableSnow();

	if( m_bEnableColorFilter == true ) {
		if( !m_pColorAdjFilter ) m_pColorAdjFilter = (CEtColorAdjustTexFilter*)CreateFilter( SF_COLORADJUSTTEX );
		m_pColorAdjFilter->SetParam( *envInfo.GetColorAdjShadow(), *envInfo.GetColorAdjMidtones(), *envInfo.GetColorAdjHighlights(), envInfo.GetColorAdjSaturation() );
	}
	else {
		SAFE_DELETE( m_pColorAdjFilter );
	}
	if( m_bEnableBloomFilter == true ) {
		if( !m_pBloomFilter ) m_pBloomFilter = (CEtBloomFilter*)CreateFilter( SF_BLOOM );
		m_pBloomFilter->SetBloomScale( envInfo.GetBloomScale() );
		m_pBloomFilter->SetBloomSensitivity( envInfo.GetBloomSensitivity() );
	}
	else {
		SAFE_DELETE( m_pBloomFilter );
	}

	if( envInfo.GetEnvironmentTexture() && strlen(envInfo.GetEnvironmentTexture()) > 0 ) {
		EtTextureHandle hTemp = m_hEnvironmentTexture;
		m_hEnvironmentTexture = EternityEngine::LoadEnvTexture( envInfo.GetEnvironmentTexture() );
		SAFE_RELEASE_SPTR( hTemp );
	}
	else {
		SAFE_RELEASE_SPTR( m_hEnvironmentTexture );
	}

	if( m_bEnableWeather ) {
		EtRainHandle hTemp;
		if( m_hWeather ) hTemp = m_hWeather;
		m_hWeather = CEtRainEffect::CreateRainEffect( envInfo.GetWeatherTexture() );
		SAFE_RELEASE_SPTR( hTemp );

		if( m_hWeather ) {
			m_hWeather->SetRainSpeed( envInfo.GetWeatherSpeed() );
			m_hWeather->SetStretchValue( envInfo.GetWeatherStretch() );
			m_hWeather->SetLayerCount( envInfo.GetWeatherLayerCount() );
			m_hWeather->SetRainSpeedDecreaseRate( envInfo.GetWeatherLayerDecreaseRate() );
			m_hWeather->SetRainDir( *envInfo.GetWeatherLayerDir() );
		}
	}
	else {
		SAFE_RELEASE_SPTR( m_hWeather );
	}

	if( m_bEnableSplashDrop ) {
		EtRainDropSplashHandle hTemp;
		if( m_hSplashDrop ) hTemp = m_hSplashDrop;
		m_hSplashDrop = CEtRainDropSplash::CreateRainDropSplash( envInfo.GetSplashDropTexture(), envInfo.GetSplashDropSize() );
		SAFE_RELEASE_SPTR( hTemp );

		if( m_hSplashDrop ) {
			m_hSplashDrop->SetLifeTime( envInfo.GetSplashDropLifeTime() );
			m_hSplashDrop->SetCreationRadius( (float)envInfo.GetSplashDropRadius() );
			m_hSplashDrop->SetSplashPerSecond( envInfo.GetSplashDropCount() );
		}
	}
	else {
		SAFE_RELEASE_SPTR( m_hSplashDrop );
	}

	if( m_bEnableHeatHazeFilter ) {
		if( !m_pHeatHazeFilter ) m_pHeatHazeFilter = (CEtHeatHazeFilter*)CreateFilter( SF_HEATHAZE );
		if( m_pHeatHazeFilter ) {
			m_pHeatHazeFilter->SetDistortionPower( envInfo.GetHeatHazeDistortionPower() );
			m_pHeatHazeFilter->SetWaveSpeed( envInfo.GetHeatHazeWaveSpeed() );
		}
	}
	else {
		SAFE_DELETE( m_pHeatHazeFilter );
	}

	if( m_bEnableDOFFilter ) {
		if( !m_pDOFFilter ) m_pDOFFilter = (CEtDOFFilter*)CreateFilter( SF_DOF );
		if( m_pDOFFilter ) {
			m_pDOFFilter->SetFocusDistance( envInfo.GetDOFFocusDistance() );
			m_pDOFFilter->SetNearDOFStart( envInfo.GetDOFNearStart() );
			m_pDOFFilter->SetNearDOFEnd( envInfo.GetDOFNearEnd() );
			m_pDOFFilter->SetFarDOFStart( envInfo.GetDOFFarStart() );
			m_pDOFFilter->SetFarDOFEnd( envInfo.GetDOFFarEnd() );
			m_pDOFFilter->SetNearBlurSize( envInfo.GetDOFNearBlur() );
			m_pDOFFilter->SetFarBlurSize( envInfo.GetDOFFarBlur() );
		}
	}
	else {
		SAFE_DELETE( m_pDOFFilter );
	}
	if( m_bEnableSnow ) {
		EtSnowHandle hTemp;
		if( m_hSnow ) hTemp = m_hSnow;
		m_hSnow = CEtSnowEffect::CreateSnowEffect( envInfo.GetSnowTexture(), envInfo.GetSnowCount(), envInfo.GetSnowSize() );
		SAFE_RELEASE_SPTR( hTemp );

		if( m_hSnow ) {
			m_hSnow->SetSnowTimeOut( envInfo.GetSnowTimeOut() );
			m_hSnow->SetSnowHeight( envInfo.GetSnowHeight() );
			m_hSnow->SetIsUp(envInfo.GetIsUp());
			m_hSnow->SetSnowAniSpeed(envInfo.GetSnowAniSpeed());
		}
	}
	else {
		SAFE_RELEASE_SPTR( m_hSnow );
	}
}

void CRenderBase::InitializeEnviLight( CEnviElement *pElement, bool bRefreshAll )
{
	EtLightHandle hLight;
	CEnvInfo& envInfo = pElement->GetInfo();

	if( bRefreshAll == false && envInfo.GetLightCount() != m_hVecLight.size() ) bRefreshAll = true;

	if( bRefreshAll ) {
		SAFE_RELEASE_SPTRVEC( m_hVecLight );

		for( DWORD i=0; i<envInfo.GetLightCount(); i++ ) {
			SLightInfo Info = *envInfo.GetLight(i)->pInfo;

			Info.Diffuse.r *= Info.Diffuse.a;
			Info.Diffuse.g *= Info.Diffuse.a;
			Info.Diffuse.b *= Info.Diffuse.a;

			Info.Specular.r *= Info.Specular.a;
			Info.Specular.g *= Info.Specular.a;
			Info.Specular.b *= Info.Specular.a;

			hLight = EternityEngine::CreateLight( &Info );
			m_hVecLight.push_back( hLight );
		}
	}
	else {
		for( DWORD i=0; i<envInfo.GetLightCount(); i++ ) {
			SLightInfo Info = *envInfo.GetLight(i)->pInfo;

			Info.Diffuse.r *= Info.Diffuse.a;
			Info.Diffuse.g *= Info.Diffuse.a;
			Info.Diffuse.b *= Info.Diffuse.a;

			Info.Specular.r *= Info.Specular.a;
			Info.Specular.g *= Info.Specular.a;
			Info.Specular.b *= Info.Specular.a;

			*m_hVecLight[i]->GetLightInfo() = Info;
		}
	}
}

void CRenderBase::InitializeEnviLensFlare( CEnviElement *pElement, bool bRefreshAll )
{
	CEnvInfo& envInfo = pElement->GetInfo();

	if( bRefreshAll == false && envInfo.GetLightCount() != m_hVecLight.size() ) bRefreshAll = true;

	if( bRefreshAll ) {
		SAFE_RELEASE_SPTRVEC( m_hVecLensFlare );
		MatrixEx CrossTemp;
		for( DWORD i=0; i<envInfo.GetLightCount(); i++ ) {
			EtLensFlareHandle hLensFlare;
			CEnvInfo::LightInfoStruct *pStruct = envInfo.GetLight(i);
			if( pStruct->bEnable == true ) {
				hLensFlare = CEtLensFlare::CreateLensFlare( pStruct->pInfo->Direction, pStruct->szSunTexture.c_str(), pStruct->szLensFlareTexture.c_str() );
				hLensFlare->SetFlareRadius( pStruct->fSunScale, pStruct->fLensScale );

				CrossTemp.Identity();
				CrossTemp.m_vZAxis = pStruct->pInfo->Direction;
				CrossTemp.MakeUpCartesianByZAxis();

				CrossTemp.RotateYAxis( pStruct->fRevisionYAxisAngle );
				CrossTemp.RotatePitch( pStruct->fRevisionXAxisAngle );
				hLensFlare->SetDirection( CrossTemp.m_vZAxis );
			}
			m_hVecLensFlare.push_back( hLensFlare );
		}
	}
	else {
		MatrixEx CrossTemp;
		for( DWORD i=0; i<envInfo.GetLightCount(); i++ ) {
			CEnvInfo::LightInfoStruct *pStruct = envInfo.GetLight(i);
			if( pStruct->bEnable ) {
				CrossTemp.Identity(); 
				CrossTemp.m_vZAxis = pStruct->pInfo->Direction;
				CrossTemp.MakeUpCartesianByZAxis();

				CrossTemp.RotateYAxis( pStruct->fRevisionYAxisAngle );
				CrossTemp.RotatePitch( pStruct->fRevisionXAxisAngle );

				if( pStruct->fSunScale != m_hVecLensFlare[i]->GetSunRadius() ||
					pStruct->fLensScale != m_hVecLensFlare[i]->GetFlareRadius() ||
					pStruct->pInfo->Direction != CrossTemp.m_vZAxis )
					m_hVecLensFlare[i]->SetFlareRadius( pStruct->fSunScale, pStruct->fLensScale );
				m_hVecLensFlare[i]->SetSunTexture( pStruct->szSunTexture.c_str() );
				m_hVecLensFlare[i]->SetFlareTexture( pStruct->szLensFlareTexture.c_str() );
				m_hVecLensFlare[i]->SetDirection( CrossTemp.m_vZAxis );
			}

		}
	}
}

void CRenderBase::InitializeEnviBGM( CEnviElement *pElement, bool bRefreshAll )
{
	CEnvInfo& envInfo = pElement->GetInfo();

	if( bRefreshAll ) {
		if( m_nEnviBGMIndex != -1 ) {
			CEtSoundEngine::GetInstance().RemoveSound( m_nEnviBGMIndex );
			m_nEnviBGMIndex = -1;
		}
		SAFE_RELEASE_SPTR( m_hEnviBGMChannel );
	}

	// Envi BGM Setting
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( pSector ) {
		if( pSector->GetSoundInfo() ) {
			pSector->GetSoundInfo()->SetVolume( pSector->GetSoundInfo()->GetVolume() );
			if( envInfo.GetEnviBGM() && strlen(envInfo.GetEnviBGM()) > 0 ) {
				if( bRefreshAll ) m_nEnviBGMIndex = CEtSoundEngine::GetInstance().LoadSound( envInfo.GetEnviBGM(), false, true );
				if( m_nEnviBGMIndex != -1 ) {
					if( bRefreshAll ) m_hEnviBGMChannel = CEtSoundEngine::GetInstance().PlaySound( "BGM", m_nEnviBGMIndex, true, false );

					if( m_hEnviBGMChannel ) {
						float fBGMVolume = pSector->GetSoundInfo()->GetVolume();

						m_hEnviBGMChannel->SetVolume( fBGMVolume * envInfo.GetEnviBGMRatio() );
						pSector->GetSoundInfo()->SetVolume( fBGMVolume * ( 1.f - envInfo.GetEnviBGMRatio() ) );
					}
				}
			}
		}
	}
}

void CRenderBase::Render( CGlobalValue::VIEW_TYPE Type )
{
	if( m_bActivate == false ) return;
	if( CFileServer::GetInstance().IsConnect() == false ) return;

	if( m_FrameSync.CheckSync() == true ) {
		LOCAL_TIME LocalTime = m_FrameSync.GetMSTime();
		if( CGlobalValue::GetInstance().m_bSimulation == true ) {
			CTdDummyActor::GetInstance().Render( LocalTime );
		}
		if( CGlobalValue::GetInstance().m_pParentView ) {
			((CEtWorldPainterView*)CGlobalValue::GetInstance().m_pParentView)->GetDummyView()->SendMessage( UM_CUSTOM_RENDER );
		}

		switch( Type ) {
			case CGlobalValue::GRID:
				CTEtWorld::GetInstance().RenderGrid();
				break;
			case CGlobalValue::BRUSH:
			case CGlobalValue::PROP:
			case CGlobalValue::WATER:
			case CGlobalValue::NAVIGATION:
			case CGlobalValue::EVENT:
			case CGlobalValue::SOUND:
			case CGlobalValue::TRIGGER:
				CTEtWorld::GetInstance().RenderHeightmap( CGlobalValue::GetInstance().m_SelectGrid );
				break;
		}

		CPoint p;
		GetCursorPos( &p );
		ScreenToClient( CGlobalValue::GetInstance().m_pParentView->m_hWnd, &p );

		EtVector3 Direction, Origin, vPickPos;
		int nCameraIndex = 0;
		if( m_CameraHandle )
		{
			nCameraIndex = m_CameraHandle->GetMyItemIndex();
		}
		CEtCamera::GetItem( nCameraIndex )->CalcPositionAndDir( p.x, p.y, Origin, Direction );
		bool bPick = CTEtWorld::GetInstance().Pick( Origin, Direction, vPickPos );

		CString szFrame;
		if( bPick == true ) 
			szFrame.Format( "FPS : %.2f ( %.2f, %.2f, %.2f )", m_FrameSync.GetFps(), vPickPos.x, vPickPos.y, vPickPos.z );
		else szFrame.Format( "FPS : %.2f ( N/A )", m_FrameSync.GetFps() );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.f ), szFrame );

		if( !CGlobalValue::GetInstance().IsCheckOutMe() ) {
			CString szFrame;
			szFrame.Format( "Can't CheckOut! View Only", m_FrameSync.GetFps() );
			EternityEngine::DrawText2D( EtVector2( 0.f, 0.03f ), szFrame, 0xffff0000 );
		}

		if( m_hSkyBox ) {
			EtMatrix mat;
			EtMatrixIdentity( &mat );
			memcpy( &mat._41, &m_matExWorld.m_vPosition.x, sizeof(EtVector3) );
			m_hSkyBox->Update( &mat );
		}

		CRect rcRect;
		CalcRenderRect( rcRect );

		if( s_bThreadRenderTarget == false ) {
//			Lock();
			s_bRenderMainScene = true;

			EternityEngine::ClearScreen( D3DCOLOR_ARGB( 0, 40, 40, 40 ) );
			m_fDelta = ( LocalTime - m_PrevLocalTime ) * 0.001f;
			EternityEngine::RenderFrame( m_fDelta );
			EternityEngine::ShowFrame( &rcRect );


			s_bRenderMainScene = false;
//			Unlock();
		}
		m_PrevLocalTime = LocalTime;
	}
}

void CRenderBase::Process( CGlobalValue::VIEW_TYPE Type )
{
	if( m_bActivate == false ) return;
	if( CFileServer::GetInstance().IsConnect() == false ) return;

	if( m_pEtSound ) {
		if( !CGlobalValue::GetInstance().m_bSimulation )
			m_pEtSound->SetListener( m_matExWorld.m_vPosition, m_matExWorld.m_vZAxis, m_matExWorld.m_vYAxis );

		m_pEtSound->Process( 0.f );
	}
	m_FrameSync.UpdateTime();

	CGlobalValue::GetInstance().ProcessAsyncKey();
	LOCAL_TIME LocalTime = m_FrameSync.GetMSTime();
	CInputDevice::GetInstance().Process( LocalTime, m_fDelta );
}


void CRenderBase::Reconstruction()
{
	if( m_bActivate == false ) return;

	CRect rcRect;
	CalcRenderRect( rcRect );

	EternityEngine::ReinitializeEngine( rcRect.Width(), rcRect.Height() );
	CGlobalValue::GetInstance().RefreshRender();
}

void CRenderBase::CalcRenderRect( CRect &rcRect )
{
	GetClientRect( m_hWnd, &rcRect );
	if( CGlobalValue::GetInstance().m_pParentView ) {
		CRect rcTabRect;
		CDummyView *pDummyView = ((CEtWorldPainterView*)CGlobalValue::GetInstance().m_pParentView)->GetDummyView();
		CDummyView::TabViewStruct *pStruct = pDummyView->GetTabViewStruct( CGlobalValue::GetInstance().m_nActiveView );
		if( pStruct->pView == NULL ) return;
		pStruct->pView->GetClientRect( &rcTabRect );

		pDummyView->ClientToScreen( &rcRect );
		pStruct->pView->ClientToScreen( &rcTabRect );

		int nLeft = rcRect.left - rcTabRect.left;
		int nTop = rcRect.top - rcTabRect.top;
		int nRight = rcRect.right - rcTabRect.right;
		int nBottom = rcRect.bottom - rcTabRect.bottom;

		GetClientRect( m_hWnd, &rcRect );
		rcRect += CRect( nLeft, nTop, -nRight, -nBottom );
	}
}

void CRenderBase::UpdateCamera( MatrixEx &Cross )
{
	m_matExWorld = Cross;
	if( !m_CameraHandle ) return;
	m_CameraHandle->Update( Cross );
}

void CRenderBase::ChangeShaderFolder( const char *szFullPath )
{
	CEtResourceMng::GetInstance().RemoveResourcePath( m_szShaderFolder, true );
	m_szShaderFolder = szFullPath;
	CEtResourceMng::GetInstance().AddResourcePath( m_szShaderFolder, true );
}

/*
void CRenderBase::ChangeResourceFolder( const char *szFullPath )
{
	CEtResourceMng::GetInstance().RemoveResourcePath( m_szResourceFolder, true );
	m_szResourceFolder = szFullPath;
	CEtResourceMng::GetInstance().AddResourcePath( m_szResourceFolder, true );
}
*/

void CRenderBase::SetDummyMovement( EtVector3 &vMovement )
{
	if( m_bEnableWeather && m_hWeather ) {
		m_hWeather->SetMovement( vMovement );
	}
}

void CRenderBase::ToggleCameraTopView()
{
	m_bToggleCameraTopView = !m_bToggleCameraTopView;
	if( m_bToggleCameraTopView ) {
		SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
		if( Sector == -1 ) return;
		CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
		if( !pSector ) return;

		m_matExWorldBackup = m_matExWorld;

		float fSizeX = pSector->GetParentGrid()->GetGridWidth() * 100.f;
		float fSizeY = pSector->GetParentGrid()->GetGridHeight() * 100.f;

		m_matExWorld.Identity();
		m_matExWorld.RotatePitch( 90.f );
		m_matExWorld.m_vPosition = *pSector->GetOffset();
		m_matExWorld.m_vPosition.y = min( fSizeX, fSizeY ) * 1.2f;

	}
	else {
		m_matExWorld = m_matExWorldBackup;
	}
	CGlobalValue::GetInstance().RefreshRender();
}