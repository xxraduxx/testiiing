#pragma once



// CDummyView �� ���Դϴ�.

class CDummyView : public CFormView
{
	DECLARE_DYNCREATE(CDummyView)

protected:
	CDummyView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CDummyView();

public:
	enum { IDD = IDD_DUMMYVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	struct TabViewStruct {
		CXTPTabManagerItem *pItem;
		CWnd *pView;
	};

	void InitView( CXTPTabControl *pControl );
	void Refresh( int nActiveView, bool bForceRefresh = false );
	void SetSelectChangeView( int nActiveView );
	TabViewStruct *GetTabViewStruct( int nIndex ) { return &m_pTabViewItem[nIndex]; }

protected:
	TabViewStruct *m_pTabViewItem;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


