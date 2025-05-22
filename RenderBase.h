#pragma once

#include "Singleton.h"
#include "FrameSync.h"
#include "GlobalValue.h"
#include "EtMatrixEx.h"
#include "EtSoundChannel.h"
#include "EtRainEffect.h"
#include "EtLensFlare.h"
#include "EtRainDropSplash.h"
#include "EtSnowEffect.h"

class CEnviElement;
class CEtSoundEngine;
class CCriticalSection;
class CRenderBase : public CSingleton<CRenderBase>, public CCriticalSection {
public:
	CRenderBase();
	~CRenderBase();

public:
	void Initialize( HWND hWnd );
	void Finalize();
	void Reconstruction();
	void Render( CGlobalValue::VIEW_TYPE Type );
	void Process( CGlobalValue::VIEW_TYPE Type );

	EtCameraHandle GetCameraHandle() { return m_CameraHandle; }
	LOCAL_TIME GetLocalTime() { return m_FrameSync.GetMSTime(); }

	void UpdateCamera( MatrixEx &Cross );
	MatrixEx *GetLastUpdateCamera() { return &m_matExWorld; }
	void InitializeEnvi( CEnviElement *pElement );
	void InitializeEnviLight( CEnviElement *pElement, bool bRefreshAll = true );
	void InitializeEnviBGM( CEnviElement *pElement, bool bRefreshAll = true );
	void InitializeEnviLensFlare( CEnviElement *pElement, bool bRefreshAll = true );

	CEtSoundEngine *GetSoundEngine() { return m_pEtSound; }

	void ChangeShaderFolder( const char *szFullPath );
	CString GetShaderFolder() { return m_szShaderFolder; }

//	void ChangeResourceFolder( const char *szFullPath );
//	CString GetResourceFolder() { return m_szResourceFolder; }

	void SetDummyMovement( EtVector3 &vMovement );

	void ToggleCameraTopView();

	static bool s_bRenderMainScene;
	static bool s_bThreadRenderTarget;

protected:
	bool m_bActivate;
	HWND m_hWnd;
	CFrameSync m_FrameSync;
	EtCameraHandle m_CameraHandle;
	EtCameraHandle m_TopViewCameraHandle;
	MatrixEx m_matExWorld;

	bool m_bEnableColorFilter;
	bool m_bEnableBloomFilter;
	bool m_bEnableWeather;
	bool m_bEnableSplashDrop;
	bool m_bEnableHeatHazeFilter;
	bool m_bEnableDOFFilter;
	bool m_bEnableSnow;
	CEtColorAdjustTexFilter *m_pColorAdjFilter;
	CEtBloomFilter *m_pBloomFilter;
	CEtHeatHazeFilter *m_pHeatHazeFilter;
	CEtDOFFilter *m_pDOFFilter;

	EtObjectHandle m_hSkyBox;
	std::vector<EtLightHandle> m_hVecLight;
	std::vector<EtLensFlareHandle> m_hVecLensFlare;
	LOCAL_TIME m_PrevLocalTime;

	CEtSoundEngine *m_pEtSound;

	int m_nEnviBGMIndex;
	EtSoundChannelHandle m_hEnviBGMChannel;
	EtTextureHandle m_hEnvironmentTexture;

	EtRainHandle m_hWeather;
	EtRainDropSplashHandle m_hSplashDrop;
	EtSnowHandle m_hSnow;

	bool m_bToggleCameraTopView;
	MatrixEx m_matExWorldBackup;
	// Directory
	CString m_szShaderFolder;
//	CString m_szResourceFolder;
	float m_fDelta;

protected:
	void RenderReducedScale();
	void CalcRenderRect( CRect &rcRect );
};

