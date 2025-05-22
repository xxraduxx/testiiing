#pragma once

#include "PropSignalItem.h"

class CSignalItem;
class CPropEditWizardCommon {
public:
	enum VIEW_TYPE {
		UNKNOWN_TYPE,
		SELECT_EDIT_TYPE,
		CREATE_SIGNAL,
		SELECT_SIGNAL,
		CREATE_PARAMETER,
		SELECT_PARAMETER,
		FINISH,
	};

	CPropEditWizardCommon() {}
	virtual ~CPropEditWizardCommon() {}

protected:
	VIEW_TYPE m_PrevViewType;


public:
	virtual VIEW_TYPE GetNextView() { return UNKNOWN_TYPE; }
	VIEW_TYPE GetPrevView() { return m_PrevViewType; }
	void SetPrevViewType( VIEW_TYPE View ) { m_PrevViewType = View; }
	CWnd *GetWizardWindow( CPropEditWizardCommon::VIEW_TYPE Type );

	virtual bool IsVisivleNextButton() { return true; }
	virtual bool IsVisivlePrevButton() { return true; }
	virtual bool IsVisivleFinishButton() { return true; }
	virtual bool IsFinishOrCancel() { return true; }

	virtual void PreProcess() {}
	virtual bool PostProcess() { return true; }

	static CPropSignalItem *s_pCurrentSignalItem;
	static CUnionValueProperty *s_pCurrentPropertyVariable;
	static int s_nStartType;
	static CString s_szCurrentSignalName;
	static CString s_szCurrentPropertyName;
};

struct SignalPropEditWizardStruct {
	CPropEditWizardCommon::VIEW_TYPE ViewType;
	CRuntimeClass *pRuntimeClass;
	CWnd *pThis;
};


extern SignalPropEditWizardStruct g_SignalPropEditWizardStructDefine[];