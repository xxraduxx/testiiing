#pragma once
#include "afxcmn.h"



// CBrushPaneView 폼 뷰입니다.
//#include "SliderNumCtrlEx.h"
#include "afxwin.h"

#include "NumMiniSlider.h"
#include "FloatMiniSlider.h"
#include "NumMiniSliderRange.h"
#include "FloatMiniSliderRange.h"
class CBrushPaneView : public CFormView
{
	DECLARE_DYNCREATE(CBrushPaneView)

protected:
	CBrushPaneView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CBrushPaneView();

public:
	enum { IDD = IDD_BRUSHPANEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	bool m_bActivate;

	CNumMiniSlider m_DiameterCtrl;
	CFloatMiniSlider m_HardnessCtrl;
	CFloatMiniSlider m_StrongCtrl;
	CFloatMiniSliderRange m_RadianCtrl;
	CNumMiniSliderRange m_HeightCtrl;

	CXTButton m_BrushDef1;
	CXTButton m_BrushDef2;
	CXTButton m_BrushDef3;
	CXTButton m_BrushDef4;
	CXTButton m_BrushDef5;
	CXTButton m_BrushDef6;
	CXTButton m_BrushDef7;
	CXTButton m_BrushDef8;


	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg LRESULT OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnUpdateUseButton( WPARAM wParam, LPARAM lParam );

	LRESULT OnRefresh( WPARAM wParam, LPARAM lParam );
};


