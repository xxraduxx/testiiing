#pragma once
#include "afxcmn.h"

// CTileManagerPaneView 폼 뷰입니다.
#include "InputReceiver.h"


struct TileStruct {
	CString szFileName;
	CString szFolder;
	char cPrevFlag;

	D3DSURFACE_DESC SurfaceDesc;
	CBitmap *pImage;
	CDC *pmemDC;

	TileStruct() {
		cPrevFlag = 0;
		memset( &SurfaceDesc, 0, sizeof(D3DSURFACE_DESC) );
		pImage = NULL;
		pmemDC = NULL;
	}
	~TileStruct() {
		SAFE_DELETE( pImage );
		SAFE_DELETE( pmemDC );
	}
};

class CTileIconItem {
public:
	CTileIconItem();
	~CTileIconItem();

	int m_nIndex;
protected:
	CSize m_Size;
	TileStruct *m_pStruct;

public:
	void SetImage( int nX, int nY, void *pPtr );

	void DrawItem( CRect *rcRect, CDC *pDC, bool bSelect );
	TileStruct *GetStruct() { return m_pStruct; }


	static std::vector<CTileIconItem> s_vecList;
	static int AddItem( TileStruct *pStruct, int nIndex );
	static void RemoveItem( int nIndex );
	static void DeleteAllItems();
};


class CTileManagerPaneView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CTileManagerPaneView)

protected:
	CTileManagerPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CTileManagerPaneView();

public:
	enum { IDD = IDD_TILEMANAGERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;
	CXTPToolBar m_wndToolBar2;

	std::map<CString, CString> m_MapTileList;
	std::vector<TileStruct *> m_pVecTileList;
	static HANDLE s_hThreadHandle;
	unsigned m_dwThreadIndex;
	static bool s_bExitThread;
	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;
	CSize m_IconSize;
	CSize m_IconSpace;
	CString m_szCurrentFolder;

	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSelectIndex;
	int m_nCurPrevImageSize;
	bool m_bLBDown;
	bool m_bDragBegin;
	CImageList *m_pDragImage;
	bool m_bDragClickFlag;
	int m_nSelectFolderOffsetStart;
	int m_nSelectFolderOffsetEnd;


	bool m_bShowPreview;

	void SearchDefaultInfo( char *szFolder = NULL );
	bool SearchPrevInfo();
	void ResetInfoList();

	void CreateMemDC();
	void DeleteMemDC();
	void ResetScroll();
	void DeleteInTileStruct( DWORD dwIndex );
	bool CheckAnyoneUseTexture( CString szTextureName, CString *szResult );
//	bool CheckSelectIcon( DWORD dwIndex, CPoint p );
	CString GetCurrentDir();

	void RefreshFullName();

public:
	int m_nThreadStatus;

	void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );
	void KillThread();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnChangeFolderFromExplorer( WPARAM wParam, LPARAM lParam );
	LRESULT OnChangeFolderNameFromExplorer( WPARAM wParam, LPARAM lParam );
	LRESULT OnMoveFileFromExplorer( WPARAM wParam, LPARAM lParam );

	static UINT __stdcall BeginThread( void *pParam );
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTextureadd();
	afx_msg void OnTexturerefresh();
	afx_msg void OnTextureremove();
	afx_msg void OnTexturepreview();
	afx_msg void OnTexturesize();
	afx_msg LRESULT OnGetPreviewImage( WPARAM wParam, LPARAM lParam );
	afx_msg void OnUpdateTexturesize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTextureAdd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTextureRemove(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTextureRefresh(CCmdUI *pCmdUI);
};


