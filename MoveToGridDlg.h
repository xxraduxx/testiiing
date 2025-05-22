#pragma once
#include "afxwin.h"


// CMoveToGridDlg dialog
class CEtWorldGrid;

class CMoveToGridDlg : public CDialog
{
	DECLARE_DYNAMIC(CMoveToGridDlg)

public:
	CMoveToGridDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMoveToGridDlg();

public:
	void InsertGridName( CEtWorldGrid *pGrid, CComboBox &ComboBox );

protected:
	CComboBox m_SelCombo;
	CComboBox m_OrigCombo;
	CComboBox m_TarCombo;
protected:
	bool IsExistParent( CEtWorldGrid *pGrid, CString &szGridName );

// Dialog Data
	enum { IDD = IDD_MOVEGRIDDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeCombo3();
};
