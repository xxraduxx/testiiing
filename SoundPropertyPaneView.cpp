// SoundPropertyPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundPropertyPaneView.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "TEtWorldSound.h"
#include "TEtWorldSoundEnvi.h"
#include "EtSoundEngine.h"
#include "fmod.hpp"
#include "RenderBase.h"


// CSoundPropertyPaneView
PropertyGridBaseDefine g_SoundRootPropertyDefine[] = {
	{ "Common", "Play Sound", CUnionValueProperty::Boolean, "사운드 플레이 할까?", TRUE },
	{ "Common", "Volume", CUnionValueProperty::Float, "볼륨|0.f|1.f|0.001f", TRUE },
	{ NULL },
};

PropertyGridBaseDefine g_SoundBGMPropertyDefine[] = {
	{ "Common", "File Name", CUnionValueProperty::String_FileOpen, "파일 이름", FALSE },
	{ "Common", "Volume", CUnionValueProperty::Float, "볼륨|0.f|1.f|0.001f", TRUE },
	/*
	{ "DSP - OSCILLATOR", "Enable", CUnionValueProperty::Boolean, "DSP - OSCILLATOR", TRUE },
	{ "DSP - LOWPASS", "Enable", CUnionValueProperty::Boolean, "DSP - LOWPASS", TRUE },
	{ "DSP - ITLOWPASS", "Enable", CUnionValueProperty::Boolean, "DSP - ITLOWPASS", TRUE },
	{ "DSP - HIGHPASS", "Enable", CUnionValueProperty::Boolean, "DSP - HIGHPASS", TRUE },
	{ "DSP - ECHO", "Enable", CUnionValueProperty::Boolean, "DSP - ECHO", TRUE },
	{ "DSP - FLANGE", "Enable", CUnionValueProperty::Boolean, "DSP - FLANGE", TRUE },
	{ "DSP - DISTORTION", "Enable", CUnionValueProperty::Boolean, "DSP - DISTORTION", TRUE },
	{ "DSP - NORMALIZE", "Enable", CUnionValueProperty::Boolean, "DSP - NORMALIZE", TRUE },
	{ "DSP - PARAMEQ", "Enable", CUnionValueProperty::Boolean, "DSP - PARAMEQ", TRUE },
	{ "DSP - PITCHSHIFT", "Enable", CUnionValueProperty::Boolean, "DSP - PITCHSHIFT", TRUE },
	{ "DSP - CHORUS", "Enable", CUnionValueProperty::Boolean, "DSP - CHORUS", TRUE },
	{ "DSP - REVERB", "Enable", CUnionValueProperty::Boolean, "DSP - REVERB", TRUE },
	{ "DSP - VSTPLUGIN", "Enable", CUnionValueProperty::Boolean, "DSP - VSTPLUGIN", TRUE },
	{ "DSP - WINAMPPLUGIN", "Enable", CUnionValueProperty::Boolean, "DSP - WINAMPPLUGIN", TRUE },
	{ "DSP - ITECHO", "Enable", CUnionValueProperty::Boolean, "DSP - ITECHO", TRUE },
	{ "DSP - COMPRESSOR", "Enable", CUnionValueProperty::Boolean, "DSP - COMPRESSOR", TRUE },
	{ "DSP - SFXREVERB", "Enable", CUnionValueProperty::Boolean, "DSP - SFXREVERB", TRUE },
	{ "DSP - LOWPASS_SIMPLE", "Enable", CUnionValueProperty::Boolean, "DSP - LOWPASS_SIMPLE", TRUE },
	*/
	{ NULL },
};

PropertyGridBaseDefine g_SoundEnviPropertyDefine[] = {
	{ "Common", "Count", CUnionValueProperty::Integer, "갯수", FALSE },
	{ NULL },
};

PropertyGridBaseDefine g_SoundPropListPropertyDefine[] = {
	{ "Common", "Sound Prop Count", CUnionValueProperty::Integer, "프랍 사운드 갯수", FALSE },
	{ NULL },
};

PropertyGridBaseDefine g_SoundEnviItemPropertyDefine[] = {
	{ "Common", "File Name", CUnionValueProperty::String_FileOpen, "파일 이름", FALSE },
	{ "Common", "Position", CUnionValueProperty::Vector3, "포지션", TRUE },
	{ "Common", "Range", CUnionValueProperty::Float, "반지름", TRUE },
	{ "Common", "RollOff", CUnionValueProperty::Float, "감쇠 시작 범위|0.f|1.f|0.001f", TRUE },
	{ "Common", "Volume", CUnionValueProperty::Float, "볼륨|0.f|1.f|0.001f", TRUE },
	{ "Common", "Stream", CUnionValueProperty::Boolean, "스트림 플레이?", TRUE },
	{ NULL },
};


IMPLEMENT_DYNCREATE(CSoundPropertyPaneView, CFormView)

CSoundPropertyPaneView::CSoundPropertyPaneView()
	: CFormView(CSoundPropertyPaneView::IDD)
{
	m_bActivate = false;
	m_nSelectType = -1;
}

CSoundPropertyPaneView::~CSoundPropertyPaneView()
{
	ResetPropertyGrid();
}

void CSoundPropertyPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundPropertyPaneView, CFormView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CSoundPropertyPaneView diagnostics

#ifdef _DEBUG
void CSoundPropertyPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSoundPropertyPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSoundPropertyPaneView message handlers

void CSoundPropertyPaneView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );
	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

BOOL CSoundPropertyPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
	//	return __super::OnEraseBkgnd(pDC);
}

void CSoundPropertyPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CSoundPropertyPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
	m_nSelectType = -1;

	if( wParam == 0 ) return S_OK;

	PropertyGridBaseDefine *pDefine = NULL;
	switch( wParam ) {
		case 1: pDefine = g_SoundRootPropertyDefine;break;
		case 2: 
			{
				pDefine = g_SoundBGMPropertyDefine;
				/*
				static std::vector<CUnionValueProperty *> VecList;
				CUnionValueProperty *pVariable;
				pDefine = g_SoundBGMPropertyDefine;
				for( int i=0; ; i++ ) {
					if( pDefine[i].szCategory == NULL ) break;
					pVariable = new CUnionValueProperty( pDefine[i].Type );
					pVariable->SetCategory( pDefine[i].szCategory );
					pVariable->SetDescription( pDefine[i].szStr );
					pVariable->SetSubDescription( pDefine[i].szDescription );
					pVariable->SetDefaultEnable( pDefine[i].bEnable );
					VecList.push_back( pVariable );
				}

				int nDspList[] = {
					FMOD_DSP_TYPE_MIXER,
					FMOD_DSP_TYPE_OSCILLATOR,
					FMOD_DSP_TYPE_LOWPASS,
					FMOD_DSP_TYPE_ITLOWPASS,
					FMOD_DSP_TYPE_HIGHPASS,
					FMOD_DSP_TYPE_ECHO,
					FMOD_DSP_TYPE_FLANGE,
					FMOD_DSP_TYPE_DISTORTION,
					FMOD_DSP_TYPE_NORMALIZE,
					FMOD_DSP_TYPE_PARAMEQ,
					FMOD_DSP_TYPE_PITCHSHIFT,
					FMOD_DSP_TYPE_CHORUS,
					FMOD_DSP_TYPE_REVERB,
					FMOD_DSP_TYPE_VSTPLUGIN,
					FMOD_DSP_TYPE_WINAMPPLUGIN,
					FMOD_DSP_TYPE_ITECHO,
					FMOD_DSP_TYPE_COMPRESSOR,
					FMOD_DSP_TYPE_SFXREVERB,
					FMOD_DSP_TYPE_LOWPASS_SIMPLE,
					-1,
				};
				char *szDspList[] = {
					"MIXER",
					"OSCILLATOR",
					"LOWPASS",
					"ITLOWPASS",
					"HIGHPASS",
					"ECHO",
					"FLANGE",
					"DISTORTION",
					"NORMALIZE",
					"PARAMEQ",
					"PITCHSHIFT",
					"CHORUS",
					"REVERB",
					"VSTPLUGIN",
					"WINAMPPLUGIN",
					"ITECHO",
					"COMPRESSOR",
					"SFXREVERB",
					"LOWPASS_SIMPLE",
					NULL,
				};

				CEtSoundEngine *pEngine = CRenderBase::GetInstance().GetSoundEngine();
				FMOD::System *pSystem = pEngine->GetFMODSystem();
				FMOD::DSP *pDsp;

				CString szStr, szSubDesc;
				char szName[256], szLabel[256], szDesc[512];
				float fMin, fMax;
				int nParamCount;
				for( int i=0;; i++ ) {
					if( nDspList[i] == -1 ) break;
					pVariable = new CUnionValueProperty( CUnionValueProperty::Boolean );
					szStr.Format( "DSP - %s",  szDspList[i] );
					pVariable->SetCategory( szStr );
					pVariable->SetDescription( "Enable" );
					pVariable->SetDefaultEnable( TRUE );

					if( pSystem->createDSPByType( (FMOD_DSP_TYPE)nDspList[i], &pDsp ) != FMOD_OK ) {
						SAFE_DELETE( pVariable );
						continue;
					}
					VecList.push_back( pVariable );

					pDsp->getNumParameters( &nParamCount );
					for( int j=0; j<nParamCount; j++ ) {
						memset( szName, 0, sizeof(szName) );
						memset( szLabel, 0, sizeof(szLabel) );
						memset( szDesc, 0, sizeof(szDesc) );
						pDsp->getParameterInfo( j, szName, szLabel, szDesc, 512, &fMin, &fMax );

						pVariable = new CUnionValueProperty( CUnionValueProperty::Float );
						pVariable->SetCategory( szStr );
						pVariable->SetDescription( szName );
						szSubDesc.Format( "%s|%.2f|%.2f|0.0001f", szDesc, fMin, fMax );
						pVariable->SetSubDescription( szSubDesc.GetBuffer(), true );
						pVariable->SetDefaultEnable( TRUE );
						VecList.push_back( pVariable );
					}
					pDsp->release();

				}
				RefreshPropertyGrid( &VecList );

				return S_OK;
				*/
			}
			break;
		case 3: pDefine = g_SoundEnviPropertyDefine;	break;
		case 4: pDefine = g_SoundPropListPropertyDefine;	break;
		case 5: pDefine = g_SoundEnviItemPropertyDefine;	break;
		default:
			break;
	}
	m_nSelectType = (int)wParam;
	RefreshPropertyGrid( pDefine );

	return S_OK;
}

LRESULT CSoundPropertyPaneView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CSoundPropertyPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( m_nSelectType ) {
		case 1:	// Sound Property
			switch( dwIndex ) {
				case 0:
					pVariable->SetVariable( CGlobalValue::GetInstance().m_bPlaySound );
					break;
				case 1:
					pVariable->SetVariable( CGlobalValue::GetInstance().m_fMasterVolume );
					break;
			}
			break;
		case 2:	// BGM Property
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
				if( pSound == NULL ) break;

				pSound->OnSetValue( pVariable, dwIndex );
			}
			break;
		case 3:	// Envi Property
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
				if( pSound == NULL ) break;
				switch( dwIndex ) {
					case 0:
						pVariable->SetVariable( (int)pSound->GetEnviCount() );
						break;
				}
			}
			break;
		case 4:	// PropList Property
			break;
		case 5:	// Envi Item
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
				if( pSound == NULL ) break;
				CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)CGlobalValue::GetInstance().m_pSelectSound;
				if( pEnvi == NULL ) break;

				pEnvi->OnSetValue( pVariable, dwIndex );
			}
			break;
	}
}

void CSoundPropertyPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( m_nSelectType ) {
		case 1:	// Sound Property
			switch( dwIndex ) {
				case 0:
					{
						CGlobalValue::GetInstance().m_bPlaySound = pVariable->GetVariableBool();

						SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
						if( Sector == -1 ) break;
						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
						if( pSector == NULL ) break;
						CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
						if( pSound == NULL ) break;

						pSound->Stop();
						if( pVariable->GetVariableBool() == true ) pSound->Play();
					}
					break;
				case 1:
					{
						SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
						if( Sector == -1 ) break;
						CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
						if( pSector == NULL ) break;
						CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
						if( pSound == NULL ) break;

						CGlobalValue::GetInstance().m_fMasterVolume = pVariable->GetVariableFloat();

						pSound->SetVolume( pSound->GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
						for( DWORD i=0; i<pSound->GetEnviCount(); i++ ) {
							CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)pSound->GetEnviFromIndex(i);
							pEnvi->SetVolume( pEnvi->GetVolume() * CGlobalValue::GetInstance().m_fMasterVolume );
						}
					}
					break;
			}
			break;
		case 2:	// BGM Property
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
				if( pSound == NULL ) break;
				pSound->OnChangeValue( pVariable, dwIndex );
			}
			break;
		case 3:	// Envi Property
			break;
		case 4:	// PropList Property
			break;
		case 5:	// Envi Item
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;
				CTEtWorldSound *pSound = (CTEtWorldSound *)pSector->GetSoundInfo();
				if( pSound == NULL ) break;
				CTEtWorldSoundEnvi *pEnvi = (CTEtWorldSoundEnvi *)CGlobalValue::GetInstance().m_pSelectSound;
				if( pEnvi == NULL ) break;

				pEnvi->OnChangeValue( pVariable, dwIndex );
			}
			break;
	}
	CGlobalValue::GetInstance().SetModify();
}

void CSoundPropertyPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}
