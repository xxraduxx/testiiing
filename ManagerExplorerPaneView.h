#pragma once
#include "afxcmn.h"



// CManagerExplorerPaneView 폼 뷰입니다.
#include "EditLabelTreeCtrl.h"

class CManagerExplorerPaneView : public CFormView
{
	DECLARE_DYNCREATE(CManagerExplorerPaneView)

protected:
	CManagerExplorerPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CManagerExplorerPaneView();

public:
	enum { IDD = IDD_MANAGEREXPLORERPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	int m_nCurrentManager;
	CEditLabelTreeCtrl m_TreeCtrl;
	CImageList m_ImageList;
	HTREEITEM m_hRootTree;
	HTREEITEM m_hNationRootTree;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;

	struct DirectoryStruct {
		HTREEITEM hItem;
		HTREEITEM hParent;
		CString szFullPath;
	};
	std::vector<DirectoryStruct> m_VecDirList;
	CMenu *m_pContextMenu;
	CString m_szPrevLabelString;

protected:
	void RefreshFolder( CString &szFolder, HTREEITEM hParentItem );
	void CalcSelectItem();
	CWnd *GetCurrentWnd();
	CString GetCurrentDefaultFolder();
	CString GetCurrentDefaultNationFolder();
	void GetFolderName( CString &szFolder, std::vector<CString> &szVecList );
	bool CheckIgnoreFolder( char *szName );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnManagerexplorerCreatefolder();
	afx_msg void OnManagerexplorerDeletefolder();
	afx_msg void OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
};


