#include "StdAfx.h"
#include "TEtWorldWater.h"
#include "TEtWorldWaterRiver.h"
#include "TEtWorldGrid.h"
#include "TEtWorldSector.h"
#include "UnionValueProperty.h"
#include "PropertyGridCreator.h"
#include "SundriesFuncEx.h"
#include "resource.h"
#include "UserMessage.h"
#include "PaneDefine.h"
#include "MainFrm.h"
#include "EtWater.h"
#include <map>
using namespace std;

#include "RenderBase.h"

PropertyGridBaseDefine g_WaterSeaPropertyDefine[] = {
	{ "Shader", "Effect Name", CUnionValueProperty::String_FileOpen, "Shader File Name|Shader Effect File|*.fx", FALSE },
	{ "Common", "Enable", CUnionValueProperty::Boolean, "Enable Water", TRUE },
	{ "Common", "Water Level", CUnionValueProperty::Float, "Water Level", TRUE },
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
	{ "Common", "Mesh Grid Size", CUnionValueProperty::Float, "매쉬 그리드 사이즈(Default=200(cm))", TRUE },
	{ NULL },
};

CTEtWorldWater::CTEtWorldWater( CEtWorldSector *pSector )
: CEtWorldWater( pSector )
{
	m_pMesh = NULL;
	m_szShaderName = "water.fx";
	m_nEngineWaterCount = 0;

	SAFE_DELETEA( m_pAlphaTable );
	if( m_pAlphaTable == NULL ) {
		m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];
		for( int i=0; i<m_nAlphaTableHeight; i++ ) {
			for( int j=0; j<m_nAlphaTableWidth; j++ ) {
				m_pAlphaTable[ i * m_nAlphaTableWidth + j ] = 128;
			}
		}
	}

	CreateMesh();
	RefreshShader();
	RefreshTexture();
	RefreshTextureAlpha();
}

CTEtWorldWater::~CTEtWorldWater()
{
	if( m_bEnable ) DelEngineWaterCount();
	std::map<std::string, EtTextureHandle>::iterator it;
	for( it = m_MapTextureList.begin(); it!=m_MapTextureList.end(); it++ ) {
		SAFE_RELEASE_SPTR( it->second );
	}
	m_MapTextureList.clear();
	//SAFE_DELETE_PVEC( m_pVecPropertyList );
	for( DWORD MACRO_0 = 0; MACRO_0< m_pVecPropertyList.size(); MACRO_0++ ) { \
		if( m_pVecPropertyList[MACRO_0] ) m_pVecPropertyList[MACRO_0]; \
	} 
	m_pVecPropertyList.clear();


	SAFE_RELEASE_SPTR( m_hMaterial );
	SAFE_RELEASE_SPTR( m_hAlphaTexture );
	SAFE_DELETE( m_pMesh );
	SAFE_DELETE( m_pAlphaTable );
	SAFE_DELETE_PVEC( m_pVecRiver );
}

bool CTEtWorldWater::Load( const char *szFileName )
{
	bool bResult = CEtWorldWater::Load( szFileName );
	if( m_bEnable ) AddEngineWaterCount();

	CRenderBase::GetInstance().Lock();
	CreateMesh();
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
					EtTextureHandle hTexture = LoadResource( pVariable->GetVariableString(), RT_TEXTURE );
					int nTexIndex = hTexture.GetIndex();
					m_MapTextureList.insert( make_pair( m_pVecCustomParamList[i]->szName, hTexture ) );
					if( nTexIndex == -1 ) {
						nTexIndex = CEtTexture::GetBlankTexture().GetIndex();
					}
					AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, m_pVecCustomParamList[i]->szName.c_str(), &nTexIndex );
				}
				break;
		}
	}
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

	CRenderBase::GetInstance().Unlock();

	CWnd *pWnd = GetPaneWnd( WATERPROP_PANE );
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1, (LPARAM)this );

	return bResult;
}

CEtWorldWaterRiver *CTEtWorldWater::AllocRiver()
{
	return new CTEtWorldWaterRiver( this );
}


bool CTEtWorldWater::Save( const char *szFileName )
{
	FILE *fp;
	CString szName;
	fopen_s( &fp, szFileName, "wb" );
	if( fp == NULL ) return false;

	// 실제 데이터들
	fwrite( &m_bEnable, sizeof(bool), 1, fp );
	fwrite( &m_nAlphaTableWidth, sizeof(int), 1, fp );
	fwrite( &m_nAlphaTableHeight, sizeof(int), 1, fp );
	fwrite( &m_fGridSize, sizeof(float), 1, fp );
	fwrite( &m_vUVRatio, sizeof(EtVector2), 1, fp );
	fwrite( &m_fWaterLevel, sizeof(float), 1, fp );
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
					bool bEmptyTexture = false;
					if( pParam->nTextureIndex == -1 ) bEmptyTexture = true;
					if( CEtTexture::GetBlankTexture().GetIndex() == pParam->nTextureIndex ) bEmptyTexture = true;
					if( bEmptyTexture ) {
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
	int nRiverCount = (int)m_pVecRiver.size();
	fwrite( &nRiverCount, sizeof(int), 1, fp );
	for( int i=0; i<nRiverCount; i++ ) {
		((CTEtWorldWaterRiver*)m_pVecRiver[i])->Save( fp );
	}

	fseek( fp, nOffset, SEEK_SET );
	fwrite( &nCustomParamCount, sizeof(int), 1, fp );
	fclose(fp);

	return true;
}

void CTEtWorldWater::Render( LOCAL_TIME LocalTime )
{
	if( m_bEnable ) {
		GetEtWater()->SetWaterHeight( GetEngineWaterIndex(this), m_fWaterLevel );
		GetEtWater()->SetWaterIgnoreBake( GetEngineWaterIndex(this), false );

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

	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		((CTEtWorldWaterRiver*)m_pVecRiver[i])->Render( LocalTime );
	}	
}

bool CTEtWorldWater::CreateMesh()
{
	SAFE_DELETE( m_pMesh );
	m_pMesh = new CEtMeshStream;

	RefreshVertex();
	RefreshIndex();
	RefreshNormal();
	RefreshTextureCoord();

	return true;
}

void CTEtWorldWater::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
{
	if( dwIndex >= 2 ) {
		pImp->SetReadOnly( pVariable, !m_bEnable );
	}
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( (char*)m_szShaderName.c_str() );
			break;
		case 1:
			pVariable->SetVariable( m_bEnable );
			break;
		case 2:
			pVariable->SetVariable( m_fWaterLevel );
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
		case 13:
			pVariable->SetVariable( m_fGridSize );
			break;

	};
	int nOffset = ( sizeof(g_WaterSeaPropertyDefine) / sizeof(PropertyGridBaseDefine) ) - 1;
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

void CTEtWorldWater::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
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
			if( m_bEnable ) DelEngineWaterCount();
			m_bEnable = pVariable->GetVariableBool();
			if( m_bEnable ) AddEngineWaterCount();
			for( int i=2; i<(int)m_pVecPropertyList.size(); i++ ) {
				pImp->SetReadOnly( i, !m_bEnable );
			}
			break;
		case 2:
			m_fWaterLevel = pVariable->GetVariableFloat();
			//GetEtWater()->SetWaterHeight( m_fWaterLevel );
			RefreshVertex();
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
		case 13:
			m_fGridSize = pVariable->GetVariableFloat();
			CreateMesh();
			break;
  
	}
	int nOffset = ( sizeof(g_WaterSeaPropertyDefine) / sizeof(PropertyGridBaseDefine) ) - 1;
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
					if( nTexIndex == -1 ) {
						nTexIndex = CEtTexture::GetBlankTexture().GetIndex();
					}
					AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, pVariable->GetDescription(), &nTexIndex );
				}
				break;
		}

	}

	CGlobalValue::GetInstance().RefreshRender();
}

void CTEtWorldWater::OnSelectChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pImp )
{
}


void CTEtWorldWater::RefreshTextureCoord()
{
	if( !m_pMesh ) return;

	float fWidth = (float)m_pSector->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)m_pSector->GetParentGrid()->GetGridHeight() * 100.f;

	int nWidthCount = (int)( ( fWidth / m_fGridSize ) + 1.f );
	int nHeightCount = (int)( ( fHeight / m_fGridSize ) + 1.f );

	int nSize = nWidthCount * nHeightCount;
	EtVector2 *pUV = new EtVector2[ nSize ];

	for( int j=0; j<nHeightCount; j++ ) {
		for( int i=0; i<nWidthCount; i++ ) {
			pUV[ j * nWidthCount + i ].x = ( m_vUVRatio.x / (float)(nWidthCount-1) ) * (float)i;
			pUV[ j * nWidthCount + i ].y = ( m_vUVRatio.y / (float)(nHeightCount-1) ) * (float)j;
		}
	}

	CMemoryStream Stream( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 0, nSize );

	for( int j=0; j<nHeightCount; j++ ) {
		for( int i=0; i<nWidthCount; i++ ) {
			pUV[ j * nWidthCount + i ].x = ( 1.f / (float)(nWidthCount-1) ) * (float)i;
			pUV[ j * nWidthCount + i ].y = ( 1.f / (float)(nHeightCount-1) ) * (float)j;
		}
	}

	Stream.Initialize( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 1, nSize );


	SAFE_DELETE( pUV );
}

void CTEtWorldWater::RefreshVertex()
{
	if( !m_pMesh ) return;

	EtVector3 vOffset = *m_pSector->GetOffset();
	float fWidth = (float)m_pSector->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)m_pSector->GetParentGrid()->GetGridHeight() * 100.f;

	int nWidthCount = (int)( ( fWidth / m_fGridSize ) + 1.f );
	int nHeightCount = (int)( ( fHeight / m_fGridSize ) + 1.f );

	int nSize = nWidthCount * nHeightCount;
	EtVector3 *pVertex = new EtVector3[ nSize ];

	for( int j=0; j<nHeightCount; j++ ) {
		for( int i=0; i<nWidthCount; i++ ) {
			pVertex[ j * nWidthCount + i ].x = vOffset.x - ( fWidth / 2.f ) + ( i * m_fGridSize );
			pVertex[ j * nWidthCount + i ].y = GetWaterLevel();
			pVertex[ j * nWidthCount + i ].z = vOffset.z - ( fHeight / 2.f ) + ( j * m_fGridSize );
		}
	}

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_POSITION, 0, nSize );
	SAFE_DELETE( pVertex );
}

void CTEtWorldWater::RefreshIndex()
{
	if( !m_pMesh ) return;

	float fWidth = (float)m_pSector->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)m_pSector->GetParentGrid()->GetGridHeight() * 100.f;

	int nWidthCount = (int)( ( fWidth / m_fGridSize ) );
	int nHeightCount = (int)( ( fHeight / m_fGridSize ) );

	int nSize = nWidthCount * nHeightCount * 6;
	WORD *pIndex = new WORD[ nSize ];

	int nOffset;
	for( int j=0; j<nHeightCount; j++ ) {
		for( int i=0; i<nWidthCount; i++ ) {
			nOffset = (j*6) * nWidthCount + (i*6);

			pIndex[nOffset] = ( j * ( nWidthCount + 1 ) ) + i;
			pIndex[nOffset + 1] = ( (j+1) * ( nWidthCount + 1 ) ) + i;
			pIndex[nOffset + 2] = ( (j+1) * ( nWidthCount + 1 ) ) + (i+1);
			pIndex[nOffset + 3] = ( j * ( nWidthCount + 1 ) ) + i;
			pIndex[nOffset + 4] = ( (j+1) * ( nWidthCount + 1 ) ) + (i+1);
			pIndex[nOffset + 5] = ( j * ( nWidthCount + 1 ) ) + (i+1);
		}
	}

	CMemoryStream Stream( pIndex, sizeof(WORD) * nSize );
	m_pMesh->LoadIndexStream( &Stream, false, nSize );

	SAFE_DELETE( pIndex );
}

void CTEtWorldWater::RefreshNormal()
{
	if( !m_pMesh ) return;

	EtVector3 vOffset = *m_pSector->GetOffset();
	float fWidth = (float)m_pSector->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)m_pSector->GetParentGrid()->GetGridHeight() * 100.f;

	int nWidthCount = (int)( ( fWidth / m_fGridSize ) + 1.f );
	int nHeightCount = (int)( ( fHeight / m_fGridSize ) + 1.f );

	int nSize = nWidthCount * nHeightCount;
	EtVector3 *pVertex = new EtVector3[ nSize ];

	for( int j=0; j<nHeightCount; j++ ) {
		for( int i=0; i<nWidthCount; i++ ) {
			pVertex[ j * nWidthCount + i ] = EtVector3( 0.f, 1.f, 0.f );
		}
	}

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_NORMAL, 0, nSize );
	SAFE_DELETE( pVertex );
}

void CTEtWorldWater::RefreshShader()
{
	SAFE_RELEASE_SPTR( m_hMaterial );
	//SAFE_DELETE_PVEC( m_pVecPropertyList );

	for( DWORD MACRO_0 = 0; MACRO_0< m_pVecPropertyList.size(); MACRO_0++ ) { \
		if( m_pVecPropertyList[MACRO_0] ) m_pVecPropertyList[MACRO_0]; \
	} 
	m_pVecPropertyList.clear();

	CUnionValueProperty *pVariable;
	for( DWORD i=0; ; i++ ) {
		if( g_WaterSeaPropertyDefine[i].szCategory == NULL ) break;

		pVariable = new CUnionValueProperty( g_WaterSeaPropertyDefine[i].Type );
		pVariable->SetCategory( g_WaterSeaPropertyDefine[i].szCategory );
		pVariable->SetDescription( g_WaterSeaPropertyDefine[i].szStr );
		pVariable->SetSubDescription( g_WaterSeaPropertyDefine[i].szDescription, true );
		pVariable->SetDefaultEnable( g_WaterSeaPropertyDefine[i].bEnable );
		pVariable->SetCustomDialog( g_WaterSeaPropertyDefine[i].bCustomDialog );

		m_pVecPropertyList.push_back( pVariable );
	}

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
		if( _stricmp( szName, "g_LastViewMat" ) == NULL ) 
		{
			AddCustomParam( m_vecCustomParam, EPT_MATRIX_PTR, m_hMaterial, "g_LastViewMat", GetEtWater()->GetWaterBakeViewMat() );
			continue;		
		}
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
	if( pWnd ) pWnd->SendMessage( UM_REFRESH_PANE_VIEW, 1, (LPARAM)this );

}

void CTEtWorldWater::RefreshTexture( bool bInit )
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

void CTEtWorldWater::RefreshWaterTexture()
{
	if( !m_hMaterial ) return;
	int nTexIndex = -1;
	if( m_bEnable ) {
		EtTextureHandle hTexture = GetEtWater()->GetWaterMapTexture( GetEngineWaterIndex(this) );
		nTexIndex = hTexture.GetIndex();
	}
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_WaterMapTex", &nTexIndex );
}

void CTEtWorldWater::RefreshTextureAlpha()
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

void CTEtWorldWater::GenerationAlphaMap()
{
	SAFE_DELETEA( m_pAlphaTable );
	m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];

	EtVector3 vOffset = *m_pSector->GetOffset();
	float fWidth = (float)m_pSector->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)m_pSector->GetParentGrid()->GetGridHeight() * 100.f;

	EtVector2 vPos;
	float fWidthDist = fWidth / (float)m_nAlphaTableWidth;
	float fHeightDist = fHeight / (float)m_nAlphaTableHeight;
	float fValue;

	float fMin = (float)m_nMinAlpha;
	float fMax = (float)m_nMaxAlpha;
	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			vPos.x = ( fWidthDist / 2.f ) + ( ( vOffset.x - ( fWidth / 2.f ) ) + ( fWidthDist * i ) );
			vPos.y = ( fHeightDist / 2.f ) + ( ( vOffset.z - ( fHeight / 2.f ) ) + ( fHeightDist * j ) );
			fValue = m_pSector->GetHeightToWorld( vPos.x, vPos.y );
			if( fValue > m_fWaterLevel )
				m_pAlphaTable[j*m_nAlphaTableWidth+i] = (BYTE)fMin;
			else m_pAlphaTable[j*m_nAlphaTableWidth+i] = (BYTE)fMax;
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

float CTEtWorldWater::GetNewAlpha( float *pBuffer, int x, int y, int nBlurValue, float **ppfTable )
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

CTEtWorldWaterRiver *CTEtWorldWater::AddRiver( EtVector3 &vPos )
{
	CTEtWorldWaterRiver *pRiver = (CTEtWorldWaterRiver *)AllocRiver();
	CString szName = GetRiverUniqueName();
	pRiver->SetName( szName.GetBuffer() );
	pRiver->InsertPoint( vPos );
	m_pVecRiver.push_back( pRiver );

	return pRiver;
}

CString CTEtWorldWater::GetRiverUniqueName()
{
	int nCount = 0;
	char szStr[64];
	for( int i=0; i<1000; i++ ) {
		sprintf_s( szStr, "River - #%03d", i );
		if( GetRiverFromName( szStr ) == NULL ) return CString(szStr);
	}
	return "";
}

void CTEtWorldWater::RemoveRiver( CTEtWorldWaterRiver *pRiver )
{
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		if( m_pVecRiver[i] == pRiver ) {
			SAFE_DELETE( pRiver );
			m_pVecRiver.erase( m_pVecRiver.begin() + i );
			return;
		}
	}
}

bool CTEtWorldWater::RemoveRiverPoint( CTEtWorldWaterRiver::RiverPoint *pPoint )
{
	CTEtWorldWaterRiver *pRiver;
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		pRiver = (CTEtWorldWaterRiver *)m_pVecRiver[i];
		if( pRiver->RemovePoint( pPoint ) ) {
			if( pRiver->GetPointCount() <= 1 ) {
				RemoveRiver( pRiver );
			}
			return true;
		}
	}
	return false;
}

void CTEtWorldWater::AddEngineWaterCount()
{
	m_nEngineWaterCount++;
	GetEtWater()->Initialize( m_nEngineWaterCount );

	RefreshWaterTexture();
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		((CTEtWorldWaterRiver*)m_pVecRiver[i])->RefreshWaterTexture();
	}
}

void CTEtWorldWater::DelEngineWaterCount()
{
	if( m_nEngineWaterCount == 0 ) return;
	m_nEngineWaterCount--;
	GetEtWater()->Initialize( m_nEngineWaterCount );

	RefreshWaterTexture();
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		((CTEtWorldWaterRiver*)m_pVecRiver[i])->RefreshWaterTexture();
	}
}

int CTEtWorldWater::GetEngineWaterIndex( void *pWaterObject )
{
	int nIndex = 0;
	if( m_bEnable ) {
		nIndex++;
		if( pWaterObject == this ) return 0;
	}
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		if( ((CTEtWorldWaterRiver*)m_pVecRiver[i])->IsDestroy() ) {
			nIndex--;
			continue;
		}
		if( pWaterObject == m_pVecRiver[i] ) return nIndex + i;
	}
	return -1;
}
