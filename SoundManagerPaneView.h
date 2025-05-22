#pragma once
#include "InputReceiver.h"
#include "fmod.hpp"
#include "fmod.h"

// CSoundManagerPaneView 폼 뷰입니다.

class CEtSoundChannel;
struct SoundStruct {
	CString szFileName;
	CString szFolder;
	char cPrevFlag;

	CString szInfo;
	CEtSoundChannel *pChannel;

	SoundStruct() {
		cPrevFlag = 0;
		pChannel = NULL;
	}
	~SoundStruct() {
	}
};

class CSoundIconItem {
public:
	CSoundIconItem();
	~CSoundIconItem();

	int m_nIndex;
protected:
	CSize m_Size;
	SoundStruct *m_pStruct;

public:
	void SetImage( int nX, int nY, void *pPtr );

	void DrawItem( CRect *rcRect, CDC *pDC, bool bSelect );
	SoundStruct *GetStruct() { return m_pStruct; }


	CRect GetSoundButtonRect( CRect *rcRect );
	static std::vector<CSoundIconItem> s_vecList;
	static int AddItem( SoundStruct *pStruct, int nIndex );
	static void RemoveItem( int nIndex );
	static void DeleteAllItems();
};


class CSoundManagerPaneView : public CFormView, public CInputReceiver
{
	DECLARE_DYNCREATE(CSoundManagerPaneView)

protected:
	CSoundManagerPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CSoundManagerPaneView();

public:
	enum { IDD = IDD_SOUNDMANAGERPANEVIEW };
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
	static bool s_bExitThread;
	unsigned m_dwThreadIndex;
	int m_nThreadStatus;

	CDC *m_pmemDC;
	CBitmap *m_pdcBitmap;
	CDC *m_pBackImagememDC;
	CBitmap *m_pBackImagedcBitmap;

	std::map<CString, CString> m_MapSoundList;
	std::vector<SoundStruct *> m_pVecSoundList;

	CSize m_IconSize;
	CSize m_IconSpace;
	CSize m_ScrollSize;
	CSize m_ScrollPos;
	int m_nSelectIndex;
	int m_nViewFileType;
	int m_nSelectFolderOffsetStart;
	int m_nSelectFolderOffsetEnd;

	bool m_bLBDown;
	bool m_bDragBegin;
	bool m_bDragClickFlag;
	CImageList *m_pDragImage;

	CString m_szCurrentFolder;

	void CreateMemDC();
	void DeleteMemDC();
	void ResetScroll();
	void DeleteInSoundStruct( DWORD dwIndex );
	bool CheckAnyoneUseSound( CString szSoundName );

	void SearchDefaultInfo( char *szFolder = NULL );
	bool SearchPrevInfo();
	void ResetInfoList();
	CString GetCurrentDir();
	static UINT __stdcall BeginThread( void *pParam );
	void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	bool CalcAccompanimentFile( const char *szSoundName, std::vector<CString> &szVecList );
	void RefreshFullName();

public:
	SoundStruct *FindSoundStructFromIndex( int nIndex );

public:
	void KillThread();
	CDC *GetBackImage() { return m_pBackImagememDC; }
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
	afx_msg void OnSoundadd();
	afx_msg void OnSoundremove();
	afx_msg void OnSoundMode();
	afx_msg void OnSoundRefresh();
	afx_msg void OnUpdateSoundMode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSoundAdd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSoundRemove(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSoundRefresh(CCmdUI *pCmdUI);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


