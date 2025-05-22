#include "StdAfx.h"
#include "EventEditWizardCommon.h"

#include "resource.h"

#include "EventEdit_SelectEditType.h"
#include "EventEdit_CreateControl.h"
#include "EventEdit_SelectControl.h"
#include "EventEdit_Finish.h"
#include "EventEdit_CreateParameter.h"
#include "EventEdit_SelectParameter.h"

int CEventEditWizardCommon::s_nStartType = -1;

CString CEventEditWizardCommon::s_szCurrentSignalName;
CEventSignalItem *CEventEditWizardCommon::s_pCurrentSignalItem = NULL;

CString CEventEditWizardCommon::s_szCurrentPropertyName;
CUnionValueProperty *CEventEditWizardCommon::s_pCurrentPropertyVariable = NULL;


SignalEventEditWizardStruct g_SignalEventEditWizardStructDefine[] = {
	{ CEventEditWizardCommon::SELECT_EDIT_TYPE, RUNTIME_CLASS( CEventEdit_SelectEditType ), NULL },
	{ CEventEditWizardCommon::CREATE_SIGNAL, RUNTIME_CLASS( CEventEdit_CreateControl ), NULL },
	{ CEventEditWizardCommon::SELECT_SIGNAL, RUNTIME_CLASS( CEventEdit_SelectControl ), NULL },
	{ CEventEditWizardCommon::FINISH, RUNTIME_CLASS( CEventEdit_Finish ), NULL },
	{ CEventEditWizardCommon::CREATE_PARAMETER, RUNTIME_CLASS( CEventEdit_CreateParameter ), NULL },
	{ CEventEditWizardCommon::SELECT_PARAMETER, RUNTIME_CLASS( CEventEdit_SelectParameter ), NULL },
	/*
	{ CEventEditWizardCommon::SELECT_EDIT_TYPE, RUNTIME_CLASS( CEventEditView_SelectEditType ), NULL },
	{ CEventEditWizardCommon::CREATE_EVENTAREA, RUNTIME_CLASS( CEventEditView_CreateModifySignal ), NULL },
	{ CEventEditWizardCommon::SELECT_EVENTAREA, RUNTIME_CLASS( CEventEditView_SelectSignal ), NULL },
	{ CEventEditWizardCommon::FINISH, RUNTIME_CLASS( CEventEditView_Finish ), NULL },
	{ CEventEditWizardCommon::CREATE_PARAMETER, RUNTIME_CLASS( CEventEditView_CreateModifyParameter ), NULL },
	{ CEventEditWizardCommon::SELECT_PARAMETER, RUNTIME_CLASS( CEventEditView_SelectParameter ), NULL },
	*/
	{ CEventEditWizardCommon::UNKNOWN_TYPE, NULL, NULL },
};

CWnd *CEventEditWizardCommon::GetWizardWindow( CEventEditWizardCommon::VIEW_TYPE Type )
{
	int nCount = sizeof(g_SignalEventEditWizardStructDefine) / sizeof(SignalEventEditWizardStruct);
	for( int i=0; i<nCount; i++ ) {
		if( g_SignalEventEditWizardStructDefine[i].ViewType == Type ) {
			if( g_SignalEventEditWizardStructDefine[i].pThis == NULL ) continue;
			CWnd *pWnd = ((CFrameWnd*)g_SignalEventEditWizardStructDefine[i].pThis)->GetWindow(GW_CHILD);
			return pWnd;
		}
	}
	return NULL;
}