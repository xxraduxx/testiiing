// MakeMinimapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "MakeMinimapDlg.h"
#include "EnviControl.h"
#include "TEtWorld.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "FileServer.h"
#include "ImagePrevDlg.h"


// CMakeMinimapDlg dialog

IMPLEMENT_DYNAMIC(CMakeMinimapDlg, CDialog)

CMakeMinimapDlg::CMakeMinimapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMakeMinimapDlg::IDD, pParent)
	, m_bRenderTerrain(FALSE)
	, m_bRenderProp(FALSE)
	, m_bAttributeAlpha(FALSE)
{

}

CMakeMinimapDlg::~CMakeMinimapDlg()
{
}

void CMakeMinimapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ResXCombo);
	DDX_Control(pDX, IDC_COMBO3, m_ResYCombo);
	DDX_Control(pDX, IDC_COMBO2, m_EnviCombo);
	DDX_Check(pDX, IDC_CHECK1, m_bRenderTerrain);
	DDX_Check(pDX, IDC_CHECK2, m_bRenderProp);
	DDX_Check(pDX, IDC_CHECK3, m_bAttributeAlpha);
	DDX_Control(pDX, IDC_CHECK1, m_RenderTerrain);
	DDX_Control(pDX, IDC_CHECK2, m_RenderProp);
	DDX_Control(pDX, IDC_CHECK3, m_AttributeAlpha);
}


BEGIN_MESSAGE_MAP(CMakeMinimapDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMakeMinimapDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMakeMinimapDlg message handlers

BOOL CMakeMinimapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_RenderTerrain.SetCheck( TRUE );
	m_RenderProp.SetCheck( TRUE );
	m_AttributeAlpha.SetCheck( TRUE );
	m_ResXCombo.SetCurSel(3);
	m_ResYCombo.SetCurSel(3);
	m_EnviCombo.SetCurSel(0);

	CEnviElement *pActive = CEnviControl::GetInstance().GetActiveElement();
	for( DWORD i=0; i<CEnviControl::GetInstance().GetCount(); i++ ) {
		CEnviElement *pElement = CEnviControl::GetInstance().GetElement(i);
		m_EnviCombo.InsertString( m_EnviCombo.GetCount(), pElement->GetInfo().GetName() );
		if( pActive ) {
			if( strcmp( pActive->GetInfo().GetName(), pElement->GetInfo().GetName() ) == NULL ) {
				m_EnviCombo.SetCurSel(i);
			}
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMakeMinimapDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
	if( Sector == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
	if( !pSector ) return;

	CString szFileList;
	TCHAR szFilter[] = _T( "All Support File(*.dds;*.tga;*.jpg;*.bmp)|*.dds;*.tga;*.jpg;*.bmp|DirectX Texture File (*.dds)|*.dds|Bitmap File (*.bmp)|*.bmp|Tga File (*.tga)|*.tga|Jpeg Texture File (*.jpg)|*.jpg|Png Texture File (*.png)|*.png|All Files (*.*)|*.*||" );

	CImagePrevDlg dlg(FALSE, _T("dds;tga;jpg;bmp;png"), _T("*.dds;*.tga;*.jpg;*.bmp;*.png"), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING, szFilter );

	dlg.SetExportMode();
	if( dlg.DoModal() != IDOK ) return;

	CString szSelectEnvi, szResX, szResY;
	m_EnviCombo.GetLBText( m_EnviCombo.GetCurSel(), szSelectEnvi );
	m_ResXCombo.GetLBText( m_ResXCombo.GetCurSel(), szResX );
	m_ResYCombo.GetLBText( m_ResYCombo.GetCurSel(), szResY );
	int nWidth = atoi(szResX);
	int nHeight = atoi(szResY);

	CString szFileName = dlg.GetPathName();

	pSector->MakeMinimap( nWidth, nHeight, szFileName, szSelectEnvi, ( m_RenderTerrain.GetCheck() == TRUE ), ( m_RenderProp.GetCheck() == TRUE ), ( m_AttributeAlpha.GetCheck() == TRUE ) );

	OnOK();
}
