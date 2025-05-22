#pragma once
#include "afxcmn.h"
#include "SoundListTreeCtrl.h"


// CSoundListPaneView form view

class CSoundListPaneView : public CFormView
{
	DECLARE_DYNCREATE(CSoundListPaneView)

protected:
	CSoundListPaneView();           // protected constructor used by dynamic creation
	virtual ~CSoundListPaneView();

public:
	enum { IDD = IDD_SOUNDLISTPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	SoundListTreeCtrl m_TreeCtrl;
	bool m_bActivate;
	CImageList m_ImageList;
	CTypedPtrList<CPtrList, HTREEITEM> m_SelectItemList;
	HTREEITEM m_hRootItem;
	HTREEITEM m_hBGMItem;
	HTREEITEM m_hEnviItem;
	HTREEITEM m_hPropItem;
	CString m_szPrevLabelString;

protected:
	void CalcSelectItem();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDropItem(WPARAM, LPARAM);
	afx_msg LRESULT OnDropMoveItem(WPARAM, LPARAM);
	afx_msg LRESULT OnAddSoundEnvi( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelectChange( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBeginlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeleditTree1(NMHDR *pNMHDR, LRESULT *pResult);
};


