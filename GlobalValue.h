#pragma once

#include "Singleton.h"
#include "../EtWorldBase/Common.h"

class CTEtWorldDecal;
#include "EternityEngine.h"
#include "TEtWorldWaterRiver.h"
struct irange {
	int nMin;
	int nMax;
	irange() { nMin = nMax = 0; }
	irange( int Min, int Max ) { nMin = Min; nMax = Max; }
};

struct frange {
	float fMin;
	float fMax;
	frange() { fMin = fMax = 0.f; }
	frange( float Min, float Max ) { fMin = Min; fMax = Max; }
};

enum MouseFlag {
	NONE = 0x00,
	LB_DOWN = 0x01,
	RB_DOWN = 0x02,
	WB_DOWN = 0x04,
};

class CEtWorldProp;
class CEtWorldEventArea;
class CEtWorldSoundEnvi;
class CTEtTriggerElement;
class CInputReceiver;
class CGlobalValue : public CSingleton<CGlobalValue> {
public:
	CGlobalValue();
	virtual ~CGlobalValue();

	CView *m_pParentView;

	enum VIEW_TYPE {
		GRID,
		BRUSH,
		PROP,
		WATER,
		NAVIGATION,
		EVENT,
		SOUND,
		TRIGGER,
		RDUNGEON,
		VIEW_TYPE_AMOUNT,
	};
	int m_ViewTypeLoadLevel[VIEW_TYPE_AMOUNT];


	CString m_szViewerLocation;
	CString m_szActionLocation;

	float m_fFps;
	bool m_bMaintenanceCamera;
	// Work View Infomation
	int m_nActiveView;
	bool m_bModify;

	// Grid Infomation
	SectorIndex m_SelectGrid;
	SectorIndex m_CheckOutGrid;
	CString m_szSelectGrid;
	CString m_szCheckOutGrid;

	// Brush Infomation
	int m_nBrushType;
	int m_nBrushDiameter;
	float m_fBrushHardness;
	float m_fBrushStrong;
	frange m_rRadian;
	irange m_rHeight;
	bool m_bLockSideHeight;
	bool m_bEnableGrassIndex[4];

	// Navigation
	bool m_bAutoDiagonal;

	// Layer Infomation
	int m_nSelectLayer;
	bool m_bEditBlock;
	bool m_bCanDragTile;
	std::vector<int> m_nVecEditBlockList;
	
	// Camera Infomation
	float m_fCamNear;
	float m_fCamFar;
	float m_fCamFOV;
	float m_fCamSpeed;

	// Modify Infomation
	std::vector<int> m_nVecModifyTabList;
	bool m_bAutoCheckIn;

	// Random Dungeon Default Infomation;
	IntVec3 m_DungeonSize;
	int m_nDungeonSparseness;
	int m_nDungeonRandomness;
	int m_nDungeonDeadendRemove;
	int m_nDungeonSeed;

	// Simulation
	bool m_bSimulation;
	bool m_bCanSimulation;

	// PropPool Infomation;
	char m_cDrawType;
	bool m_bRandomPosition;
	bool m_bRandomRotationX;
	bool m_bRandomRotationY;
	bool m_bRandomRotationZ;
	bool m_bLockHeightNormal;
	float m_fMinPropDistance;
	bool m_bIgnoreDistanceSize;
	bool m_bLockScaleAxis;
	float m_fHeightDigPer;
	frange m_rScaleXRange;
	frange m_rScaleYRange;
	frange m_rScaleZRange;
	
	// Prop Infomation
	DNVector(CEtWorldProp *) m_pVecSelectPropList;
	bool m_bControlAxis;
	int m_nSelectAxis;

	// Event Infomation
//	int m_nSelectControlIndex;
	CString m_szSelectControlName;
	CEtWorldEventArea *m_pSelectEvent;

	// Sound
	bool m_bPlaySound;
	float m_fMasterVolume;
	CEtWorldSoundEnvi *m_pSelectSound;

	// Attribute
	char m_cSelectAttribute;

	// Trigger
	int m_nPickType;
	CTEtTriggerElement *m_pPickStandbyElement;
	int m_nPickParamIndex;

	// Decal
	CTEtWorldDecal *m_pSelectDecal;

	// Water
	std::vector<CTEtWorldWaterRiver *> m_pVecSelectRiverList;
	std::vector<CTEtWorldWaterRiver::RiverPoint *> m_pVecSelectPointList;

	// Render Option
	bool m_bShowProp;
	bool m_bLoadedProp;
	bool m_bShowPropCollision;
	float m_fPropAlpha;

	bool m_bShowWater;
	bool m_bLoadedWater;

	bool m_bLoadedAttribute;
	bool m_bLoadedNavigation;
	bool m_bShowNavigation;
	bool m_bShowAttribute;
	bool m_bDrawRevisionEmptyMeshProp;

	bool m_bShowEventArea;
	bool m_bLoadedEventArea;

	void ShowProp( bool bShow );
	void ShowPropCollision( bool bShow );
	void SetPropAlpha( float fValue );
	void ShowWater( bool bShow );
	void ShowAttribute( bool bShow );
	void ShowNavigation( bool bShow );
	void DrawRevisionEmptyMeshProp( bool bShow );
	void ApplyRenderOption();
	void CheckAccelerationKey( CInputReceiver *pReceiver );
	void ShowEventArea( bool bShow );

	// Func
	bool CheckAndSelectGrid( const char *szGridName, SectorIndex GridIndex );
	bool IsCheckOutMe();
	bool IsActiveRenderView( CView *pView );
	
	void RefreshRender();
	void SetModify( int nTabIndex = -1, bool bModify = true );
	bool IsModify( int nTabIndex = -1 );

	bool Save( int nIndex = -1 );
	CString GetSaveDesc( int nIndex );
	DWORD GetSaveCount();
	bool CheckSaveMessage( HWND hWnd, bool bExit = false );

	bool m_bKeyFlags[256];

	short GetAsyncKeyState( int vKey );
	void ReleaseAsyncKeyState( int vKey );
	void ProcessAsyncKey();

	void UpdateInitDesc( const char *szStr, ... );
	void UpdateProgress( int nMax, int nCur );

#ifdef _CHECK_MISSING_INDEX
	std::string m_szFolderName;
	CString		m_strPreviousGridName;
	bool		m_bIsFirst;
#endif
	
};