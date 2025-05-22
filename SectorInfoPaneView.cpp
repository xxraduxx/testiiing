// SectorInfoPaneView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "SectorInfoPaneView.h"
#include "UserMessage.h"
#include "TEtWorld.h"
#include "TEtWorldSector.h"
#include "PaneDefine.h"
#include "MainFrm.h"



// CSectorInfoPaneView
PropertyGridBaseDefine g_SectorInfoPropertyDefine[] = {
	{ "Terrain", "Height Multiply", CUnionValueProperty::Float, "높이맵 비율(1=1cm)", TRUE },
	{ "Terrain", "TileSize", CUnionValueProperty::Float, "타일 사이즈(Unit:cm)", FALSE },
	{ "Terrain", "Sector Block Count", CUnionValueProperty::Vector2, "섹터 블럭 카운트(Unit:Count)", FALSE },
	{ "Terrain", "Sector Block Size", CUnionValueProperty::Vector2, "블럭당 사이즈(Unit:m)", FALSE },
	{ "Terrain", "Show Side Sector", CUnionValueProperty::Boolean, "인접한 섹터 보임/숨김", TRUE },
	{ "Shader", "Shader Type", CUnionValueProperty::String_FileOpen, "쉐이더 타입", TRUE },
	{ "Block Parameter", "Layer 4 Anti Stretch", CUnionValueProperty::Boolean, "4번 레이어 늘어남 방지", FALSE },
	{ "Block Parameter", "Use Detail Texture", CUnionValueProperty::Boolean, "디테일 텍스쳐 사용", FALSE },
	{ "Block Parameter", "Layer 1 Distance", CUnionValueProperty::Float, "1번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 2 Distance", CUnionValueProperty::Float, "2번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 3 Distance", CUnionValueProperty::Float, "3번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 4 Distance", CUnionValueProperty::Float, "4번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 5 Distance", CUnionValueProperty::Float, "5번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 6 Distance", CUnionValueProperty::Float, "6번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 7 Distance", CUnionValueProperty::Float, "7번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 8 Distance", CUnionValueProperty::Float, "8번 레이어 텍스쳐 길이", FALSE },
	{ "Block Parameter", "Layer 1 Rotation", CUnionValueProperty::Float, "1번 레이어 회전량|0.f|360.f|0.1f", FALSE },
	{ "Block Parameter", "Layer 2 Rotation", CUnionValueProperty::Float, "2번 레이어 회전량|0.f|360.f|0.1f", FALSE },
	{ "Block Parameter", "Layer 3 Rotation", CUnionValueProperty::Float, "3번 레이어 회전량|0.f|360.f|0.1f", FALSE },
	{ "Block Parameter", "Layer 4 Rotation", CUnionValueProperty::Float, "4번 레이어 회전량|0.f|360.f|0.1f", FALSE },
	{ NULL },
};


IMPLEMENT_DYNCREATE(CSectorInfoPaneView, CFormView)

CSectorInfoPaneView::CSectorInfoPaneView()
	: CFormView(CSectorInfoPaneView::IDD)
{
	m_bActivate = false;
	m_dwShaderParamCount = 0;
	m_pVecPropertyList.clear();
}

CSectorInfoPaneView::~CSectorInfoPaneView()
{
	ResetPropertyGrid();
	SAFE_DELETE_PVEC( m_pVecPropertyList );
}

void CSectorInfoPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSectorInfoPaneView, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE( UM_REFRESH_PANE_VIEW, OnRefresh )
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()


// CSectorInfoPaneView 진단입니다.

#ifdef _DEBUG
void CSectorInfoPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSectorInfoPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSectorInfoPaneView 메시지 처리기입니다.

void CSectorInfoPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( m_bActivate == true ) return;
	m_bActivate = true;

	CRect rcRect;
	GetClientRect( &rcRect );
	m_PropertyGrid.Create( rcRect, this, 0 );
	m_PropertyGrid.SetCustomColors( RGB(200, 200, 200), 0, RGB(182, 210, 189), RGB(247, 243, 233), 0 );

	SendMessage( UM_REFRESH_PANE_VIEW );
}

void CSectorInfoPaneView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( m_PropertyGrid ) {
		m_PropertyGrid.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

LRESULT CSectorInfoPaneView::OnRefresh( WPARAM wParam, LPARAM lParam )
{
	ResetPropertyGrid();
/*
	m_dwShaderParamCount = 0;
	SAFE_DELETE_PVEC( m_pVecPropertyList );
	CUnionValueProperty *pVariable;
	// Default 설정 집어넣쿠.
	for( DWORD i=0; ; i++ ) {
		if( g_SectorInfoPropertyDefine[i].szCategory == NULL ) break;
		pVariable = new CUnionValueProperty( g_SectorInfoPropertyDefine[i].Type );
		pVariable->SetDescription( g_SectorInfoPropertyDefine[i].szStr );
		pVariable->SetSubDescription( g_SectorInfoPropertyDefine[i].szDescription );
		pVariable->SetCategory( g_SectorInfoPropertyDefine[i].szCategory );
		pVariable->SetDefaultEnable( g_SectorInfoPropertyDefine[i].bEnable );
		m_pVecPropertyList.push_back( pVariable );
	}


	RefreshPropertyGrid( &m_pVecPropertyList );
*/
	RefreshPropertyGrid( g_SectorInfoPropertyDefine );
	return S_OK;
}


void CSectorInfoPaneView::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:	// Multiply
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					pVariable->SetVariable( pSector->GetHeightMultiply() );
				}
				SetReadOnly( pVariable, !CGlobalValue::GetInstance().IsCheckOutMe() );
			}
			break;
		case 1:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					pVariable->SetVariable( pSector->GetTileSize() );
				}
			}
			break;
		case 2:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					int nX, nY;
					pSector->GetBlockCount( nX, nY );
					pVariable->SetVariable( EtVector2( (float)nX, (float)nY ) );
				}
			}
			break;
		case 3:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					int nX, nY;
					pSector->GetBlockSize( nX, nY );
					nX *= (int)( pSector->GetTileSize() / 100.f );
					nY *= (int)( pSector->GetTileSize() / 100.f );
					pVariable->SetVariable( EtVector2( (float)nX, (float)nY ) );
				}
			}
			break;
		case 4:	// Show Side Sector
			{
				pVariable->SetVariable( CGlobalValue::GetInstance().m_bLockSideHeight );
			}
			break;
		case 5:	// Shader Name
			pVariable->SetVariable( "LayeredTerrain.fx" );
			break;
		case 6:	// Stretch
			{
				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) pVariable->SetDefaultEnable( FALSE );
				else pVariable->SetDefaultEnable( TRUE );

				if( pVecList->size() > 0 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector == -1 ) break;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector == NULL ) break;

					int nTrue = 0;
					int nFalse = 0;
					for( DWORD i=0; i<pVecList->size(); i++ ) {
						int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
						if( nBlockType == TT_NORMAL || nBlockType == TT_DETAILNORMAL ) nFalse++;
						else nTrue++;
					}
					if( nTrue >= nFalse )
						pVariable->SetVariable( true );
					else pVariable->SetVariable( false );
				}
			}
			break;
		case 7:	// Detail Texture
			{
				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) pVariable->SetDefaultEnable( FALSE );
				else pVariable->SetDefaultEnable( TRUE );

				if( pVecList->size() > 0 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector == -1 ) break;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector == NULL ) break;

					int nTrue = 0;
					int nFalse = 0;
					for( DWORD i=0; i<pVecList->size(); i++ ) {
						int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
						if( nBlockType == TT_NORMAL || nBlockType == TT_CLIFF ) nFalse++;
						else nTrue++;
					}
					if( nTrue >= nFalse )
						pVariable->SetVariable( true );
					else pVariable->SetVariable( false );
				}
			}
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			{
				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) pVariable->SetDefaultEnable( FALSE );
				else pVariable->SetDefaultEnable( TRUE );

				if( pVecList->size() > 0 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector == -1 ) break;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector == NULL ) break;

					float fDistance = 0.f;
					int nCount = 0;
					int nFalse = 0;
					for( DWORD i=0; i<pVecList->size(); i++ ) {
						if( (int)dwIndex - 8 >= pSector->GetTextureCount( (*pVecList)[i] ) ) {
							pVariable->SetDefaultEnable( FALSE );
							continue;
						}
						fDistance += pSector->GetTextureDistance( (*pVecList)[i], dwIndex - 8 );
						nCount++;
					}
					if( nCount > 0 ) fDistance /= nCount;
					pVariable->SetVariable( fDistance );
				}
				SetReadOnly( dwIndex, !pVariable->IsDefaultEnable() );
			}
			break;
		case 16:
		case 17:
		case 18:
		case 19:
			{
				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) pVariable->SetDefaultEnable( FALSE );
				else pVariable->SetDefaultEnable( TRUE );

				if( pVecList->size() > 0 ) {
					SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
					if( Sector == -1 ) break;
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( pSector == NULL ) break;

					float fRotation = 0.f;
					int nCount = 0;
					for( DWORD i=0; i<pVecList->size(); i++ ) {
						if( (int)dwIndex - 16 >= pSector->GetTextureCount( (*pVecList)[i] ) ) {
							pVariable->SetDefaultEnable( FALSE );
							continue;
						}
						fRotation += pSector->GetTextureRotation( (*pVecList)[i], dwIndex - 16 );
						nCount++;
					}
					if( nCount > 0 ) fRotation /= nCount;
					pVariable->SetVariable( fRotation );
				}
				SetReadOnly( dwIndex, !pVariable->IsDefaultEnable() );
			}
			break;

	};

	// Shader Param
}

void CSectorInfoPaneView::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
	switch( dwIndex ) {
		case 0:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					if( MessageBox( "정말 바꿀래?", "오래걸려", MB_YESNO ) == IDYES ) {
						CActionElementModifyHeightMultiply *pAction = new CActionElementModifyHeightMultiply( pSector );
						pAction->ModifyHeightMultiply( pVariable->GetVariableFloat() );

						pSector->SetHeightMultiply( pVariable->GetVariableFloat() );

						pAction->AddAction();

						CGlobalValue::GetInstance().SetModify();
					}
					else pVariable->SetVariable( pSector->GetHeightMultiply() );
				}
			}
			break;
		case 4:	// Show Side Sector
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector != -1 ) {
					CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
					SectorIndex SideSector[8];
					SideSector[0] = SectorIndex( Sector.nX - 1, Sector.nY - 1 );
					SideSector[1] = SectorIndex( Sector.nX, Sector.nY - 1 );
					SideSector[2] = SectorIndex( Sector.nX + 1, Sector.nY - 1 );
					SideSector[3] = SectorIndex( Sector.nX - 1, Sector.nY );
					SideSector[4] = SectorIndex( Sector.nX + 1, Sector.nY );
					SideSector[5] = SectorIndex( Sector.nX - 1, Sector.nY + 1 );
					SideSector[6] = SectorIndex( Sector.nX, Sector.nY + 1 );
					SideSector[7] = SectorIndex( Sector.nX + 1, Sector.nY + 1 );

					for( int i=0; i<8; i++ ) {
						if( SideSector[i].nX < 0 || SideSector[i].nX >= (int)CTEtWorld::GetInstance().GetGridX() ) continue;
						if( SideSector[i].nY < 0 || SideSector[i].nY >= (int)CTEtWorld::GetInstance().GetGridY() ) continue;

						CTEtWorldSector *pSideSector = CTEtWorld::GetInstance().GetSector( SideSector[i] );
						if( pSideSector->IsEmpty() ) continue;
						if( pVariable->GetVariableBool() == true ) {
							CGlobalValue::GetInstance().m_bLockSideHeight = true;
							if( pSideSector->LoadSector( CTEtWorldSector::SRL_TERRAIN ) == true ) {
								pSideSector->Show( true );
								if( !CGlobalValue::GetInstance().IsCheckOutMe() ) continue;
								CRect rcRect = CRect( -1, -1, -1, -1 );

								// 사이드 마춰준다.
								switch(i) {
									case 1:	// 위
									case 6: // 아래
										{
											int nSourceCount = pSector->GetTileWidthCount();
											int nTargetCount = pSideSector->GetTileWidthCount();

											short *pSourceHeight = pSector->GetHeightPtr();
											short *pTargetHeight = pSideSector->GetHeightPtr();
											float fRatio = pSideSector->GetTileHeightCount() / (float)pSector->GetTileHeightCount();
											float fMultiplyRatio = pSideSector->GetHeightMultiply() / pSector->GetHeightMultiply();

											if( i == 1 ) {
												for( int i=0; i<nSourceCount; i++ ) {
													int nSourceIndex = i;
													int nTargetIndex = (int)( ( pSideSector->GetTileHeightCount() - 1 ) * pSideSector->GetTileWidthCount() + ( i * fRatio ) );
													int nValue = (int)( pTargetHeight[ nTargetIndex ] * fMultiplyRatio );
													if( nValue < -32767 ) nValue = -32767;
													else if( nValue > 32767 ) nValue = 32767;
													pSourceHeight[ nSourceIndex ] = nValue;
												}
												rcRect = CRect( 0, 0, pSector->GetTileWidthCount(), 1 );
											}
											else if( i == 6 ) {
												for( int i=0; i<nSourceCount; i++ ) {
													int nSourceIndex = ( pSector->GetTileHeightCount() - 1 ) * pSector->GetTileWidthCount() + i ;
													int nTargetIndex = (int)( i * fRatio );
													int nValue = (int)( pTargetHeight[ nTargetIndex ] * fMultiplyRatio );
													if( nValue < -32767 ) nValue = -32767;
													else if( nValue > 32767 ) nValue = 32767;
													pSourceHeight[ nSourceIndex ] = nValue;
												}
												rcRect = CRect( 0, pSector->GetTileHeightCount() - 1, pSector->GetTileWidthCount(), pSector->GetTileHeightCount() );
											}
										}
										break;
									case 3: // 왼쪽
									case 4: // 오른쪽
										{
											int nSourceCount = pSector->GetTileHeightCount();
											int nTargetCount = pSideSector->GetTileHeightCount();

											short *pSourceHeight = pSector->GetHeightPtr();
											short *pTargetHeight = pSideSector->GetHeightPtr();
											float fRatio = pSideSector->GetTileWidthCount() / (float)pSector->GetTileWidthCount();
											float fMultiplyRatio = pSideSector->GetHeightMultiply() / pSector->GetHeightMultiply();

											if( i == 3 ) {
												for( int i=0; i<nSourceCount; i++ ) {
													int nSourceIndex = i * pSector->GetTileWidthCount() ;
													int nTargetIndex = (int)( ( i * fRatio ) * (float)pSideSector->GetTileWidthCount() + ( pSideSector->GetTileWidthCount() - 1 ) );
													int nValue = (int)( pTargetHeight[ nTargetIndex ] * fMultiplyRatio );
													if( nValue < -32767 ) nValue = -32767;
													else if( nValue > 32767 ) nValue = 32767;
													pSourceHeight[ nSourceIndex ] = nValue;
												}
												rcRect = CRect( 0, 0, 1, pSector->GetTileHeightCount() );
											}
											else if( i == 4 ) {
												for( int i=0; i<nSourceCount; i++ ) {
													int nSourceIndex = i * pSector->GetTileWidthCount() + ( pSector->GetTileWidthCount() - 1 );
													int nTargetIndex = (int)( ( i * fRatio ) * (float)pSideSector->GetTileWidthCount() );
													int nValue = (int)( pTargetHeight[ nTargetIndex ] * fMultiplyRatio );
													if( nValue < -32767 ) nValue = -32767;
													else if( nValue > 32767 ) nValue = 32767;
													pSourceHeight[ nSourceIndex ] = nValue;
												}
												rcRect = CRect( pSector->GetTileWidthCount() - 1, 0, pSector->GetTileWidthCount(), pSector->GetTileHeightCount() );
											}
										}
										break;
								}
								pSector->UpdateHeight( rcRect );
								pSector->SetLastUpdateRect( rcRect );
								pSector->ResetUpdateInfo();
							}							
						}
						else {
							CGlobalValue::GetInstance().m_bLockSideHeight = false;
							pSideSector->FreeSector( CTEtWorldSector::SRL_TERRAIN );
						}
					}
				}
			}
			break;
		case 6:	// Stretch
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) break;

				CActionElementChangeBlockType *pAction = new CActionElementChangeBlockType( pSector );
				CString szStr;
				szStr.Format( "%s Anti Layer4 Stretch", pVariable->GetVariableBool() == true ? "Enable" : "False" );
				pAction->SetDesc( szStr );

				TerrainType Type;
				if( pVariable->GetVariableBool() == true ) Type = TT_CLIFF;
				else Type = TT_NORMAL;

				std::vector<int> nVecBlockType;
				std::vector<int> nVecPrevBlockType;
				for( DWORD i=0; i<pVecList->size(); i++ ) {
					int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
					nVecPrevBlockType.push_back( nBlockType );

					if( nBlockType == TT_DETAILNORMAL || nBlockType == TT_DETAILCLIFF )
						pSector->ChangeBlockType( (*pVecList)[i], (TerrainType)( 2 + Type ) );
					else pSector->ChangeBlockType( (*pVecList)[i], Type );

					nVecBlockType.push_back( pSector->GetBlockType( (*pVecList)[i] ) );

				}

				pAction->ChangeBlockType( (*pVecList), nVecBlockType, nVecPrevBlockType );
				pAction->AddAction();



				CGlobalValue::GetInstance().SetModify();
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 7:	// Detail Texture
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) break;

				CActionElementChangeBlockType *pAction = new CActionElementChangeBlockType( pSector );
				CString szStr;
				szStr.Format( "%s Detail Texture", pVariable->GetVariableBool() == true ? "Enable" : "False" );
				pAction->SetDesc( szStr );

				TerrainType Type;
				if( pVariable->GetVariableBool() == true ) Type = TT_DETAILNORMAL;
				else Type = TT_NORMAL;

				std::vector<int> nVecBlockType;
				std::vector<int> nVecPrevBlockType;
				for( DWORD i=0; i<pVecList->size(); i++ ) {
					int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
					nVecPrevBlockType.push_back( nBlockType );

					if( nBlockType == TT_CLIFF || nBlockType == TT_DETAILCLIFF )
						pSector->ChangeBlockType( (*pVecList)[i], (TerrainType)( 1 + Type ) );
					else pSector->ChangeBlockType( (*pVecList)[i], Type );

					nVecBlockType.push_back( pSector->GetBlockType( (*pVecList)[i] ) );
				}

				pAction->ChangeBlockType( (*pVecList), nVecBlockType, nVecPrevBlockType );
				pAction->AddAction();

				CGlobalValue::GetInstance().SetModify();
				CGlobalValue::GetInstance().RefreshRender();

				// Layer Prev 창을 강제로 Refresh 해준다.
				CWnd *pWnd = GetPaneWnd( LAYER_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );

				// Layer Distance 리플레쉬 해준다.
				RefreshPropertyGridVariable();
			}
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) break;

				for( DWORD i=0; i<pVecList->size(); i++ ) {
					int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
					pSector->ChangeTextureDistance( (*pVecList)[i], dwIndex - 8, pVariable->GetVariableFloat() );

					// Layer 4 는 Cliff 일 경우Update 해줘야 한다.
					if( dwIndex == 11 ) {
						if( nBlockType == TT_CLIFF || nBlockType == TT_DETAILCLIFF ) {
							int nBlockX, nBlockY;
							int nBlockSizeX, nBlockSizeY;
							pSector->GetBlockCount( nBlockX, nBlockY );
							pSector->GetBlockSize( nBlockSizeX, nBlockSizeY );
							int nX = ( (*pVecList)[i] % nBlockY ) * nBlockSizeX;
							int nY = ( (*pVecList)[i] / nBlockY ) * nBlockSizeY;
							pSector->UpdateTextureCoord( CRect( nX, nY, nX + nBlockSizeX, nY + nBlockSizeY ) );
						}
					}
				}

				CGlobalValue::GetInstance().SetModify();
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 16:
		case 17:
		case 18:
		case 19:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );
				if( pSector == NULL ) break;

				std::vector<int> *pVecList = &CGlobalValue::GetInstance().m_nVecEditBlockList;
				if( pVecList->size() == 0 ) break;

				for( DWORD i=0; i<pVecList->size(); i++ ) {
					int nBlockType = pSector->GetBlockType( (*pVecList)[i] );
					pSector->ChangeTextureRotation( (*pVecList)[i], dwIndex - 16, pVariable->GetVariableFloat() );
				}

				CGlobalValue::GetInstance().SetModify();
				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
	};

	// Shader Param
}

void CSectorInfoPaneView::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex )
{
}

LRESULT CSectorInfoPaneView::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	return ProcessNotifyGrid( wParam, lParam );
}


CActionElementModifyHeightMultiply::CActionElementModifyHeightMultiply( CTEtWorldSector *pSector )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
	m_fHeight = m_fPrevHeight = 0.f;
}

CActionElementModifyHeightMultiply::~CActionElementModifyHeightMultiply()
{
}

bool CActionElementModifyHeightMultiply::Redo()
{
	m_pSector->SetHeightMultiply( m_fHeight );
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return true;
}

bool CActionElementModifyHeightMultiply::Undo()
{
	m_pSector->SetHeightMultiply( m_fPrevHeight );
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	return true;
}

void CActionElementModifyHeightMultiply::ModifyHeightMultiply( float fHeight )
{
	m_fHeight = fHeight;
	m_fPrevHeight = m_pSector->GetHeightMultiply();

	CString szStr;
	szStr.Format( "Height Multiply : %.2f->%.2f", m_fPrevHeight, m_fHeight );
	SetDesc( szStr );

}



CActionElementChangeBlockType::CActionElementChangeBlockType( CTEtWorldSector *pSector )
: CActionElement( &s_BrushActionCommander )
{
	m_pSector = pSector;
}

CActionElementChangeBlockType::~CActionElementChangeBlockType()
{
}

bool CActionElementChangeBlockType::Redo()
{
	for( DWORD i=0; i<m_nVecBlockList.size(); i++ ) {
		m_pSector->ChangeBlockType( m_nVecBlockList[i], (TerrainType)m_nVecBlockType[i] );
	}
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

bool CActionElementChangeBlockType::Undo()
{
	for( DWORD i=0; i<m_nVecBlockList.size(); i++ ) {
		m_pSector->ChangeBlockType( m_nVecBlockList[i], (TerrainType)m_nVecPrevBlockType[i] );
	}
	CWnd *pWnd = GetPaneWnd( SECTOR_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW );
	pWnd = GetPaneWnd( LAYER_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1 );
	CGlobalValue::GetInstance().RefreshRender();
	return true;
}

void CActionElementChangeBlockType::ChangeBlockType( std::vector<int> &nVecList, std::vector<int> &nVecTypeList, std::vector<int> &nVecPrevTypeList )
{
	m_nVecBlockList = nVecList;
	m_nVecBlockType = nVecTypeList;
	m_nVecPrevBlockType = nVecPrevTypeList;
}
