#pragma once

// CEnviPaneView form view

class CEnviBasePaneView;
class CEnviLightPaneView;
class CEnviPaneView : public CFormView
{
	DECLARE_DYNCREATE(CEnviPaneView)

protected:
	CEnviPaneView();           // protected constructor used by dynamic creation
	virtual ~CEnviPaneView();

public:
	enum { IDD = IDD_ENVIPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;
	CXTPTabControl m_TabCtrl;
	CEnviBasePaneView *m_pBaseView;
	CEnviLightPaneView *m_pLightView;

public:
	CString GetCurrentDir();
	bool CalcAccompanimentFile( const char *szSkinName, std::vector<CString> &szVecList );
	void CopyAccompanimentFile( const char *szSkinName );

	CEnviBasePaneView *GetBaseView() { return m_pBaseView; }
	CEnviLightPaneView *GetLightView() { return m_pLightView; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEnviNew();
	afx_msg void OnEnviSaveas();
	afx_msg void OnEnviReload();
	afx_msg void OnEnviDelete();
	afx_msg void OnEnviLoadlist();
	afx_msg LRESULT OnSetModify( WPARAM wParam, LPARAM lParam );
	afx_msg void OnEnviBakelightmap();
	afx_msg void OnUpdateEnviBakelightmap(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEnviReload(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEnviSaveas(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEnviDelete(CCmdUI *pCmdUI);
	afx_msg LRESULT OnSoundPaneDropMoveItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSoundPaneDropItem( WPARAM wParam, LPARAM lParam );

};


