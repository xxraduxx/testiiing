#pragma once


#include "ScriptControl.h"
// CScriptParamStatic

class CTEtTriggerElement;
#define UM_SCRIPTSTATIC_CLICK WM_USER + 9876

class CParamInputDlg;
class CScriptParamStatic : public CStatic
{
	DECLARE_DYNAMIC(CScriptParamStatic)

public:
	CScriptParamStatic();
	virtual ~CScriptParamStatic();

protected:
	CFont m_Font[2];
	bool m_bCreateFont, _bMouseInControl;
	HCURSOR	_hHandCursor, _hArrowCursor;
	CTEtTriggerElement *m_pElement;
	CScriptControl::DefineScriptStruct *m_pStruct;

protected:
	void CreateFont();
	bool InCaptionRange(CPoint &point);
	int GetCaptionIndex(CPoint &point);

public:
	void SetScriptStruct( CTEtTriggerElement *pElement );

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnParamPick( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


