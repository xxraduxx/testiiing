#pragma once
#include "afxcmn.h"



// CPropListPaneView form view

class CPropListPaneView : public CFormView
{
	DECLARE_DYNCREATE(CPropListPaneView)

protected:
	CPropListPaneView();           // protected constructor used by dynamic creation
	virtual ~CPropListPaneView();

protected:
	bool m_bActivate;
	int m_nSortedCol;
	bool m_bAscending;
	CXTHeaderCtrl m_header;
	bool m_bSorting;

protected:
	CXTListCtrl m_ListCtrl;
	void SortColumn(int iCol, bool bAsc);

public:
	enum { IDD = IDD_PROPLISTPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnEnablePane( WPARAM wParam, LPARAM lParam );
	afx_msg void OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult);
};


