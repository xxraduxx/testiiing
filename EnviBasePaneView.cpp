// EnviBasePaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "EnviBasePaneView.h"
#include "EnviPaneView.h"
#include "UserMessage.h"
#include "EnviControl.h"
#include "RenderBase.h"


// CEnviBasePaneView
PropertyGridBaseDefine g_EnviPropertyDefine[] = {
	{ "Common", "Skybox", CUnionValueProperty::String_FileOpen, "Sky Box Skin|Eternity Engine Skin File|*.skn", TRUE },
	{ "Common", "Ambient", CUnionValueProperty::Color, "Ambient Color", TRUE },
	{ "Fog", "Fog Color", CUnionValueProperty::Color, "Fog Color", TRUE },
	{ "Fog", "Near", CUnionValueProperty::Float, "Fog Near", TRUE },
	{ "Fog", "Far", CUnionValueProperty::Float, "Fog Far", TRUE },
	{ "Environment", "Environment Texture", CUnionValueProperty::String_FileOpen, "Environment Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Color Filter", "Enable", CUnionValueProperty::Boolean, "Enable Color Filter", TRUE },
	{ "Color Filter", "Shadow-Red", CUnionValueProperty::Float, "Shadow Red|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Shadow-Green", CUnionValueProperty::Float, "Shadow Green|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Shadow-Blue", CUnionValueProperty::Float, "Shadow Blue|-1.f|1.f|0.0001f", TRUE },
/*10*/	{ "Color Filter", "Midtones-Red", CUnionValueProperty::Float, "Midtones Red|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Midtones-Green", CUnionValueProperty::Float, "Midtones Green|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Midtones-Blue", CUnionValueProperty::Float, "Midtones Blue|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Highlights-Red", CUnionValueProperty::Float, "Highlights Red|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Highlights-Green", CUnionValueProperty::Float, "Highlights Green|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Highlights-Blue", CUnionValueProperty::Float, "Highlights Blue|-1.f|1.f|0.0001f", TRUE },
	{ "Color Filter", "Saturation", CUnionValueProperty::Float, "Saturation|0.f|5.f|0.0001f", TRUE },
	{ "Bloom Filter", "Enable", CUnionValueProperty::Boolean, "Enable Bloom Filter", TRUE },
	{ "Bloom Filter", "Scale", CUnionValueProperty::Float, "Bloom Scale|0.f|2.f|0.001f", TRUE },
	{ "Bloom Filter", "Sensitivity", CUnionValueProperty::Float, "Bloom Sensitivity|4.f|10.f|0.001f", TRUE },
/*20*/	{ "BlurSize", "Lightmap Blur Size", CUnionValueProperty::Integer, "Lightmap Blur Size|1|16|1", TRUE },
	{ "Environment", "BGM", CUnionValueProperty::String_FileOpen, "Sound File|All Support Files|*.wav;*.mp3;*.ogg;*.wmv;*.wma", TRUE },
	{ "Environment", "BGM Ratio", CUnionValueProperty::Float, "Default BGM Ratio|0.f|1.f|0.001f", TRUE },
	{ "Weather Base - Rain", "Enable", CUnionValueProperty::Boolean, "Enable Rain Control", TRUE },
	{ "Weather Base - Rain", "Texture", CUnionValueProperty::String_FileOpen, "Rain Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Weather Base - Rain", "Speed", CUnionValueProperty::Float, "Speed|0.f|10.f|0.0001f", TRUE },
	{ "Weather Base - Rain", "Stretch", CUnionValueProperty::Float, "Stretch|0.f|10.f|0.0001f", TRUE },
	{ "Weather DropEffect", "Enable Splash Drop Effect", CUnionValueProperty::Boolean, "Enable Weather Splash Drop Effect", TRUE },
	{ "Weather DropEffect", "SplashDrop Texture", CUnionValueProperty::String_FileOpen, "Weather SplashDrop Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Weather DropEffect", "Size", CUnionValueProperty::Float, "SplashDrop Size|0.1|100.f|0.0001f", TRUE },
/*30*/	{ "Weather DropEffect", "LifeTime", CUnionValueProperty::Float, "SplashDrop Lifetime|0.f|5.f|0.0001f", TRUE },
	{ "Weather DropEffect", "Radius", CUnionValueProperty::Integer, "SplashDrop Radius|0|5000|1", TRUE },
	{ "Weather DropEffect", "Count", CUnionValueProperty::Integer, "SplashDrop Create Per Second|0|5000|1", TRUE },
	{ "Weather Base - Rain", "Layer Count", CUnionValueProperty::Integer, "Layer Count|1|4|1", TRUE },
	{ "Weather Base - Rain", "Layer Decrease Rate", CUnionValueProperty::Float, "레이어별 속도 감쇠율|0.1f|1.f|0.9f", TRUE },
	{ "Weather Base - Rain", "Direction", CUnionValueProperty::Vector3, "!Directional", TRUE },
	{ "HeatHaze", "Enable", CUnionValueProperty::Boolean, "Enable Heat Haze", TRUE },
	{ "HeatHaze", "Distortion Power", CUnionValueProperty::Float, "Distortion Power|0.1f|5.f|0.001f", TRUE },
	{ "HeatHaze", "Wave Speed", CUnionValueProperty::Float, "Wave Speed|0.1f|10.f|0.001f", TRUE },
	{ "DepthOfField", "Enable", CUnionValueProperty::Boolean, "Enable Depth of field", TRUE },
/*40*/ { "DepthOfField", "Focus Distance", CUnionValueProperty::Float, "Focus Distance(cm)", TRUE },
	{ "DepthOfField", "Near Start", CUnionValueProperty::Float, "Near Start(cm)", TRUE },
	{ "DepthOfField", "Near End", CUnionValueProperty::Float, "Near End(cm)", TRUE },
	{ "DepthOfField", "Far Start", CUnionValueProperty::Float, "Far Start(cm)", TRUE },
	{ "DepthOfField", "Far End", CUnionValueProperty::Float, "Far End(cm)", TRUE },
	{ "DepthOfField", "Near Blur", CUnionValueProperty::Float, "Near Blur|1.f|36.f|0.001f", TRUE },
	{ "DepthOfField", "Far Blur", CUnionValueProperty::Float, "Far Blur|1.f|36.f|0.001f", TRUE },
	{ "Weather Base - Snow", "Enable", CUnionValueProperty::Boolean, "Enable Snow Control", TRUE },
	{ "Weather Base - Snow", "Texture", CUnionValueProperty::String_FileOpen, "Snow Texture|Direct Draw Surface File|*.dds", TRUE },
	{ "Weather Base - Snow", "Count", CUnionValueProperty::Integer, "Snow Count|0|5000|1", TRUE },
/*50*/	{ "Weather Base - Snow", "TimeOut", CUnionValueProperty::Integer, "Snow TimeOut|0|15000|1", TRUE },
	{ "Snow Base - Snow", "Height", CUnionValueProperty::Float, "Snow Height|0.f|5000.f|0.1f", TRUE },
	{ "Snow Base - Snow", "IsUp", CUnionValueProperty::Boolean, "Snow Dir False(Down)/True(Up)", FALSE },
	{ "Snow Base - Snow", "SnowSize", CUnionValueProperty::Float, "Snow Size|0|100|1", TRUE },
	{ "Snow Base - Snow", "AniSpeed", CUnionValueProperty::Float, "AniSpeed|0.001f|5.0f|0.001f", TRUE },
	{ "Environment Effect", "Environment Effect", CUnionValueProperty::String_FileOpen, "Environment Effect|Eternity Action File (*.act)|*.act", TRUE },
	{ NULL },
};

IMPLEMENT_DYNCREATE(CEnviBasePaneView, CFormView)

CEnviBasePaneView::CEnviBasePaneView()
	: CFormView(CEnviBasePaneView::IDD)
{
	m_bActivate = false;
}

CEnviBasePaneView::~CEnviBasePaneView()
{
	ResetPropertyGrid();
}

void CEnviBasePaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEnviBasePaneView, CFormView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnNotifyGrid )
	ON_MESSAGE( UM_SOUNDPANE_DROPMOVEITEM, OnSoundPaneDropMoveItem )
	ON_MESSAGE( UM_SOUNDPANE_DROPITEM, OnSoundPaneDropItem )

END_MESSAGE_MAP()


// CEnviBasePaneView diagnostics

#ifdef _DEBUG
void CEnviBasePaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEnviBasePaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEnviBasePaneView message handlers

void CEnviBasePaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );

	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0);

	SendMessage( UM_REFRESH_PANE_VIEW );
}


void CEnviBasePaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
//		m_PropertyGrid.MoveWindow( &rcRect );
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

BOOL CEnviBasePaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

LRESULT CEnviBasePaneView::OnNotifyGrid( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}


LRESULT CEnviBasePaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	if( !m_PropertyGrid ) return S_OK;
	ResetPropertyGrid();
	RefreshPropertyGrid( g_EnviPropertyDefine );
	return S_OK;
}

void CEnviBasePaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	CEnvInfo& envInfo = pElement->GetInfo();

	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (char*)envInfo.GetSkyBox() );
			break;
		case 1:
			pVariable->SetVariable( (int)envInfo.GetAmbient() );
			break;
		case 2:
			pVariable->SetVariable( (int)envInfo.GetFogColor() );
			break;
		case 3:
			pVariable->SetVariable( envInfo.GetFogNear() );
			break;
		case 4:
			pVariable->SetVariable( envInfo.GetFogFar() );
			break;
		case 5:
			pVariable->SetVariable( (char*)envInfo.GetEnvironmentTexture() );
			break;
		case 6:
			pVariable->SetVariable( envInfo.IsEnableColorFilter() );
			break;
		case 7:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjShadow()->x );
			break;
		case 8:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjShadow()->y );
			break;
		case 9:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjShadow()->z );
			break;
		case 10:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjMidtones()->x );
			break;
		case 11:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjMidtones()->y );
			break;
		case 12:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjMidtones()->z );
			break;
		case 13:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjHighlights()->x );
			break;
		case 14:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjHighlights()->y );
			break;
		case 15:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjHighlights()->z );
			break;
		case 16:
			SetReadOnly( pVariable, !envInfo.IsEnableColorFilter() );
			pVariable->SetVariable( envInfo.GetColorAdjSaturation() );
			break;
		case 17:
			pVariable->SetVariable( envInfo.IsEnableBloomFilter() );
			break;
		case 18:
			SetReadOnly( pVariable, !envInfo.IsEnableBloomFilter() );
			pVariable->SetVariable( envInfo.GetBloomScale() );
			break;
		case 19:
			SetReadOnly( pVariable, !envInfo.IsEnableBloomFilter() );
			pVariable->SetVariable( envInfo.GetBloomSensitivity() );
			break;
		case 20:
			pVariable->SetVariable( envInfo.GetLightmapBlurSize() );
			break;
		case 21:
			pVariable->SetVariable( (CHAR*)envInfo.GetEnviBGM() );
			break;
		case 22:
			pVariable->SetVariable( envInfo.GetEnviBGMRatio() );
			break;
		case 23:
			pVariable->SetVariable( envInfo.IsEnableWeather() );
			break;
		case 24:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( (CHAR*)envInfo.GetWeatherTexture() );
			break;
		case 25:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( envInfo.GetWeatherSpeed() );
			break;
		case 26:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( envInfo.GetWeatherStretch() );
			break;
		case 27:
			pVariable->SetVariable( envInfo.IsEnableSplashDrop() );
			break;
		case 28:
			SetReadOnly( pVariable, !envInfo.IsEnableSplashDrop() );
			pVariable->SetVariable( (CHAR*)envInfo.GetSplashDropTexture() );
			break;
		case 29:
			SetReadOnly( pVariable, !envInfo.IsEnableSplashDrop() );
			pVariable->SetVariable( envInfo.GetSplashDropSize() );
			break;
		case 30:
			SetReadOnly( pVariable, !envInfo.IsEnableSplashDrop() );
			pVariable->SetVariable( envInfo.GetSplashDropLifeTime() );
			break;
		case 31:
			SetReadOnly( pVariable, !envInfo.IsEnableSplashDrop() );
			pVariable->SetVariable( envInfo.GetSplashDropRadius() );
			break;
		case 32:
			SetReadOnly( pVariable, !envInfo.IsEnableSplashDrop() );
			pVariable->SetVariable( envInfo.GetSplashDropCount() );
			break;
		case 33:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( envInfo.GetWeatherLayerCount() );
			break;
		case 34:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( envInfo.GetWeatherLayerDecreaseRate() );
			break;
		case 35:
			SetReadOnly( pVariable, !envInfo.IsEnableWeather() );
			pVariable->SetVariable( *envInfo.GetWeatherLayerDir() );
			break;
		case 36:
			pVariable->SetVariable( envInfo.IsEnableHeatHaze() );
			break;
		case 37:
			SetReadOnly( pVariable, !envInfo.IsEnableHeatHaze() );
			pVariable->SetVariable( envInfo.GetHeatHazeDistortionPower() );
			break;
		case 38:
			SetReadOnly( pVariable, !envInfo.IsEnableHeatHaze() );
			pVariable->SetVariable( envInfo.GetHeatHazeWaveSpeed() );
			break;
		case 39:
			pVariable->SetVariable( envInfo.IsEnableDOF() );
			break;
		case 40:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFFocusDistance() );
			break;
		case 41:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFNearStart() );
			break;
		case 42:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFNearEnd() );
			break;
		case 43:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFFarStart() );
			break;
		case 44:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFFarEnd() );
			break;
		case 45:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFNearBlur() );
			break;
		case 46:
			SetReadOnly( pVariable, !envInfo.IsEnableDOF() );
			pVariable->SetVariable( envInfo.GetDOFFarBlur() );
			break;
		case 47:
			pVariable->SetVariable( envInfo.IsEnableSnow() );
			break;
		case 48:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( (CHAR*)envInfo.GetSnowTexture() );
			break;
		case 49:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetSnowCount() );
			break;
		case 50:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetSnowTimeOut() );
			break;
		case 51:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetSnowHeight() );
			break;
		case 52:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetIsUp() );
			break;
		case 53:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetSnowSize() );
			break;
		case 54:
			SetReadOnly( pVariable, !envInfo.IsEnableSnow() );
			pVariable->SetVariable( envInfo.GetSnowAniSpeed() );
			break;
		case 55:
			pVariable->SetVariable( (char*)envInfo.GetEnvironmentEffect() );
			break;
	}
	if( strcmp( envInfo.GetName(), "Edit Set" ) == NULL ) {
		SetReadOnly( pVariable, true );
	}
}

void CEnviBasePaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	CEnviElement *pElement = CEnviControl::GetInstance().GetActiveElement();
	CEnvInfo& envInfo = pElement->GetInfo();

	switch( dwIndex ) {
		case 0:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetSkyBox( szTemp );
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetParent()->GetParent();
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 1:
			envInfo.SetAmbient( pVariable->GetVariableInt() );
			break;
		case 2:
			envInfo.SetFogColor( pVariable->GetVariableInt() );
			break;
		case 3:
			envInfo.SetFogNear( pVariable->GetVariableFloat() );
			break;
		case 4:
			envInfo.SetFogFar( pVariable->GetVariableFloat() );
			break;
		case 5:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetEnvironmentTexture( szTemp );
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetParent()->GetParent();
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 6:
			envInfo.EnableColorFilter( pVariable->GetVariableBool() );
			SetReadOnly( 7, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 8, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 9, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 10, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 11, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 12, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 13, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 14, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 15, !envInfo.IsEnableColorFilter() );
			SetReadOnly( 16, !envInfo.IsEnableColorFilter() );
			break;
		case 7:
		case 8:
		case 9:
			{
				EtVector4 vTemp = *envInfo.GetColorAdjShadow();
				if( dwIndex == 7 ) vTemp.x = pVariable->GetVariableFloat();
				if( dwIndex == 8 ) vTemp.y = pVariable->GetVariableFloat();
				if( dwIndex == 9 ) vTemp.z = pVariable->GetVariableFloat();
				envInfo.SetColorAdjShadow( vTemp );
			}
			break;
		case 10:
		case 11:
		case 12:
			{
				EtVector4 vTemp = *envInfo.GetColorAdjMidtones();
				if( dwIndex == 10 ) vTemp.x = pVariable->GetVariableFloat();
				if( dwIndex == 11 ) vTemp.y = pVariable->GetVariableFloat();
				if( dwIndex == 12 ) vTemp.z = pVariable->GetVariableFloat();
				envInfo.SetColorAdjMidtones( vTemp );
			}
			break;
		case 13:
		case 14:
		case 15:
			{
				EtVector4 vTemp = *envInfo.GetColorAdjHighlights();
				if( dwIndex == 13 ) vTemp.x = pVariable->GetVariableFloat();
				if( dwIndex == 14 ) vTemp.y = pVariable->GetVariableFloat();
				if( dwIndex == 15 ) vTemp.z = pVariable->GetVariableFloat();
				envInfo.SetColorAdjHighlights( vTemp );
			}
			break;
		case 16:
			envInfo.SetColorAdjSaturation( pVariable->GetVariableFloat() );
			break;
		case 17:
			envInfo.EnableBloomFilter( pVariable->GetVariableBool() );
			SetReadOnly( 18, !envInfo.IsEnableBloomFilter() );
			SetReadOnly( 19, !envInfo.IsEnableBloomFilter() );
			break;
		case 18:
			envInfo.SetBloomScale( pVariable->GetVariableFloat() );
			break;
		case 19:
			envInfo.SetBloomSensitivity( pVariable->GetVariableFloat() );
			break;
		case 20:
			envInfo.SetLightmapBlurSize( pVariable->GetVariableInt() );
			break;
		case 21:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetEnviBGM( szTemp );
				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 22:
			envInfo.SetEnviBGMRatio( pVariable->GetVariableFloat() );
			CRenderBase::GetInstance().InitializeEnviBGM( pElement, false );
			return;
		case 23:
			envInfo.EnableWeather( pVariable->GetVariableBool() );
			SetReadOnly( 24, !envInfo.IsEnableWeather() );
			SetReadOnly( 25, !envInfo.IsEnableWeather() );
			SetReadOnly( 26, !envInfo.IsEnableWeather() );
			SetReadOnly( 33, !envInfo.IsEnableWeather() );
			SetReadOnly( 34, !envInfo.IsEnableWeather() );
			SetReadOnly( 35, !envInfo.IsEnableWeather() );
			break;
		case 24:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetWeatherTexture( szTemp );
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetParent()->GetParent();
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}
				
				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 25:
			envInfo.SetWeatherSpeed( pVariable->GetVariableFloat() );
			break;
		case 26:
			envInfo.SetWeatherStretch( pVariable->GetVariableFloat() );
			break;
		case 27:
			envInfo.EnableSplashDrop( pVariable->GetVariableBool() );
			SetReadOnly( 28, !envInfo.IsEnableSplashDrop() );
			SetReadOnly( 29, !envInfo.IsEnableSplashDrop() );
			SetReadOnly( 30, !envInfo.IsEnableSplashDrop() );
			SetReadOnly( 31, !envInfo.IsEnableSplashDrop() );
			SetReadOnly( 32, !envInfo.IsEnableSplashDrop() );
			break;
		case 28:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetSplashDropTexture( szTemp );
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetParent()->GetParent();
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 29:
			envInfo.SetSplashDropSize( pVariable->GetVariableFloat() );
			break;
		case 30:
			envInfo.SetSplashDropLifeTime( pVariable->GetVariableFloat() );
			break;
		case 31:
			envInfo.SetSplashDropRadius( pVariable->GetVariableInt() );
			break;
		case 32:
			envInfo.SetSplashDropCount( pVariable->GetVariableInt() );
			break;
		case 33:
			envInfo.SetWeatherLayerCount( pVariable->GetVariableInt() );
			break;
		case 34:
			envInfo.SetWeatherLayerDecreaseRate( pVariable->GetVariableFloat() );
			break;
		case 35:
			envInfo.SetWeatherLayerDir( pVariable->GetVariableVector3() );
			break;
		case 36:
			envInfo.EnableHeatHaze( pVariable->GetVariableBool() );
			SetReadOnly( 37, !envInfo.IsEnableHeatHaze() );
			SetReadOnly( 38, !envInfo.IsEnableHeatHaze() );
			break;
		case 37:
			envInfo.SetHeatHazeDistortionPower( pVariable->GetVariableFloat() );
			break;
		case 38:
			envInfo.SetHeatHazeWaveSpeed( pVariable->GetVariableFloat() );
			break;
		case 39:
			envInfo.EnableDOF( pVariable->GetVariableBool() );
			SetReadOnly( 40, !envInfo.IsEnableDOF() );
			SetReadOnly( 41, !envInfo.IsEnableDOF() );
			SetReadOnly( 42, !envInfo.IsEnableDOF() );
			SetReadOnly( 43, !envInfo.IsEnableDOF() );
			SetReadOnly( 44, !envInfo.IsEnableDOF() );
			SetReadOnly( 45, !envInfo.IsEnableDOF() );
			SetReadOnly( 46, !envInfo.IsEnableDOF() );
			break;
		case 40:
			envInfo.SetDOFFocusDistance( pVariable->GetVariableFloat() );
			break;
		case 41:
			envInfo.SetDOFNearStart( pVariable->GetVariableFloat() );
			break;
		case 42:
			envInfo.SetDOFNearEnd( pVariable->GetVariableFloat() );
			break;
		case 43:
			envInfo.SetDOFFarStart( pVariable->GetVariableFloat() );
			break;
		case 44:
			envInfo.SetDOFFarEnd( pVariable->GetVariableFloat() );
			break;
		case 45:
			envInfo.SetDOFNearBlur( pVariable->GetVariableFloat() );
			break;
		case 46:
			envInfo.SetDOFFarBlur( pVariable->GetVariableFloat() );
			break;
		case 47:
			envInfo.EnableSnow( pVariable->GetVariableBool() );
			SetReadOnly( 48, !envInfo.IsEnableSnow() );
			SetReadOnly( 49, !envInfo.IsEnableSnow() );
			SetReadOnly( 50, !envInfo.IsEnableSnow() );
			SetReadOnly( 51, !envInfo.IsEnableSnow() );
			SetReadOnly( 52, !envInfo.IsEnableSnow() );
			SetReadOnly( 53, !envInfo.IsEnableSnow() );
			SetReadOnly( 54, !envInfo.IsEnableSnow() );
			break;
		case 48:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );

				envInfo.SetSnowTexture( szTemp );
				if( !szStr.IsEmpty() ) {
					CEnviPaneView *pParent = (CEnviPaneView *)GetParent()->GetParent();
					pParent->CopyAccompanimentFile( pVariable->GetVariableString() );
				}

				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
		case 49:
			envInfo.SetSnowCount( pVariable->GetVariableInt() );
			break;
		case 50:
			envInfo.SetSnowTimeOut( pVariable->GetVariableInt() );
			break;
		case 51:
			envInfo.SetSnowHeight( pVariable->GetVariableFloat() );
			break;
		case 52:
			envInfo.SetIsUp(pVariable->GetVariableBool());
			break;
		case 53:
			envInfo.SetSnowSize(pVariable->GetVariableFloat());
			break;
		case 54:
			envInfo.SetSnowAniSpeed(pVariable->GetVariableFloat());
			break;
		case 55:
			{
				char szTemp[256] = { 0, };
				CString szStr = pVariable->GetVariableString();
				if( !szStr.IsEmpty() )
					_GetFullFileName( szTemp, _countof(szTemp), pVariable->GetVariableString() );
				envInfo.SetEnvironmentEffect( szTemp );
				pVariable->SetVariable( szTemp );
				UpdateItem();
			}
			break;
	}
	CEnviControl::GetInstance().ActivateElement( envInfo.GetName() );
}


LRESULT CEnviBasePaneView::OnSoundPaneDropMoveItem( WPARAM wParam, LPARAM lParam )
{
	return S_OK;
}

LRESULT CEnviBasePaneView::OnSoundPaneDropItem( WPARAM wParam, LPARAM lParam )
{
	return S_OK;
}
