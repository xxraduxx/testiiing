#include "StdAfx.h"
#include "TEtWorldWaterRiver.h"
#include "UnionValueProperty.h"
#include "PropertyGridCreator.h"
#include "PointAxisRenderObject.h"
#include "RenderBase.h"
#include "TEtWorldWater.h"
#include "TEtWorldSector.h"
#include "UnionValueProperty.h"
#include "resource.h"
#include "UserMessage.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "TEtWorldGrid.h"
#include "SundriesFuncEx.h"
#include "EtWater.h"
#include "EngineUtil.h"
#include <map>
using namespace std;

PropertyGridBaseDefine g_WaterRiverPropertyDefine[] = {
	{ "Shader", "Effect Name", CUnionValueProperty::String_FileOpen, "Shader File Name|Shader Effect File|*.fx", FALSE },
	{ "Common", "Interpolation", CUnionValueProperty::Integer_Combo, "타입으로 사용되지 않으며 Lerp체크시 반사하지 않는 물로 바뀝니다.|Linear|Lerp|Hermite", TRUE },
	{ "Common", "Width", CUnionValueProperty::Float, "River Width|0.f|10000.f|1.f", TRUE },
	{ "Common", "UV U Ratio", CUnionValueProperty::Float, "Texture UV", TRUE },
	{ "Common", "UV V Ratio", CUnionValueProperty::Float, "Texture UV", TRUE },
	{ "Alphamap", "Width", CUnionValueProperty::Integer_Combo, "알파맵 가로 사이즈|32|64|128|256|512|1024|2048", TRUE },
	{ "Alphamap", "Height", CUnionValueProperty::Integer_Combo, "알파맵 세로 사이즈|32|64|128|256|512|1024|2048", TRUE },
	{ "Alphamap", "Border Reduction", CUnionValueProperty::Integer, "경계면 축소 카운트(Default=10)|0|50|1", TRUE },
	{ "Alphamap", "Blur Value", CUnionValueProperty::Integer, "블러값(Default=10)|0|30|1", TRUE },
	{ "Alphamap", "Blur Epsilon", CUnionValueProperty::Float, "블러값2 위의 값이 커지면 가치 커져야 한다.(Default=7.f)|0.1f|30.f|0.01f", TRUE },
	{ "Alphamap", "Min Alpha", CUnionValueProperty::Integer, "알파 최소값|0|255|1", TRUE },
	{ "Alphamap", "Max Alpha", CUnionValueProperty::Integer, "알파 최대값|0|255|1", TRUE },
	{ "Alphamap", "Revision Alpha", CUnionValueProperty::Float, "계산 후 알파값 보정|0.f|5.f|0.001f", TRUE },
	{ NULL },
};



CTEtWorldWaterRiver::CTEtWorldWaterRiver( CEtWorldWater *pWater )
: CEtWorldWaterRiver( pWater )
{
	m_pMesh = NULL;
	m_szShaderName = "water.fx";
	m_bDestroy = false;

	SAFE_DELETEA( m_pAlphaTable );
	if( m_pAlphaTable == NULL ) {
		m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];
		for( int i=0; i<m_nAlphaTableHeight; i++ ) {
			for( int j=0; j<m_nAlphaTableWidth; j++ ) {
				m_pAlphaTable[ i * m_nAlphaTableWidth + j ] = 128;
			}
		}
	}
	((CTEtWorldWater*)m_pWater)->AddEngineWaterCount();

	CreateMesh();
	RefreshShader();

	if( !m_hAlphaTexture ) {
		RefreshTexture();
		RefreshTextureAlpha();
	}
}

CTEtWorldWaterRiver::~CTEtWorldWaterRiver()
{
	m_bDestroy = true;
	((CTEtWorldWater*)m_pWater)->DelEngineWaterCount();

	std::map<std::string, EtTextureHandle>::iterator it;
	for( it = m_MapTextureList.begin(); it!=m_MapTextureList.end(); it++ ) {
		SAFE_RELEASE_SPTR( it->second );
	}
	m_MapTextureList.clear();
	SAFE_RELEASE_SPTR( m_hAlphaTexture );
	SAFE_DELETE_PVEC( m_pVecPropertyList );
	SAFE_RELEASE_SPTR( m_hMaterial );
	SAFE_RELEASE_SPTR( m_hAlphaTexture );
	SAFE_DELETE( m_pMesh );
	SAFE_DELETEA( m_pAlphaTable );

	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		SAFE_DELETE( m_pVecPointList[i]->pAxis );
	}
	SAFE_DELETE_PVEC( m_pVecPointList );
	SAFE_DELETE_VEC( m_VecPrimitiveList );
}

CTEtWorldWaterRiver::RiverPoint *CTEtWorldWaterRiver::InsertPoint( EtVector3 &vPos, EtVector3 *pInsertAfter )
{
	RiverPoint *pPoint = new RiverPoint;
	pPoint->vPos = vPos;
	pPoint->bSelect = false;
	pPoint->pAxis = new CPointAxisRenderObject( &pPoint->vPos );
	pPoint->pAxis->Initialize();
	pPoint->pAxis->Show( pPoint->bSelect );

	if( pInsertAfter ) {
		for( DWORD i=(int)m_pVecPointList.size()-1; i>=0; i-- ) {
			if( m_pVecPointList[i]->vPos == *pInsertAfter ) {
				m_pVecPointList.insert( m_pVecPointList.begin() + i + 1, pPoint );
				break;
			}
		}
	}
	else {
		m_pVecPointList.push_back( pPoint );
	}
	UpdatePoint();
	return pPoint;
}

bool CTEtWorldWaterRiver::RemovePoint( RiverPoint *pPoint )
{
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( m_pVecPointList[i] == pPoint ) {
			m_pVecPointList[i]->bSelect = false;
			m_pVecPointList[i]->pAxis->Show( false );
			SAFE_DELETE( m_pVecPointList[i]->pAxis );
			SAFE_DELETE( m_pVecPointList[i] );
			m_pVecPointList.erase( m_pVecPointList.begin() + i );
			UpdatePoint();
			return true;
		}
	}
	return false;
}

void CTEtWorldWaterRiver::Render( LOCAL_TIME LocalTime )
{
	EtVector3 vLine[2];
	DWORD dwColor;

	if( CGlobalValue::GetInstance().m_nActiveView == CGlobalValue::WATER ) {
		for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
			if( i < m_pVecPointList.size() - 1 ) {
				vLine[0] = m_pVecPointList[i]->vPos;
				vLine[1] = m_pVecPointList[i+1]->vPos;

				EternityEngine::DrawLine3D( vLine[0], vLine[1], 0xFF2222FF );
			}
			if( m_pVecPointList[i]->bSelect ) dwColor = 0xFFFF0000;
			else dwColor = 0xFFFFFFFF;
			EternityEngine::DrawPoint3D( m_pVecPointList[i]->vPos, dwColor, NULL, false );
		}

		if( m_VecPrimitiveList.size() >= 4 ) {
			for( DWORD i=0; i<m_VecPrimitiveList.size() - 3; i+=2 ) {
				EternityEngine::DrawLine3D( m_VecPrimitiveList[i], m_VecPrimitiveList[i+1] );
				EternityEngine::DrawLine3D( m_VecPrimitiveList[i], m_VecPrimitiveList[i+2] );
				EternityEngine::DrawLine3D( m_VecPrimitiveList[i], m_VecPrimitiveList[i+3] );
				EternityEngine::DrawLine3D( m_VecPrimitiveList[i+1], m_VecPrimitiveList[i+3] );

			}
			EternityEngine::DrawLine3D( m_VecPrimitiveList[m_VecPrimitiveList.size()-2], m_VecPrimitiveList[m_VecPrimitiveList.size()-1] );
		}
	}

	if( m_pVecPointList.empty() ) return;
	GetEtWater()->SetWaterHeight( ((CTEtWorldWater*)m_pWater)->GetEngineWaterIndex(this), m_pVecPointList[0]->vPos.y );
	GetEtWater()->SetWaterIgnoreBake( ((CTEtWorldWater*)m_pWater)->GetEngineWaterIndex(this), m_InterpolationType == CEtWorldWaterRiver::CatmullRom );
	if( m_pVecPointList.size() > 1 ) {
		SRenderStackElement RenderElement;
		EtMatrix WorldMat;

		EtMatrixIdentity( &WorldMat );
		RenderElement.hMaterial = m_hMaterial;
		RenderElement.nTechniqueIndex = 0;
		RenderElement.WorldMat = WorldMat;
		RenderElement.PrevWorldMat = WorldMat;
		RenderElement.nSaveMatIndex = -1;
		RenderElement.pvecCustomParam = &m_vecCustomParam;
		RenderElement.pRenderMeshStream = m_pMesh;
		RenderElement.renderPriority = RP_HIGH;
		GetCurRenderStack()->AddAlphaRenderElement( RenderElement );
	}
}

void CTEtWorldWaterRiver::RenderCustom( float fElapsedTime )
{
}

bool CTEtWorldWaterRiver::CheckSelect( EtVector2 vMin, EtVector2 vMax, char cFlag )
{
	EtMatrix matWorld, matTemp;

	MatrixEx *pVec = CRenderBase::GetInstance().GetLastUpdateCamera();
	EtMatrix matView = *(*pVec);
	EtMatrixInverse( &matView, NULL, &matView );

	EtMatrix matProj = *CRenderBase::GetInstance().GetCameraHandle()->GetProjMat();
	EtMatrixIdentity( &matWorld );

	EtMatrixMultiply( &matTemp, &matView, &matWorld );
	EtMatrixMultiply( &matTemp, &matTemp, &matProj );

	D3DXMatrixInverse( &matView, NULL, &matView );

	EtVector3 Pos, Normal;
	bool bFlag = false;
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		Pos = m_pVecPointList[i]->vPos;

		EtVec3Normalize( &Normal, &Pos );

		EtVec3TransformCoord( &Pos, &Pos, &matTemp );

		if( CheckRect( Pos.x, Pos.y, vMin, vMax ) == true ) 
		{
			switch( cFlag ) {
				case -1:
				case 0:
				case 1:
					m_pVecPointList[i]->bSelect = true;
					m_pVecPointList[i]->pAxis->Show( true );
					if( std::find( CGlobalValue::GetInstance().m_pVecSelectPointList.begin(), CGlobalValue::GetInstance().m_pVecSelectPointList.end(), m_pVecPointList[i] )
						== CGlobalValue::GetInstance().m_pVecSelectPointList.end() )
						CGlobalValue::GetInstance().m_pVecSelectPointList.push_back( m_pVecPointList[i] );

					if( std::find( CGlobalValue::GetInstance().m_pVecSelectRiverList.begin(), CGlobalValue::GetInstance().m_pVecSelectRiverList.end(), this )
						== CGlobalValue::GetInstance().m_pVecSelectRiverList.end() )
						CGlobalValue::GetInstance().m_pVecSelectRiverList.push_back( this );

					if( cFlag == -1 ) return true;
					bFlag = true;
					break;
				case 2:	//삭제
					{
						m_pVecPointList[i]->bSelect = false;
						m_pVecPointList[i]->pAxis->Show( false );
						std::vector<RiverPoint *>::iterator it = std::find( CGlobalValue::GetInstance().m_pVecSelectPointList.begin(), CGlobalValue::GetInstance().m_pVecSelectPointList.end(), m_pVecPointList[i] );
						if( it != CGlobalValue::GetInstance().m_pVecSelectPointList.end() ) {
							CGlobalValue::GetInstance().m_pVecSelectPointList.erase( it );
						}
						bFlag = false;
					}
					break;
			}
		}
		else {
			switch( cFlag ) 
			{
				case -1:
				case 0:
					m_pVecPointList[i]->bSelect = false;
					m_pVecPointList[i]->pAxis->Show( false );
					std::vector<RiverPoint *>::iterator it = std::find( CGlobalValue::GetInstance().m_pVecSelectPointList.begin(), CGlobalValue::GetInstance().m_pVecSelectPointList.end(), m_pVecPointList[i] );
					if( it != CGlobalValue::GetInstance().m_pVecSelectPointList.end() ) {
						CGlobalValue::GetInstance().m_pVecSelectPointList.erase( it );
					}
					break;
			}
		}
	}
	bool bSelect = false;
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( m_pVecPointList[i]->bSelect == true ) {
			bSelect = true;
			break;
		}
	}
	if( bSelect ) {
		if( std::find( CGlobalValue::GetInstance().m_pVecSelectRiverList.begin(), CGlobalValue::GetInstance().m_pVecSelectRiverList.end(), this )
			== CGlobalValue::GetInstance().m_pVecSelectRiverList.end() )
			CGlobalValue::GetInstance().m_pVecSelectRiverList.push_back( this );
	}
	else {
		std::vector<CTEtWorldWaterRiver *>::iterator it = std::find( CGlobalValue::GetInstance().m_pVecSelectRiverList.begin(), CGlobalValue::GetInstance().m_pVecSelectRiverList.end(), this );
		if( it != CGlobalValue::GetInstance().m_pVecSelectRiverList.end() )
			CGlobalValue::GetInstance().m_pVecSelectRiverList.erase( it );
	}
	return bFlag;
}

void CTEtWorldWaterRiver::RefreshPrimitive()
{
	switch( m_InterpolationType ) {
		case Linear:
			{
				if( (int)m_pVecPointList.size() < 2 ) return;
				EtVector3 vDir, vCross;
				EtVector3 vVertex;

				m_VecPrimitiveList.clear();
				for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
					if( i == m_pVecPointList.size() - 1 ) {
						vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
					}
					else {
						vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
					}
					EtVec3Normalize( &vDir, &vDir );
					EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

					vVertex = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) );
					vVertex.y = m_pVecPointList[i]->vPos.y;
					m_VecPrimitiveList.push_back( vVertex );

					vVertex = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) );
					vVertex.y = m_pVecPointList[i]->vPos.y;
					m_VecPrimitiveList.push_back( vVertex );
				}
			}
			break;
		case CatmullRom:
			{
				// m_InterpolationType을 어쩔 수 없이 다른 의미로 사용하게 되었다.
				// EtWorldWaterRiver.h의 m_InterpolationType 참고하기 바란다.
				// Linear내용 그대로 복사.
				if( (int)m_pVecPointList.size() < 2 ) return;
				EtVector3 vDir, vCross;
				EtVector3 vVertex;

				m_VecPrimitiveList.clear();
				for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
					if( i == m_pVecPointList.size() - 1 ) {
						vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
					}
					else {
						vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
					}
					EtVec3Normalize( &vDir, &vDir );
					EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

					vVertex = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) );
					vVertex.y = m_pVecPointList[i]->vPos.y;
					m_VecPrimitiveList.push_back( vVertex );

					vVertex = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) );
					vVertex.y = m_pVecPointList[i]->vPos.y;
					m_VecPrimitiveList.push_back( vVertex );
				}
			}
			break;
		case Helmite:
			{
			}
			break;
	}
}

bool CTEtWorldWaterRiver::CreateMesh()
{
	SAFE_DELETE( m_pMesh );
	m_pMesh = new CEtMeshStream;

	RefreshVertex();
	RefreshIndex();
	RefreshNormal();
	RefreshTextureCoord();

	return true;
}


void CTEtWorldWaterRiver::RefreshTextureCoord()
{
	if( !m_pMesh ) return;
	if( m_pVecPointList.size() < 2 ) return;

	float fWidth = (float)GetWater()->GetSector()->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)GetWater()->GetSector()->GetParentGrid()->GetGridHeight() * 100.f;

	EtVector3 vOffset = *GetWater()->GetSector()->GetOffset();
	vOffset.x -= ( GetWater()->GetSector()->GetTileWidthCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.z -= ( GetWater()->GetSector()->GetTileHeightCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;

	int nSize = (int)m_pVecPointList.size() * 2;
	EtVector2 *pUV = new EtVector2[ nSize ];

	EtVector3 vDir, vCross;
	EtVector3 vVertex;

	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( i == m_pVecPointList.size() - 1 ) {
			vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
		}
		else {
			vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
		}
		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		vVertex = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) ) - vOffset;
		pUV[(i*2)].x = ( m_vUVRatio.x / fWidth ) * vVertex.x;
		pUV[(i*2)].y = ( m_vUVRatio.y / fHeight ) * vVertex.z;

		vVertex = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) ) - vOffset;
		pUV[(i*2)+1].x = ( m_vUVRatio.x / fWidth ) * vVertex.x;
		pUV[(i*2)+1].y = ( m_vUVRatio.y / fHeight ) * vVertex.z;
	}

	CMemoryStream Stream( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 0, nSize );

	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( i == m_pVecPointList.size() - 1 ) {
			vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
		}
		else {
			vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
		}
		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		vVertex = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) );
		pUV[(i*2)].x = ( 1.f / m_vSize.x ) * ( vVertex.x - m_vOffset.x );
		pUV[(i*2)].y = ( 1.f / m_vSize.y ) * ( vVertex.z - m_vOffset.y );

		vVertex = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) );
		pUV[(i*2)+1].x = ( 1.f / m_vSize.x ) * ( vVertex.x - m_vOffset.x );
		pUV[(i*2)+1].y = ( 1.f / m_vSize.y ) * ( vVertex.z - m_vOffset.y );
	}

	Stream.Initialize( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 1, nSize );

	SAFE_DELETE( pUV );
}

void CTEtWorldWaterRiver::RefreshVertex()
{
	if( !m_pMesh ) return;
	if( m_pVecPointList.size() < 2 ) return;

	int nSize = (int)m_pVecPointList.size() * 2;
	EtVector3 *pVertex = new EtVector3[ nSize ];
	EtVector3 vDir, vCross;
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( i == m_pVecPointList.size() - 1 ) {
			vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
		}
		else {
			vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
		}
		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		pVertex[ i*2 ] = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) );
		pVertex[ (i*2) + 1 ] = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) );
	}

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_POSITION, 0, nSize );
	SAFE_DELETE( pVertex );
}

void CTEtWorldWaterRiver::RefreshIndex()
{
	if( !m_pMesh ) return;
	if( m_pVecPointList.size() < 2 ) return;

	int nSize = ( (int)m_pVecPointList.size() - 1 ) * 6;
	WORD *pIndex = new WORD[ nSize ];

	int nOffset;
	for( int i=0; i<(int)m_pVecPointList.size()-1; i++ ) {
		nOffset = (i*6);
		pIndex[nOffset] = (i*2);
		pIndex[nOffset+1] = ((i+1)*2);
		pIndex[nOffset+2] = (i*2)+1;
		pIndex[nOffset+3] = (i*2)+1;
		pIndex[nOffset+4] = ((i+1)*2);
		pIndex[nOffset+5] = ((i+1)*2)+1;
	}
	CMemoryStream Stream( pIndex, sizeof(WORD) * nSize );
	m_pMesh->LoadIndexStream( &Stream, false, nSize );

	SAFE_DELETE( pIndex );
}

void CTEtWorldWaterRiver::RefreshNormal()
{
	if( !m_pMesh ) return;
	if( m_pVecPointList.size() < 2 ) return;

	int nSize = (int)m_pVecPointList.size() * 2;
	EtVector3 *pVertex = new EtVector3[ nSize ];
	EtVector3 vDir, vCross;
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( i == m_pVecPointList.size() - 1 ) {
			vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
		}
		else {
			vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
		}
		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		pVertex[ i*2 ] = EtVector3( 0.f, 1.f, 0.f );
		pVertex[ (i*2) + 1 ] = EtVector3( 0.f, 1.f, 0.f );
	}

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_NORMAL, 0, nSize );
	SAFE_DELETE( pVertex );
}

void CTEtWorldWaterRiver::RefreshShader()
{
	SAFE_RELEASE_SPTR( m_hMaterial );
	RefreshPropertyList();

	CUnionValueProperty *pVariable;
	m_hMaterial = LoadResource( m_szShaderName.c_str(), RT_SHADER );
	EtParameterHandle hParam;
	EffectParamType Type;
	for( int i=0; i<m_hMaterial->GetCustomParamCount(); i++ ) {

		hParam = m_hMaterial->GetCustomParamHandle(i);
		Type = m_hMaterial->GetCustomParamType(i);
		const char *szName = m_hMaterial->GetParameterName( hParam );
		// 보여줄 필요 없는 파라메터 여기서 걸러주자.
		if( _stricmp( szName, "g_TransparencyTex" ) == NULL ) continue;
		if( _stricmp( szName, "g_WaterMapTex" ) == NULL ) continue;
		if( _stricmp( szName, "g_LastViewMat" ) == NULL ) continue;
		///////////////////////////////////////////////
		if( Type == EPT_TEX ) {
			pVariable = new CUnionValueProperty( CUnionValueProperty::String_FileOpen );
			pVariable->SetCategory( "Texture" );
			pVariable->SetDescription( szName );
			pVariable->SetSubDescription( "Texture File|All Texture Files|*.dds;*.jpg;*.tga;*.bmp" );
			pVariable->SetDefaultEnable( TRUE );
			int nIndex = -1;
			AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, szName, &nIndex );
		}
		else {
			switch( Type ) {
				case EPT_VECTOR:
					{
						pVariable = new CUnionValueProperty( CUnionValueProperty::Vector4Color );

						EtVector4 vVec = pVariable->GetVariableVector4();
						AddCustomParam( m_vecCustomParam, EPT_VECTOR, m_hMaterial, szName, &vVec );
					}
					break;
				case EPT_FLOAT:
					{
						pVariable = new CUnionValueProperty( CUnionValueProperty::Float );

						float fValue = pVariable->GetVariableFloat();
						AddCustomParam( m_vecCustomParam, EPT_FLOAT, m_hMaterial, szName, &fValue );
					}
					break;
				case EPT_INT:
					{
						pVariable = new CUnionValueProperty( CUnionValueProperty::Integer );

						int nValue = pVariable->GetVariableInt();
						AddCustomParam( m_vecCustomParam, EPT_INT, m_hMaterial, szName, &nValue );
					}
					break;
			}
			pVariable->SetCategory( "Shader Parameter" );
			pVariable->SetDescription( szName );
			pVariable->SetSubDescription( "Shader Setting Value" );
			pVariable->SetDefaultEnable( TRUE );
		}
		m_pVecPropertyList.push_back( pVariable );
	}

	CWnd *pWnd = GetPaneWnd( WATERPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 2, (LPARAM)this );
}

void CTEtWorldWaterRiver::RefreshTexture( bool bInit )
{
	SAFE_RELEASE_SPTR( m_hAlphaTexture );

	m_hAlphaTexture = CEtTexture::CreateNormalTexture( m_nAlphaTableWidth, m_nAlphaTableHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
	if( bInit ) {
		int nStride;
		DWORD *pPtr = (DWORD*)m_hAlphaTexture->Lock( nStride, true );

		for( int j=0; j<m_nAlphaTableHeight; j++ ) {
			for( int i=0; i<m_nAlphaTableWidth; i++ ) {
				*pPtr = D3DCOLOR_ARGB( 128, 0, 0, 0 );
				pPtr++;
			}
		}
		m_hAlphaTexture->Unlock();
	}
	if( m_hAlphaTexture ) {
		int nTexIndex = m_hAlphaTexture.GetIndex();
		AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_TransparencyTex", &nTexIndex );
	}
	RefreshWaterTexture();
}

void CTEtWorldWaterRiver::RefreshWaterTexture()
{
	if( !m_hMaterial ) return;
	int nIndex = ((CTEtWorldWater*)m_pWater)->GetEngineWaterIndex(this);
	int nTexIndex = -1;
	if( nIndex >= 0 ) {
		EtTextureHandle hTexture = GetEtWater()->GetWaterMapTexture( nIndex );
		nTexIndex = hTexture.GetIndex();
	}
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_WaterMapTex", &nTexIndex );
}

void CTEtWorldWaterRiver::RefreshTextureAlpha()
{
	int nStride;
	float fValue;
	DWORD *pPtr = (DWORD*)m_hAlphaTexture->Lock( nStride, true );

	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			fValue = m_pAlphaTable[j*m_nAlphaTableWidth+i] * m_fRevisionAlpha;
			if( fValue < 0.f ) fValue = 0.f;
			if( fValue > 255.f ) fValue = 255.f;
			*pPtr = D3DCOLOR_ARGB( (BYTE)fValue, 0, 0, 0 );
			pPtr++;
		}
	}

	m_hAlphaTexture->Unlock();
}

bool CTEtWorldWaterRiver::GetRiverHeight( EtVector2 &vPos, float &fHeight )
{
	EtVector3 vVertex[3];
	EtVector3 vPosTemp = EtVector3( vPos.x, -10000.f, vPos.y );
	float fDist, fBary1, fBary2;

	for( DWORD i=0; i<m_VecPrimitiveList.size() - 3; i+=2 ) {
		vVertex[0] = m_VecPrimitiveList[i];
		vVertex[1] = m_VecPrimitiveList[i+2];
		vVertex[2] = m_VecPrimitiveList[i+1];
		if( TestLineToTriangle( vPosTemp, EtVector3( 0.f, 1.f, 0.f ), vVertex[0], vVertex[1], vVertex[2], fDist, fBary1, fBary2 ) == true ) {
			fHeight = vPosTemp.y + fDist;
			return true;
		}

		vVertex[0] = m_VecPrimitiveList[i+1];
		vVertex[1] = m_VecPrimitiveList[i+2];
		vVertex[2] = m_VecPrimitiveList[i+3];
		if( TestLineToTriangle( vPosTemp, EtVector3( 0.f, 1.f, 0.f ), vVertex[0], vVertex[1], vVertex[2], fDist, fBary1, fBary2 ) == true ) {
			fHeight = vPosTemp.y + fDist;
			return true;
		}
	}
	return false;
}

void CTEtWorldWaterRiver::GenerationAlphaMap()
{
	SAFE_DELETEA( m_pAlphaTable );
	m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];

	EtVector2 vPos;
	float fValue;

	float fMin = (float)m_nMinAlpha;
	float fMax = (float)m_nMaxAlpha;
	float fHeight;
	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			vPos.x = m_vOffset.x + ( ( m_vSize.x / (float)m_nAlphaTableWidth ) * i );
			vPos.y = m_vOffset.y + ( ( m_vSize.y / (float)m_nAlphaTableHeight ) * j );
			fValue = GetWater()->GetSector()->GetHeightToWorld( vPos.x, vPos.y );
			if( GetRiverHeight( vPos, fHeight ) == true ) {
				if( fValue > fHeight )
					m_pAlphaTable[j*m_nAlphaTableWidth+i] = (BYTE)fMin;
				else m_pAlphaTable[j*m_nAlphaTableWidth+i] = (BYTE)fMax;
			}
			else {
				m_pAlphaTable[j*m_nAlphaTableWidth+i] = (BYTE)fMin;
			}
		}
	}
	// 버퍼를 Float 로 변형한 담에 하자.
	int nBufferSize = m_nAlphaTableWidth * m_nAlphaTableHeight;
	float *pTable = new float[nBufferSize];
	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			int nIndex = j * m_nAlphaTableWidth + i;
			pTable[nIndex] = (float)m_pAlphaTable[nIndex];
		}
	}

	float *pTempBuf1 = new float[nBufferSize];
	float *pTempBuf2 = new float[nBufferSize];
	int nTempWidth = m_nAlphaTableWidth - 1;
	int nTempHeight = m_nAlphaTableHeight - 1;

	float fReduction = ( fMax - fMin ) / (float)m_nBorderReduction;
	for( int g=0; g<m_nBorderReduction; g++ ) {
		memcpy( pTempBuf1, pTable, sizeof(float) * m_nAlphaTableWidth * m_nAlphaTableHeight );
		memcpy( pTempBuf2, pTable, sizeof(float) * m_nAlphaTableWidth * m_nAlphaTableHeight );
		fMin = fReduction * g;
		fValue = fReduction * (g+1);
		for( int j=0; j<nTempHeight; j++ ) {
			for( int i=0; i<nTempWidth; i++ ) {
				int nIndex = j * m_nAlphaTableWidth + i;
				if( pTable[nIndex] == fMin && pTable[nIndex+1] == fMax ) {
					pTempBuf1[nIndex+1] = fValue;
				}
				else if( pTable[nIndex] == fMax && pTable[nIndex+1] == fMin ) {
					pTempBuf1[nIndex] = fValue;
				}
			}
		}
		for( int j=0; j<nTempHeight; j++ ) {
			for( int i=0; i<nTempWidth; i++ ) {
				int nIndex = j * m_nAlphaTableWidth + i;
				int nIndex2 = (j+1) * m_nAlphaTableWidth + i;
				if( pTable[nIndex] == fMin && pTable[nIndex2] == fMax ) {
					pTempBuf2[nIndex2] = fValue;
				}
				else if( pTable[nIndex] == fMax && pTable[nIndex2] == fMin ) {
					pTempBuf2[nIndex] = fValue;
				}
			}
		}
		for( int j=0; j<m_nAlphaTableHeight; j++ ) {
			for( int i=0; i<m_nAlphaTableWidth; i++ ) {
				int nIndex = j*m_nAlphaTableWidth+i;
				if( pTempBuf1[nIndex] == fValue || pTempBuf2[nIndex] == fValue ) pTable[nIndex] = fValue;
			}
		}
	}

	SAFE_DELETE( pTempBuf1 );
	SAFE_DELETE( pTempBuf2 );

	if( m_nGaussianValue > 0 ) {
		int nBlurValue = m_nGaussianValue;
		float **ppfGaussianTable;

		int nTableSize = ( nBlurValue * 2 ) + 1;
		ppfGaussianTable = new float*[nTableSize];
		for( int i=0; i<nTableSize; i++ )
			ppfGaussianTable[i] = new float[nTableSize];
		CalcGaussianTable( nBlurValue, m_fGaussianEpsilon, ppfGaussianTable );

		float *pTempBuf = new float[nBufferSize];

		memcpy( pTempBuf, pTable, sizeof(float) * m_nAlphaTableWidth * m_nAlphaTableHeight );

		for( int j=0; j<m_nAlphaTableHeight; j++ ) {
			for( int i=0; i<m_nAlphaTableWidth; i++ ) {
				if( i < nBlurValue ) continue;
				if( i >= m_nAlphaTableWidth - nBlurValue ) continue;
				if( j < nBlurValue ) continue;
				if( j >= m_nAlphaTableHeight - nBlurValue ) continue;

				pTable[j*m_nAlphaTableWidth+i] = GetNewAlpha( pTempBuf, i, j, nBlurValue, ppfGaussianTable );
			}
		}
		for( int i=0; i<nTableSize; i++ ) {
			SAFE_DELETE( ppfGaussianTable[i] );
		}
		SAFE_DELETE( ppfGaussianTable );
		SAFE_DELETE( pTempBuf );
	}
	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			int nIndex = j * m_nAlphaTableWidth + i;
			m_pAlphaTable[nIndex] = (BYTE)pTable[nIndex];
		}
	}
	SAFE_DELETE( pTable );

	RefreshTexture();
	RefreshTextureAlpha();
}

float CTEtWorldWaterRiver::GetNewAlpha( float *pBuffer, int x, int y, int nBlurValue, float **ppfTable )
{
	int nTableSize = ( nBlurValue * 2 ) + 1;

	x -= nBlurValue;
	y -= nBlurValue;

	float fValue = 0.f;
	for( int j=0; j<nTableSize; j++ ) {
		for( int i=0; i<nTableSize; i++ ) {
			int nOffset = (y+j) * m_nAlphaTableWidth + (x+i);

			fValue += pBuffer[nOffset] * ppfTable[i][j];
		}
	}
	if( fValue > 255.f ) fValue = 255.f;
	else if( fValue < 0.f ) fValue = 0.f;
	return fValue;
}



void CTEtWorldWaterRiver::RefreshPropertyList()
{
	SAFE_DELETE_PVEC( m_pVecPropertyList );

	CUnionValueProperty *pVariable;
	for( DWORD i=0; ; i++ ) {
		if( g_WaterRiverPropertyDefine[i].szCategory == NULL ) break;

		pVariable = new CUnionValueProperty( g_WaterRiverPropertyDefine[i].Type );
		pVariable->SetCategory( g_WaterRiverPropertyDefine[i].szCategory );
		pVariable->SetDescription( g_WaterRiverPropertyDefine[i].szStr );
		pVariable->SetSubDescription( g_WaterRiverPropertyDefine[i].szDescription, true );
		pVariable->SetDefaultEnable( g_WaterRiverPropertyDefine[i].bEnable );
		pVariable->SetCustomDialog( g_WaterRiverPropertyDefine[i].bCustomDialog );

		m_pVecPropertyList.push_back( pVariable );
	}
}

void CTEtWorldWaterRiver::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (char*)m_szShaderName.c_str() );
			break;
		case 1:
			pVariable->SetVariable( (int)m_InterpolationType );
			break;
		case 2:
			pVariable->SetVariable( m_fWidth );
			break;
		case 3:
			pVariable->SetVariable( m_vUVRatio.x );
			break;
		case 4:
			pVariable->SetVariable( m_vUVRatio.y );
			break;
		case 5:
			switch( m_nAlphaTableWidth ) {
				case 32: pVariable->SetVariable(0); break;
				case 64: pVariable->SetVariable(1); break;
				case 128: pVariable->SetVariable(2); break;
				case 256: pVariable->SetVariable(3); break;
				case 512: pVariable->SetVariable(4); break;
				case 1024: pVariable->SetVariable(5); break;
				case 2048: pVariable->SetVariable(6); break;
			}
			break;
		case 6:
			switch( m_nAlphaTableHeight ) {
				case 32: pVariable->SetVariable(0); break;
				case 64: pVariable->SetVariable(1); break;
				case 128: pVariable->SetVariable(2); break;
				case 256: pVariable->SetVariable(3); break;
				case 512: pVariable->SetVariable(4); break;
				case 1024: pVariable->SetVariable(5); break;
				case 2048: pVariable->SetVariable(6); break;
			}
			break;
		case 7:
			pVariable->SetVariable( m_nBorderReduction );
			break;
		case 8:
			pVariable->SetVariable( m_nGaussianValue );
			break;
		case 9:
			pVariable->SetVariable( m_fGaussianEpsilon );
			break;
		case 10:
			pVariable->SetVariable( m_nMinAlpha );
			break;
		case 11:
			pVariable->SetVariable( m_nMaxAlpha );
			break;
		case 12:
			pVariable->SetVariable( m_fRevisionAlpha );
			break;
	}

	int nOffset = ( sizeof(g_WaterRiverPropertyDefine) / sizeof(PropertyGridBaseDefine) ) - 1;
	if( dwIndex - nOffset >= 0 && dwIndex - nOffset < m_pVecPropertyList.size() ) {
		EtParameterHandle hParam;
		EffectParamType Type;

		for( DWORD i=0; i<m_vecCustomParam.size(); i++ ) {
			hParam = m_hMaterial->GetCustomParamHandle(i);
			Type = m_hMaterial->GetCustomParamType(i);
			const char *szName = m_hMaterial->GetParameterName( hParam );
			if( _stricmp( szName, pVariable->GetDescription() ) == NULL ) {

				switch( pVariable->GetType() ) {
					case CUnionValueProperty::Float:
						pVariable->SetVariable( m_vecCustomParam[i].fFloat );
						break;
					case CUnionValueProperty::Integer:
						pVariable->SetVariable( m_vecCustomParam[i].nInt );
						break;
					case CUnionValueProperty::Vector4Color:
						pVariable->SetVariable( *(EtVector4*)m_vecCustomParam[i].fFloat4 );
						break;
					case CUnionValueProperty::String_FileOpen:
						{
							std::map<std::string, EtTextureHandle>::iterator it = m_MapTextureList.find(pVariable->GetDescription());
							if( it != m_MapTextureList.end() ) {
								if( it->second )
									pVariable->SetVariable( (char*)it->second->GetFileName() );
							}
						}
						break;
				}
				break;
			}
		}
	}

}

void CTEtWorldWaterRiver::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
{
	CGlobalValue::GetInstance().SetModify();
	switch( dwIndex ) {
		case 0:
			m_szShaderName = pVariable->GetVariableString();
			RefreshShader();
			{
				CWnd *pWnd = GetPaneWnd( WATERPROP_PANE );
				if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1, (LPARAM)this );
			}
			break;
		case 1:
			m_InterpolationType = (InterpolationType)pVariable->GetVariableInt();
			break;
		case 2:
			m_fWidth = pVariable->GetVariableFloat();
			break;
		case 3:
			m_vUVRatio.x = pVariable->GetVariableFloat();
			RefreshTextureCoord();
			break;
		case 4:
			m_vUVRatio.y = pVariable->GetVariableFloat();
			RefreshTextureCoord();
			break;
		case 5:
			switch( pVariable->GetVariableInt() ) {
				case 0: m_nAlphaTableWidth = 32; break;
				case 1: m_nAlphaTableWidth = 64; break;
				case 2: m_nAlphaTableWidth = 128; break;
				case 3: m_nAlphaTableWidth = 256; break;
				case 4: m_nAlphaTableWidth = 512; break;
				case 5: m_nAlphaTableWidth = 1024; break;
				case 6: m_nAlphaTableWidth = 2048; break;
			}
			break;
		case 6:
			switch( pVariable->GetVariableInt() ) {
				case 0: m_nAlphaTableHeight = 32; break;
				case 1: m_nAlphaTableHeight = 64; break;
				case 2: m_nAlphaTableHeight = 128; break;
				case 3: m_nAlphaTableHeight = 256; break;
				case 4: m_nAlphaTableHeight = 512; break;
				case 5: m_nAlphaTableHeight = 1024; break;
				case 6: m_nAlphaTableHeight = 2048; break;
			}
			break;
		case 7:
			m_nBorderReduction = pVariable->GetVariableInt();
			break;
		case 8:
			m_nGaussianValue = pVariable->GetVariableInt();
			break;
		case 9:
			m_fGaussianEpsilon = pVariable->GetVariableFloat();
			break;
		case 10:
			m_nMinAlpha = pVariable->GetVariableInt();
			break;
		case 11:
			m_nMaxAlpha = pVariable->GetVariableInt();
			break;
		case 12:
			m_fRevisionAlpha = pVariable->GetVariableFloat();
			RefreshTextureAlpha();
			break;
	}
	int nOffset = ( sizeof(g_WaterRiverPropertyDefine) / sizeof(PropertyGridBaseDefine) ) - 1;
	if( dwIndex - nOffset >= 0 && dwIndex - nOffset < m_pVecPropertyList.size() ) {
		switch( pVariable->GetType() ) {
			case CUnionValueProperty::Float:
				{
					float fValue = pVariable->GetVariableFloat();
					AddCustomParam( m_vecCustomParam, EPT_FLOAT, m_hMaterial, pVariable->GetDescription(), &fValue );
				}
				break;
			case CUnionValueProperty::Integer:
				{
					int nValue = pVariable->GetVariableInt();
					AddCustomParam( m_vecCustomParam, EPT_INT, m_hMaterial, pVariable->GetDescription(), &nValue );
				}
				break;
			case CUnionValueProperty::Vector4Color:
				{
					EtVector4 vVec = pVariable->GetVariableVector4();
					AddCustomParam( m_vecCustomParam, EPT_VECTOR, m_hMaterial, pVariable->GetDescription(), &vVec );
				}
				break;
			case CUnionValueProperty::String_FileOpen:
				{
					OutputDebug( "Tex : %s %s\n", pVariable->GetDescription(), pVariable->GetVariableString() );
					std::map<std::string, EtTextureHandle>::iterator it = m_MapTextureList.find(pVariable->GetDescription());
					if( it != m_MapTextureList.end() ) {
						SAFE_RELEASE_SPTR( it->second );
						m_MapTextureList.erase( it );
					}
					EtTextureHandle hTexture = LoadResource( pVariable->GetVariableString(), RT_TEXTURE );

					int nTexIndex = hTexture.GetIndex();
					m_MapTextureList.insert( make_pair( pVariable->GetDescription(), hTexture ) );

					AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, pVariable->GetDescription(), &nTexIndex );
				}
				break;
		}

	}
	UpdatePoint();
	CGlobalValue::GetInstance().RefreshRender();
}

void CTEtWorldWaterRiver::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
{
}


void CTEtWorldWaterRiver::RefreshSize()
{
	if( m_pVecPointList.size() < 2 ) return;
	EtVector2 vMin = EtVector2( FLT_MAX, FLT_MAX );
	EtVector2 vMax = EtVector2( FLT_MAX, FLT_MAX );

	EtVector3 vDir, vVertex, vCross;
	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		if( i == m_pVecPointList.size() - 1 ) {
			vDir = m_pVecPointList[i]->vPos - m_pVecPointList[i-1]->vPos;
		}
		else {
			vDir = m_pVecPointList[i+1]->vPos - m_pVecPointList[i]->vPos;
		}
		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		vVertex = m_pVecPointList[i]->vPos + ( vCross * ( m_fWidth / 2.f ) );
		if( vMin.x == FLT_MAX || vVertex.x < vMin.x ) vMin.x = vVertex.x;
		if( vMax.x == FLT_MAX || vVertex.x > vMax.x ) vMax.x = vVertex.x;
		if( vMin.y == FLT_MAX || vVertex.z < vMin.y ) vMin.y = vVertex.z;
		if( vMax.y == FLT_MAX || vVertex.z > vMax.y ) vMax.y = vVertex.z;

		vVertex = m_pVecPointList[i]->vPos - ( vCross * ( m_fWidth / 2.f ) );
		if( vMin.x == FLT_MAX || vVertex.x < vMin.x ) vMin.x = vVertex.x;
		if( vMax.x == FLT_MAX || vVertex.x > vMax.x ) vMax.x = vVertex.x;
		if( vMin.y == FLT_MAX || vVertex.z < vMin.y ) vMin.y = vVertex.z;
		if( vMax.y == FLT_MAX || vVertex.z > vMax.y ) vMax.y = vVertex.z;

	}
	m_vOffset = vMin;
	m_vSize.x = vMax.x - vMin.x;
	m_vSize.y = vMax.y - vMin.y;
}

void CTEtWorldWaterRiver::UpdatePoint()
{

	RefreshPrimitive();

	SAFE_DELETE( m_pMesh );
	m_pMesh = new CEtMeshStream;
	RefreshSize();
	RefreshVertex();
	RefreshIndex();
	RefreshNormal();
	RefreshTextureCoord();
}

bool CTEtWorldWaterRiver::Save( FILE *fp )
{
	CString szName;
	// 실제 데이터들
	WriteCString( &CString(m_szName.c_str()), fp );
	fwrite( &m_InterpolationType, sizeof(int), 1, fp );
	fwrite( &m_fWidth, sizeof(float), 1, fp );
	fwrite( &m_nAlphaTableWidth, sizeof(int), 1, fp );
	fwrite( &m_nAlphaTableHeight, sizeof(int), 1, fp );
	fwrite( &m_vUVRatio, sizeof(EtVector2), 1, fp );

	int nCount = (int)m_pVecPointList.size();
	fwrite( &nCount, sizeof(int), 1, fp );
	EtVector3 vOffset = *GetWater()->GetSector()->GetOffset();
	vOffset.x -= ( GetWater()->GetSector()->GetTileWidthCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.z -= ( GetWater()->GetSector()->GetTileHeightCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;
	EtVector3 vPos;
	for( int i=0; i<nCount; i++ ) {
		vPos = m_pVecPointList[i]->vPos - vOffset;
		fwrite( &vPos, sizeof(EtVector3), 1, fp );
	}
	fwrite( m_pAlphaTable, sizeof(BYTE) * ( m_nAlphaTableWidth * m_nAlphaTableHeight ), 1, fp );

	// 툴에서만 사용하는 파라매터지만 가치 저장하자.
	fwrite( &m_nBorderReduction, sizeof(int), 1, fp );
	fwrite( &m_nGaussianValue, sizeof(int), 1, fp );
	fwrite( &m_fGaussianEpsilon, sizeof(float), 1, fp );
	fwrite( &m_nMinAlpha, sizeof(int), 1, fp );
	fwrite( &m_nMaxAlpha, sizeof(int), 1, fp );
	fwrite( &m_fRevisionAlpha, sizeof(float), 1, fp );

	szName = m_szShaderName.c_str(); 
	WriteCString( &szName, fp );

	long nOffset = ftell(fp);
	int nCustomParamCount = 0;
	fwrite( &nCustomParamCount, sizeof(int), 1, fp );

	EtParameterHandle hParam;
	EffectParamType Type;
	for( int i=0; i<m_hMaterial->GetCustomParamCount(); i++ ) {
		hParam = m_hMaterial->GetCustomParamHandle(i);
		Type = m_hMaterial->GetCustomParamType(i);
		szName = m_hMaterial->GetParameterName( hParam );

		SCustomParam *pParam = NULL;
		for( DWORD j=0; j<m_vecCustomParam.size(); j++ ) {
			if( m_vecCustomParam[j].hParamHandle == hParam &&
				m_vecCustomParam[j].Type == Type ) {
					pParam = &m_vecCustomParam[j];
					break;
			}
		}
		if( pParam == NULL ) continue;
		if( _stricmp( szName, "g_TransparencyTex" ) == NULL ) continue;
		if( _stricmp( szName, "g_WaterMapTex" ) == NULL ) continue;
		if( _stricmp( szName, "g_LastViewMat" ) == NULL ) continue;

		WriteCString( &szName, fp );
		fwrite( &Type, sizeof(EffectParamType), 1, fp );
		switch( Type ) {
			case EPT_INT: fwrite( &pParam->nInt, sizeof(int), 1, fp ); break;
			case EPT_FLOAT: fwrite( &pParam->fFloat, sizeof(float), 1, fp ); break;
			case EPT_VECTOR: fwrite( pParam->fFloat4, sizeof(float)*4, 1, fp ); break;
			case EPT_TEX: 
				{
					if( pParam->nTextureIndex == -1 ) {
						szName.Empty();
						WriteCString( &szName, fp );
						break;
					}
					EtTextureHandle hTexture = CEtResource::GetResource( pParam->nTextureIndex );
					if( !hTexture ) continue;
					szName = hTexture->GetFileName();
					if( szName.IsEmpty() ) continue;
					WriteCString( &szName, fp );
					break;
				}
			case EPT_MATRIX: fwrite( pParam->fFloat4, sizeof(float)*16, 1, fp ); break;
		}
		nCustomParamCount++;
	}

	long nLastSeek = ftell(fp);
	fseek( fp, nOffset, SEEK_SET );
	fwrite( &nCustomParamCount, sizeof(int), 1, fp );
	fseek( fp, nLastSeek, SEEK_SET );

	return true;
}

bool CTEtWorldWaterRiver::Load( CStream *pStream )
{
	bool bResult = CEtWorldWaterRiver::Load( pStream );
	if( !bResult ) return false;

	for( DWORD i=0; i<m_pVecPointList.size(); i++ ) {
		SAFE_DELETE( m_pVecPointList[i]->pAxis );
	}
	SAFE_DELETE_PVEC( m_pVecPointList );

	EtVector3 vOffset = *GetWater()->GetSector()->GetOffset();
	vOffset.x -= ( GetWater()->GetSector()->GetTileWidthCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.z -= ( GetWater()->GetSector()->GetTileHeightCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;

	for( DWORD i=0; i<m_vVecPointList.size(); i++ ) {
		InsertPoint( m_vVecPointList[i] + vOffset );
	}

	UpdatePoint();
	RefreshShader();

	for( DWORD i=0; i<m_pVecCustomParamList.size(); i++ ) {
		CUnionValueProperty *pVariable = NULL;
		for( DWORD j=0; j<m_pVecPropertyList.size(); j++ ) {
			if( _stricmp( m_pVecCustomParamList[i]->szName.c_str(), m_pVecPropertyList[j]->GetDescription() ) == NULL ) {
				pVariable = m_pVecPropertyList[j];
				break;
			}
		}
		if( pVariable == NULL ) continue;
		switch( m_pVecCustomParamList[i]->nType ) {
			case EPT_INT:
				pVariable->SetVariable( m_pVecCustomParamList[i]->nInt );
				AddCustomParam( m_vecCustomParam, EPT_INT, m_hMaterial, m_pVecCustomParamList[i]->szName.c_str(), &m_pVecCustomParamList[i]->nInt );
				break;
			case EPT_FLOAT:
				pVariable->SetVariable( m_pVecCustomParamList[i]->fFloat );
				AddCustomParam( m_vecCustomParam, EPT_FLOAT, m_hMaterial, m_pVecCustomParamList[i]->szName.c_str(), &m_pVecCustomParamList[i]->fFloat );
				break;
			case EPT_VECTOR:
				pVariable->SetVariable( *(EtVector4*)m_pVecCustomParamList[i]->fFloat4 );
				AddCustomParam( m_vecCustomParam, EPT_VECTOR, m_hMaterial, m_pVecCustomParamList[i]->szName.c_str(), &m_pVecCustomParamList[i]->fFloat4 );
				break;
			case EPT_TEX:
				{
					pVariable->SetVariable( m_pVecCustomParamList[i]->szTexName );

					std::map<std::string, EtTextureHandle>::iterator it = m_MapTextureList.find(m_pVecCustomParamList[i]->szName);
					if( it != m_MapTextureList.end() ) {
						SAFE_RELEASE_SPTR( it->second );
						m_MapTextureList.erase( it );
					}
					EtTextureHandle 	hTexture = LoadResource( pVariable->GetVariableString(), RT_TEXTURE );					
					int nTexIndex = hTexture.GetIndex();
					m_MapTextureList.insert( make_pair( m_pVecCustomParamList[i]->szName, hTexture ) );

					AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, m_pVecCustomParamList[i]->szName.c_str(), &nTexIndex );
				}
				break;
		}
	}

	AddCustomParam( m_vecCustomParam, EPT_MATRIX_PTR, m_hMaterial, "g_LastViewMat", GetEtWater()->GetWaterBakeViewMat() );

	if( m_pAlphaTable == NULL ) {
		m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];
		for( int i=0; i<m_nAlphaTableHeight; i++ ) {
			for( int j=0; j<m_nAlphaTableWidth; j++ ) {
				m_pAlphaTable[ i * m_nAlphaTableWidth + j ] = 128;
			}
		}
	}

	RefreshTexture();
	RefreshTextureAlpha();

	return bResult;
}
