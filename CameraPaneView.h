#pragma once



#include "NumMiniSlider.h"
#include "FloatMiniSlider.h"
// CCameraPaneView �� ���Դϴ�.

class CCameraPaneView : public CFormView
{
	DECLARE_DYNCREATE(CCameraPaneView)

protected:
	CCameraPaneView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CCameraPaneView();

public:
	enum { IDD = IDD_CAMERAPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate();
	afx_msg LRESULT OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam );


	bool m_bActivate;
	CNumMiniSlider m_NearCtrl;
	CNumMiniSlider m_FarCtrl;
	CFloatMiniSlider m_FovCtrl;
	CFloatMiniSlider m_CamSpeedCtrl;
};


