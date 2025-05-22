#include "StdAfx.h"
#include "EnviControl.h"
#include "RenderBase.h"
#include "FileServer.h"
#include "SundriesFuncEx.h"
#include "resource.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "UserMessage.h"

CEnviControl g_EnviControl;

CEnviElement::CEnviElement()
{
	/*
	m_dwAmbient = RGB(255,255,255);
	m_dwFogColor = RGB(255,255,255);
	m_fFogNear = 1000000.f;
	m_fFogFar = 1000000.f;
	m_fSaturation = 1.f;
	m_fBloomScale = 1.f;
	m_fBloomSensitivity = 6.f;
	m_bEnableColorFilter = false;
	m_bEnableBloomFilter = false;
	m_fEnviBGMRatio = 0.5f;

	m_nLightmapBlurSize = 1;

	m_bEnableWeather = false;
	m_fWeatherSpeed = 1.f;
	m_fWeatherStretch = 1.f;

	m_bEnableSplashDrop = false;
	m_fSplashDropSize = 1.f;
	m_fSplashDropLifeTime = 0.2f;
	m_nSplashDropRadius = 500;
	m_nSplashDropCount = 1500;
	*/
	Reset();
}

CEnviElement::~CEnviElement()
{
	
}

bool CEnviElement::Activate()
{
	CRenderBase::GetInstance().Lock();

	CRenderBase::GetInstance().InitializeEnvi( this );
	CRenderBase::GetInstance().InitializeEnviLight( this );
	CRenderBase::GetInstance().InitializeEnviLensFlare( this );
	CRenderBase::GetInstance().InitializeEnviBGM( this );

	CRenderBase::GetInstance().Unlock();
	return true;
}

void CEnviElement::Save()
{
	CString szFullName;
	if( strcmp( m_EnvInfo.GetName(), "Edit Set" ) == NULL ) return;
	szFullName.Format( "%s\\Resource\\Envi\\%s.env", CFileServer::GetInstance().GetLocalWorkingFolder(), m_EnvInfo.GetName() );
	
	m_EnvInfo.Save(szFullName.GetBuffer());
}

bool CEnviElement::Load()
{
	CString szFullName;
	szFullName.Format( "%s\\Resource\\Envi\\%s.env", CFileServer::GetInstance().GetLocalWorkingFolder(), m_EnvInfo.GetName() );
	
	return m_EnvInfo.Load(szFullName.GetBuffer());
}

void CEnviElement::Reset()
{
	m_EnvInfo.Reset();
}

void CEnviElement::Delete()
{
	CString szFullName;
	if( strcmp( m_EnvInfo.GetName(), "Edit Set" ) == NULL ) return;
	szFullName.Format( "%s\\Resource\\Envi\\%s.env", CFileServer::GetInstance().GetLocalWorkingFolder(), m_EnvInfo.GetName() );

	DeleteFile( szFullName );
}


// DWORD CEnviElement::GetLightCount()
// {
// 	return (DWORD)m_pVecLightList.size();
// }
// 
// CEnviElement::LightInfoStruct *CEnviElement::GetLight( DWORD dwIndex )
// {
// 	if( dwIndex < 0 || dwIndex >= (DWORD)m_pVecLightList.size() ) return NULL;
// 	return m_pVecLightList[dwIndex];
// }
// 
// void CEnviElement::AddLight( SLightInfo &Info )
// {
// 	LightInfoStruct *pStruct = new LightInfoStruct;
// 	pStruct->pInfo = new SLightInfo;
// 	*pStruct->pInfo = Info;
// 
// 	m_pVecLightList.push_back( pStruct );
// }
// 
// void CEnviElement::RemoveLight( int nIndex )
// {
// 	if( nIndex < 0 || nIndex >= (int)m_pVecLightList.size() ) return;
// 	SAFE_DELETE( m_pVecLightList[nIndex]->pInfo );
// 	SAFE_DELETE( m_pVecLightList[nIndex] );
// 	m_pVecLightList.erase( m_pVecLightList.begin() + nIndex );
// }


CEnviControl::CEnviControl()
{
	CreateElement( "Edit Set", NULL );
	CEnviElement *pElement = GetElement(0);
	SLightInfo Info;
	pElement->GetInfo().AddLight( Info );

	m_pActiveElement = m_pVecList[0];
}

CEnviControl::~CEnviControl()
{
	SAFE_DELETE_PVEC( m_pVecList );
//	Disconnect();
}


bool CEnviControl::CreateElement( const char *szName, const char *szCopyTo )
{
	CEnviElement *pElement = new CEnviElement;
	CEnvInfo& envInfo = pElement->GetInfo();

	envInfo.SetName( szName );
	if( szCopyTo ) {
		CEnviElement *pCopyTo = FindElement( szCopyTo );
		CEnvInfo& copyToInfo = pCopyTo->GetInfo();

		if( pCopyTo ) {
			envInfo.SetAmbient( copyToInfo.GetAmbient() );
			envInfo.SetFogColor( copyToInfo.GetFogColor() );
			envInfo.SetFogNear( copyToInfo.GetFogNear() );
			envInfo.SetFogFar( copyToInfo.GetFogFar() );
			envInfo.SetSkyBox( copyToInfo.GetSkyBox() );
			envInfo.SetEnvironmentTexture( copyToInfo.GetEnvironmentTexture() );

			envInfo.EnableColorFilter( copyToInfo.IsEnableColorFilter() );
			envInfo.SetColorAdjShadow( *copyToInfo.GetColorAdjShadow() );
			envInfo.SetColorAdjMidtones( *copyToInfo.GetColorAdjMidtones() );
			envInfo.SetColorAdjHighlights( *copyToInfo.GetColorAdjHighlights() );
			envInfo.SetColorAdjSaturation( copyToInfo.GetColorAdjSaturation() );

			envInfo.EnableBloomFilter( copyToInfo.IsEnableBloomFilter() );
			envInfo.SetBloomScale( copyToInfo.GetBloomScale() );
			envInfo.SetBloomSensitivity( copyToInfo.GetBloomSensitivity() );

			envInfo.SetEnviBGM( copyToInfo.GetEnviBGM() );
			envInfo.SetEnviBGMRatio( copyToInfo.GetEnviBGMRatio() );

			envInfo.SetLightmapBlurSize( copyToInfo.GetLightmapBlurSize() );

			envInfo.EnableWeather( copyToInfo.IsEnableWeather() );
			envInfo.SetWeatherTexture( copyToInfo.GetWeatherTexture() );
			envInfo.SetWeatherSpeed( copyToInfo.GetWeatherSpeed() );
			envInfo.SetWeatherStretch( copyToInfo.GetWeatherStretch() );
			envInfo.SetWeatherLayerCount( copyToInfo.GetWeatherLayerCount() );
			envInfo.SetWeatherLayerDecreaseRate( copyToInfo.GetWeatherLayerDecreaseRate() );
			envInfo.SetWeatherLayerDir( *copyToInfo.GetWeatherLayerDir() );

			envInfo.EnableSplashDrop( copyToInfo.IsEnableSplashDrop() );
			envInfo.SetSplashDropTexture( copyToInfo.GetSplashDropTexture() );
			envInfo.SetSplashDropSize( copyToInfo.GetSplashDropSize() );
			envInfo.SetSplashDropLifeTime( copyToInfo.GetSplashDropLifeTime() );
			envInfo.SetSplashDropRadius( copyToInfo.GetSplashDropRadius() );
			envInfo.SetSplashDropCount( copyToInfo.GetSplashDropCount() );

			envInfo.EnableHeatHaze( copyToInfo.IsEnableHeatHaze() );
			envInfo.SetHeatHazeDistortionPower( copyToInfo.GetHeatHazeDistortionPower() );
			envInfo.SetHeatHazeWaveSpeed( copyToInfo.GetHeatHazeWaveSpeed() );

			envInfo.EnableDOF( copyToInfo.IsEnableDOF() );
			envInfo.SetDOFFocusDistance( copyToInfo.GetDOFFocusDistance() );
			envInfo.SetDOFNearStart( copyToInfo.GetDOFNearStart() );
			envInfo.SetDOFNearEnd( copyToInfo.GetDOFNearEnd() );
			envInfo.SetDOFFarStart( copyToInfo.GetDOFFarStart() );
			envInfo.SetDOFFarEnd( copyToInfo.GetDOFFarEnd() );
			envInfo.SetDOFNearBlur( copyToInfo.GetDOFNearBlur() );
			envInfo.SetDOFFarBlur( copyToInfo.GetDOFFarBlur() );

			envInfo.EnableSnow( copyToInfo.IsEnableSnow() );
			envInfo.SetSnowTexture( copyToInfo.GetSnowTexture() );
			envInfo.SetSnowCount( copyToInfo.GetSnowCount() );
			envInfo.SetSnowTimeOut( copyToInfo.GetSnowTimeOut() );
			envInfo.SetSnowHeight( copyToInfo.GetSnowHeight() );

			for( DWORD i=0; i<copyToInfo.GetLightCount(); i++ ) {
				envInfo.AddLight( *copyToInfo.GetLight(i)->pInfo );
			}
		}

	}
	pElement->Save();
	m_pVecList.push_back( pElement );
	return true;
}

void CEnviControl::RemoveElement( const char *szName )
{
	CEnviElement *pElement = FindElement( szName );
	if( !pElement ) return;

	pElement->Delete();
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( m_pVecList[i] == pElement ) {
			m_pVecList.erase( m_pVecList.begin() + i );
			SAFE_DELETE( pElement );
			break;
		}
	}
}

void CEnviControl::SaveElement( const char *szName )
{
	CEnviElement *pElement = FindElement( szName );
	if( !pElement ) return;

	pElement->Save();
}

void CEnviControl::LoadElement( const char *szName )
{
	CEnviElement *pElement = FindElement( szName );
	if( !pElement ) return;

	pElement->Reset();
	pElement->Load();
}

void CEnviControl::Connect()
{
	CGlobalValue::GetInstance().UpdateInitDesc( "Register Environments.." );

	std::vector<std::string> szVecList;
	CString szEnviFolder;
	szEnviFolder.Format( "%s\\Resource\\Envi", CFileServer::GetInstance().GetLocalWorkingFolder() );
	FindFileListInDirectory( szEnviFolder, "*.env", szVecList, false, false );

	for( DWORD i=0; i<szVecList.size(); i++ ) {
		char szTemp[256] = { 0, };
		_GetFileName( szTemp, _countof(szTemp), szVecList[i].c_str() );

		CEnviElement *pElement = new CEnviElement;
		pElement->GetInfo().SetName( szTemp );
		if( pElement->Load() == false ) {
			SAFE_DELETE( pElement );
			continue;
		}

		m_pVecList.push_back( pElement );
	}
	CWnd *pWnd = GetPaneWnd( ENVI_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );


}

void CEnviControl::Disconnect()
{
	ResetElement();
	if( CRenderBase::IsActive() ) {
		CRenderBase::GetInstance().InitializeEnvi( m_pVecList[0] );
		CRenderBase::GetInstance().InitializeEnviLight( m_pVecList[0] );
		CRenderBase::GetInstance().InitializeEnviLensFlare( m_pVecList[0] );
	}
	m_pActiveElement = m_pVecList[0];
}

void CEnviControl::ResetElement()
{
	SAFE_DELETE_PVEC( m_pVecList );
	CreateElement( "Edit Set", NULL );
	CEnviElement *pElement = GetElement(0);
	SLightInfo Info;
	pElement->GetInfo().AddLight( Info );
}

void CEnviControl::ActivateElement( const char *szName )
{
	CEnviElement *pElement = FindElement( szName );
	if( !pElement ) return;

	pElement->Activate();
	m_pActiveElement = pElement;

	CGlobalValue::GetInstance().RefreshRender();
}

void CEnviControl::RefreshElement()
{
	if( !m_pActiveElement ) return;
	m_pActiveElement->Activate();

	CGlobalValue::GetInstance().RefreshRender();
}

CEnviElement *CEnviControl::FindElement( const char *szName )
{
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( strcmp( m_pVecList[i]->GetInfo().GetName(), szName ) == NULL ) return m_pVecList[i];
	}
	return NULL;
}

bool CEnviControl::IsExistElement( const char *szName )
{
	if( FindElement( szName ) ) return true;
	return false;
}
