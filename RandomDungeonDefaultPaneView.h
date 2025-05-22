#pragma once



// CRandomDungeonDefaultPaneView 폼 뷰입니다.

#include "PropertyGridCreator.h"
class CRandomDungeonDefaultPaneView : public CFormView, public CPropertyGridImp
{
	DECLARE_DYNCREATE(CRandomDungeonDefaultPaneView)

protected:
	CRandomDungeonDefaultPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CRandomDungeonDefaultPaneView();

public:
	enum { IDD = IDD_RANDOMDUNGEONDEFAULTPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
protected:
	bool m_bActivate;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	LRESULT OnNotifyGrid( WPARAM wParam, LPARAM lParam );

public:
	virtual void OnSetValue( CUnionValueProperty *pVariable, DWORD dwIndex );
	virtual void OnChangeValue( CUnionValueProperty *pVariable, DWORD dwIndex );

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
};


