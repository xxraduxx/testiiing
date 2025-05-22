#pragma once

#include "PropertyGridCreator.h"

// CIgnorePropFolderView �� ���Դϴ�.

class CIgnorePropFolderView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CIgnorePropFolderView)

protected:
	CIgnorePropFolderView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnGridNotify(WPARAM, LPARAM);
};


