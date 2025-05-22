#pragma once

#include "PropertyGridCreator.h"

// CIgnorePropFolderView 폼 뷰입니다.

class CIgnorePropFolderView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CIgnorePropFolderView)

protected:
	CIgnorePropFolderView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CIgnorePropFolderView();

public:
	enum { IDD = IDD_IGNOREPROPFOLDERDLG };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;

public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnGridNotify(WPARAM, LPARAM);
};


