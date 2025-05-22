#pragma once
#include "afxcmn.h"



// CGridDescriptionPaneView form view

class CTEtWorldGrid;
class CGridDescriptionPaneView : public CFormView
{
	DECLARE_DYNCREATE(CGridDescriptionPaneView)

protected:
	CGridDescriptionPaneView();           // protected constructor used by dynamic creation
	virtual ~CGridDescriptionPaneView();

public:
	enum { IDD = IDD_GRIDDESCRIPTIONPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	bool m_bActivate;
	CRichEditCtrl m_RichEdit;
	CTEtWorldGrid *m_pGrid;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnChangeRichedit21();
	afx_msg void OnEnUpdateRichedit21();
};


