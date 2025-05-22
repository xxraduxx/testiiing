#pragma once



// CPropPoolPaneView form view

class CPropPoolView;
class CPropPoolPropertiesView;

class CPropPoolPaneView : public CFormView
{
	DECLARE_DYNCREATE(CPropPoolPaneView)

protected:
	CPropPoolPaneView();           // protected constructor used by dynamic creation
	virtual ~CPropPoolPaneView();

public:
	enum { IDD = IDD_PORPPOOLPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	CXTPTabControl m_TabCtrl;
	CPropPoolView *m_pPoolView;
	CPropPoolPropertiesView *m_pPoolPropertiesView;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDblClickItem( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};


