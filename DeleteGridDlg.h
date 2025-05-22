#pragma once
#include "afxwin.h"


// CDeleteGridDlg dialog

class CEtWorldGrid;
class CDeleteGridDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeleteGridDlg)

public:
	CDeleteGridDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDeleteGridDlg();

// Dialog Data
	enum { IDD = IDD_DELETEGRID };

protected:
	void InsertGridName( CEtWorldGrid *pGrid );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CComboBox m_Combo;
};
