#pragma once
#include "afxcmn.h"

// CLoginListPaneView �� ���Դϴ�.

class CLoginListPaneView : public CFormView
{
	DECLARE_DYNCREATE(CLoginListPaneView)

protected:
	CLoginListPaneView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CLoginListPaneView();

public:
	enum { IDD = IDD_LOGINLISTPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CXTListCtrl m_ListCtrl;
	bool m_bActivate;

	void UpdateLoginList();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
};


