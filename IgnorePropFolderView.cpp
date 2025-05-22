// IgnorePropFolderView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "IgnorePropFolderView.h"
#include "UserMessage.h"
#include "TEtWorldSector.h"
#include "TEtWorld.h"


// CIgnorePropFolderView
PropertyGridBaseDefine g_IgnorePropFolderPropertyDefine[] = {
	{ "Property", "Ignore Prop Folder", CUnionValueProperty::String, "무시할 프랍폴더 리스트. 구분자는 ;입니다.", TRUE },
	{ NULL },
};

IMPLEMENT_DYNCREATE(CIgnorePropFolderView, CFormView)

CIgnorePropFolderView::CIgnorePropFolderView()
	: CFormView(CIgnorePropFolderView::IDD)
{
	m_bActivate = false;
}

CIgnorePropFolderView::~CIgnorePropFolderView()
{
	ResetPropertyGrid();
}

void CIgnorePropFolderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIgnorePropFolderView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE( XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CIgnorePropFolderView 진단입니다.

#ifdef _DEBUG
void CIgnorePropFolderView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CIgnorePropFolderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CIgnorePropFolderView 메시지 처리기입니다.

void CIgnorePropFolderView::OnInitialUpdate()
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

void CIgnorePropFolderView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CIgnorePropFolderView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();

	//	PropertyGridBaseDefine *pDefine = NULL;
	RefreshPropertyGrid( g_IgnorePropFolderPropertyDefine );

	return S_OK;
}

LRESULT CIgnorePropFolderView::OnGridNotify( WPARAM wParam, LPARAM lParam )
{
	return ProcessNotifyGrid( wParam, lParam );
}

void CIgnorePropFolderView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	switch( dwIndex ) {
		case 0:
			CString szStr;
			std::vector<std::string> &vecIgnorePropFolder = pSector->GetIgnorePropFolderList();
			for( int i = 0; i < (int)vecIgnorePropFolder.size(); ++i ) {
				szStr += vecIgnorePropFolder[i].c_str();
				if( i < (int)vecIgnorePropFolder.size()-1 )
					szStr += ";";
			}
			pVariable->SetVariable( szStr.GetBuffer() );
			break;
	}
}

void CIgnorePropFolderView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	SectorIndex Index = CGlobalValue::GetInstance().m_SelectGrid;
	if( Index == -1 ) return;
	CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Index );
	if( pSector == NULL ) return;

	switch( dwIndex ) {
		case 0:
			std::vector<std::string> vecIgnorePropFolder;
			std::string szTemp = pVariable->GetVariableString();
			ToLowerA( szTemp );
			TokenizeA( szTemp, vecIgnorePropFolder, ";" );
			pSector->SetIgnorePropFolderList( vecIgnorePropFolder );
			pSector->ShowProp( true );
			break;
	}

	CGlobalValue::GetInstance().SetModify();
}