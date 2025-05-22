#pragma once

#include "BrushPaneView.h"
#include "LayerPaneView.h"
#include "GridInfoPaneView.h"
#include "LoginListPaneView.h"
#include "CameraPaneView.h"
#include "SectorInfoPaneView.h"
#include "TileManagerPaneView.h"
#include "ManagerExplorerPaneView.h"
#include "RandomDungeonDefaultPaneView.h"
#include "SkinManagerPaneView.h"
#include "SoundManagerPaneView.h"
#include "PropPoolPaneView.h"
#include "PropPropertiesPaneView.h"
#include "EnviPaneView.h"
#include "EventAreaPaneView.h"
#include "EventPropertiesPaneView.h"
#include "SoundListPaneView.h"
#include "SoundPropertyPaneView.h"
#include "NaviPropertyPaneView.h"
#include "RenderOptionPaneView.h"
#include "PropListPaneView.h"
#include "TriggerPaneView.h"
#include "TriggerPropertiesPaneView.h"
#include "TriggerAttributePaneView.h"
#include "WaterPaneView.h"
#include "WaterPropPaneView.h"
#include "TriggerValueDefinePaneView.h"
#include "GridListPaneView.h"
#include "GridDescriptionPaneView.h"
#include "DecalListPaneView.h"
#include "IgnorePropFolderView.h"

enum PANE_DEFINE {
	BRUSH_PANE = 230,
	LAYER_PANE = 201,
	GRID_PANE = 202,
	LOGINLIST_PANE = 203,
	CAMERA_PANE = 204,
	SECTOR_PANE = 205,
	TILEMNG_PANE = 206,
	RANDOM_DEFAULT_PANE = 207,
	MNG_EXPLORER_PANE = 208,
	SKINMNG_PANE = 209,
	PROPPOOL_PANE = 211,
	PROPPROP_PANE = 212,
	ENVI_PANE = 213,
	EVENT_PANE = 214,
	EVENTPROP_PANE = 215,
	SOUNDMNG_PANE = 216,
	SOUNDLIST_PANE = 217,
	SOUNDPROP_PANE = 218,
	NAVIPROP_PANE = 219,
	RENDEROPTION_PANE = 220,
	PROPLIST_PANE = 221,
	TRIGGER_PANE = 222,
	TRIGGER_PROP_PANE = 223,
	TRIGGER_ATTR_PANE = 224,
	WATER_PANE = 225,
	WATERPROP_PANE = 226,
	TRIGGER_VALUE_PANE = 227,
	GRIDLIST_PANE = 228,
	GRIDDESC_PANE = 229,
	DECALLIST_PANE = 231,
	IGNOREPROP_PANE = 232,
};

struct PaneCreateStruct {
	int nPaneID;
	int nDockPaneID;
	int nAttachPaneID;
	CRuntimeClass *pClass;
	char *szName;
	XTPDockingPaneDirection Direction;
	int nLeft, nTop, nRight, nBottom;
	BOOL bShow;

	CXTPDockingPane *pThis;
	BOOL bAutoShowHide;
};

static struct PaneCreateStruct g_PaneList[] = {
	{ BRUSH_PANE, -1, -1, RUNTIME_CLASS( CBrushPaneView ), "Brush", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ CAMERA_PANE, -1, 0, RUNTIME_CLASS( CCameraPaneView ), "Camera", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ LAYER_PANE, 0, -1, RUNTIME_CLASS( CLayerPaneView ), "Layer", xtpPaneDockBottom, 0, 0, 230, 250, FALSE, NULL, TRUE },
	{ SECTOR_PANE, -1, 2, RUNTIME_CLASS( CSectorInfoPaneView ), "Sector Infomation", xtpPaneDockBottom, 0, 0, 230, 250, FALSE, NULL, TRUE },
	{ GRID_PANE, -1, -1, RUNTIME_CLASS( CGridInfoPaneView ), "Grid Infomation", xtpPaneDockLeft, 0, 0, 230, 250, TRUE, NULL, TRUE },
	{ LOGINLIST_PANE, 4, -1, RUNTIME_CLASS( CLoginListPaneView ), "Login User", xtpPaneDockBottom, 0, 0, 230, 150, TRUE, NULL, TRUE },
	{ TILEMNG_PANE, -1, -1, RUNTIME_CLASS( CTileManagerPaneView ), "Tile Manager", (XTPDockingPaneDirection)-1, 0, 0, 700, 400, FALSE, NULL, TRUE },
	{ RANDOM_DEFAULT_PANE, -1, 2, RUNTIME_CLASS( CRandomDungeonDefaultPaneView ), "Dungeon Generation", xtpPaneDockBottom, 0, 0, 230, 150, FALSE, NULL, TRUE },
	{ MNG_EXPLORER_PANE, -1, -1, RUNTIME_CLASS( CManagerExplorerPaneView ), "Explorer", (XTPDockingPaneDirection)-1, 0, 0, 250, 550, FALSE, NULL, TRUE },
	{ SKINMNG_PANE, -1, -1, RUNTIME_CLASS( CSkinManagerPaneView ), "Skin Manager", (XTPDockingPaneDirection)-1, 0, 0, 700, 550, FALSE, NULL, TRUE },
	{ PROPPOOL_PANE, 0, -1, RUNTIME_CLASS( CPropPoolPaneView ), "Brush Pool", xtpPaneDockBottom, 0, 0, 230, 250, FALSE, NULL, TRUE },
	{ PROPPROP_PANE, -1, 10, RUNTIME_CLASS( CPropPropertiesPaneView ), "Prop Properties", xtpPaneDockBottom, 0, 0, 230, 250, FALSE, NULL, TRUE },
	{ ENVI_PANE, -1, 0, RUNTIME_CLASS( CEnviPaneView ), "Environment", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ EVENT_PANE, -1, 0, RUNTIME_CLASS( CEventAreaPaneView ), "Event Area", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ EVENTPROP_PANE, 0, -1, RUNTIME_CLASS( CEventPropertiesPaneView ), "Event Properties", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ SOUNDMNG_PANE, -1, -1, RUNTIME_CLASS( CSoundManagerPaneView ), "Sound Manager", (XTPDockingPaneDirection)-1, 0, 0, 700, 400, FALSE, NULL, TRUE },
	{ SOUNDLIST_PANE, -1, 0, RUNTIME_CLASS( CSoundListPaneView ), "Sound List", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ SOUNDPROP_PANE, 0, -1, RUNTIME_CLASS( CSoundPropertyPaneView ), "Sound Properties", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ NAVIPROP_PANE, 0, -1, RUNTIME_CLASS( CNaviPropertyPaneView ), "Navigation Properties", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ RENDEROPTION_PANE, -1, 0, RUNTIME_CLASS( CRenderOptionPaneView ), "RenderOption", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ PROPLIST_PANE, -1, 0, RUNTIME_CLASS( CPropListPaneView ), "PropList", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ TRIGGER_PANE, -1, -1, RUNTIME_CLASS( CTriggerPaneView ), "TriggerList", xtpPaneDockRight, 0, 0, 250, 150, FALSE, NULL, TRUE },
	{ TRIGGER_PROP_PANE, 21, -1, RUNTIME_CLASS( CTriggerPropertiesPaneView ), "Trigger Properties", xtpPaneDockBottom, 0, 0, 700, 300, FALSE, NULL, TRUE },
	{ TRIGGER_ATTR_PANE, -1, 22, RUNTIME_CLASS( CTriggerAttributePaneView ), "Trigger Attribute", xtpPaneDockBottom, 0, 0, 700, 300, FALSE, NULL, TRUE },
	{ WATER_PANE, -1, 0, RUNTIME_CLASS( CWaterPaneView ), "Water", xtpPaneDockLeft, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ WATERPROP_PANE, 24, -1, RUNTIME_CLASS( CWaterPropPaneView ), "Water Properties", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ TRIGGER_VALUE_PANE, -1, 21, RUNTIME_CLASS( CTriggerValueDefinePaneView ), "Trigger Value Define", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ GRIDLIST_PANE, -1, 4, RUNTIME_CLASS( CGridListPaneView ), "GridList", xtpPaneDockBottom, 0, 0, 230, 150, TRUE, NULL, TRUE },
	{ GRIDDESC_PANE, 4, -1, RUNTIME_CLASS( CGridDescriptionPaneView ), "Description", xtpPaneDockBottom, 0, 0, 230, 150, TRUE, NULL, TRUE },
	{ DECALLIST_PANE, -1, 0, RUNTIME_CLASS( CDecalListPaneView ), "DecalList", xtpPaneDockBottom, 0, 0, 230, 400, FALSE, NULL, TRUE },
	{ IGNOREPROP_PANE, -1, -1, RUNTIME_CLASS( CIgnorePropFolderView ), "Ignore PropFolder", (XTPDockingPaneDirection)-1, 0, 0, 230, 160, FALSE, NULL, TRUE },
	
};

#define GetPaneWnd(id)	((CMainFrame*)AfxGetMainWnd())->GetPaneFrame(id);