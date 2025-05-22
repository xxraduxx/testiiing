#pragma once



#include "InputReceiver.h"
// CSkinManagerPaneView 폼 뷰입니다.

struct SkinStruct {
	CString szFileName;
	CString szFolder;
	char cPrevFlag;

	D3DSURFACE_DESC SurfaceDesc;
	CBitmap *pImage;
	CDC *pmemDC;

	SkinStruct() {
		cPrevFlag = 0;
		memset( &SurfaceDesc, 0, sizeof(D3DSURFACE_DESC) );
		pImage = NULL;
		pmemDC = NULL;
	}
	~SkinStruct() {
		SAFE_DELETE( pImage );
		SAFE_DELETE( pmemDC );
	}
};

class CSkinIconItem {
public:
	CSkinIconItem();
	~CSkinIconItem();

	int m_nIndex;
protected:
	CSize m_Size;
	SkinStruct *m_pStruct;

public:
	void SetImage( int nX, int nY, void *pPtr );

	void DrawItem( CRect *rcRect, CDC *pDC, bool bSelect );
	SkinStruct *GetStruct() { return m_pStruct; }


	static std::vector<CSkinIconItem> s_vecList;
	static int AddItem( SkinStruct *pStruct, int nIndex );
	static void RemoveItem( int nIndex );
	static void DeleteAllItems();
};


class CSkinManagerPaneView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CSkinManagerPaneView)

protected:
	CSkinManagerPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CSkinManagerPaneView();

public:
	enum { IDD = IDD_SKINMANAGERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;
	CXTPToolBar m_wndToolBar2;

	static HANDLE s_hThreadHandle;
	unsigned m_dwThreadIndex;
	int m_nThreadStatus;
	DWORD m_dwThreadExitCode;
	static bool s_bExitThread;
	static HANDLE s_hThreadEndRequest;

	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;

	std::map<CString, CString> m_MapSkinList;
	std::vector<SkinStruct *> m_pVecSkinList;

	CSize m_IconSize;
	CSize m_IconSpace;
	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSelectIndex;
	int m_nSelectFolderOffsetStart;
	int m_nSelectFolderOffsetEnd;

	bool m_bShowPreview;
	int m_nCurPrevImageSize;
	bool m_bLBDown;
	bool m_bDragBegin;
	bool m_bDragClickFlag;
	CImageList *m_pDragImage;

	CString m_szRootFolder;
	CString m_szCurrentFolder;

	CMenu *m_pContextMenu;

	void CreateMemDC();
	void DeleteMemDC();
	void ResetScroll();
	void DeleteInSkinStruct( DWORD dwIndex );
	bool CheckAnyoneUseSkin( CString szSkinName, CString *szResult );

	void SearchDefaultInfo( char *szFolder = NULL );
	bool SearchPrevInfo();
	void ResetInfoList();
	CString GetCurrentDir();
	static UINT __stdcall BeginThread( void *pParam );
	void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	bool CalcAccompanimentFile( const char *szSkinName, std::vector<CString> &szVecList );

	void RefreshFullName();


public:
	void KillThread();
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnChangeFolderFromExplorer( WPARAM wParam, LPARAM lParam );
	LRESULT OnChangeFolderNameFromExplorer( WPARAM wParam, LPARAM lParam );
	LRESULT OnMoveFileFromExplorer( WPARAM wParam, LPARAM lParam );
	LRESULT OnGetPreviewImage( WPARAM wParam, LPARAM lParam );
	afx_msg void OnDestroy();
	afx_msg void OnSkinadd();
	afx_msg void OnSkinRefresh();
	afx_msg void OnSkinremove();
	afx_msg void OnSkinpreview();
	afx_msg void OnSkinMode();
	afx_msg void OnSkinSize();
	afx_msg void OnUpdateSkinSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSkinMode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSkinAdd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSkinRemove(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSkinRefresh(CCmdUI *pCmdUI);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSkinmanagerOpentoetviewer();
	afx_msg void OnSkinmanagerOpentoetactiontool();
};


