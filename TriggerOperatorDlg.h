#pragma once


// CTriggerOperatorDlg dialog

#include "NumMiniSlider.h"
class CTriggerOperatorDlg : public CDialog
{
	DECLARE_DYNAMIC(CTriggerOperatorDlg)

public:
	CTriggerOperatorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTriggerOperatorDlg();

public:
	void SetValue( int nIndex, int nValue );
	int GetValue( int nIndex );

// Dialog Data
	enum { IDD = IDD_TRIGGEROPERATORDLG };

protected:
	CNumMiniSlider m_RandomPercent;
	int m_nRandomValue;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnUpdateNumMiniSlider( WPARAM wParam, LPARAM lParam );
};
