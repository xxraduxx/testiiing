#pragma once
#include "afxwin.h"


// CCreateGridDlg dialog

class CEtWorldGrid;
class CCreateGridDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateGridDlg)

public:
	CCreateGridDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateGridDlg();

// Dialog Data
	enum { IDD = IDD_CREATEGRID };

public:
	DWORD m_dwSizeX;
	DWORD m_dwSizeY;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	DWORD m_dwTileSize;
	CComboBox m_Combo;
	CString m_szGridName;

protected:
	void InsertParentName( CEtWorldGrid *pGrid );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
