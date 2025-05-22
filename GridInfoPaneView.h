#pragma once
#include "afxcmn.h"



// CGridInfoPaneView 폼 뷰입니다.
#include "PropertyGridCreator.h"
class CGridInfoPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CGridInfoPaneView)

protected:
	CGridInfoPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CGridInfoPaneView();

public:
	enum { IDD = IDD_GRIDINFOPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:

protected:
	bool m_bActivate;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnNotifyGrid( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


