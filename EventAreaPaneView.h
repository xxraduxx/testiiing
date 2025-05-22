#pragma once
#include "afxcmn.h"

#include "EditLabelListCtrl.h"


// CEventAreaPaneView form view

class CActionElement;
class CEventAreaPaneView : public CFormView
{
	DECLARE_DYNCREATE(CEventAreaPaneView)

protected:
	CEventAreaPaneView();           // protected constructor used by dynamic creation
	virtual ~CEventAreaPaneView();

public:
	enum { IDD = IDD_EVENTAREAPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;
	CXTPToolBar m_wndToolBar;
	CEditLabelListCtrl m_ListCtrl;
	CString m_szPrevEditString;

	int m_nSortedCol;
	bool m_bAscending;
	CXTHeaderCtrl m_header;
	bool m_bSorting;

	CActionElement *m_pCurAction;

	void SortColumn(int iCol, bool bAsc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelChanged( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnControlChanged( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnRefreshInfo( WPARAM wParam, LPARAM lParam );
	afx_msg void OnEventList();
	afx_msg void OnUpdateEventList(CCmdUI *pCmdUI);
	afx_msg void OnLvnBeginlabeleditList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnEnablePane( WPARAM wParam, LPARAM lParam );
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult);
};


