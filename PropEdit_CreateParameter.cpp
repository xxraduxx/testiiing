// PropEdit_CreateParameter.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropEdit_CreateParameter.h"
#include "UnionValueProperty.h"


// CPropEdit_CreateParameter

IMPLEMENT_DYNCREATE(CPropEdit_CreateParameter, CFormView)

CPropEdit_CreateParameter::CPropEdit_CreateParameter()
	: CFormView(CPropEdit_CreateParameter::IDD)
	, m_szParamName(_T(""))
	, m_szParamDesc(_T(""))
	, m_fValueMin(0)
	, m_fValueMax(0)
	, m_szValueData(_T(""))
{
	m_bActivate = false;
	m_pmemDC = NULL;
	m_pdcBitmap = NULL;

}

CPropEdit_CreateParameter::~CPropEdit_CreateParameter()
{
	DeleteMemDC();
}

void CPropEdit_CreateParameter::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ValueType);
	DDX_Text(pDX, IDC_EDIT1, m_szParamName);
	DDX_Text(pDX, IDC_EDIT5, m_szParamDesc);
	DDX_Text(pDX, IDC_EDIT2, m_fValueMin);
	DDX_Text(pDX, IDC_EDIT7, m_fValueMax);
	DDX_Text(pDX, IDC_EDIT9, m_szValueData);
}

BEGIN_MESSAGE_MAP(CPropEdit_CreateParameter, CFormView)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPropEdit_CreateParameter::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_EDIT1, &CPropEdit_CreateParameter::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CPropEdit_CreateParameter diagnostics

#ifdef _DEBUG
void CPropEdit_CreateParameter::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEdit_CreateParameter::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CPropEdit_CreateParameter::PreProcess()
{
	m_szParamName = s_pCurrentPropertyVariable->GetDescription();
	if( s_pCurrentPropertyVariable->GetSubDescription() )
		m_szParamDesc = _GetSubStrByCount( 0, (char*)s_pCurrentPropertyVariable->GetSubDescription() );
	else m_szParamDesc.Empty();

	int nCurSel = 0;
	switch( s_pCurrentPropertyVariable->GetType() ) {
		case CUnionValueProperty::Integer:
			nCurSel = 0;
			if( s_pCurrentPropertyVariable->GetSubDescription() ) {
				if( _GetSubStrByCount( 3, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) ) {
					m_fValueMin = (float)( (int)atoi( _GetSubStrByCount( 1, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) ) );
					m_fValueMax = (float)( (int)atoi( _GetSubStrByCount( 2, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) ) );
					nCurSel = 1;
				}
			}
			break;
		case CUnionValueProperty::Float:
			nCurSel = 4;
			if( s_pCurrentPropertyVariable->GetSubDescription() ) {
				if( _GetSubStrByCount( 3, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) ) {
					m_fValueMin = (float)atof( _GetSubStrByCount( 1, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) );
					m_fValueMax = (float)atof( _GetSubStrByCount( 2, (char*)s_pCurrentPropertyVariable->GetSubDescription() ) );
					nCurSel = 5;
				}
			}
			break;
		case CUnionValueProperty::Color:
			nCurSel = 2;
			break;
		case CUnionValueProperty::Integer_Combo:
			if( s_pCurrentPropertyVariable->GetSubDescription() ) {
				m_szValueData.Empty();
				for( int i=0; ; i++ ) {
					const char *pStr = _GetSubStrByCount( 1+i, (char*)s_pCurrentPropertyVariable->GetSubDescription() );
					if( pStr == NULL ) break;
					else if( i > 0 ) m_szValueData += "|";

					m_szValueData += pStr;

				}
				//				m_szValueData = _GetSubStrByCount( 1, (char*)s_pCurrentPropertyVariable->GetSubDescription() );
			}
			else m_szValueData.Empty();
			nCurSel = 3;
			break;
		case CUnionValueProperty::Vector3:
			nCurSel = 7;
			break;
		case CUnionValueProperty::Vector2:
			m_szParamDesc = (char*)s_pCurrentPropertyVariable->GetSubDescription();
			nCurSel = 6;
			break;
		case CUnionValueProperty::Vector4:
			nCurSel = 8;
			break;
		case CUnionValueProperty::Vector4Color:
			nCurSel = 9;
			break;
		case CUnionValueProperty::Boolean:
			nCurSel = 10;
			break;
		case CUnionValueProperty::String:
			nCurSel = 11;
			break;
		case CUnionValueProperty::String_FileOpen:
			if( s_pCurrentPropertyVariable->GetSubDescription() ) {
				m_szValueData.Empty();
				for( int i=0; ; i++ ) {
					const char *pStr = _GetSubStrByCount( 1+i, (char*)s_pCurrentPropertyVariable->GetSubDescription() );
					if( pStr == NULL ) break;
					else if( i > 0 ) m_szValueData += "|";

					m_szValueData += pStr;
				}
			}
			else m_szValueData.Empty();
			nCurSel = 12;
			break;
	}
	m_ValueType.SetCurSel( nCurSel );

	EnableAdditionalControl( nCurSel );
	UpdateData( FALSE );
}

bool CPropEdit_CreateParameter::PostProcess()
{
	UpdateData();
	if( m_szParamName.IsEmpty() ) {
		MessageBox( "이름을 입력해 주세요~", "에러", MB_OK );
		return false;
	}
	if( m_szParamDesc.IsEmpty() ) {
		MessageBox( "부연설명을 입력해 주세요", "에러", MB_OK );
		return false;
	}
	bool bImpossibleChar = false;
	for( int i=0; i<m_szParamName.GetLength(); i++ ) {
		if( m_szParamName[i] < 'A' || m_szParamName[i] > 'z' ) {
			bImpossibleChar = true;
			break;
		}
	}
	if( bImpossibleChar ) {
		MessageBox( "이름에 사용 불가능한 문자가 있습니다.", "에러", MB_OK );
		return false;
	}
	/*
	bImpossibleChar = false;
	for( int i=0; i<m_szParamDesc.GetLength(); i++ ) {
	if( m_szParamDesc[i] < 'A' || m_szParamDesc[i] > 'z' ) {
	bImpossibleChar = true;
	break;
	}
	}
	if( bImpossibleChar ) {
	MessageBox( "부연설명에 사용 불가능한 문자가 있습니다.", "에러", MB_OK );
	return false;
	}
	*/


	if( s_pCurrentSignalItem->IsExistParameter( m_szParamName, s_szCurrentPropertyName ) == true ) {
		MessageBox( "중복되는 이름이 있습니다.", "에러", MB_OK );
		return false;
	}


	switch( m_ValueType.GetCurSel() ) {
		case 1:
			{
				bool bFlag = true;
				if( (int)m_fValueMin >= (int)m_fValueMax ) bFlag = false;
				if( (int)m_fValueMin < INT_MIN ) bFlag = false;
				if( (int)m_fValueMin > INT_MAX ) bFlag = false;
				if( bFlag == false ) {
					MessageBox( "최대 최소값을 설정해 주세요.", "에러", MB_OK );
					return false;
				}
			}
			break;
		case 3:
			{
				bool bFlag = true;
				if( m_szValueData.IsEmpty() ) bFlag = false;

				if( bFlag == true ) {
					int nCount = 0;
					const char *pPtr;
					for( int i=0; ; i++ ) {
						pPtr = _GetSubStrByCount( i, m_szValueData.GetBuffer() );
						if( pPtr == NULL ) break;
						nCount++;
					}
					if( nCount < 2 ) bFlag = false;
				}

				if( bFlag == false ) {
					MessageBox( "콤보박스 스트링을 입력해 주세요", "에러", MB_OK );
					return false;
				}
			}
			break;
		case 5:
			{
				bool bFlag = true;
				if( m_fValueMin >= m_fValueMax ) bFlag = false;
				//				if( m_fValueMin < FLT_MIN ) bFlag = false;
				//				if( m_fValueMax > FLT_MAX ) bFlag = false;
				if( bFlag == false ) {
					MessageBox( "최대 최소값을 설정해 주세요.", "에러", MB_OK );
					return false;
				}
			}
			break;
	}

	switch( m_ValueType.GetCurSel() ) {
		case 0:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Integer );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 1:
			{
				s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Integer );
				CString szStr;
				szStr.Format( "%s|%d|%d|1", m_szParamDesc, (int)m_fValueMin, (int)m_fValueMax );
				s_pCurrentPropertyVariable->SetSubDescription( szStr.GetBuffer(), true );
			}
			break;
		case 2:
			{
				s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Color );
				s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			}
			break;
		case 3:
			{
				s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Integer_Combo );
				CString szStr;
				szStr.Format( "%s|%s", m_szParamDesc, m_szValueData );
				s_pCurrentPropertyVariable->SetSubDescription( szStr.GetBuffer(), true );
			}
			break;
		case 4:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Float );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 5:
			{
				s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Float );
				CString szStr;
				szStr.Format( "%s|%.f|%.f|0.0001f", m_szParamDesc, m_fValueMin, m_fValueMax );
				s_pCurrentPropertyVariable->SetSubDescription( szStr.GetBuffer(), true );
			}
			break;
		case 6:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Vector2 );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 7:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Vector3 );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 8:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Vector4 );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 9:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Vector4Color );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 10:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::Boolean );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 11:
			s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::String );
			s_pCurrentPropertyVariable->SetSubDescription( m_szParamDesc.GetBuffer(), true );
			break;
		case 12:
			{
				s_pCurrentPropertyVariable->ChangeType( CUnionValueProperty::String_FileOpen );
				CString szStr;
				szStr.Format( "%s|%s", m_szParamDesc, m_szValueData );
				s_pCurrentPropertyVariable->SetSubDescription( szStr.GetBuffer(), true );
			}
			break;
	}
	s_pCurrentPropertyVariable->SetDescription( m_szParamName );

	return true;
}

void CPropEdit_CreateParameter::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	m_ValueType.AddString( "INT" );
	m_ValueType.AddString( "INT - min,max" );
	m_ValueType.AddString( "INT - color" );
	m_ValueType.AddString( "INT - combo" );
	m_ValueType.AddString( "FLOAT" );
	m_ValueType.AddString( "FLOAT - min,max" );
	m_ValueType.AddString( "VECTOR2" );
	m_ValueType.AddString( "VECTOR3" );
	m_ValueType.AddString( "VECTOR4" );
	m_ValueType.AddString( "VECTOR4 - color" );
	m_ValueType.AddString( "BOOL" );
	m_ValueType.AddString( "STRING" );
	m_ValueType.AddString( "STRING - FileFind" );

	m_ValueType.SetCurSel(0);

	EnableAdditionalControl( -1 );

}

void CPropEdit_CreateParameter::CreateMemDC()
{
	if( m_pdcBitmap || m_pmemDC ) return;

	m_pmemDC = new CDC;
	m_pdcBitmap = new CBitmap;

	CRect rcRect = CRect( 0, 0, 210, 110 );

	CDC *pDC = GetDC();
	m_pmemDC->CreateCompatibleDC( pDC );
	m_pdcBitmap->CreateCompatibleBitmap( pDC, rcRect.Width(), rcRect.Height() );
	ReleaseDC( pDC );
}

void CPropEdit_CreateParameter::DeleteMemDC()
{
	if( m_pdcBitmap ) {
		m_pdcBitmap->DeleteObject();
		SAFE_DELETE( m_pdcBitmap );
	}
	if( m_pmemDC ) {
		m_pmemDC->DeleteDC();
		SAFE_DELETE( m_pmemDC );
	}
}


void CPropEdit_CreateParameter::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CreateMemDC();

	CRect rcRect = CRect( 0, 0, 210, 110 );

	CBitmap *pOldBitmap;
	pOldBitmap = m_pmemDC->SelectObject( m_pdcBitmap );

	// 배경체우구
	m_pmemDC->FillSolidRect( &rcRect, RGB(247, 243, 233) );

	// 카테고리 색 채워주고
	CRect rcCategory = CRect( 0, 0, 15, 110 );
	m_pmemDC->FillSolidRect( &rcCategory, RGB(182, 210, 189) );
	rcCategory = CRect( 0, 0, 210, 15 );
	m_pmemDC->FillSolidRect( &rcCategory, RGB(182, 210, 189) );

	// 카테코리 가로줄
	rcCategory = CRect( 0, 15, 210, 30 );
	for( int i=0; i<7; i++ ) {
		m_pmemDC->Draw3dRect( &rcCategory, RGB(182, 210, 189), RGB(182, 210, 189) );
		rcCategory.top += 28;
		rcCategory.bottom += 28;
	}
	// 카테고리 세로줄
	rcCategory = CRect( 100, 0, 210, 111 );
	m_pmemDC->Draw3dRect( &rcCategory, RGB(182, 210, 189), RGB(182, 210, 189) );
	// 보기싫은거 지워주고
	rcCategory = CRect( 209, 15, 210, 110 );
	m_pmemDC->Draw3dRect( &rcCategory, RGB(247, 243, 233), RGB(247, 243, 233) );

	// 글씨써준다.
	m_pmemDC->SetBkMode( TRANSPARENT );
	m_pmemDC->SelectStockObject( DEFAULT_GUI_FONT );

	rcCategory = CRect( 15, 2, 210, 15 );
	m_pmemDC->SetTextColor( RGB(100,100,100) );
	m_pmemDC->DrawText( "Parameter", &rcCategory, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

	rcCategory = CRect( 20, 17, 100, 29 );
	m_pmemDC->SetTextColor( RGB(50,50,50) );
	m_pmemDC->DrawText( m_szParamName, &rcCategory, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

	rcCategory = CRect( 105, 17, 200, 29 );
	CString szValue;
	switch( m_ValueType.GetCurSel() ) {
		case 0:	szValue = "0";	break;
		case 4:	szValue = "0.000";	break;
		case 1:
		case 5:
			{
				CRect rcTemp = CRect( 150, 17, 210, 28 );
				m_pmemDC->Draw3dRect( &rcTemp, RGB(0, 0, 0), RGB(255,255,255) );
				rcTemp.DeflateRect( 1, 1, 1, 1 );
				m_pmemDC->FillSolidRect( &rcTemp, RGB( 255,255,255 ) );
				rcTemp.DeflateRect( 1, 1, 40, 1 );
				m_pmemDC->FillSolidRect( &rcTemp, RGB( 80, 80, 80 ) );
				if( m_ValueType.GetCurSel() == 1 ) szValue = "0";
				else szValue = "0.000";
			}
			break;
		case 2:
		case 9:
			{
				CRect rcTemp = CRect( 104, 17, 127, 28 );
				m_pmemDC->Draw3dRect( &rcTemp, RGB(0, 0, 0), RGB(0,0,0) );
				rcTemp.DeflateRect( 1, 1, 1, 1 );
				m_pmemDC->FillSolidRect( &rcTemp, RGB( 255,0,0 ) );
				rcCategory.left += 28;

				if( m_ValueType.GetCurSel() == 2 ) szValue = "255; 0; 0";
				else szValue = "1.00; 0.00; 0.00; 1.00";

			}
			break;
		case 6:	szValue = "0.50; 0.30";	break;
		case 7:	szValue = "0.50; 0.30; 0.70";	break;
		case 8: szValue = "0.50; 0.30; 0.70; 0.90";	break;
		case 10: szValue = "True";	break;
		case 11: szValue = "Hello World!";	break;
		case 12: 
			{
				CRect rcTemp = CRect( 196, 17, 210, 28 );
				m_pmemDC->Draw3dRect( &rcTemp, RGB(255,255,255), RGB(0,0,0) );
				rcTemp.DeflateRect( 1, 1, 1, 1 );
				m_pmemDC->FillSolidRect( &rcTemp, RGB( 180,180,180 ) );

				m_pmemDC->SetTextColor( RGB(0,0,0) );
				rcTemp.DeflateRect( 1, 1, 1, 1 );
				m_pmemDC->DrawText( "...", &rcTemp, DT_SINGLELINE | DT_CENTER | DT_VCENTER );

				szValue = "Eternity.skn";
			}
			break;
		case 3:
			{
				CRect rcTemp = CRect( 105, 29, 208, 100 );
				m_pmemDC->Draw3dRect( &rcTemp, RGB(0,0,0), RGB(255,255,255) );
				rcTemp.DeflateRect( 1, 1, 1, 1 );
				m_pmemDC->FillSolidRect( &rcTemp, RGB( 255,255,255) );

				char *szTemp[] = { "Stand", "Run", "Walk", "Die", "Down" };
				rcTemp = CRect( 107, 30, 208, 41 );
				for( int i=0; i<5; i++ ) {
					m_pmemDC->SetTextColor( RGB(50,50,50) );
					m_pmemDC->DrawText( szTemp[i], &rcTemp, DT_SINGLELINE | DT_LEFT | DT_VCENTER );
					rcTemp.top += 13;
					rcTemp.bottom += 13;
				}
				szValue = "Attack";
			}
			break;
	}
	m_pmemDC->SetTextColor( RGB(50,50,50) );
	m_pmemDC->DrawText( szValue, &rcCategory, DT_SINGLELINE | DT_LEFT | DT_VCENTER );


	CRect rcStatic, rcWnd;
	CPoint cPoint;
	CWnd *pStatic = GetDlgItem( IDC_STATIC_PREVIEW );
	pStatic->GetWindowRect( &rcStatic );
	GetWindowRect( &rcWnd );
	cPoint.x = rcStatic.left - rcWnd.left;
	cPoint.y = rcStatic.top - rcWnd.top;
	cPoint += CPoint( 15, 25 );


	dc.BitBlt( cPoint.x, cPoint.y, rcRect.Width(), rcRect.Height(), m_pmemDC, 0, 0, SRCCOPY );

	m_pmemDC->SelectObject(pOldBitmap);

}

void CPropEdit_CreateParameter::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	DeleteMemDC();
	CreateMemDC();
}

void CPropEdit_CreateParameter::EnableAdditionalControl( int nCurSel )
{
	std::vector<int> nVecEnableList;

	if( nCurSel > -1 ) EnableAdditionalControl( -1 );
	switch( nCurSel ) {
		case -1:
			nVecEnableList.push_back( IDC_STATIC_ADDITIONAL );
			nVecEnableList.push_back( IDC_STATIC_MIN );
			nVecEnableList.push_back( IDC_STATIC_MAX );
			nVecEnableList.push_back( IDC_STATIC_DATA );
			nVecEnableList.push_back( IDC_EDIT2 );
			nVecEnableList.push_back( IDC_EDIT7 );
			nVecEnableList.push_back( IDC_EDIT9 );
			break;
		case 1:
		case 5:
			nVecEnableList.push_back( IDC_STATIC_ADDITIONAL );
			nVecEnableList.push_back( IDC_STATIC_MIN );
			nVecEnableList.push_back( IDC_STATIC_MAX );
			nVecEnableList.push_back( IDC_EDIT2 );
			nVecEnableList.push_back( IDC_EDIT7 );
			break;
		case 3:
		case 12:
			nVecEnableList.push_back( IDC_STATIC_ADDITIONAL );
			nVecEnableList.push_back( IDC_STATIC_DATA );
			nVecEnableList.push_back( IDC_EDIT9 );
			break;
	}
	for( DWORD i=0; i<nVecEnableList.size(); i++ ) {
		GetDlgItem( nVecEnableList[i] )->ShowWindow( ( nCurSel == -1 ) ? SW_HIDE : SW_SHOW );
	}
}
void CPropEdit_CreateParameter::OnCbnSelchangeCombo1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	EnableAdditionalControl( m_ValueType.GetCurSel() );
	UpdateData();
	Invalidate( FALSE );
}

void CPropEdit_CreateParameter::OnEnChangeEdit1()
{
	UpdateData();
	Invalidate( FALSE );
}
