#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SoundPrevDlg.h"
#include "resource.h"
#include <process.h>
#include "RenderBase.h"
#include "fmod.hpp"
#include "EtSoundEngine.h"


// CSoundPrevDlg
HANDLE CSoundPrevDlg::s_hThreadHandle = NULL;
int CSoundPrevDlg::s_nThreadStatus = 0;

IMPLEMENT_DYNAMIC(CSoundPrevDlg, CFileDialog)

CSoundPrevDlg::CSoundPrevDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SOUNDPREVDLG);

	s_nThreadStatus = 0;
	m_dwThreadIndex = 0;
	m_pChannel = NULL;
	m_pSound = NULL;
}

CSoundPrevDlg::~CSoundPrevDlg()
{
}


BEGIN_MESSAGE_MAP(CSoundPrevDlg, CFileDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_PLAYBUTTON, &CSoundPrevDlg::OnBnClickedPlay)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CSoundPrevDlg message handlers


BOOL CSoundPrevDlg::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

UINT __stdcall CSoundPrevDlg::BeginThread( void *pParam )
{
	CSoundPrevDlg *pThis = (CSoundPrevDlg *)pParam;
	if( CSoundPrevDlg::s_hThreadHandle == NULL ) {
		pThis->s_nThreadStatus = -1;
		_endthreadex( 0 );
		return 0;
	}

	if( CSoundPrevDlg::s_nThreadStatus == 0 ) {
		CSoundPrevDlg::s_nThreadStatus = 1;
		CString szFullName = pThis->GetPathName().GetString();

		FILE *fp;
		fopen_s( &fp, szFullName, "rb" );
		if( fp == NULL ) {
			pThis->s_nThreadStatus = -1;
			_endthreadex( 0 );
			return 0;
		}
		fclose(fp);

		FMOD::System *pSystem = CRenderBase::GetInstance().GetSoundEngine()->GetFMODSystem();
		if( pThis->m_pChannel ) {
			pThis->m_pChannel->stop();
			pThis->m_pChannel = NULL;
		}
		if( pThis->m_pSound ) {
			pThis->m_pSound->release();
			pThis->m_pChannel = NULL;
		}
		FMOD_SOUND_TYPE Type;
		FMOD_SOUND_FORMAT Format;
		int nChannel;
		int nBits;
		unsigned int nLength;
		if( pSystem->createStream( szFullName.GetBuffer(), FMOD_DEFAULT, NULL, &pThis->m_pSound ) == FMOD_OK ) {
			pThis->m_pSound->getFormat( &Type, &Format, &nChannel, &nBits );
			pThis->m_pSound->getLength( &nLength, FMOD_TIMEUNIT_MS );

			pThis->m_szFileInfo = "";

			pThis->m_szFileInfo = "Type : ";
			switch( Format ) {
				case FMOD_SOUND_FORMAT_NONE:	pThis->m_szFileInfo += "Unknown";	break;
				case FMOD_SOUND_FORMAT_PCM8:	pThis->m_szFileInfo += "PCM8";	break;
				case FMOD_SOUND_FORMAT_PCM16:	pThis->m_szFileInfo += "PCM16";	break;
				case FMOD_SOUND_FORMAT_PCM24:	pThis->m_szFileInfo += "PCM24";	break;
				case FMOD_SOUND_FORMAT_PCM32:	pThis->m_szFileInfo += "PCM32";	break;
				case FMOD_SOUND_FORMAT_PCMFLOAT:pThis->m_szFileInfo += "PCMFLOAT";	break;
				case FMOD_SOUND_FORMAT_GCADPCM:	pThis->m_szFileInfo += "GameCube DSP";	break;
				case FMOD_SOUND_FORMAT_IMAADPCM:pThis->m_szFileInfo += "ADPCM";	break;
				case FMOD_SOUND_FORMAT_VAG:		pThis->m_szFileInfo += "PS2";	break;
				case FMOD_SOUND_FORMAT_XMA:		pThis->m_szFileInfo += "XBox360";	break;
				case FMOD_SOUND_FORMAT_MPEG:	pThis->m_szFileInfo += "MPEG2";	break;
			}
			pThis->m_szFileInfo += "\nFormat : ";
			switch( Type ) {
				case FMOD_SOUND_TYPE_UNKNOWN:	pThis->m_szFileInfo += "Unknown";	break;
				case FMOD_SOUND_TYPE_AAC:		pThis->m_szFileInfo += "AAC";	break;
				case FMOD_SOUND_TYPE_AIFF:      pThis->m_szFileInfo += "AIFF";	break;
				case FMOD_SOUND_TYPE_ASF:       pThis->m_szFileInfo += "Microsoft Advanced Systems Format";break;
				case FMOD_SOUND_TYPE_AT3:       pThis->m_szFileInfo += "Sony ATRAC 3 format";	break;
				case FMOD_SOUND_TYPE_CDDA:      pThis->m_szFileInfo += "Digital CD audio";	break;
				case FMOD_SOUND_TYPE_DLS:       pThis->m_szFileInfo += "Sound font";	break;
				case FMOD_SOUND_TYPE_FLAC:      pThis->m_szFileInfo += "FLAC lossless codec";	break;
				case FMOD_SOUND_TYPE_FSB:       pThis->m_szFileInfo += "FMOD Sample Bank";	break;
				case FMOD_SOUND_TYPE_GCADPCM:   pThis->m_szFileInfo += "GameCube ADPCM";	break;
				case FMOD_SOUND_TYPE_IT:        pThis->m_szFileInfo += "Impulse Tracker";	break;
				case FMOD_SOUND_TYPE_MIDI:      pThis->m_szFileInfo += "MIDI";	break;
				case FMOD_SOUND_TYPE_MOD:       pThis->m_szFileInfo += "Protracker / Fasttracker MOD";	break;
				case FMOD_SOUND_TYPE_MPEG:      pThis->m_szFileInfo += "MP2/MP3 MPEG";	break;
				case FMOD_SOUND_TYPE_OGGVORBIS: pThis->m_szFileInfo += "Ogg vorbis";	break;
				case FMOD_SOUND_TYPE_PLAYLIST:  pThis->m_szFileInfo += "Information only from ASX/PLS/M3U/WAX playlists";	break;
				case FMOD_SOUND_TYPE_RAW:       pThis->m_szFileInfo += "Raw PCM data";	break;
				case FMOD_SOUND_TYPE_S3M:       pThis->m_szFileInfo += "ScreamTracker 3";	break;
				case FMOD_SOUND_TYPE_SF2:       pThis->m_szFileInfo += "Sound font 2 format";	break;
				case FMOD_SOUND_TYPE_USER:      pThis->m_szFileInfo += "User created sound";	break;
				case FMOD_SOUND_TYPE_WAV:       pThis->m_szFileInfo += "Microsoft WAV";	break;
				case FMOD_SOUND_TYPE_XM:        pThis->m_szFileInfo += "FastTracker 2 XM";	break;
				case FMOD_SOUND_TYPE_XMA:       pThis->m_szFileInfo += "Xbox360 XMA";	break;
				case FMOD_SOUND_TYPE_VAG:       pThis->m_szFileInfo += "PlayStation 2 / PlayStation Portable adpcm VAG format";	break;
			}

			char szTemp[64];
			sprintf_s( szTemp, "\nChannel : %d", nChannel );
			pThis->m_szFileInfo += szTemp;

			sprintf_s( szTemp, "\nBitrate : %d bit", nBits );
			pThis->m_szFileInfo += szTemp;

			int nMinute = nLength / 60000;
			nLength %= 60000;
			int nSec = nLength / 1000;
			nLength %= 1000;
			int nMil = (int)( nLength * 0.1f );
			sprintf_s( szTemp, "\nLength : %02d:%02d:%02d", nMinute, nSec, nMil );
			pThis->m_szFileInfo += szTemp;

		}


		pThis->Invalidate();
	}
	pThis->s_nThreadStatus = -1;

	_endthreadex( 0 );
	return 0;
}

void CSoundPrevDlg::RefreshPreview()
{
	if( s_hThreadHandle ) {
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;

		while(1) {
			Sleep(1);
			if( s_nThreadStatus == -1 ) break;
		}
	}

	s_nThreadStatus = 0;
	s_hThreadHandle = (HANDLE)_beginthreadex( NULL, 1000000, BeginThread, (void*)this, 0, &m_dwThreadIndex );
	while(1) {
		if( s_nThreadStatus == 1 || s_nThreadStatus == -1 ) break;
		Sleep(1);
	}
}

void CSoundPrevDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFileDialog::OnPaint()을(를) 호출하지 마십시오.


	CRect rcRect = GetPrevRect();

	dc.SetBkMode( TRANSPARENT );
	dc.SelectStockObject( ANSI_VAR_FONT );
	dc.IntersectClipRect( &rcRect ); 
	rcRect.left += 1; rcRect.top += 1;
	dc.SetTextColor( RGB( 255, 255, 255 ) );
	dc.DrawText( m_szFileInfo, rcRect, 0 );

	rcRect.left -= 1; rcRect.top -= 1;
	dc.SetTextColor( RGB( 0, 0, 0 ) );
	dc.DrawText( m_szFileInfo, rcRect, 0 );

	dc.SelectClipRgn( NULL );
}


CRect CSoundPrevDlg::GetPrevRect()
{
	CWnd *pWnd = GetDlgItem( IDC_STATIC_PREV );
	CRect rcRect, rcTemp[2];
	CPoint pt;
	pWnd->GetClientRect( &rcRect );

	GetClientRect( &rcTemp[0] );
	ClientToScreen( &rcTemp[0] );

	pWnd->GetClientRect( &rcTemp[1] );
	pWnd->ClientToScreen( &rcTemp[1] );

	pt.x = rcTemp[1].left - rcTemp[0].left;
	pt.y = rcTemp[1].top - rcTemp[0].top;

	rcTemp[0] = CRect( pt.x, pt.y, pt.x + rcRect.Width(), pt.y + rcRect.Height() );
	rcTemp[0].DeflateRect( 5, 19, 5, 5 );

	return rcTemp[0];
}
void CSoundPrevDlg::OnFileNameChange()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CFileDialog::OnFileNameChange();
	UpdateData( TRUE );

	RefreshPreview();
}


BOOL CSoundPrevDlg::OnFileNameOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	BOOL bResult = CFileDialog::OnFileNameOK();

	return bResult;
}

void CSoundPrevDlg::OnDestroy()
{
	if( s_hThreadHandle ) {
		CloseHandle( s_hThreadHandle );
		s_hThreadHandle = NULL;
		m_dwThreadIndex = 0;
		while(1) {
			Sleep(1);
			if( s_nThreadStatus == -1 ) break;
		}
	}

	if( m_pChannel ) m_pChannel->stop();
	if( m_pSound ) m_pSound->release();

	CFileDialog::OnDestroy();
}


void CSoundPrevDlg::OnBnClickedPlay()
{
	if( !m_pSound ) return;
	if( m_pChannel ) m_pChannel->stop();
	FMOD::System *pSystem = CRenderBase::GetInstance().GetSoundEngine()->GetFMODSystem();
	pSystem->playSound( FMOD_CHANNEL_FREE, m_pSound, false, &m_pChannel );
}