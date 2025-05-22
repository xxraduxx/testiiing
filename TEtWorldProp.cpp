#include "StdAfx.h"
#include "TEtWorldProp.h"
#include "EtWorldSector.h"
#include "AxisRenderObject.h"
#include "RenderBase.h"
#include "EtOctree.h"
#include "TEtWorldSector.h"
#include "TEtWorld.h"
#include "DNTableFile.h"
#include "EtResourceMng.h"
#include "PropSignalItem.h"
#include "PropSignalManager.h"
#include "SundriesFuncEx.h"
#include "EtEngine.h"

PropertyGridBaseDefine g_PropPropertyDefine[] = {
	{ "Common", "Position", CUnionValueProperty::Vector3, "World Position", TRUE },
	{ "Common", "Rotation", CUnionValueProperty::Vector3, "World Rotation", TRUE },
	{ "Common", "Scale", CUnionValueProperty::Vector3, "World Scale", TRUE },
	{ "Common", "Cast Shadow", CUnionValueProperty::Boolean, "Cast Shadow", TRUE },
	{ "Common", "Receive Shadow", CUnionValueProperty::Boolean, "Receive Shadow", TRUE },
	{ "Common", "Ignore Build NavMesh", CUnionValueProperty::Boolean, "네비게이션 메쉬를 무시할껀지.", TRUE },	
	{ "Common", "Ignore Build ColMesh", CUnionValueProperty::Boolean, "컬리젼 메쉬를 무시할껀지.", TRUE },	
	{ "Common", "Action", CUnionValueProperty::Integer_Combo, "액션 리스트", TRUE },
	{ "Common", "Lock", CUnionValueProperty::Boolean, "컨트롤 락", TRUE },
	{ "Common", "NavMesh Type", CUnionValueProperty::Integer_Combo, "네비메쉬 타입|NONE|DRY_GROUND|SHORT_GRASS|LONG_GRASS|STONE|WET_GROUND|SAND|SNOW|WATER|TEMP1|TEMP2|TEMP3|TEMP4|TEMP5", TRUE },
	{ NULL },
};


CTEtWorldProp::CTEtWorldProp()
{
	m_pAxisRenderObject = NULL;
	m_bSelect = false;
	m_bShow = true;
	m_bIncludeNavMesh = true;

	CUnionValueProperty *pPropItem;
	for( int i=0; ; i++ ) {
		if( g_PropPropertyDefine[i].szCategory == NULL ) break;
		pPropItem = new CUnionValueProperty( g_PropPropertyDefine[i].Type );
		pPropItem->SetCategory( g_PropPropertyDefine[i].szCategory );
		pPropItem->SetDescription( g_PropPropertyDefine[i].szStr );
		pPropItem->SetSubDescription( g_PropPropertyDefine[i].szDescription );
		pPropItem->SetDefaultEnable( g_PropPropertyDefine[i].bEnable );
		m_pVecList.push_back( pPropItem );
		m_nCustomParamOffset = i + 1;
	}
}

CTEtWorldProp::~CTEtWorldProp()
{
	SAFE_DELETE_PVEC( m_pVecList );
	FreeObject();
}


bool CTEtWorldProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	CEtWorldProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );

	bool bResult = LoadObject();

	if( m_Handle ) m_Handle->SetCollisionGroup( COLLISION_GROUP_STATIC(1) );
	if( CTEtWorldSector::s_pPropSOX ) {
		int nIndex = CTEtWorldSector::s_pPropSOX->GetItemIDFromField( "_Name", m_szPropName.c_str() );
		if( nIndex != -1 ) {
			bool bIsStaticCollision = ( CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_IsStaticCollision" )->GetInteger() ) ? true : false;
			m_bIncludeNavMesh = ( CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_IsIncludeNavMesh" )->GetInteger() ) ? true : false;
			if( bIsStaticCollision == false ) {
				if( m_Handle ) m_Handle->SetCollisionGroup( COLLISION_GROUP_DYNAMIC(1) );
			}

			EnableCastLightmap( ( CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_IsCastLightmap" )->GetInteger() ) ? true : false );
			EnableLightmapInfluence( ( CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_IsLightmapInfluence" )->GetInteger() ) ? true : false );
			EnableCashWater( ( CTEtWorldSector::s_pPropSOX->GetFieldFromLablePtr( nIndex, "_IsCastWater" )->GetInteger() ) ? true : false );
		}
		else {
			EnableCastLightmap( true );
			EnableLightmapInfluence( false );
			EnableCashWater( true );
		}
	}

	return bResult;
}

bool CTEtWorldProp::LoadObject()
{
	if( !m_Handle ) {
		if( g_pEtRenderLock ) g_pEtRenderLock->Lock();
		m_Handle = EternityEngine::CreateStaticObject( m_szPropName.c_str() );
		if( g_pEtRenderLock ) g_pEtRenderLock->UnLock();
	}
	if( !m_Handle ) return false;

	UpdateMatrixEx();

	Show( true );
	m_Handle->EnableShadowCast( false );
	m_Handle->EnableShadowReceive( false );
	m_Handle->EnableLightMapCast( false );
	m_Handle->EnableLightMapInfluence( false );

	if( m_pAxisRenderObject == NULL ) {
		m_pAxisRenderObject = new CAxisRenderObject( this );
		m_pAxisRenderObject->Initialize();
		m_pAxisRenderObject->Show( false );
	}

	m_Handle->SetObjectAlpha( CGlobalValue::GetInstance().m_fPropAlpha );
	m_Handle->ShowCollisionPrimitive( CGlobalValue::GetInstance().m_bShowPropCollision );
	m_Handle->EnableCollision( !IsIgnoreBuildColMesh() );

	m_Handle->Update( m_matExWorld );
	m_pAxisRenderObject->Update( m_matExWorld );

	EnableCastShadow( m_bCastShadow );
	EnableReceiveShadow( m_bReceiveShadow );
	EnableCastLightmap( m_bCastLightmap );
	EnableLightmapInfluence( m_bLightmapInfluence );

	return true;
}

void CTEtWorldProp::LoadNavMesh()
{
	m_NavData.Clear();

	char szNavName[ _MAX_PATH ], *pFindPtr;
	strcpy_s( szNavName, _MAX_PATH, m_szPropName.c_str() );
	pFindPtr = strrchr( szNavName, '.' );
	if( pFindPtr )
	{
		strcpy_s( pFindPtr, 5, ".nav" );
		CResMngStream Stream( szNavName, true, true );
		if( Stream.IsValid() )
		{
			EtMatrix mat, matScale, matTemp;
			mat = *m_matExWorld;
			matTemp = mat;
			EtMatrixScaling( &matScale, m_vScale.x, m_vScale.y, m_vScale.z );
			EtMatrixMultiply( &matTemp, &matScale, &mat );

			m_NavData.LoadFromStream( &Stream, &matTemp );
			m_NavData.LinkCells();
		}
	}
}

void CTEtWorldProp::UpdateMatrixEx()
{
	m_matExWorld.Identity();

	EtVector3 vOffset = *m_pParentSector->GetOffset();
	vOffset.x -= ( m_pParentSector->GetTileWidthCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( m_pParentSector->GetTileHeightCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;

	m_matExWorld.m_vPosition = m_vPosition + vOffset;
	m_matExWorld.RotatePitch( m_vRotation.x );
	m_matExWorld.RotateRoll( m_vRotation.z );
	m_matExWorld.RotateYaw( m_vRotation.y );
	if( m_Handle ) m_Handle->Update( m_matExWorld );

	if( m_pCurOctreeNode ) {
		SSphere Sphere;
		GetBoundingSphere( Sphere );
		m_pCurOctreeNode = ((CTEtWorldSector*)m_pParentSector)->GetPropOctree()->Update( this, Sphere, m_pCurOctreeNode );
	}
	if( m_pAxisRenderObject ) m_pAxisRenderObject->Update( m_matExWorld );
}

bool CTEtWorldProp::FreeObject()
{
	m_bSelect = false;
	if( m_pAxisRenderObject ) {
		m_pAxisRenderObject->Show( false );
		if( CAxisRenderObject::m_nSelectAxis != -1 )
			CGlobalValue::GetInstance().m_bControlAxis = false;
	}
	SAFE_DELETE( m_pAxisRenderObject );
	SAFE_RELEASE_SPTR( m_Handle );
	return true;
}

bool CTEtWorldProp::Render( LOCAL_TIME LocalTime )
{
	if( !m_Handle ) return false;

	EtMatrix mat, matScale, matTemp;
	mat = *m_matExWorld;
	matTemp = mat;
	EtMatrixScaling( &matScale, m_vScale.x, m_vScale.y, m_vScale.z );
	EtMatrixMultiply( &matTemp, &matScale, &mat );

	if( CGlobalValue::GetInstance().m_bShowPropCollision )
		m_Handle->EnableCollision( !IsIgnoreBuildColMesh() );
	m_Handle->SetCollisionScale( m_vScale );
	m_Handle->Update( &matTemp );

	if( m_Handle->GetMesh()->GetSubMeshCount() == 0 && CGlobalValue::GetInstance().m_bDrawRevisionEmptyMeshProp ) {
		if( m_bSelect )
			DrawBoundingBox( mat, D3DCOLOR_XRGB( 255, 255, 255 ), 1.f );
		else DrawBoundingBox( mat, D3DCOLOR_XRGB( 255, 0, 0), 1.f );
	}
	else {
		if( m_bSelect ) {
			DrawBoundingBox( mat );
		}
	}

	return true;
}

bool CTEtWorldProp::IsEmptyPickMesh()
{
	bool bResult = false;

	if( m_Handle->GetMesh()->GetSubMeshCount() == 0 ) bResult = true;

	return bResult;
}

void CTEtWorldProp::DrawBoundingBox( EtMatrix &mat, D3DCOLOR Color, float fLengthPer )
{
	m_pAxisRenderObject->Update( &mat );
	float fLength = EtVec3Length( &( m_matExWorld.m_vPosition - CRenderBase::GetInstance().GetLastUpdateCamera()->m_vPosition ) );
	m_pAxisRenderObject->SetScale( fLength / 200.f );

	SOBB Box;
	if( m_Handle ) {
		m_Handle->GetBoundingBox( Box );
		/*
		/*SAABox AABox;
		
		EtMatrix matIdentity;
		EtMatrixIdentity( &matIdentity );
		Box.Init( AABox, matIdentity );
		*/
	}
	std::vector<EtVector3> vVecList;
	for( DWORD i=0; i<8; i++ ) vVecList.push_back( Box.Vertices[i] );
	/*
	for( int i=0; i<(int)vVecList.size(); i+=2 ) {
		EtVec3TransformCoord( &vVecList[i], &vVecList[i], &mat );
		EtVec3TransformCoord( &vVecList[i+1], &vVecList[i+1], &mat );
	}
	*/

	EtVector3 vPos[2];

	vPos[0] = vVecList[0];
	vPos[1] = vPos[0] + ( ( vVecList[1] - vVecList[0] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[2] - vVecList[0] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[4] - vVecList[0] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );


	vPos[0] = vVecList[1];
	vPos[1] = vPos[0] + ( ( vVecList[0] - vVecList[1] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[3] - vVecList[1] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[5] - vVecList[1] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[0] = vVecList[2];
	vPos[1] = vPos[0] + ( ( vVecList[0] - vVecList[2] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[3] - vVecList[2] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[6] - vVecList[2] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );


	vPos[0] = vVecList[3];
	vPos[1] = vPos[0] + ( ( vVecList[1] - vVecList[3] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[2] - vVecList[3] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[7] - vVecList[3] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );


	vPos[0] = vVecList[4];
	vPos[1] = vPos[0] + ( ( vVecList[0] - vVecList[4] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[5] - vVecList[4] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[6] - vVecList[4] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[0] = vVecList[5];
	vPos[1] = vPos[0] + ( ( vVecList[1] - vVecList[5] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[4] - vVecList[5] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[7] - vVecList[5] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );


	vPos[0] = vVecList[6];
	vPos[1] = vPos[0] + ( ( vVecList[2] - vVecList[6] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[4] - vVecList[6] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[7] - vVecList[6] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[0] = vVecList[7];
	vPos[1] = vPos[0] + ( ( vVecList[3] - vVecList[7] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[5] - vVecList[7] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );

	vPos[1] = vPos[0] + ( ( vVecList[6] - vVecList[7] ) * fLengthPer );
	EternityEngine::DrawLine3D( vPos[0], vPos[1], Color );
} 

void CTEtWorldProp::SetSelect( bool bValue )
{
	m_bSelect = bValue;
	if( !m_pAxisRenderObject ) return;
	m_pAxisRenderObject->Show( m_bSelect );
}

bool CTEtWorldProp::CheckAxis( EtVector3 &vOrig, EtVector3 &vDir )
{
	if( !m_pAxisRenderObject ) return false;
	if( m_bControlLock ) return false;
	if( m_pAxisRenderObject->CheckAxis( vOrig, vDir ) != -1 ) return true;
	return false;
}

void CTEtWorldProp::MoveAxis( int nX, int nY, float fSpeed )
{
	if( !m_pAxisRenderObject ) return;
	if( m_bControlLock ) return;
	bool bMoveable = true;
	if( CAxisRenderObject::m_nSelectAxis != -1 ) {
		if( GetAKState( VK_LCONTROL ) < 0 ) {
			bMoveable = false;
			float fValue = ( nX * 0.1f ) + ( nY * 0.1f );
			m_vPreRotation = m_vRotation;
			switch( CAxisRenderObject::m_nSelectAxis ) {
				case 0:	m_vRotation.x += fValue;	
					UpdateMatrixEx();
					break;
				case 1:	m_vRotation.y += fValue;	
					UpdateMatrixEx();
					break;
				case 2:	m_vRotation.z += fValue;	
					UpdateMatrixEx();
					break;
			}
		}
		if( GetAKState( VK_LMENU ) < 0 ) {
			bMoveable = false;
			float fValue = ( nX * 0.005f ) + ( -nY * 0.005f );
			switch( CAxisRenderObject::m_nSelectAxis ) {
				case 0:	m_vScale.x += fValue;	break;
				case 1:	m_vScale.y += fValue;	break;
				case 2:	m_vScale.z += fValue;	break;
				case 3:	
					m_vScale.x += fValue;
					m_vScale.y += fValue;
					m_vScale.z += fValue;
					break;
			}
		}
	}
	if( bMoveable )
		m_pAxisRenderObject->MoveAxis( nX, nY, fSpeed );

}

/*
int CTEtWorldProp::GetAdditionalPropInfoSize()
{
	return 0;
}

void CTEtWorldProp::AllocAdditionalPropInfo()
{
}
*/

void CTEtWorldProp::WritePropInfo( FILE *fp )
{
	int nSize = sizeof(PropStruct);// + GetAdditionalPropInfoSize();
	long nTell = ftell(fp);
	fwrite( &nSize, sizeof(int), 1, fp );

	PropStruct Struct;
	memset( &Struct, 0, sizeof(PropStruct) );

	sprintf_s( Struct.szPropName, m_szPropName.c_str() );
	Struct.vPosition = m_vPosition;
	Struct.vRotation = m_vRotation;
	Struct.vScale = m_vScale;

	Struct.bCastShadow = m_bCastShadow;
	Struct.bReceiveShadow = m_bReceiveShadow;
	Struct.nUniqueID = m_nCreateUniqueID;
	Struct.bIgnoreBuildNavMesh = m_bIgnoreBuildNavMesh;
	Struct.bIgnoreBuildColMesh = m_bIgnoreBuildColMesh;
	Struct.bControlLock = m_bControlLock;
	Struct.nNavType = m_nNavType;	

	fwrite( &Struct, sizeof(PropStruct), 1, fp );

	WriteCustomInfo(fp);

	long nLastTell = ftell(fp);
	fseek( fp, nTell, SEEK_SET );
	nSize = nLastTell - nTell - sizeof(int);
	fwrite( &nSize, sizeof(int), 1, fp );
	fseek( fp, nLastTell, SEEK_SET );
	/*
	if( GetAdditionalPropInfoSize() > 0 ) 
		fwrite( GetAdditionalPropInfo(), GetAdditionalPropInfoSize(), 1, fp );
	*/
}

bool CTEtWorldProp::ReadPropInfo( FILE *fp )
{
	int nCurSize = 0;
	PropStruct Struct;
	memset( &Struct, 0, sizeof(PropStruct) );

	fread( &nCurSize, sizeof(int), 1, fp );
	fread( &Struct, sizeof(PropStruct), 1, fp );
	m_szPropName = Struct.szPropName;
	m_vPosition = Struct.vPosition;
	m_vRotation = Struct.vRotation;
	m_vScale = Struct.vScale;
	m_bCastShadow = Struct.bCastShadow;
	m_bReceiveShadow = Struct.bReceiveShadow;
	m_nCreateUniqueID = Struct.nUniqueID;
	m_bIgnoreBuildNavMesh = Struct.bIgnoreBuildNavMesh;
	m_bIgnoreBuildColMesh = Struct.bIgnoreBuildColMesh;
	m_bControlLock = Struct.bControlLock;
	m_nNavType = Struct.nNavType;

	ReadCustomInfo(fp);

	return true;
}

void CTEtWorldProp::WriteCustomInfo( FILE *fp )
{
	CalcArray();

	union {
		int nBuffer[256];
		float fBuffer[256];
	};
	memset( nBuffer, 0, sizeof(nBuffer) );

	int nVector2Index = 0;
	int nVector3Index = 0;
	int nVector4Index = 0;
	int nStringIndex = 0;

	CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex( m_nClassID );

	int nOffset;
	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);
		switch( m_pVecList[i]->GetType() ) {
			case CUnionValueProperty::Integer:
			case CUnionValueProperty::Integer_Combo:
			case CUnionValueProperty::Color:
				nBuffer[nOffset] = m_pVecList[i]->GetVariableInt();
				break;
			case CUnionValueProperty::Boolean:
				nBuffer[nOffset] = (BOOL)m_pVecList[i]->GetVariableBool();
				break;
			case CUnionValueProperty::Float:
				fBuffer[nOffset] = m_pVecList[i]->GetVariableFloat();
				break;
			case CUnionValueProperty::Vector2:
				nBuffer[nOffset] = nVector2Index;
				nVector2Index++;
				break;
			case CUnionValueProperty::Vector3:
				nBuffer[nOffset] = nVector3Index;
				nVector3Index++;
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				nBuffer[nOffset] = nVector4Index;
				nVector4Index++;
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				nBuffer[nOffset] = nStringIndex;
				nStringIndex++;
				break;
		}
	}
	fwrite( nBuffer, sizeof(nBuffer), 1, fp );

	// Array 저장
	fwrite( &nVector2Index, sizeof(int), 1, fp );
	if( m_VecVector2List.size() ) {
		fwrite( &m_nVecVector2TableIndexList[0], sizeof(int), m_nVecVector2TableIndexList.size(), fp );
		fwrite( &m_VecVector2List[0], sizeof(EtVector2), m_VecVector2List.size(), fp );
	}

	fwrite( &nVector3Index, sizeof(int), 1, fp );
	if( m_VecVector3List.size() ) {
		fwrite( &m_nVecVector3TableIndexList[0], sizeof(int), m_nVecVector3TableIndexList.size(), fp );
		fwrite( &m_VecVector3List[0], sizeof(EtVector3), m_VecVector3List.size(), fp );
	}

	fwrite( &nVector4Index, sizeof(int), 1, fp );
	if( m_VecVector4List.size() ) {
		fwrite( &m_nVecVector4TableIndexList[0], sizeof(int), m_nVecVector4TableIndexList.size(), fp );
		fwrite( &m_VecVector4List[0], sizeof(EtVector4), m_VecVector4List.size(), fp );
	}

	fwrite( &nStringIndex, sizeof(int), 1, fp );
	if( m_VecStringList.size() )
		fwrite( &m_nVecStringTableIndexList[0], sizeof(int), m_nVecStringTableIndexList.size(), fp );
	for( DWORD i=0; i<m_VecStringList.size(); i++ ) {
		WriteCString( &m_VecStringList[i], fp );
	}
}

void CTEtWorldProp::ReadCustomInfo( FILE *fp )
{
	union {
		int nBuffer[256];
		float fBuffer[256];
	};
	memset( nBuffer, 0, sizeof(nBuffer));
	fread( nBuffer, sizeof(nBuffer), 1, fp );

	int nVector2Index = 0;
	int nVector3Index = 0;
	int nVector4Index = 0;
	int nStringIndex = 0;
	m_VecVector2List.clear();
	m_VecVector3List.clear();
	m_VecVector4List.clear();
	m_VecStringList.clear();
	m_nVecVector2TableIndexList.clear();
	m_nVecVector3TableIndexList.clear();
	m_nVecVector4TableIndexList.clear();
	m_nVecStringTableIndexList.clear();

	int nCount;
	EtVector2 Vector2;
	EtVector3 Vector3;
	EtVector4 Vector4;
	CString szStr;

	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector2List.resize( nCount );
		fread( &m_VecVector2List[0], sizeof(EtVector2), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector3List.resize( nCount );
		fread( &m_VecVector3List[0], sizeof(EtVector3), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector4List.resize( nCount );
		fread( &m_VecVector4List[0], sizeof(EtVector4), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	fseek( fp, sizeof(int) * nCount, SEEK_CUR );
	for( int i=0; i<nCount; i++ ) {
		ReadCString( &szStr, fp );
		m_VecStringList.push_back( szStr );
	}

	CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex( m_nClassID );

	int nOffset;

	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		CUnionValueProperty *pSource = pItem->GetParameter(i-m_nCustomParamOffset);
		CUnionValueProperty *pVariable = m_pVecList[i];
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);

		switch( pSource->GetType() ) {
			case CUnionValueProperty::Integer:
			case CUnionValueProperty::Integer_Combo:
			case CUnionValueProperty::Color:
				pVariable->SetVariable( (int)nBuffer[nOffset] );
				break;
			case CUnionValueProperty::Boolean:
				pVariable->SetVariable( (bool)(nBuffer[nOffset]==TRUE) );
				break;
			case CUnionValueProperty::Float:
				pVariable->SetVariable( fBuffer[nOffset] );
				break;
			case CUnionValueProperty::Vector2:
				if( nVector2Index < (int)m_VecVector2List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR2)m_VecVector2List[nVector2Index] );
					nVector2Index++;
				}
				break;
			case CUnionValueProperty::Vector3:
				if( nVector3Index < (int)m_VecVector3List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR3)m_VecVector3List[nVector3Index] );
					nVector3Index++;
				}
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				if( nVector4Index < (int)m_VecVector4List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR4)m_VecVector4List[nVector4Index] );
					nVector4Index++;
				}
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				if( nStringIndex < (int)m_VecStringList.size() ) {
					pVariable->SetVariable( (char*)m_VecStringList[nStringIndex].GetBuffer() );
					nStringIndex++;
				}
				break;
		}
	}
}


void CTEtWorldProp::EnableCastShadow( bool bEnable )
{
	CEtWorldProp::EnableCastShadow( bEnable );
	if( m_Handle ) m_Handle->EnableShadowCast( bEnable );

	/*
	bool bCastLightmap;
	if( !IsCastShadow() && !IsReceiveShadow() ) bCastLightmap = true;
	else bCastLightmap = false;
	m_Handle->EnableLightMapCast( bCastLightmap );
	*/
}

void CTEtWorldProp::EnableReceiveShadow( bool bEnable )
{
	CEtWorldProp::EnableReceiveShadow( bEnable );
	if( m_Handle ) m_Handle->EnableShadowReceive( bEnable );

	/*
	bool bCastLightmap;
	if( !IsCastShadow() && !IsReceiveShadow() ) bCastLightmap = true;
	else bCastLightmap = false;
	m_Handle->EnableLightMapCast( bCastLightmap );
	*/
}

void CTEtWorldProp::EnableCastLightmap( bool bEnable )
{
	CEtWorldProp::EnableCastLightmap( bEnable );
	if( m_Handle ) m_Handle->EnableLightMapCast( bEnable );
}

void CTEtWorldProp::EnableLightmapInfluence( bool bEnable )
{
	CEtWorldProp::EnableLightmapInfluence( bEnable );
	if( m_Handle ) m_Handle->EnableLightMapInfluence( bEnable );
}

void CTEtWorldProp::EnableCashWater( bool bEnable )
{
	CEtWorldProp::EnableCastWater( bEnable );
	if( m_Handle ) m_Handle->EnableWaterCast( bEnable );
}
/*
PropertyGridBaseDefine *CTEtWorldProp::GetPropertyDefine()
{
	return g_PropPropertyDefine;
}
*/

void CTEtWorldProp::OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
	switch( dwIndex ) {
		case 0:
			pVariable->SetVariable( m_matExWorld.m_vPosition );
			break;
		case 1:
			pVariable->SetVariable( *GetRotation() );
			break;
		case 2:
			pVariable->SetVariable( *GetScale() );
			break;
		case 3:
			pVariable->SetVariable( IsCastShadow() );
			break;
		case 4:
			pVariable->SetVariable( IsReceiveShadow() );
			break;
		case 5:
			pVariable->SetVariable( IsIgnoreBuildNavMesh() );
			break;
		case 6:
			pVariable->SetVariable( IsIgnoreBuildColMesh() );
			break;
		case 7:
			pProperty->SetReadOnly( pVariable, true );
			break;
		case 8:
			pVariable->SetVariable( IsControlLock() );
			break;
		case 9:
			pVariable->SetVariable( GetNavType() );
			break;
	}
}

void CTEtWorldProp::OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex, CPropertyGridImp *pProperty )
{
	switch( dwIndex ) {
		case 0:
			{
				SectorIndex Sector = CGlobalValue::GetInstance().m_SelectGrid;
				if( Sector == -1 ) break;
				CTEtWorldSector *pSector = CTEtWorld::GetInstance().GetSector( Sector );

				EtVector3 vPos = pVariable->GetVariableVector3();
				EtVector3 vOffset = *pSector->GetOffset();
				vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.z -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
				vOffset.y = 0.f;
				vPos -= vOffset;

				SetPosition( vPos );
				UpdateMatrixEx();

				CGlobalValue::GetInstance().RefreshRender();
			}
			break;
		case 1:
			SetRotation( pVariable->GetVariableVector3() );
			UpdateMatrixEx();

			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 2:
			SetScale( pVariable->GetVariableVector3() );
			UpdateMatrixEx();

			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 3:
			EnableCastShadow( pVariable->GetVariableBool() );
			break;
		case 4:
			EnableReceiveShadow( pVariable->GetVariableBool() );
			break;
		case 5:
			EnableIgnoreBuildNavMesh( pVariable->GetVariableBool() );
			break;
		case 6:
			EnableIgnoreBuildColMesh( pVariable->GetVariableBool() );
			CGlobalValue::GetInstance().RefreshRender();
			break;
		case 8:
			EnableControlLock( pVariable->GetVariableBool() );
			break;
		case 9:
			SetNavType( pVariable->GetVariableInt() );
			break;
	}
}

void CTEtWorldProp::Show( bool bShow )
{
	m_bShow = bShow;
	if( m_Handle ) m_Handle->ShowObject( bShow );
}

void CTEtWorldProp::CalcArray()
{
	m_VecVector2List.clear();
	m_VecVector3List.clear();
	m_VecVector4List.clear();
	m_VecStringList.clear();
	m_nVecVector2TableIndexList.clear();
	m_nVecVector3TableIndexList.clear();
	m_nVecVector4TableIndexList.clear();
	m_nVecStringTableIndexList.clear();

	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		CUnionValueProperty *pVariable = m_pVecList[i];
		switch( pVariable->GetType() ) {
			case CUnionValueProperty::Vector2:
				m_VecVector2List.push_back( pVariable->GetVariableVector2() );
				m_nVecVector2TableIndexList.push_back(i-m_nCustomParamOffset);
				break;
			case CUnionValueProperty::Vector3:
				m_VecVector3List.push_back( pVariable->GetVariableVector3() );
				m_nVecVector3TableIndexList.push_back(i-m_nCustomParamOffset);
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				m_VecVector4List.push_back( pVariable->GetVariableVector4() );
				m_nVecVector4TableIndexList.push_back(i-m_nCustomParamOffset);
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				m_VecStringList.push_back( CString( pVariable->GetVariableString() ) );
				m_nVecStringTableIndexList.push_back(i-m_nCustomParamOffset);
				break;
		}
	}
}

void CTEtWorldProp::AllocCustomInfo()
{
	CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex( GetClassID() );
	if( pItem != NULL ) {
		CUnionValueProperty *pPropItem;
		for( DWORD i=0; i<pItem->GetParameterCount(); i++ ) {
			CUnionValueProperty *pVariable = pItem->GetParameter(i);
			pPropItem = new CUnionValueProperty( pVariable->GetType() );
			*pPropItem = *pVariable;
			pPropItem->SetCategory( "Custom Parameter" );
			m_pVecList.push_back( pPropItem );
			SetDefaultCustomValue( GetClassID(), (DWORD)m_pVecList.size()-1 );
		}
	}
}

void CTEtWorldProp::SetDefaultCustomValue( int nClass, DWORD dwIndex )
{
	DWORD dwCustomIndex = dwIndex - m_nCustomParamOffset;

	if( nClass == 12 ) {	// PTE_Buff
		switch( dwCustomIndex )
		{
		case 0:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 25006 );
			break;
		case 1:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 20230 );
			break;
		case 2:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 1 );
			break;
		case 3:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 1000.0f );
			break;
		case 4:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 0 );
			break;
		}
	}
	else if( nClass == 13 ) {	// PTE_BuffBroken
		switch( dwCustomIndex )
		{
		case 0:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 25002 );
			break;
		case 1:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 2000 );
			break;
		case 2:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 0 );
			break;
		case 3:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 20201 );
			break;
		case 4:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 1 );
			break;
		case 5:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 1000.0f );
			break;
		case 6:
			m_pVecList[m_nCustomParamOffset+dwCustomIndex]->SetVariable( 1 );
			break;
		}
	}
}

void CTEtWorldProp::CopyCustomParameter( CTEtWorldProp *pProp )
{
	SAFE_DELETE_PVEC( m_pVecList );
	CUnionValueProperty *pVariable;
	for( DWORD i=0; i<pProp->m_pVecList.size(); i++ ) {
		pVariable = new CUnionValueProperty( pProp->m_pVecList[i]->GetType() );
		*pVariable = *pProp->m_pVecList[i];
		m_pVecList.push_back( pVariable );
	}
}


#ifdef _CHECK_MISSING_INDEX
bool CTEtWorldProp::ReadPropInfo_ForTheLog( FILE *fp )
{
	int nCurSize = 0;
	PropStruct Struct;
	memset( &Struct, 0, sizeof(PropStruct) );

	fread( &nCurSize, sizeof(int), 1, fp );
	fread( &Struct, sizeof(PropStruct), 1, fp );
	m_szPropName = Struct.szPropName;
	m_vPosition = Struct.vPosition;
	m_vRotation = Struct.vRotation;
	m_vScale = Struct.vScale;
	m_bCastShadow = Struct.bCastShadow;
	m_bReceiveShadow = Struct.bReceiveShadow;
	m_nCreateUniqueID = Struct.nUniqueID;
	m_bIgnoreBuildNavMesh = Struct.bIgnoreBuildNavMesh;
	m_bIgnoreBuildColMesh = Struct.bIgnoreBuildColMesh;
	m_bControlLock = Struct.bControlLock;

	ReadCustomInfo_ForTheLog(fp);

	return true;
}

void CTEtWorldProp::ReadCustomInfo_ForTheLog( FILE *fp )
{
	union {
		int nBuffer[256];
		float fBuffer[256];
	};
	memset( nBuffer, 0, sizeof(nBuffer));
	fread( nBuffer, sizeof(nBuffer), 1, fp );

	int nVector2Index = 0;
	int nVector3Index = 0;
	int nVector4Index = 0;
	int nStringIndex = 0;
	m_VecVector2List.clear();
	m_VecVector3List.clear();
	m_VecVector4List.clear();
	m_VecStringList.clear();
	m_nVecVector2TableIndexList.clear();
	m_nVecVector3TableIndexList.clear();
	m_nVecVector4TableIndexList.clear();
	m_nVecStringTableIndexList.clear();

	int nCount;
	EtVector2 Vector2;
	EtVector3 Vector3;
	EtVector4 Vector4;
	CString szStr;

	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector2List.resize( nCount );
		fread( &m_VecVector2List[0], sizeof(EtVector2), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector3List.resize( nCount );
		fread( &m_VecVector3List[0], sizeof(EtVector3), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	if( nCount ) {
		fseek( fp, sizeof(int) * nCount, SEEK_CUR );
		m_VecVector4List.resize( nCount );
		fread( &m_VecVector4List[0], sizeof(EtVector4), nCount, fp );
	}
	fread( &nCount, sizeof(int), 1, fp );
	fseek( fp, sizeof(int) * nCount, SEEK_CUR );
	for( int i=0; i<nCount; i++ ) {
		ReadCString( &szStr, fp );
		m_VecStringList.push_back( szStr );
	}

	CPropSignalItem *pItem = CPropSignalManager::GetInstance().GetSignalItemFromUniqueIndex( m_nClassID );

	int nOffset;

	for( DWORD i=m_nCustomParamOffset; i<m_pVecList.size(); i++ ) {
		CUnionValueProperty *pSource = pItem->GetParameter(i-m_nCustomParamOffset);
		CUnionValueProperty *pVariable = m_pVecList[i];
		nOffset = pItem->GetParameterUniqueIndex(i-m_nCustomParamOffset);

		switch( pSource->GetType() ) {
			case CUnionValueProperty::Integer:
			case CUnionValueProperty::Integer_Combo:
			case CUnionValueProperty::Color:
				pVariable->SetVariable( (int)nBuffer[nOffset] );
				break;
			case CUnionValueProperty::Boolean:
				pVariable->SetVariable( (bool)(nBuffer[nOffset]==TRUE) );
				break;
			case CUnionValueProperty::Float:
				pVariable->SetVariable( fBuffer[nOffset] );
				break;
			case CUnionValueProperty::Vector2:
				if( nVector2Index < (int)m_VecVector2List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR2)m_VecVector2List[nVector2Index] );
					nVector2Index++;
				}
				break;
			case CUnionValueProperty::Vector3:
				if( nVector3Index < (int)m_VecVector3List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR3)m_VecVector3List[nVector3Index] );
					nVector3Index++;
				}
				break;
			case CUnionValueProperty::Vector4:
			case CUnionValueProperty::Vector4Color:
				if( nVector4Index < (int)m_VecVector4List.size() ) {
					pVariable->SetVariable( (D3DXVECTOR4)m_VecVector4List[nVector4Index] );
					nVector4Index++;
				}
				break;
			case CUnionValueProperty::String:
			case CUnionValueProperty::String_FileOpen:
				if( nStringIndex < (int)m_VecStringList.size() ) {
					pVariable->SetVariable( (char*)m_VecStringList[nStringIndex].GetBuffer() );
					nStringIndex++;
				}
				break;
		}

		if( CGlobalValue::GetInstance().m_bIsFirst )
		{
			CGlobalValue::GetInstance().m_strPreviousGridName = pItem->GetName();
		}

		if( CGlobalValue::GetInstance().m_bIsFirst )
		{
			if( strcmp( pVariable->GetCategory() , "Custom Parameter" ) == 0 && strcmp( pVariable->GetDescription() , "nSkillTableID" ) == 0 )
			{
				if( pVariable->GetVariableInt() <= 0 )
				{
					LogWnd::Log( 0, _T("Grid: %s / PropType:%s"), CGlobalValue::GetInstance().m_szFolderName.c_str() , pItem->GetName() );
					CGlobalValue::GetInstance().m_bIsFirst = false;
				}
			}
		}
		else
		{
			if( strcmp( CGlobalValue::GetInstance().m_strPreviousGridName , pItem->GetName() ) == 1 )
			{
				if( strcmp( pVariable->GetCategory() , "Custom Parameter" ) == 0 && strcmp( pVariable->GetDescription() , "nSkillTableID" ) == 0 )
				{
					if( pVariable->GetVariableInt() <= 0 )
					{
						LogWnd::Log( 0, _T("Grid: %s / PropType:%s"), CGlobalValue::GetInstance().m_szFolderName.c_str() , pItem->GetName() );
						CGlobalValue::GetInstance().m_strPreviousGridName = pItem->GetName();
					}
				}
			}
		}
	} // end of for
}
#endif