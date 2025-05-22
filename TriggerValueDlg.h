#pragma once


// CTriggerValueDlg dialog

#include "UnknownRepository.h"
#include "EtTrigger.h"

class CTriggerValueDlg : public CDialog, public CUnknownRepository<CEtTrigger::DefineValue *>
{
	DECLARE_DYNAMIC(CTriggerValueDlg)

public:
	CTriggerValueDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTriggerValueDlg();

protected:
	CComboBox m_Combo;
	CEdit m_Name;
	CEdit m_DefaultValue;

// Dialog Data
	enum { IDD = IDD_TRIGGERVALUEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCombo1();
	virtual void OnOK();
	virtual void OnCancel();
};
