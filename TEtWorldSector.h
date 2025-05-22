#pragma once

#include "EtWorldSector.h"
#include "Timer.h"
#include "EtQuadtree.h"


class CPropPoolStage;
class CTEtWorldProp;
class CEtWorldProp;
class CActionElement;
class CEtWorldEventArea;
class CEtWorldSoundEnvi;
class CNavCell;
class NavigationMesh;
class NavigationCell;

namespace EtNavigationMesh
{
	class CEtNavigationMeshGenerator;
}

using namespace EtNavigationMesh;

#include "DNTableFile.h"
class CTEtWorldDecal;
class CTEtWorldWaterRiver;

class CTEtWorldSector : public CEtWorldSector
{
public:
	CTEtWorldSector();
	virtual ~CTEtWorldSector();

	enum SECTOR_RW_LEVEL {
		SRL_EMPTY = 0x0000,
		SRL_PREVIEW = 0x0001, // 미리보기, 이름정도.
		SRL_TERRAIN = 0x0002,	// Layer, Height
		SRL_PROP = 0x0004,	// Prop 정보
		SRL_EVENT = 0x0008,	// Event 정보
		SRL_SOUND = 0x0010,	// Sound 정보
		SRL_ATTRIBUTE = 0x0020,// 바닥속성 정보
		SRL_NAVIGATION = 0x0040, // 네비게이션 메쉬 정보
		SRL_TRIGGER = 0x0080, // 트리거 정보
		SRL_WATER = 0x0100, // 물 정보
	};

	struct EventControlStruct {
		char *szControlName;
		DWORD dwColor;
		DWORD dwSideColor;
		DWORD dwSelectColor;
		DWORD dwSelectSideColor;
		float fYOrder;
	};

	static DNTableFileFormat *s_pPropSOX;

	enum INFO_FILE_TYPE {
		IFT_CREATE = 0,
		IFT_READ,
		IFT_WRITE,
		IFT_DELETE,
	};
protected:

	bool m_bEmpty;
	CString m_szDescription;
	DWORD *m_pAlpha;
	char *m_pGrass;
	EtBaseTexture *m_pThumbnail;
	EtVector3 m_vPickPos;
	EtVector3 m_vPickPrevPos;
	std::vector<int> m_nVecBlockTypeList;
	std::vector< std::vector<CString> > m_szVecLayerTexture;
	std::vector< std::vector<float> > m_fVecLayerTextureDistance;
	std::vector< std::vector<float> > m_fVecLayerTextureRotation;
	CString m_szGrassTexture;
	EtVector3 m_vAreaOffset;
	int m_nSectorRWLevel;
	CRect m_rcLastUpdate;
	int m_nBlockCount;
	bool m_bShowWater;

	float m_fGrassWidth[4];
	float m_fGrassHeightMin[4];
	float m_fGrassHeightMax[4];
	float m_fGrassShakeMin;
	float m_fGrassShakeMax;

	int m_nDrawAttributeCount;
	std::vector<DWORD> m_dwVecAttributePrimitiveCount;
	std::vector<SPrimitiveDraw3D *> m_pVecAttributePrimitive;
	std::vector<bool *> m_bVecAttributePrimitiveType;

	CEtNavigationMeshGenerator *m_pNavCell;
	NavigationMesh *m_pNavMesh;

	EtVector3 m_vEventAreaPrevCenter;

	float m_fDecalRotate;
	EtColor m_vDecalColor;
	float m_fDecalAlpha;
	CString m_szDecalTextureName;
	CString m_szReadyOnlyMsg;

	bool m_bShowAttribute;
	bool m_bShowNavigation;
	bool m_bShowEventArea;

protected:
	virtual void InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, std::vector< std::vector<float> > &fVecLayerTextureDistance ) {}
	bool CreateTerrain( STerrainInfo *pInfo = NULL );
	STerrainInfo GetCreateTerrainInfo();

	bool HeightmapFile( INFO_FILE_TYPE Type );
	bool AlphaTableFile( INFO_FILE_TYPE Type );
	bool GrassTableFile( INFO_FILE_TYPE Type );
	bool TextureTableFile( INFO_FILE_TYPE Type );
	bool ThumbnailFile( INFO_FILE_TYPE Type );
//	bool PropInfoFile( INFO_FILE_TYPE Type, bool bReadOnly = false );
	bool EventInfoFile( INFO_FILE_TYPE Type );
	bool SoundInfoFile( INFO_FILE_TYPE Type );
	bool AttributeInfoFile( INFO_FILE_TYPE Type );
	bool NavigationInfoFile( INFO_FILE_TYPE Type );
	bool TriggerInfoFile( INFO_FILE_TYPE Type );
	bool WaterInfoFile( INFO_FILE_TYPE Type );
	bool DecalInfoFile( INFO_FILE_TYPE Type, bool bReadOnly = false );
	bool SectorSizeInfoFile( INFO_FILE_TYPE Type );

	void FlushResource();
	bool CheckExistLoadLevel( SECTOR_RW_LEVEL Level, bool bAdd );

	CString GetWorkingPath();
	bool IsReadOnlyFile( const char *szFileName );

	void CalcSectorSize( float &fHeightCenter, float &fSize );

	bool WriteCollisionHeight( FILE *fp );

public:
	bool DefaultInfoFile( INFO_FILE_TYPE Type );
	bool PropInfoFile( INFO_FILE_TYPE Type, bool bReadOnly = false );

	virtual NavigationMesh *GetNavMesh() { return m_pNavMesh; }

public:
	virtual bool Initialize( CEtWorldGrid *pParentGrid, SectorIndex Index );

	bool IsEmpty() { return m_bEmpty ;}
	void Show( bool bShow );
	void ShowProp( bool bShow );
	void ShowWater( bool bShow );
	void ShowPropCollision( bool bShow );
	void ShowAttribute( bool bShow );
	void ShowNavigation( bool bShow );
	void ShowEventArea( bool bShow );

	CString GetDescription() { return m_szDescription ;}
	void SetDescription( CString szStr );

	bool LoadSector( int Level );
	bool SaveSector( int Level, bool bWarningMsg = false );
	bool FreeSector( int Level );

	bool CreateSector();
	bool EmptySector();
	bool IsExistRWLevel( SECTOR_RW_LEVEL Level ) { return ( m_nSectorRWLevel & Level ) ? true : false ; }
	EtBaseTexture *GetThumbnail() { return m_pThumbnail; }

	void ResetUpdateInfo();
	void ResetUpdateGrassInfo();

	int GetPropIndexFromPtr( CEtWorldProp *pProp );

	void UpdateBrush( int nX, int nY, float fRadian, DWORD dwColor );
	bool BeginRectBrush( int nX, int nY );
	void PickProp( int nX, int nY, DNVector(CEtWorldProp *) &pVecList );
	void UpdateRectBrush( int nX, int nY, DWORD dwColor, DNVector(CEtWorldProp *) &pVecList );

	int UpdateAreaControlType( int nX, int nY, const char *szControlName );
	CEtWorldEventArea *UpdateAreaRectBrush( int nX, int nY, DWORD dwColor, const char *szControlName );
	void ControlEventArea( int nX, int nY, int nControlType );

	CEtWorldSoundEnvi *UpdateSoundRectBrush( int nX, int nY, DWORD dwColor );
	void ControlSound( int nX, int nY, int nControlType );

	void ApplyHeight( CRect &rcRect, int nPtrWidth, int nPtrHeight, short *pHeight, float fEpsilon );
	void ApplyHeight( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, bool bMousePos = true, int nApplyBrushIndex = -1 );
	void ApplyAlpha( int nLayerIndex, int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, int nApplyBrushIndex );
	void ApplyGrass( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, int nModifyType, bool bMousePos = true, int nApplyBrushIndex = -1 );
	void ApplyGrassInfo();

	short *GetHeightPtr();
	DWORD *GetAlphaPtr();
	char *GetGrassPtr();

	int GetSelectBlock( int nX, int nY, bool bCheckPick = true );
	int GetCurrentBlock();

	void UpdateHeight( CRect &rcRect );
	void UpdateAlpha( CRect &rcRect );
	void UpdateTextureCoord( CRect &rcRect );
	void UpdateGrass( CRect &rcRect );

	int GetBlockCount();
	void GetBlockCount( int &nCountX, int &nCountY );
	void GetBlockSize( int &nSizeX, int &nSizeY );
	CRect GetLastUpdateRect() { return m_rcLastUpdate; }
	void SetLastUpdateRect( CRect &rcRect ) { m_rcLastUpdate = rcRect; }

	void SetHeightMultiply( float fValue );
	SAABox *GetBoundingBox( int nBlockIndex ) { return m_Handle->GetBoundingBox( nBlockIndex ); }

	void ReloadTexture();
	int GetTextureCount( int nBlockIndex );
	const char *GetTextureName( int nBlockIndex, int nTextureIndex );
	const char *GetTextureSemanticName( int nBlockIndex, int nTextureIndex );
	void SetTexture( int nBlockIndex, int nTextureIndex, const char *szName );

	const char *GetGrassTextureName( int nBlockIndex );
	void SetGrassTexture( int nBlockIndex, const char *szName );

	void ChangeBlockType( int nBlockIndex, TerrainType Type );
	TerrainType GetBlockType( int nBlockIndex );

	void ChangeTextureDistance( int nBlockIndex, int nTextureIndex, float fDistance );
	float GetTextureDistance( int nBlockIndex, int nTextureIndex );

	void ChangeTextureRotation( int nBlockIndex, int nTextureIndex, float fRotation );
	float GetTextureRotation( int nBlockIndex, int nTextureIndex );

	EtVector3 GetPickPos() { return m_vPickPos; }

	void Render( LOCAL_TIME LocalTime );

	void GetUseTextureList( std::map<CString,CString> &szMapList );
	void GetUsePropList( std::map<CString,CString> &szMapList );
	// Prop 함수들
	CEtOctree<CEtWorldProp *> *GetPropOctree() { return m_pPropOctree; }
	virtual CEtWorldProp *AllocProp( int nClass );
	void AddProp( int nX, int nY, CPropPoolStage *pPoolStage, CActionElement *pElement, float fRadian, float fHardness, irange rHeight, frange rRadian, bool bRandomPos, bool bRandomRotateX, bool bRandomRotateY, bool bRandomRotateZ, bool bLockHeightProp, bool bLockScaleAxis, frange rScaleX, frange rScaleY, frange rScaleZ, float fHeightDig, float fMinPropDistance, bool bIgnoreDistanceSize );
	void AddProp( CPropPoolStage *pPoolStage, CActionElement *pElement, DWORD dwPropIndex, EtVector3 vPickPos, float fRadian, irange rHeight, frange rRadian, bool bRandomPos, bool bRandomRotateX, bool bRandomRotateY, bool bRandomRotateZ, bool bLockHeightProp, bool bLockScaleAxis, frange rScaleX, frange rScaleY, frange rScaleZ, float fHeightDig, float fMinPropDistance, bool bIgnoreDistanceSize );
	CEtWorldProp *CloneProp( CEtWorldProp *pProp, CActionElement *pElement );
	int GetPropClassID( const char *szFileName );
	std::vector<std::string> & GetIgnorePropFolderList() { return m_vecIgnorePropFolder; }
	void SetIgnorePropFolderList( std::vector<std::string> & vecIgnorePropFolder );

	// Lightmap 함수들
	bool EnableLightmap( bool bEnable );
	void SaveLightmap();

	// Event Area 함수들
	virtual CEtWorldEventControl *AllocControl();

	void InsertControl( const char *szControlName );
	CEtWorldEventControl *GetControlFromName( const char *szControlName );

	int GetEventAreaCreateUniqueCount() { return m_nEventAreaCreateUniqueCount; }
	void SetEventAreaCreateUniqueCount( int nValue ) { m_nEventAreaCreateUniqueCount = nValue; }
	void AddArea( const char *szControlName, const char *szName, CActionElement *pElement );
	CEtWorldEventArea *CopyArea( CEtWorldEventArea *pOrigArea, CActionElement *pElement );
	void RemoveArea( const char *szControlName, const char *szName, CActionElement *pElement );
	void RemoveArea( const char *szControlName, int nUniqueID, CActionElement *pElement );
	void DrawArea( const char *szControlName );
	void DrawArea( EtVector3 &vMin, EtVector3 &vMax, float fRotate, EtVector3 &vOffset, float fYOrder, DWORD dwColor, DWORD dwSideColor, const char *szStr );

	short GetNewHeight( short *pHeight, int x, int y, int nBlurValue, float **ppfTable );
	CString GetEventControlNameFromCreateUniqueID( int nValue );

	// Grass 함수들
	float GetGrassWidth( int nIndex ) { return m_fGrassWidth[nIndex]; }
	float GetGrassHeightMin( int nIndex ) { return m_fGrassHeightMin[nIndex]; }
	float GetGrassHeightMax( int nIndex ) { return m_fGrassHeightMax[nIndex]; }
	float GetGrassShakeMin() { return m_fGrassShakeMin; }
	float GetGrassShakeMax() { return m_fGrassShakeMax; }
	void SetGrassWidth( int nIndex, float fValue ) { m_fGrassWidth[nIndex] = fValue; }
	void SetGrassHeightMin( int nIndex, float fValue ) { m_fGrassHeightMin[nIndex] = fValue; }
	void SetGrassHeightMax( int nIndex, float fValue ) { m_fGrassHeightMax[nIndex] = fValue; }
	void SetGrassShakeMin( float fValue ) { m_fGrassShakeMin = fValue; }
	void SetGrassShakeMax( float fValue ) { m_fGrassShakeMax = fValue; }


	// Sound 함수들
//	CEtWorldSound *GetSoundControl() { return m_pSoundInfo; }
	virtual CEtWorldSound *AllocSound();
	bool AddSound( const char *szName );
	void RemoveSound( const char *szName );
	void DrawSound();
	void DrawSound( EtVector3 &vPos, EtVector3 &vOffset, float fRange, float fRollOff, DWORD dwColor );

	// Attribute 함수들
	char *GetAttributePtr() { return m_pAttribute; }
	void DrawAttribute();
	void DrawNavigationMesh();
	void UpdateAttribute( CRect &rcRect, bool bCheckDiagonal = true );
	void CheckDiagonalAttribute( CRect rcRect );
	void ApplyAttribute( int nX, int nY, float fRadian, float fHardness, float fStrong, irange rHeight, frange rRadian, char cAttr, bool bMousePos = true );
	void BuildPropAttribute();
	void AddPropNavMesh();
	bool FindSuitableNavCell( NavigationCell *pNavCell, int nEdgeIndex );
	bool FindSuitableEdge( NavigationCell *pSourCell, int nSourEdgeIndex, NavigationCell *pClosestCell, float &fSuitableDot, int &nSuitableEdgeIndex );
	int FindNearestAttribute( EtVector3 vStart, EtVector3 vDir );
	void BuildNavCellSideAttribute();
	void GenerationNavigationMesh( bool bGeneration = true );
	bool IsGenerationNavigationMesh() { return ( m_pNavCell ) ? true : false; }
	int GetNavigationFaceCount();

	// Trigger 함수들
	virtual CEtTrigger *AllocTrigger();
	CString GetUniqueCategoryName();
	CString GetUniqueTriggerName();

	// Water 함수들
	virtual CEtWorldWater *AllocWater();
	CTEtWorldWaterRiver::RiverPoint *AddRiverPoint( CTEtWorldWaterRiver *pRiver, CTEtWorldWaterRiver::RiverPoint *pClonePoint );
	CTEtWorldWaterRiver *ApplyRiver( int nX, int nY );
	void DeleteRiverPoint( CTEtWorldWaterRiver::RiverPoint *pPoint );

	// Decal 함수들
	virtual CEtWorldDecal *AllocDecal();
	CEtWorldDecal *AddDecal( int nX, int nY, float fRadius, float fRotate, const char *szTexName, EtColor vColor, float fAlpha );
	int ScanDecal( int nX, int nY, float fRadius, std::vector<CEtWorldDecal *> &pVecResult );
	void UpdateDecal( CEtWorldDecal *pDecal );

	void SetDecalTexture( const char *szFileName );
	const char *GetDecalTextureName();
	void SetDecalRotate( float fValue ) { m_fDecalRotate = fValue; }
	void SetDecalColor( EtColor Color ) { m_vDecalColor = Color; }
	void SetDecalAlpha( float fValue ) { m_fDecalAlpha = fValue; }
	float GetDecalRotate() { return m_fDecalRotate; }
	EtColor GetDecalColor() { return m_vDecalColor; }
	float GetDecalAlpha() { return m_fDecalAlpha; }
	void ShowWarningMesasge();

	bool MakeMinimap( int nWidth, int nHeight, const char *szFileName, const char *szEnviName, bool bRenderTerrain, bool bRenderProp, bool bRenderAttribute );
	bool ExportTerrainToMaxScript( const char *szFileName );
	bool ExportPropInfoToMaxScript( const char *szFileName, const char *szMaxFolder );

	virtual bool LoadSectorSize( const char *szSectorPath );

	void LoadNavMesh();

#ifdef _TOOLCOMPILE
	EtTextureHandle GetTexture( int nBlockIndex, int nTexIndex )
	{
		if( !m_Handle ) return CEtTexture::Identity();
		return m_Handle->GetTexture( nBlockIndex, nTexIndex );
	}
	EtTextureHandle GetGrassTexture()
	{
		if( !m_Handle ) return CEtTexture::Identity();
		return m_Handle->GetGrassTextureHandle();
	}
	EtTextureHandle GetHeightTexture()
	{
		if( !m_Handle ) return CEtTexture::Identity();
		return m_Handle->GetGrassTextureHandle();
	}
#endif

public:
	typedef std::list<std::string> PROP_NAME_LIST;
	static PROP_NAME_LIST ms_PropFilteringList;
	static bool ms_IsFiltering;

	static void AddPropNameForFiltering(const char* szPropName);
	static void RemovePropNameForFiltering(const char* szPropName);
	static bool FindFilteringPropName(const char* szPropName);

	static void InitFilteringNames();

#ifdef _CHECK_MISSING_INDEX
	void CalcMissingIndex();
#endif
};