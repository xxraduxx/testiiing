#pragma once

#include "Singleton.h"
#include "EtSoundChannel.h"

#include "EnvInfo.h"

class CEnviControl;
class CEnviElement {
public:
	CEnviElement();
	virtual ~CEnviElement();

protected:
	CEnvInfo m_EnvInfo;

public:
	bool Activate();
	
	CEnvInfo& GetInfo() { return m_EnvInfo;}
	/*
	void SetName( CString szName ) { m_EnvInfo.SetName(szName.GetBuffer()); }
	void SetSkyBox( CString szName ) { m_EnvInfo.SetSkyBox(szName.GetBuffer()); }
	void SetAmbient( DWORD dwColor ) { m_EnvInfo.SetAmbient(dwColor); }
	void SetFogColor( DWORD dwColor ) { m_EnvInfo.SetFogColor(dwColor); }
	void SetFogNear( float fValue ) { m_EnvInfo.SetFogNear(fValue); }
	void SetFogFar( float fValue ) { m_EnvInfo.SetFogFar(fValue); }
	void SetEnvironmentTexture( CString szStr ) { m_EnvInfo.SetEnvironmentTexture(szStr.GetBuffer()); }
	void SetColorAdjShadow( EtVector4 &vVector ) { m_EnvInfo.SetColorAdjShadow(vVector); }
	void SetColorAdjMidtones( EtVector4 &vVector ) { m_EnvInfo.SetColorAdjMidtones(vVector); }
	void SetColorAdjHighlights( EtVector4 &vVector ) { m_EnvInfo.SetColorAdjHighlights(vVector); }
	void SetColorAdjSaturation( float fValue ) { m_EnvInfo.SetColorAdjSaturation(fValue); }
	void SetEnviBGM( CString szName ) { m_EnvInfo.SetEnviBGM(szName.GetBuffer()); }
	void SetEnviBGMRatio( float fValue ) { m_EnvInfo.SetEnviBGMRatio(fValue); }

	void SetBloomScale( float fValue ) { m_EnvInfo.SetBloomScale(fValue); }
	void SetBloomSensitivity( float fValue ) { m_EnvInfo.SetBloomSensitivity(fValue) ; }
	void SetLightmapBlurSize( int nValue ) { m_EnvInfo.SetLightmapBlurSize(nValue); }

	void EnableColorFilter( bool bEnable ) { m_EnvInfo.EnableColorFilter(bEnable); }
	void EnableBloomFilter( bool bEnable ) { m_EnvInfo.EnableBloomFilter(bEnable); }

	void EnableWeather( bool bEnable ) { m_EnvInfo.EnableWeather(bEnable); }
	void SetWeatherTexture( CString szStr ) { m_EnvInfo.SetWeatherTexture(szStr.GetBuffer()); }
	void SetWeatherSpeed( float fValue ) { m_EnvInfo.SetWeatherSpeed(fValue); }
	void SetWeatherStretch( float fValue ) { m_EnvInfo.SetWeatherStretch(fValue); }
	void SetWeatherLayerCount( int nValue ) { m_EnvInfo.SetWeatherLayerCount(nValue); }
	void SetWeatherLayerDecreaseRate( float fValue ) { m_EnvInfo.SetWeatherLayerDecreaseRate(fValue); }
	void SetWeatherLayerDir( EtVector3 &vVector ) { m_EnvInfo.SetWeatherLayerDir(vVector); }

	void EnableSplashDrop( bool bEnable ) { m_EnvInfo.EnableSplashDrop(bEnable); }
	void SetSplashDropTexture( CString szStr ) { m_EnvInfo.SetSplashDropTexture(szStr); }
	void SetSplashDropSize( float fValue ) { m_EnvInfo.SetSplashDropSize(fValue); }
	void SetSplashDropLifeTime( float fValue ) { m_EnvInfo.SetSplashDropLifeTime(fValue); }
	void SetSplashDropRadius( int nValue ) { m_EnvInfo.SetSplashDropRadius(nValue); }
	void SetSplashDropCount( int nValue ) { m_EnvInfo.SetSplashDropCount(nValue); }

	void EnableSnow( bool bEnable ) { m_EnvInfo.EnableSnow(bEnable); }
	void SetSnowTexture( CString szStr ) { m_EnvInfo.SetSnowTexture(szStr); }
	void SetSnowCount( int nValue ) { m_EnvInfo.SetSnowCount(nValue); }
	void SetSnowTimeOut( int nValue ) { m_EnvInfo.SetSnowTimeOut(nValue); }
	void SetSnowHeight( float fValue ) { m_EnvInfo.SetSnowHeight(fValue); }
	void SetIsUp(bool fValue) { m_EnvInfo.SetIsUp(fValue); }

	void EnableHeatHaze( bool bEnable ) { m_EnvInfo.EnableHeatHaze(bEnable); }
	void SetHeatHazeDistortionPower( float fValue ) { m_EnvInfo.SetHeatHazeDistortionPower(fValue); }
	void SetHeatHazeWaveSpeed( float fValue ) { m_EnvInfo.SetHeatHazeWaveSpeed(fValue); }

	void EnableDOF( bool bEnable ) { m_EnvInfo.EnableDOF(bEnable); }
	void SetDOFFocusDistance( float fValue ) { m_EnvInfo.SetDOFFocusDistance(fValue); }
	void SetDOFNearStart( float fValue ) { m_EnvInfo.SetDOFNearStart(fValue); }
	void SetDOFNearEnd( float fValue ) { m_EnvInfo.SetDOFNearEnd(fValue); }
	void SetDOFFarStart( float fValue ) { m_EnvInfo.SetDOFFarStart(fValue); }
	void SetDOFFarEnd( float fValue ) { m_EnvInfo.SetDOFFarEnd(fValue); }
	void SetDOFNearBlur( float fValue ) { m_EnvInfo.SetDOFNearBlur(fValue); }
	void SetDOFFarBlur( float fValue ) { m_EnvInfo.SetDOFFarBlur(fValue); }

	//////////////////////////////////////////////////////////
	CString GetName() { return m_EnvInfo.GetName(); }
	CString GetSkyBox() { return m_EnvInfo.GetSkyBox(); }
	DWORD GetAmbient() { return m_EnvInfo.GetAmbient(); }
	DWORD GetFogColor() { return m_EnvInfo.GetFogColor(); }
	float GetFogNear() { return m_EnvInfo.GetFogNear(); }
	float GetFogFar() { return m_EnvInfo.GetFogFar(); }
	CString GetEnvironmentTexture() { return m_EnvInfo.GetEnvironmentTexture(); }
	EtVector4 *GetColorAdjShadow() { return m_EnvInfo.GetColorAdjShadow(); }
	EtVector4 *GetColorAdjMidtones() { return m_EnvInfo.GetColorAdjMidtones(); }
	EtVector4 *GetColorAdjHighlights() { return m_EnvInfo.GetColorAdjHighlights(); }
	float GetColorAdjSaturation() { return m_EnvInfo.GetColorAdjSaturation(); }
	float GetBloomScale() { return m_EnvInfo.GetBloomScale(); }
	float GetBloomSensitivity() { return m_EnvInfo.GetBloomSensitivity(); }
	int GetLightmapBlurSize() { return m_EnvInfo.GetLightmapBlurSize(); }
	CString GetEnviBGM() { return m_EnvInfo.GetEnviBGM(); }
	float GetEnviBGMRatio() { return m_EnvInfo.GetEnviBGMRatio(); }
	bool IsEnableColorFilter() { return m_EnvInfo.IsEnableColorFilter(); }
	bool IsEnableBloomFilter() { return m_EnvInfo.IsEnableBloomFilter(); }

	bool IsEnableWeather() { return m_EnvInfo.IsEnableWeather(); }
	CString GetWeatherTexture() { return m_EnvInfo.GetWeatherTexture(); }
	float GetWeatherSpeed() { return m_EnvInfo.GetWeatherSpeed(); }
	float GetWeatherStretch() { return m_EnvInfo.GetWeatherStretch(); }
	int GetWeatherLayerCount() { return m_EnvInfo.GetWeatherLayerCount(); }
	float GetWeatherLayerDecreaseRate() { return m_EnvInfo.GetWeatherLayerDecreaseRate(); }
	EtVector3 *GetWeatherLayerDir() { return m_EnvInfo.GetWeatherLayerDir(); }

	bool IsEnableSplashDrop() { return m_EnvInfo.IsEnableSplashDrop(); }
	CString GetSplashDropTexture() { return m_EnvInfo.GetSplashDropTexture(); }
	float GetSplashDropSize() { return m_EnvInfo.GetSplashDropSize(); }
	float GetSplashDropLifeTime() { return m_EnvInfo.GetSplashDropLifeTime(); }
	int GetSplashDropRadius() { return m_EnvInfo.GetSplashDropRadius(); }
	int GetSplashDropCount() { return m_EnvInfo.GetSplashDropCount(); }

	bool IsEnableSnow() { return m_EnvInfo.IsEnableSnow(); }
	CString GetSnowTexture() { return m_EnvInfo.GetSnowTexture(); }
	int GetSnowCount() { return m_EnvInfo.GetSnowCount(); }
	int GetSnowTimeOut() { return m_EnvInfo.GetSnowTimeOut(); }
	float GetSnowHeight() { return m_EnvInfo.GetSnowHeight(); }
	bool GetIsUp() { return m_EnvInfo.GetIsUp(); }

	
	bool IsEnableHeatHaze() { return m_EnvInfo.IsEnableHeatHaze(); }
	float GetHeatHazeDistortionPower() { return m_EnvInfo.GetHeatHazeDistortionPower(); }
	float GetHeatHazeWaveSpeed() { return m_EnvInfo.GetHeatHazeWaveSpeed(); }

	bool IsEnableDOF() { return m_EnvInfo.IsEnableDOF(); }
	float GetDOFFocusDistance() { return m_EnvInfo.GetDOFFocusDistance(); }
	float GetDOFNearStart() { return m_EnvInfo.GetDOFNearStart(); }
	float GetDOFNearEnd() { return m_EnvInfo.GetDOFNearEnd(); }
	float GetDOFFarStart() { return m_EnvInfo.GetDOFFarStart(); }
	float GetDOFFarEnd() { return m_EnvInfo.GetDOFFarEnd(); }
	float GetDOFNearBlur() { return m_EnvInfo.GetDOFNearBlur(); }
	float GetDOFFarBlur() { return m_EnvInfo.GetDOFFarBlur(); }


	///////////////////////////////////////////////
	DWORD GetLightCount() { return m_EnvInfo.GetLightCount(); }
	CEnvInfo::LightInfoStruct *GetLight( DWORD dwIndex ) { return m_EnvInfo.GetLight(dwIndex); }
	void AddLight( SLightInfo &Info ) { m_EnvInfo.AddLight(Info); }
	void RemoveLight( int nIndex ) { m_EnvInfo.RemoveLight(nIndex); }
	//////////////////////////////////////////////
	*/

	void Save();
	bool Load();
	void Delete();
	void Reset();


};

class CEnviControl : public CSingleton< CEnviControl >{
public:
	CEnviControl();
	virtual ~CEnviControl();

protected:
	CEnviElement *m_pActiveElement;
	std::vector<CEnviElement *> m_pVecList;

	CEnviElement *FindElement( const char *szName );
	void ResetElement();

public:
	bool CreateElement( const char *szName, const char *szCopyTo = NULL );
	void RemoveElement( const char *szName );
	void SaveElement( const char *szName );
	void LoadElement( const char *szName );
	CEnviElement *GetActiveElement() { return m_pActiveElement; }

	void Connect();
	void Disconnect();

	void ActivateElement( const char *szName );
	void RefreshElement();

	bool IsExistElement( const char *szName );

	DWORD GetCount() { return (DWORD)m_pVecList.size(); }
	CEnviElement *GetElement( DWORD dwIndex ) { return m_pVecList[dwIndex]; }

};

extern CEnviControl g_EnviControl;